#include "mainview.h"

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
    qDebug() << "✓✓ MainView constructor";

    modelLoaded = false;
    wireframeMode = true;

    rotAngle = 0.0;
    FoV = 60.0;
    origMesh = NULL;
    dispControlMesh = true;
    dispSubdivSurface = true;
    creaseIsBeingSelected = false;
    displayCreases = 0;
}

MainView::~MainView() {
    qDebug() << "✗✗ MainView destructor";

    glDeleteBuffers(1, &meshCoordsBO);
    glDeleteBuffers(1, &meshNormalsBO);
    glDeleteBuffers(1, &meshIndexBO);
    glDeleteVertexArrays(1, &meshVAO);

    glDeleteBuffers(1, &selectionCoordsBO);
    glDeleteBuffers(1, &selectionSharpnessBO);

    glDeleteVertexArrays(1, &selectionVAO);

    debugLogger->stopLogging();

    delete mainShaderProg;
    delete selectionShader;
}

// ---

void MainView::createShaderPrograms() {
    qDebug() << ".. createShaderPrograms";

    mainShaderProg = new QOpenGLShaderProgram();
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

    mainShaderProg->link();

    uniModelViewMatrix = glGetUniformLocation(mainShaderProg->programId(), "modelviewmatrix");
    uniProjectionMatrix = glGetUniformLocation(mainShaderProg->programId(), "projectionmatrix");
    uniNormalMatrix = glGetUniformLocation(mainShaderProg->programId(), "normalmatrix");


    // Create shader program which will facilitate the visualization of selected edge.

    selectionShader = new QOpenGLShaderProgram();
    selectionShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/selectionVertex.glsl");
    selectionShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/selectionFragshader.glsl");
    selectionShader->link();

    uniSelModelViewMatrix = glGetUniformLocation(selectionShader->programId(), "modelviewmatrix");
    uniSelProjectionMatrix = glGetUniformLocation(selectionShader->programId(), "projectionmatrix");
    uniDisplayCreases = glGetUniformLocation(selectionShader->programId(), "displayCreases");

}

