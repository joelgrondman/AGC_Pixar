#include "mainview.h"
#include <limits>

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
    qDebug() << "✓✓ MainView constructor";

    modelLoaded = false;
    wireframeMode = true;
    isophotesNumStripes = 12;
    isophotesDistance = 50;
    isophotesSmoothness = 50;
    visMode = 0;
    selectedPt = -1;
    selectedEdge = -1;
    rotAngle = 0.0;
    FoV = 60.0;
}

MainView::~MainView() {
    qDebug() << "✗✗ MainView destructor";

    glDeleteBuffers(1, &meshCoordsBO);
    glDeleteBuffers(1, &meshNormalsBO);
    glDeleteBuffers(1, &meshIndexBO);
    glDeleteVertexArrays(1, &meshVAO);

    glDeleteBuffers(1, &selectionCoordsBO);
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
    uniINumIsophotes = glGetUniformLocation(mainShaderProg->programId(), "numIsophotes");
    uniIsophotesDistance = glGetUniformLocation(mainShaderProg->programId(), "isophotesDistance");
    uniIsophotesSmoothness = glGetUniformLocation(mainShaderProg->programId(), "isophotesSmoothness");
    uniVismode = glGetUniformLocation(mainShaderProg->programId(), "visMode");


    // Create shader program which will facilitate the visualization of selected edge or vertex.

    selectionShader = new QOpenGLShaderProgram();
    selectionShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/selectionVertex.glsl");
    selectionShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/selectionFragshader.glsl");
    selectionShader->link();

    uniSelModelViewMatrix = glGetUniformLocation(selectionShader->programId(), "modelviewmatrix");
    uniSelProjectionMatrix = glGetUniformLocation(selectionShader->programId(), "projectionmatrix");
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

    glBindVertexArray(0);

}

