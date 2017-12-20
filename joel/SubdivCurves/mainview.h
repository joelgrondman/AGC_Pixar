#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>

#include <QVector2D>
#include <QMouseEvent>
#include <algorithm>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *parent = 0);
  ~MainView();

  void clearArrays();
  void presetNet(unsigned short preset);
  void updateBuffers();
  void updateSplineBuffers();
  void updateCirclePosition(float pos);

  bool showNet, showCurvePts, showCircles = false, showCurvatureNormals = false, showCurvatureColors = false,
  highlight = false;
  short int selectedPt;
  short int findClosest(float x, float y);

  void setMask(QString stringMask);

  void setSubdivSteps(int steps);

  void generateSpline();

  void updateAllCirclesUniform(bool logic);

protected:
  void initializeGL();
  void paintGL();

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QOpenGLDebugLogger* debugLogger;

  QVector<QVector2D> netCoords;
  QVector<QVector2D> splineCoords;

  QOpenGLShaderProgram* mainShaderProg, *splineShaderProg, *circleShaderProg, *normalShaderProg,
    *splineColorShaderProg, *highlightShaderProg;
  GLuint netVAO, netCoordsBO,
    splineVAO, splineCoordsBO, splineColorBO;
  GLint uniformCirclePos, uniformAllCircles;

  void createShaderPrograms();
  void createBuffers();

  bool updateUniformsRequired;

  //GLint uni...

  void updateCirclePosUniform();
  void updateCurvature();
  void updateHighlightRange();


  QVector<short int> subdivMask, firstStencil, secondStencil;
  float normalizeValue;
  float positionCircle;
  int highlightRange[2] = {0,0};
  int subdivSteps;

private slots:
  void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
