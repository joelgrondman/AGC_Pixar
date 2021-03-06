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

  delete mainShaderProg, tessShaderProg;
}

// ---

void MainView::createShaderPrograms() {
  qDebug() << ".. createShaderPrograms";

  mainShaderProg = new QOpenGLShaderProgram();
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

  mainShaderProg->link();

  tessShaderProg = new QOpenGLShaderProgram();
  tessShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshadertess.glsl");
  tessShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/tesselationcontrolshader.glsl");
  tessShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/tesselationevaluationshader.glsl");
  tessShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

  tessShaderProg->link();

  uniModelViewMatrix = glGetUniformLocation(mainShaderProg->programId(), "modelviewmatrix");
  uniProjectionMatrix = glGetUniformLocation(mainShaderProg->programId(), "projectionmatrix");
  uniNormalMatrix = glGetUniformLocation(mainShaderProg->programId(), "normalmatrix");

  uniTessModelViewMatrix = glGetUniformLocation(tessShaderProg->programId(), "modelviewmatrix");
  uniTessProjectionMatrix = glGetUniformLocation(tessShaderProg->programId(), "projectionmatrix");
  uniTessNormalMatrix = glGetUniformLocation(tessShaderProg->programId(), "normalmatrix");

  uniInner1 = glGetUniformLocation(tessShaderProg->programId(), "inner1");
  uniInner2 = glGetUniformLocation(tessShaderProg->programId(), "inner2");
  uniOuter = glGetUniformLocation(tessShaderProg->programId(), "outer");
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

  if (surfacePatches) {
      //probably not the right amount to reserve
      polyIndices.reserve(currentMesh->HalfEdges.size() + currentMesh->Faces.size());
      //used to hold quad indices
      QVector<unsigned int> quadInd = QVector<unsigned int>(16,0);
      for (k=0; k<currentMesh->Faces.size(); k++) {
        n = currentMesh->Faces[k].val;

        //continue if the face is a quad
        if (currentMesh->Faces[k].val == 4) {
            //skip if one of the vertices doesn't have valency 4
            currentEdge = currentMesh->Faces[k].side;
            bool regular = true;
            for (int v = 0; v < 4; ++v) {
                if (currentEdge->target->val != 4 || currentEdge->twin->polygon == nullptr ||
                        currentEdge->twin->next->twin->polygon == nullptr) {
                    regular = false;
                    break;
                }
                currentEdge = currentEdge->next;
            }

            if (regular) {
                /*
                for (int r = 0; r < 4; ++ r) {

                    currentEdge->target->index;
                    currentEdge->twin->prev->prev->target->index;
                    currentEdge->twin->prev->twin->next->target->index;
                }
                */

                //start at one of the corners
                currentEdge = currentEdge->twin->next->twin->next->next;

                for(int r = 0; r < 4; ++r) {
                    for (int k = 0; k < 4; ++k) {
                      polyIndices.append(currentEdge->target->index);
                      if (k == 2) {
                          if (r != 3)
                            currentEdge = currentEdge->prev;
                          else
                            currentEdge = currentEdge->next->next->twin;
                      } else if (k == 3) {
                          if (r != 2)
                            currentEdge = currentEdge->prev->prev->twin->prev->prev->twin->prev->twin   ;
                          else
                            currentEdge = currentEdge->prev->prev->twin->prev->prev->twin->prev->prev;
                      } else {
                          if (r != 3)
                            currentEdge = currentEdge->prev->twin->prev;
                          else
                            currentEdge = currentEdge->next->next->twin;
                      }
                    }
                }
                polyIndices.append(maxInt);

            }
        }
      }
      //for 4 vertices per primitive
  } else {
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

}

void MainView::updateTessUniforms() {

  // mainShaderProg should be bound at this point!

  glUniformMatrix4fv(uniTessModelViewMatrix, 1, false, modelViewMatrix.data());
  glUniformMatrix4fv(uniTessProjectionMatrix, 1, false, projectionMatrix.data());
  glUniformMatrix3fv(uniTessNormalMatrix, 1, false, normalMatrix.data());

  glUniform1f(uniInner1, inner1);
  glUniform1f(uniInner2, inner2);
  glUniform1f(uniOuter, outer);

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
  glPatchParameteri(GL_PATCH_VERTICES, 16);
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

    if(surfacePatches) {

        tessShaderProg->bind();

        if (uniformUpdateRequired) {
          updateTessUniforms();
          uniformUpdateRequired = false;
        }

        renderTessMesh();

        tessShaderProg->release();

    } else {
        mainShaderProg->bind();

        if (uniformUpdateRequired) {
          updateUniforms();
          uniformUpdateRequired = false;
        }

        renderMesh();

        mainShaderProg->release();
    }

  }
}

// ---

void MainView::renderMesh() {

  glBindVertexArray(meshVAO);

  if (wireframeMode) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_LINE_LOOP, meshIBOSize, GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_LINE_LOOP, meshIBOSize, GL_UNSIGNED_INT, 0);
  }
  else {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    //glDrawElements(GL_TRIANGLE_FAN, meshIBOSize, GL_UNSIGNED_INT, 0);
    glDrawElements(GL_TRIANGLE_FAN, meshIBOSize, GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);

}

void MainView::renderTessMesh() {
    glBindVertexArray(meshVAO);

    if (wireframeMode) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glDrawElements(GL_PATCHES, meshIBOSize, GL_UNSIGNED_INT, 0);
      //glDrawElements(GL_LINE_LOOP, meshIBOSize, GL_UNSIGNED_INT, 0);
    }
    else {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

      //glDrawElements(GL_TRIANGLE_FAN, meshIBOSize, GL_UNSIGNED_INT, 0);
      glDrawElements(GL_PATCHES, meshIBOSize, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
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