void MainView::updateSelectionBuffers() {

    // Update the selected edge endpoints or vertex coordinates.

    selectedPoints.clear();
    if(selectedEdge > -1 && selectedEdge < curDispMesh->HalfEdges.size()){
        // Add edge endpoints coordiantes.
        selectedPoints.append(curDispMesh->HalfEdges[selectedEdge].target->coords);
        selectedPoints.append(curDispMesh->HalfEdges[selectedEdge].twin->target->coords);
    }else if(selectedPt > -1 && selectedPt < curDispMesh->Vertices.size()){
        // Add vertex coordiantes.
        selectedPoints.append(curDispMesh->Vertices[selectedPt].coords);
    }

    glBindBuffer(GL_ARRAY_BUFFER, selectionCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*selectedPoints.size(), selectedPoints.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated selection buffers";
}

void MainView::updateMeshBuffers(Mesh* currentMesh) {

    // Set values for edge and vertex selection.
    curDispMesh = currentMesh;
    selectedEdge = -1;

    qDebug() << ".. updateBuffers";

    unsigned int k;
    unsigned short m;
    HalfEdge* currentEdge;

    vertexCoords.clear();
    vertexCoords.reserve(currentMesh->Vertices.size());

    for (k=0; k<(unsigned)currentMesh->Vertices.size(); k++) {
        vertexCoords.append(currentMesh->Vertices[k].coords);
    }

    vertexNormals.clear();
    vertexNormals.reserve(currentMesh->Vertices.size());

    for (k=0; k<(unsigned)currentMesh->Faces.size(); k++) {
        currentMesh->setFaceNormal(&currentMesh->Faces[k]);
    }

    for (k=0; k<(unsigned)currentMesh->Vertices.size(); k++) {
        vertexNormals.append( currentMesh->computeVertexNormal(&currentMesh->Vertices[k]) );
    }

    polyIndices.clear();
    polyIndices.reserve(currentMesh->HalfEdges.size() + currentMesh->Faces.size());

    for (k=0; k<(unsigned)currentMesh->Faces.size(); k++) {
        currentEdge = currentMesh->Faces[k].side;
        for (m=0; m<3; m++) {
            polyIndices.append(currentEdge->target->index);
            currentEdge = currentEdge->next;
        }
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

    updateSelectionBuffers();

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

    selectionShader->release();


    // Update mainShaderProg uniforms.

    mainShaderProg->bind();

    glUniformMatrix4fv(uniModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(uniProjectionMatrix, 1, false, projectionMatrix.data());
    glUniformMatrix3fv(uniNormalMatrix, 1, false, normalMatrix.data());

    glUniform1i(uniINumIsophotes, isophotesNumStripes);
    glUniform1i(uniIsophotesDistance, isophotesDistance);
    glUniform1i(uniIsophotesSmoothness, isophotesSmoothness);

    glUniform1i(uniVismode, visMode);

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

        // If a vertex or edge is selected.
        if(selectedEdge > -1 || selectedPt > -1){

            // Draw the selected point or edge on top of everything.
            glClear(GL_DEPTH_BUFFER_BIT);

            selectionShader->bind();
            glBindVertexArray(selectionVAO);

            // Highlight selected vertex.
            if (selectedPt > -1 && selectedPt < curDispMesh->Vertices.size()) {
                glPointSize(12.0);
                glDrawArrays(GL_POINTS, 0, 1);
            }

            // Highlight selected edge.
            if (selectedEdge > -1 && selectedEdge < curDispMesh->HalfEdges.size()){
                glDrawArrays(GL_LINES, 0, 2);
            }

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
    }
    else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glDrawElements(GL_TRIANGLES, meshIBOSize, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    mainShaderProg->release();
}

// ---

void MainView::selectVertex(QVector4D nearpos, QVector4D farpos)
{
    selectedPt = -1;
    float min = 0.1; // The maximal allowed distance between selection line and vertex.
    QVector3D x1 = QVector3D(farpos[0],farpos[1],farpos[2]);
    QVector3D x2 = QVector3D(nearpos[0],nearpos[1],nearpos[2]);
    QVector3D direction = (x2-x1).normalized(); // Direction of the line L through the click positions when z = 1 and z = -1

    // Let P be the point on line L which is closest to vertex V.
    // Find among all vertices the V which has the distance between P and V the smallest.
    for(int i = 0; i < curDispMesh->Vertices.size();i++){
        QVector3D x0 = QVector3D(curDispMesh->Vertices[i].coords); // Position of vertex
        float d = x0.distanceToLine(x1,direction);
        if(min > d){
            min = d;
            selectedPt = i;
        }
    }
    selectedEdge = -1;
    uniformUpdateRequired = true;
    update();

    // Display vertex info
    if(selectedPt > -1){
        curDispMesh->dispVertInfo( &curDispMesh->Vertices[selectedPt]);
    }
}

void MainView::selectEdge(QVector4D nearpos, QVector4D farpos)
{
    selectedEdge = -1;
    float min = std::numeric_limits<float>::infinity();
    QVector3D x1 = QVector3D(farpos[0],farpos[1],farpos[2]);
    QVector3D x2 = QVector3D(nearpos[0],nearpos[1],nearpos[2]);
    QVector3D direction = (x2-x1).normalized(); // Direction of the line L through the click positions when z = 1 and z = -1

    for(int i = 0; i < curDispMesh->HalfEdges.size();i++){
        // Only process one of the two half edges
        if(curDispMesh->HalfEdges[i].twin->index > curDispMesh->HalfEdges[i].index){
            QVector3D edgep1 = (curDispMesh->HalfEdges[i].target->coords);
            QVector3D edgep2 = (curDispMesh->HalfEdges[i].twin->target->coords);
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
    selectedPt = -1;
    uniformUpdateRequired = true;
    update();

    // Display edge info
    if(selectedEdge > -1){
        qDebug() << "";
        curDispMesh->dispHalfEdgeInfo(&curDispMesh->HalfEdges[selectedEdge]);
        curDispMesh->dispHalfEdgeInfo(&curDispMesh->HalfEdges[curDispMesh->HalfEdges[selectedEdge].twin->index]);
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
            // Left click to select a vertex
            selectVertex(nearpos,farpos);
        }else if(event->buttons() == Qt::RightButton){
            // Right click to select an edge
            selectEdge(nearpos,farpos);
        }

        updateSelectionBuffers();
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


