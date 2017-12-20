#include "mainview.h"
#include "math.h"

MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "✓✓ MainView constructor";

  subdivSteps = 0;
  selectedPt = -1;
}

MainView::~MainView() {
  qDebug() << "✗✗ MainView destructor";

  clearArrays();

  glDeleteBuffers(1, &netCoordsBO);
  glDeleteBuffers(1, &splineCoordsBO);
  glDeleteVertexArrays(1, &netVAO);
  glDeleteVertexArrays(1, &splineVAO);

  delete mainShaderProg;
  delete splineShaderProg;
  delete circleShaderProg;
  delete normalShaderProg;
  delete splineColorShaderProg;
  delete highlightShaderProg;

  debugLogger->stopLogging();
}

// ---

void MainView::createShaderPrograms() {
  initializeOpenGLFunctions();

  // Qt approach
  mainShaderProg = new QOpenGLShaderProgram();
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

  mainShaderProg->link();

  splineColorShaderProg = new QOpenGLShaderProgram();
  splineColorShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  splineColorShaderProg->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/geometryshader_color.glsl");
  splineColorShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_spline.glsl");

  splineColorShaderProg->link();

  splineShaderProg = new QOpenGLShaderProgram();
  splineShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  splineShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_spline.glsl");

  splineShaderProg->link();

  circleShaderProg = new QOpenGLShaderProgram();
  circleShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  circleShaderProg->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/geometryshader_circles.glsl");
  circleShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_primitive.glsl");

  circleShaderProg->link();

  normalShaderProg = new QOpenGLShaderProgram();
  normalShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  normalShaderProg->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/geometryshader_normals.glsl");
  normalShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_primitive.glsl");

  normalShaderProg->link();

  highlightShaderProg = new QOpenGLShaderProgram();
  highlightShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  highlightShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_highlight.glsl");

  highlightShaderProg->link();

  uniformCirclePos = glGetUniformLocation(circleShaderProg->programId(), "circlePos");
  uniformAllCircles = glGetUniformLocation(circleShaderProg->programId(), "allCircles");

}

void MainView::createBuffers() {

  // Pure OpenGL
  glGenVertexArrays(1, &netVAO);
  glBindVertexArray(netVAO);

  glGenBuffers(1, &netCoordsBO);
  glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glGenVertexArrays(1, &splineVAO);
  glBindVertexArray(splineVAO);

  glGenBuffers(1, &splineCoordsBO);
  glBindBuffer(GL_ARRAY_BUFFER, splineCoordsBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);

}

void MainView::updateBuffers() {

  glBindBuffer(GL_ARRAY_BUFFER, netCoordsBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D)*netCoords.size(), netCoords.data(), GL_DYNAMIC_DRAW);

  // whenever coordinates change the splines have to be updated as well
  generateSpline();

  update();

}

void MainView::updateSplineBuffers() {

  glBindBuffer(GL_ARRAY_BUFFER, splineCoordsBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D)*splineCoords.size(), splineCoords.data(), GL_DYNAMIC_DRAW);


  // whener the spline coordinates change, update highlighted splines and position of circle on spline as well
  updateHighlightRange();
  updateCirclePosUniform();

}


void MainView::updateCirclePosUniform() {

  circleShaderProg->bind();

  // from range 0 to 1 determines the index where the circle must be drawn
  // 5 splines coordinates => 3 possible locations for circle
  glUniform1i(uniformCirclePos, round(positionCircle*(splineCoords.size()-3)));

  circleShaderProg->release();
}

void MainView::updateAllCirclesUniform(bool logic) {

  circleShaderProg->bind();

  // 1 is true, 0 is false
  glUniform1i(uniformAllCircles, (logic ? 1: 0));

  circleShaderProg->release();

}

void MainView::clearArrays() {

  // As of Qt 5.6, clear() does not release the memory anymore. Use e.g. squeeze()
  netCoords.clear();
  netCoords.squeeze();

  splineCoords.clear();
  splineCoords.squeeze();
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

  // Enable blending, necessary for blending the separate lines generated by geometryshader_color
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // ---

  createShaderPrograms();
  createBuffers();

  presetNet(0);
}

void MainView::paintGL() {

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  if (showNet) {

    mainShaderProg->bind();

    glBindVertexArray(netVAO);

    // Draw control net
    glDrawArrays(GL_LINE_STRIP, 0, netCoords.size());

    // Draw control net control points
    glPointSize(8.0);
    glDrawArrays(GL_POINTS, 0, netCoords.size());

    // Highlight selected control point
    if (selectedPt > -1) {
      glPointSize(12.0);
      glDrawArrays(GL_POINTS, selectedPt, 1);
    }

    mainShaderProg->release();

    glBindVertexArray(0);
  }

  if (showCurvePts) {
      // Draw splines

      // Draw splines where color indicates curvature
      if(showCurvatureColors) {
          splineColorShaderProg->bind();

          glBindVertexArray(splineVAO);
          glDrawArrays(GL_TRIANGLE_STRIP, 0, splineCoords.size());

          splineColorShaderProg->release();
      // Draw splines with one color
      } else {

          splineShaderProg->bind();

          glBindVertexArray(splineVAO);
          glDrawArrays(GL_LINE_STRIP, 0, splineCoords.size());

          splineShaderProg->release();
      }

      // highlight part of the spline (by drawing over the spline to show sharp color differences)
      if (highlight) {
          highlightShaderProg->bind();

          //qDebug() << highlightRange[0] << highlightRange[1];
          glBindVertexArray(splineVAO);
          glDrawArrays(GL_LINE_STRIP, highlightRange[0],
                  highlightRange[1]-highlightRange[0] + 1);

          highlightShaderProg->release();
      }

      // Draw curvature circles
      if (showCircles) {

          circleShaderProg->bind();

          glBindVertexArray(splineVAO);
          glPointSize(8.0);
          glDrawArrays(GL_TRIANGLE_STRIP, 0, splineCoords.size());

          circleShaderProg->release();
      }

      // Draw curvature normals
      if (showCurvatureNormals) {
          // Draw curvature circles
          normalShaderProg->bind();

          glBindVertexArray(splineVAO);
          glPointSize(8.0);
          glDrawArrays(GL_TRIANGLE_STRIP, 0, splineCoords.size());

          normalShaderProg->release();
      }

      glBindVertexArray(0);
  }


}

