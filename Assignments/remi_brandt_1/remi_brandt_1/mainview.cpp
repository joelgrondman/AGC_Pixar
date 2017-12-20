#include "mainview.h"
#include "math.h"

MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "✓✓ MainView constructor";

  selectedPt = -1;
  curvVisType = 0;
  steps = 0;
  visScale = 5;
}

MainView::~MainView() {
  qDebug() << "✗✗ MainView destructor";

  clearArrays();

  glDeleteBuffers(1, &netCoordsBO);
  glDeleteVertexArrays(1, &netVAO);

  glDeleteBuffers(1, &subdivideCoordsBO);
  glDeleteBuffers(1, &affectedVertsBO);
  glDeleteVertexArrays(1, &subdevideVAO);

  delete mainShaderProg;
  delete subdivShader;

  debugLogger->stopLogging();
}

// ---

void MainView::createShaderPrograms() {

  // Shader of control net
  mainShaderProg = new QOpenGLShaderProgram();
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshaderControlNet.glsl");
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");
  mainShaderProg->link();

  // Shader of subdivision curve
  subdivShader = new QOpenGLShaderProgram();
  subdivShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  subdivShader->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/geoshader.glsl");
  subdivShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");
  subdivShader->link();
}

void MainView::createBuffers() {

  // Control net

  glGenVertexArrays(1, &netVAO);
  glBindVertexArray(netVAO);
  glGenBuffers(1, &netCoordsBO);
  glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);


  // Subdivision curve

  glGenVertexArrays(1, &subdevideVAO);
  glBindVertexArray(subdevideVAO);

  glGenBuffers(1, &subdivideCoordsBO);
  glBindBuffer(GL_ARRAY_BUFFER, subdivideCoordsBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glGenBuffers(1, &affectedVertsBO);
  glBindBuffer(GL_ARRAY_BUFFER, affectedVertsBO);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(subdevideVAO);
}

void MainView::updateBuffers(int index){
    updateBuffers(index,-1,-1);
}

void MainView::updateBuffers(int index,int offset, int end){
    // Only update changed buffer(s)
    if(index == 0){
        glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D)*netCoords.size(), netCoords.data(), GL_DYNAMIC_DRAW);
    };
    if(index == 1 || index == 12){
        if(offset !=-1 && end!=-1){
            // Update part of buffer
            QVector<QVector2D> a;
            for(int i = offset;i< subCoords.size() && i <= end+2;i++){
                a.append(subCoords[i]);
            }
            glBindBuffer(GL_ARRAY_BUFFER, subdivideCoordsBO);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(QVector2D)*offset, sizeof(QVector2D)*(a.size()), a.data());
        }else{
            // Update complete buffer
            glBindBuffer(GL_ARRAY_BUFFER, subdivideCoordsBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D)*subCoords.size(), subCoords.data(), GL_DYNAMIC_DRAW);
        }
    };
    if(index == 2 || index == 12){
        glBindBuffer(GL_ARRAY_BUFFER, affectedVertsBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*affectedVertices.size(), affectedVertices.data(), GL_DYNAMIC_DRAW);
    }

  update();
}

void MainView::updateUniforms() {
    subdivShader->bind();
    glUniform1f(glGetUniformLocation(subdivShader->programId(), "showVis"),curvVisType);
    glUniform1f(glGetUniformLocation(subdivShader->programId(), "visScale"),visScale);
    glUniform1f(glGetUniformLocation(subdivShader->programId(), "totalNumPrimitives"),subCoords.size()-4);
    subdivShader->release();

    updateUniformsRequired = false;
}

void MainView::clearArrays() {

  // As of Qt 5.6, clear() does not release the memory anymore. Use e.g. squeeze()
  netCoords.clear();
  netCoords.squeeze();

  subCoords.clear();
  subCoords.squeeze();

  affectedVertices.clear();
  affectedVertices.squeeze();
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

  // If the application crashes here, try setting "MESA_GL_VERSION_OVERRIDE = 4.1"
  // and "MESA_GLSL_VERSION_OVERRIDE = 410" in Projects (left panel) -> Build Environment

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

  presetNet(0);
}

void MainView::paintGL() {

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (updateUniformsRequired) {
    updateUniforms();
  }

  if (showCurvePts) {
      subdivShader->bind();
      glBindVertexArray(subdevideVAO);
      glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, subCoords.size());
      glBindVertexArray(1);
      subdivShader->release();
  }

  if (showNet) {
    mainShaderProg->bind();

    glBindVertexArray(netVAO);

    // Draw control net
    glDrawArrays(GL_LINE_STRIP, 0, netCoords.size());
    glPointSize(8.0);
    glDrawArrays(GL_POINTS, 0, netCoords.size());

    // Highlight selected control point
    if (selectedPt > -1) {
      glPointSize(12.0);
      glDrawArrays(GL_POINTS, selectedPt, 1);
    }

    glBindVertexArray(0);

    mainShaderProg->release();
  }

}

