#include "mainview.h"

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
    qDebug() << "✓✓ MainView constructor";

    modelLoaded = false;
    wireframeMode = true;
    tessMode = false;

    rotAngle = 0.0;
    FoV = 60.0;

    // Initialize tessellation levels.
    tessO1=3;
    tessO2=3;
    tessO3=3;
    tessO4=3;
    tessi1=3;
    tessi2=3;
}

MainView::~MainView() {
    qDebug() << "✗✗ MainView destructor";

    glDeleteBuffers(1, &tessMeshIndexBO);
    glDeleteVertexArrays(1, &tessVAO);

    glDeleteBuffers(1, &mainMeshCoordsBO);
    glDeleteBuffers(1, &mainMeshNormalsBO);
    glDeleteBuffers(1, &mainMeshIndexBO);
    glDeleteVertexArrays(1, &mainVAO);

    debugLogger->stopLogging();

    delete tessShaderProg;
    delete mainShaderProg;
}

// ---

void MainView::createShaderPrograms() {
    qDebug() << ".. createShaderPrograms";

    // Initialize shader program which uses tessellation to render bezier patches.

    tessShaderProg = new QOpenGLShaderProgram();
    tessShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/tesVertshader.glsl");
    tessShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/tesTcs.glsl");
    tessShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/tesTes.glsl");
    tessShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/tesFragshader.glsl");

    tessShaderProg->link();

    // Initialize uniforms tessShaderProg.
    tesUniModelViewMatrix = glGetUniformLocation(tessShaderProg->programId(), "modelviewmatrix");
    tesUniProjectionMatrix = glGetUniformLocation(tessShaderProg->programId(), "projectionmatrix");
    tesUniNormalMatrix = glGetUniformLocation(tessShaderProg->programId(), "normalmatrix");
    uniTesO1 = glGetUniformLocation(tessShaderProg->programId(), "tesO1");
    uniTesO2 = glGetUniformLocation(tessShaderProg->programId(), "tesO2");
    uniTesO3 = glGetUniformLocation(tessShaderProg->programId(), "tesO3");
    uniTesO4 = glGetUniformLocation(tessShaderProg->programId(), "tesO4");
    uniTesi1 = glGetUniformLocation(tessShaderProg->programId(), "tesi1");
    uniTesi2 = glGetUniformLocation(tessShaderProg->programId(), "tesi2");



    // Initialize shader program to render in conventional modes.

    mainShaderProg = new QOpenGLShaderProgram();
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

    mainShaderProg->link();

    // Initialize uniforms mainShaderProg.
    uniModelViewMatrix = glGetUniformLocation(mainShaderProg->programId(), "modelviewmatrix");
    uniProjectionMatrix = glGetUniformLocation(mainShaderProg->programId(), "projectionmatrix");
    uniNormalMatrix = glGetUniformLocation(mainShaderProg->programId(), "normalmatrix");
}

void MainView::createBuffers() {

    qDebug() << ".. createBuffers";

    // Initialize buffers for tessellation rendering (tessShaderProg)

    glGenVertexArrays(1, &tessVAO);
    glBindVertexArray(tessVAO);

    glGenBuffers(1, &tessMeshCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, tessMeshCoordsBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &tessMeshNormalsBO);
    glBindBuffer(GL_ARRAY_BUFFER, tessMeshNormalsBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &tessMeshIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tessMeshIndexBO);

    glBindVertexArray(0);


    // Initialize buffers for conventional rendering (mainShaderProg)

    glGenVertexArrays(1, &mainVAO);
    glBindVertexArray(mainVAO);

    glGenBuffers(1, &mainMeshCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, mainMeshCoordsBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &mainMeshNormalsBO);
    glBindBuffer(GL_ARRAY_BUFFER, mainMeshNormalsBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &mainMeshIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainMeshIndexBO);

    glBindVertexArray(0);
}

