#include "mainview.h"

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
  qDebug() << "✓✓ MainView constructor";

  modelLoaded = false;
  wireframeMode = true;

  rotAngle = 0.0;
  FoV = 60.0;
}

MainView::~MainView() {
  qDebug() << "✗✗ MainView destructor";

  glDeleteBuffers(1, &meshCoordsBO);
  glDeleteBuffers(1, &meshNormalsBO);
  glDeleteBuffers(1, &meshIndexBO);
  glDeleteVertexArrays(1, &meshVAO);

  debugLogger->stopLogging();

  delete mainShaderProg;
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

  uniIsophotesBool = glGetUniformLocation(mainShaderProg->programId(),"isophotes");
  uniIsophotesFloat = glGetUniformLocation(mainShaderProg->programId(),"isophotes_range");

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
}

void MainView::updateMeshBuffers(Mesh* currentMesh) {

  qDebug() << ".. updateBuffers";

  unsigned int k;
  unsigned short m;
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
    currentEdge = currentMesh->Faces[k].side;
    for (m=0; m<3; m++) {
      polyIndices.append(currentEdge->target->index);
      currentEdge = currentEdge->next;
    }
  }

  // ---
  float smallest_distance = 1000000;
  vertex_idx = -1;
  for (Vertex vec3D : currentMesh->Vertices)
  {
      QVector3D sphere_o = vec3D.coords;     //  vertex position

      QVector3D ray_o = QVector3D(posxy.x(),posxy.y(),0);

      QVector3D ray_dir = QVector3D(0,0,-1.0);

      QVector3D dst = ray_o - sphere_o;     // ray_o = ray origin from camera

      float r2 = 1000.0;

      float B = (QVector3D::dotProduct(dst, ray_dir));     // ray_dir  = ray direction from camera towards the world
      float C = dst.lengthSquared() - r2;
      float D = (B*B - C);


      // Do we have an intersection
      if (D >= 0)
      {
          // Make sure we choose the closest to the ray's origin
          if(dst.length() < smallest_distance)
          {
              vertex_idx = vec3D.index;
              smallest_distance = dst.length();
          }
      }
  }
  //update selected vertex

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

  // mainShaderProg should be bound at this point!

  glUniformMatrix4fv(uniModelViewMatrix, 1, false, modelViewMatrix.data());
  glUniformMatrix4fv(uniProjectionMatrix, 1, false, projectionMatrix.data());
  glUniformMatrix3fv(uniNormalMatrix, 1, false, normalMatrix.data());

  glUniform1i(uniIsophotesBool,(isophotesEnabled? 1 : 0));
  glUniform1f(uniIsophotesFloat, (isophotesRange));
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

    mainShaderProg->bind();

    if (uniformUpdateRequired) {
      updateUniforms();
      uniformUpdateRequired = false;
    }

    renderMesh();

    glBindVertexArray(meshVAO);

    glPointSize(8.0);
    glDrawArrays(GL_POINTS, vertex_idx, 1);

    glBindVertexArray(0);


    mainShaderProg->release();

  }
}

void MainView::updateIsophotes(bool check) {

   isophotesEnabled = check;
   uniformUpdateRequired = true;
}

void MainView::updateIsophotesRange(float value) {

   isophotesRange = value;
   uniformUpdateRequired = true;
}

// ---

void MainView::renderMesh() {

  glBindVertexArray(meshVAO);

  if (wireframeMode) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  else {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }

  glDrawElements(GL_TRIANGLES, meshIBOSize, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);

}

// ---

void MainView::mousePressEvent(QMouseEvent* event) {
  setFocus();

  posxy = event->globalPos();

  qDebug() << posxy;

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