// ---

void MainView::presetNet(unsigned short preset) {

  selectedPt = -1;
  clearArrays();

  switch (preset) {
  case 0:
    // 'Pentagon'
    netCoords.reserve(5);
    netCoords.append(QVector2D(-0.25, -0.5));
    netCoords.append(QVector2D(-0.75, 0.0));
    netCoords.append(QVector2D(-0.25, 0.75));
    netCoords.append(QVector2D(0.75, 0.5));
    netCoords.append(QVector2D(0.5, -0.75));
    break;
  case 1:
    // 'Basis'
    netCoords.reserve(9);
    netCoords.append(QVector2D(-1.0, -0.25));
    netCoords.append(QVector2D(-0.75, -0.25));
    netCoords.append(QVector2D(-0.5, -0.25));
    netCoords.append(QVector2D(-0.25, -0.25));
    netCoords.append(QVector2D(0.0, 0.50));
    netCoords.append(QVector2D(0.25, -0.25));
    netCoords.append(QVector2D(0.5, -0.25));
    netCoords.append(QVector2D(0.75, -0.25));
    netCoords.append(QVector2D(1.0, -0.25));
    break;
  }
  subdivide();
  updateBuffers(0);

}

void MainView::setCurveVis(int index) {
   curvVisType = index;
   updateUniformsRequired = true;
   update();
}

void MainView::setMask(QString stringMask) {

    subdivMask.clear();

    QString trimmedMask;
    trimmedMask = stringMask.trimmed();

    // Convert to sequence of integers
    QTextStream intSeq(&trimmedMask);
    while (!intSeq.atEnd()) {
      int k;
      intSeq >> k;
      subdivMask.append(k);
    }

    // Stencils represent affine combinations (i.e. they should sum to unity)
    normalizeValue = 0;

    firstStencil.clear();
    secondStencil.clear();

    for (int k=0; k<subdivMask.size(); k++) {
      if (k % 2) {
        normalizeValue += subdivMask[k];
        firstStencil.append(subdivMask[k]);
      }
      else {
        secondStencil.append(subdivMask[k]);
      }
    }

    subdivide();

    qDebug() << ":: Extracted stencils" << firstStencil << "and" << secondStencil;
}

// ---


void MainView::setSteps(int s){
    steps = s;
    subdivide();
}

void MainView::subdivide(){
    subdivide(false);
}