void MainView::createBuffers() {

    qDebug() << ".. createBuffers";

    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    glGenBuffers(1, &meshCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshCoordsBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &meshNormalsBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshNormalsBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &meshIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBO);

    glBindVertexArray(0);


    // Create buffers which will facilitate the visualization of selected edge and vertex.

    glGenVertexArrays(1, &selectionVAO);
    glBindVertexArray(selectionVAO);

    glGenBuffers(1, &selectionCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionCoordsBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &selectionSharpnessBO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionSharpnessBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

}

void MainView::updateMeshBuffers(Mesh* currentMesh, Mesh* orgMesh) {

    qDebug() << ".. updateBuffers";

    curDispMesh = currentMesh;
    origMesh = orgMesh;

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

    // ---

    glBindBuffer(GL_ARRAY_BUFFER, meshCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexCoords.size(), vertexCoords.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshCoordsBO";

    glBindBuffer(GL_ARRAY_BUFFER, meshNormalsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexNormals.size(), vertexNormals.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshNormalsBO";

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBO);
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

    // Update selectionShader uniforms.

    selectionShader->bind();

    glUniformMatrix4fv(uniSelModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(uniSelProjectionMatrix, 1, false, projectionMatrix.data());
    glUniform1i(uniDisplayCreases, displayCreases);


    selectionShader->release();

    // Update mainShaderProg uniforms.

    mainShaderProg->bind();

    glUniformMatrix4fv(uniModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(uniProjectionMatrix, 1, false, projectionMatrix.data());
    glUniformMatrix3fv(uniNormalMatrix, 1, false, normalMatrix.data());

    mainShaderProg->release();
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
        if(dispSubdivSurface){
            glClearColor(0.64, 0.64, 0.64, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (uniformUpdateRequired) {
                updateUniforms();
                uniformUpdateRequired = false;
            }

            renderMesh();
        }
        if(dispControlMesh){
            // Draw control mesh and selected edges

            selectionShader->bind();

            glBindVertexArray(selectionVAO);

            glDrawArrays(GL_LINES, 0, controlMeshLines.size());

            glBindVertexArray(0);
            selectionShader->release();
        }
    }
}

// ---


void MainView::renderMesh() {
    mainShaderProg->bind();

    glBindVertexArray(meshVAO);

    if (wireframeMode) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawElements(GL_LINE_LOOP, meshIBOSize, GL_UNSIGNED_INT, 0);
    }
    else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawElements(GL_TRIANGLE_FAN, meshIBOSize, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    mainShaderProg->release();
}

// ---

void MainView::updateSelectionBuffers() {

    selectionSharpness.clear();
    controlMeshLines.clear();

    if(displayCreases){
        for(int i = 0;i < creases.size();i++){
            float color = (1023.0/creases.size())*i;
            for(int c = 0;c < creases[i].size();c++){
                int edge = creases[i][c];

                selectionSharpness.append(color);
                selectionSharpness.append(color);

                controlMeshLines.append(origMesh->HalfEdges[edge].target->coords);
                controlMeshLines.append(origMesh->HalfEdges[edge].twin->target->coords);
            }
        }
    }else{


        for(int i = 0;i < origMesh->HalfEdges.size();i++){
            if(origMesh->HalfEdges[i].index < origMesh->HalfEdges[i].twin->index){

                bool found = false;
                for(int c = 0; c < selectedEdges.size();c++){
                    if(selectedEdges[c] == origMesh->HalfEdges[i].index ||  selectedEdges[c] == origMesh->HalfEdges[i].twin->index){
                        found = true;
                        break;
                    };
                }

                if(found){
                    selectionSharpness.append(-1);
                    selectionSharpness.append(-1);
                }else {
                    selectionSharpness.append(origMesh->HalfEdges[i].sharpness);
                    selectionSharpness.append(origMesh->HalfEdges[i].sharpness);
                }

            controlMeshLines.append(origMesh->HalfEdges[i].target->coords);
            controlMeshLines.append(origMesh->HalfEdges[i].twin->target->coords);
        }
    }
    }

    glBindBuffer(GL_ARRAY_BUFFER, selectionCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*controlMeshLines.size(), controlMeshLines.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, selectionSharpnessBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*selectionSharpness.size(), selectionSharpness.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated selection buffers";
}

void MainView::selectEdge(QVector4D nearpos, QVector4D farpos){

    int selectedEdge = -1;
    float min = std::numeric_limits<float>::infinity();
    QVector3D x1 = QVector3D(farpos[0],farpos[1],farpos[2]);
    QVector3D x2 = QVector3D(nearpos[0],nearpos[1],nearpos[2]);
    QVector3D direction = (x2-x1).normalized(); // Direction of the line L through the click positions when z = 1 and z = -1

    for(int i = 0; i < origMesh->HalfEdges.size();i++){
        // Only process one of the two half edges
        if(origMesh->HalfEdges[i].twin->index > origMesh->HalfEdges[i].index){
            QVector3D edgep1 = (origMesh->HalfEdges[i].target->coords);
            QVector3D edgep2 = (origMesh->HalfEdges[i].twin->target->coords);
            // Compute sphere with center in middle of edge and through endpoints, only continue if ray intersects sphere
            QVector3D center = (edgep1+edgep2)/2;
            float radius = (center-edgep1).length();
            if(center.distanceToLine(x1,direction) <= radius){
                // Compute 50 points V on edge, for each V let P be the point on line L which is closest to vertex V.
                // Find among all edges the one which has a V which has the distance between P and V the smallest.
                for(float k = 0;k<=1;k+=.02){
                    float d = QVector3D((k*edgep1+(1-k)*edgep2)).distanceToLine(x1,direction);
                    if(min > d){
                        min = d;
                        selectedEdge = i;
                    };
                }
            }
        }
    }

    uniformUpdateRequired = true;
    update();

    // If "add crease" was pressed, add to crease, otherwise edit sharpness ui is shown

    if(creaseIsBeingSelected){
        if(selectedEdge > -1){
            selectedEdges.append(selectedEdge);
        }
    }else{
        selectedEdges.clear();
        if(selectedEdge > -1){
            // show ui
            selectionUIBox->show();
            selectionUIValue->setValue(origMesh->HalfEdges[selectedEdge].sharpness);
            selectedEdges.append(selectedEdge);
        }else{
            // hide ui
            selectionUIBox->hide();
        }
    }
}

void MainView::mousePressEvent(QMouseEvent* event) {
    setFocus();
    if(modelLoaded){

        float xRatio, yRatio;

        xRatio = (float)event->x() / width();
        yRatio = (float)event->y() / height();

        // Compute click position in normalized device coordinates
        float winX = (xRatio*2-1);
        float winY = -1*(yRatio*2-1);

        // Compute click position if z  was intended to be -1
        QVector4D nearpos = (projectionMatrix*modelViewMatrix).inverted()*QVector4D(winX,winY,-1,1);
        nearpos = (nearpos * 1/nearpos[3]);

        // Compute click position if z was intended to be 1
        QVector4D farpos = (projectionMatrix*modelViewMatrix).inverted()*QVector4D(winX,winY,1,1);
        farpos = (farpos * 1/farpos[3]);

        if(event->buttons() == Qt::LeftButton){
            // Left click to select an edge
            selectEdge(nearpos,farpos);
            updateSelectionBuffers();
        }
    }
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