void MainView::updateMeshBuffers(Mesh* currentMesh) {

    qDebug() << ".. updateBuffers";

    unsigned int k;
    unsigned short n, m;
    HalfEdge* currentEdge;

    vertexCoords.clear();
    vertexCoords.reserve(currentMesh->Vertices.size());

    for (k=0; k<currentMesh->Vertices.size(); k++) {
        vertexCoords.append(currentMesh->Vertices[k].coords);
    }

    vertexNormals.clear();
    vertexNormals.reserve(currentMesh->Vertices.size());

    for (k=0; k<currentMesh->Faces.size(); k++) {
        currentMesh->setFaceNormal(&currentMesh->Faces[k]);
    }

    for (k=0; k<currentMesh->Vertices.size(); k++) {
        vertexNormals.append( currentMesh->computeVertexNormal(&currentMesh->Vertices[k]) );
    }

    if(tessMode){
        polyIndices.clear();
        polyIndices.reserve(currentMesh->Faces.size()*16);

        // For each face F, check if F is a regular quad. If it is, continue:
        // Determine the 8 quads around F.
        // Said 9 quads consist out of 16 vertices. Store the indexes of these
        // vertices from top left (index 0) to bottom right (index 15).

        for (k=0; k<currentMesh->Faces.size(); k++) {

            Vertex* v[16];
            currentEdge = currentMesh->Faces[k].side;

            v[5] = currentEdge->target;
            currentEdge = currentEdge->next;
            v[6] = currentEdge->target;
            currentEdge = currentEdge->next;
            v[10] = currentEdge->target;
            currentEdge = currentEdge->next;
            v[9] = currentEdge->target;

            if(v[5]->val == 4 && v[6]->val == 4 && v[9]->val == 4 && v[10]->val == 4  ){
                currentEdge = currentEdge->next->twin->next;
                v[8] = currentEdge->target;
                currentEdge = currentEdge->next;
                v[4] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[0] = currentEdge->target;
                currentEdge = currentEdge->next;
                v[1] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[2] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[3] = currentEdge->target;
                currentEdge = currentEdge->next->next->twin;
                v[7] = currentEdge->target;
                currentEdge = currentEdge->next;
                v[11] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[15] = currentEdge->target;
                currentEdge = currentEdge->next;
                v[14] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[13] = currentEdge->target;
                currentEdge = currentEdge->next->twin->next;
                v[12] = currentEdge->target;

                // Append to polyIndices
                for (m=0; m<16; m++) {
                    polyIndices.append(v[m]->index);
                }
            }
        }
    }else{
        polyIndices.clear();
        polyIndices.reserve(currentMesh->HalfEdges.size() + currentMesh->Faces.size());

        for (k=0; k<currentMesh->Faces.size(); k++) {
            n = currentMesh->Faces[k].val;
            currentEdge = currentMesh->Faces[k].side;
            for (m=0; m<n; m++) {
                polyIndices.append(currentEdge->target->index);
                currentEdge = currentEdge->next;
            }
            polyIndices.append(maxInt);
        }
    }


    // ---

    glBindBuffer(GL_ARRAY_BUFFER, tessMeshCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexCoords.size(), vertexCoords.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mainMeshCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexCoords.size(), vertexCoords.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshCoordsBO";

    glBindBuffer(GL_ARRAY_BUFFER, tessMeshNormalsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexNormals.size(), vertexNormals.data(), GL_DYNAMIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, mainMeshNormalsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexNormals.size(), vertexNormals.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshNormalsBO";

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mainMeshIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*polyIndices.size(), polyIndices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tessMeshIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*polyIndices.size(), polyIndices.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshIndexBO";

    meshIBOSize = polyIndices.size();

    update();

}

void MainView::updateMatrices() {

    modelViewMatrix.setToIdentity();
    modelViewMatrix.translate(QVector3D(0.0, 0.0, -3.0));
    modelViewMatrix.scale(QVector3D(1.0, 1.0, 1.0));
    modelViewMatrix.rotate(rotAngle, QVector3D(0.0, 1.0, 0.0));

    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(FoV, dispRatio, 0.2, 4.0);

    normalMatrix = modelViewMatrix.normalMatrix();

    uniformUpdateRequired = true;
    update();

}

void MainView::updateUniforms() {

    // Update uniforms for conventional rendering.

    mainShaderProg->bind();

    glUniformMatrix4fv(uniModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(uniProjectionMatrix, 1, false, projectionMatrix.data());
    glUniformMatrix3fv(uniNormalMatrix, 1, false, normalMatrix.data());

    mainShaderProg->release();


    // Update uniforms for tessellation rendering.

    tessShaderProg->bind();

    glUniformMatrix4fv(tesUniModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(tesUniProjectionMatrix, 1, false, projectionMatrix.data());
    glUniformMatrix3fv(tesUniNormalMatrix, 1, false, normalMatrix.data());

    glUniform1f(uniTesO1, tessO1);
    glUniform1f(uniTesO2, tessO2);
    glUniform1f(uniTesO3, tessO3);
    glUniform1f(uniTesO4, tessO4);
    glUniform1f(uniTesi1, tessi1);
    glUniform1f(uniTesi2, tessi2);

    tessShaderProg->release();
}

// ---

void MainView::initializeGL() {

    initializeOpenGLFunctions();
    qDebug() << ":: OpenGL initialized";

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ), this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() ) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Default is GL_LESS
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_PRIMITIVE_RESTART);
    maxInt = ((unsigned int) -1);
    glPrimitiveRestartIndex(maxInt);

    // ---

    createShaderPrograms();
    createBuffers();

    // ---

    updateMatrices();
}

void MainView::resizeGL(int newWidth, int newHeight) {

    qDebug() << ".. resizeGL";

    dispRatio = (float)newWidth/newHeight;
    updateMatrices();

}

void MainView::paintGL() {

    if (modelLoaded) {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (uniformUpdateRequired) {
            updateUniforms();
            uniformUpdateRequired = false;
        }
        renderMesh();
    }
}

// ---

void MainView::renderMesh() {
    if(tessMode){
        tessShaderProg->bind();

        glBindVertexArray(tessVAO);

        glPatchParameteri(GL_PATCH_VERTICES, 16);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawElements(GL_PATCHES, meshIBOSize, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        tessShaderProg->release();
    }else{
        mainShaderProg->bind();

        glBindVertexArray(mainVAO);

        if (wireframeMode) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glDrawElements(GL_LINE_LOOP, meshIBOSize, GL_UNSIGNED_INT, 0);
        }else {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            glDrawElements(GL_TRIANGLE_FAN, meshIBOSize, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);

        mainShaderProg->release();
    }
}

// ---

void MainView::mousePressEvent(QMouseEvent* event) {
    setFocus();
}

void MainView::wheelEvent(QWheelEvent* event) {
    FoV -= event->delta() / 60.0;
    updateMatrices();
}

void MainView::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    case 'Z':
        wireframeMode = !wireframeMode;
        update();
        break;
    }
}

// ---

void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}