void MainView::subdivide(bool renderSubset){
    if(netCoords.size()>0 && showCurvePts){
          // The first and last point affected by a point being moved
          int offsetAffected = (renderSubset)?selectedPt:-1;
          int endAffected = offsetAffected;

          // Storage initialization for subCoords in the next step
          QVector<QVector2D> newCoordsTemp;

          // In step 0, subCoords = netCoords
          subCoords = netCoords;

          //  Mark a selected point as affected
          affectedVertices.clear();
          affectedVertices.reserve(subCoords.size());
          for(int k=0;k<subCoords.size();k++)
              affectedVertices.append((k==selectedPt)?1.0f:0.0f);

          // Apply subdivision 'steps' times.
          for(int step=0;step<steps;step++){
              newCoordsTemp.clear();
              newCoordsTemp.reserve(subCoords.size()*2-1);

              // Compute locations of points in the middle of already present points.
              for(int k=0;k<subCoords.size();k++){
                  newCoordsTemp.append(subCoords[k]);
                  if(k!=subCoords.size()-1) newCoordsTemp.append((subCoords[k]+subCoords[k+1])/2.0f);
              }
              subCoords = newCoordsTemp;
              newCoordsTemp.clear();

              // Determine position of first and last point of subdivision curve at current step
              int begin = (secondStencil.size()/2)*2 - ((secondStencil.size()%2==1)?1:0) + ((secondStencil.size()==1)?1:0);
              int end = subCoords.size()-begin;
              newCoordsTemp.reserve(end-begin+1);


              int firstAffected = -1; // Index of first,
              int lastAffected = -1; // and last point affected by control point.

              for(int k=begin;k<end;k++){
                  float isAffected = 0.0f; // When a control point is selected, this variable will be set to 1 if it is affected by said control point.
                  QVector2D newP = QVector2D(0,0); // Will store new location of point.
                  if(k%2==0){ // A point already present in step-1 is processed.
                      for(int i = 0;i<secondStencil.size();i++){
                          int index = k + 2*i - 2*(secondStencil.size()/2);
                          if(secondStencil.size()%2 == 0 && i == secondStencil.size()/2)
                              index += 2;
                          newP += secondStencil[i]/normalizeValue * subCoords[index];
                          if(index%2==0 && affectedVertices[index/2] == 1.0 && secondStencil[i] != 0) isAffected = 1.0f;
                      }
                  }else{ // A new point is processed.
                      for(int i = 0;i<firstStencil.size();i++){
                          int index = k + i - (firstStencil.size()/2);
                          if(firstStencil.size()%2 == 0 && i == firstStencil.size()/2)
                              index += 1;
                          newP += firstStencil[i]/normalizeValue * subCoords[index];
                          if(index%2==0 && affectedVertices[index/2] == 1.0 && firstStencil[i] != 0) isAffected = 1.0f;
                      }
                  }
                  newCoordsTemp.append(newP);
                  // Determine first and last affected point
                  if(firstAffected == -1 && isAffected==1)  firstAffected = k;
                  if(isAffected==1) lastAffected = k;
              }

              subCoords = newCoordsTemp;

              // Create affected points vector
              affectedVertices.clear();
              for(int k=begin;k<end;k++)
                  affectedVertices.append((k >= firstAffected && k <= lastAffected)?1.0:0.0);
              // Update first and last point affected
              if(renderSubset){
                  offsetAffected = firstAffected-secondStencil.size();
                  offsetAffected = (offsetAffected < 0)?0:offsetAffected; // make sure region to update is within index range of subCoords
                  endAffected = lastAffected+secondStencil.size();
                  endAffected = (endAffected > subCoords.size()-1)?subCoords.size()-1:endAffected; // make sure region to update is within index range of subCoords
              }
          }

          updateUniformsRequired = true;

          updateBuffers(12,offsetAffected,endAffected);
      }
}


void MainView::mousePressEvent(QMouseEvent *event) {

  // In order to allow keyPressEvents:
  setFocus();

  float xRatio, yRatio, xScene, yScene;

  xRatio = (float)event->x() / width();
  yRatio = (float)event->y() / height();

  // By default, the drawing canvas is the square [-1,1]^2:
  xScene = (1-xRatio)*-1 + xRatio*1;
  // Note that the origin of the canvas is in the top left corner (not the lower left).
  yScene = yRatio*-1 + (1-yRatio)*1;

  switch (event->buttons()) {
  case Qt::LeftButton:
    if (selectedPt > -1) {
      // De-select control point
      selectedPt = -1;
      setMouseTracking(false);
      subdivide();
    }
    else {
      // Add new control point
      netCoords.append(QVector2D(xScene, yScene));
      subdivide();
      updateBuffers(0);
    }
    break;
  case Qt::RightButton:
    // Select control point
    selectedPt = findClosest(xScene, yScene);
    update();
    break;
  }
}

void MainView::mouseMoveEvent(QMouseEvent *event) {

  if (selectedPt > -1) {
    float xRatio, yRatio, xScene, yScene;

    xRatio = (float)event->x() / width();
    yRatio = (float)event->y() / height();

    xScene = (1-xRatio)*-1 + xRatio*1;
    yScene = yRatio*-1 + (1-yRatio)*1;

    // Update position of the control point
    netCoords[selectedPt] = QVector2D(xScene, yScene);
    subdivide(true);
    updateBuffers(0);
  }

}

void MainView::keyPressEvent(QKeyEvent *event) {

  // Only works when the widget has focus!

  switch(event->key()) {
  case 'G':
    if (selectedPt > -1) {
      // Grab selected control point
      setMouseTracking(true);
    }
    break;
  case 'X':
    if (selectedPt > -1) {
      // Remove selected control point
      netCoords.remove(selectedPt);
      selectedPt = -1;
      subdivide();
      updateBuffers(0);
    }
    break;
  }

}

void MainView::setVisScale(int value){
    visScale = value;
    updateUniformsRequired = true;
    update();
}

short int MainView::findClosest(float x, float y) {

  short int ptIndex;
  float currentDist, minDist = 4;

  for (int k=0; k<netCoords.size(); k++) {
    currentDist = pow((netCoords[k].x()-x),2) + pow((netCoords[k].y()-y),2);
    if (currentDist < minDist) {
      minDist = currentDist;
      ptIndex = k;
    }
  }

  return ptIndex;

}

// ---

void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
  qDebug() << " → Log:" << Message;
}