void MainView::presetNet(unsigned short preset) {

  selectedPt = -1;
  updateHighlightRange();

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

  updateBuffers();

}

// update position of circle on the spline
void MainView::updateCirclePosition(float pos) {
  positionCircle = pos;
  updateCirclePosUniform();
}

// generate the spline given the mask, subdivsteps and netCoords
// this is done applying convolution subdivsteps times
void MainView::generateSpline() {

    splineCoords.clear();

    splineCoords = netCoords;

    // used to hold previous subdivision
    QVector<QVector2D> splineCoordsTemp;

    //recursively refine splineCoords
    for (int r = 0; r < subdivSteps; ++r) {

        splineCoordsTemp = splineCoords;

        //initialize splineCoords
        splineCoords = QVector<QVector2D>(splineCoordsTemp.size()*2 - firstStencil.size() + 1
                                          - secondStencil.size() + 1,
                                          QVector2D(0,0));

        for (int i = 0; i < splineCoordsTemp.size(); ++i) {
            // add first stencil contribution
            for (int j = 0; j < firstStencil.size(); ++j)
                if ((i + firstStencil.size() - 1 - j <= splineCoordsTemp.size() - 1) &&
                        i - j >= 0)
                    splineCoords[i*2 - 2*j] += firstStencil[j]*splineCoordsTemp[i]/normalizeValue;

            // add second stencil contribution
            for (int j = 0; j < secondStencil.size(); ++j)
                if ((i + secondStencil.size() - 1 - j <= splineCoordsTemp.size() - 1) &&
                        i - j >= 0)
                    splineCoords[2*i + 1 - 2*j] += secondStencil[j]*splineCoordsTemp[i]/normalizeValue;
        }
    }

    updateSplineBuffers();

}

// used to find the range of highlighted colors depending on the currently selected point
// uses same procedure as for generating splines
// after convolution the affected range of coordinates can be retrieved
void MainView::updateHighlightRange() {

    // currently no selection, highlight nothing
    if (selectedPt == -1) {

        highlightRange[0] = 0;
        highlightRange[1] = 0;

    } else {

        //initialization, at 0 steps of subdivision only current selection is highlighted
        QVector<bool> spread(netCoords.size(),false);
        spread[selectedPt] = true;

        // used to hold previous affected points
        QVector<bool> spreadTemp;

        //recursively refine splineCoords
        for (int r = 0; r < subdivSteps; ++r) {

            spreadTemp = spread;

            //initialize splineCoords
            spread = QVector<bool>(spreadTemp.size()*2 - firstStencil.size() + 1
                                              - secondStencil.size() + 1,
                                              false);

            for (int i = 0; i < spreadTemp.size(); ++i) {
                // add first stencil contribution
                for (int j = 0; j < firstStencil.size(); ++j)
                    if ((i + firstStencil.size() - 1 - j <= spreadTemp.size() - 1) &&
                            i - j >= 0)
                        spread[i*2 - 2*j] = (spreadTemp[i]||spread[i*2 - 2*j]);

                // add second stencil contribution
                for (int j = 0; j < secondStencil.size(); ++j)
                    if ((i + secondStencil.size() - 1 - j <= spreadTemp.size() - 1) &&
                            i - j >= 0)
                        spread[2*i + 1 - 2*j] = (spreadTemp[i]||spread[2*i + 1 - 2*j]);
            }
        }

        // determine the lowest affected coordinate, note that this coordinate is decreased by one
        // as our goal is to highlight lines, not points (neighbouring lines are affected by each affected point)
        for (int p = 0; p < spread.size(); ++p) {
            if (spread[p]) {
                highlightRange[0] = std::max(0,p-1);
                break;
            }
        }

        // determine highest affected coordinate
        for (int p = spread.size() - 1; p >= 0; --p) {
            if (spread[p]) {
                highlightRange[1] = std::min(spread.size()-1,p+1);
                break;
            }
        }
    }
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

  qDebug() << ":: Extracted stencils" << firstStencil << "and" << secondStencil;
}

// set number of subdivision steps, spline coordinates are updated accordingly
void MainView::setSubdivSteps(int steps){

    subdivSteps = steps;
    generateSpline();

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
      updateHighlightRange();
      update();
    }
    else {
      // Add new control point
      netCoords.append(QVector2D(xScene, yScene));

      updateBuffers();
    }
    break;
  case Qt::RightButton:
    // Select control point
    selectedPt = findClosest(xScene, yScene);
    updateHighlightRange();
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
    updateBuffers();
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
      updateHighlightRange();
      updateBuffers();
    }
    break;
  }

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
