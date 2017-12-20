#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>

#include <QVector2D>
#include <QMouseEvent>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *parent = 0);
  ~MainView();

  void clearArrays();
  void presetNet(unsigned short preset);
  void updateBuffers(int index, int offset, int end);
  void updateBuffers(int index);
  void setSteps(int s);
  void setVisScale(int value);
  bool showNet, showCurvePts;
  short int selectedPt;
  short int findClosest(float x, float y);

  void setMask(QString stringMask);
  void subdivide();
  void subdivide(bool renderSubset);
  void setCurveVis(int index);

protected:
  void initializeGL();
  void paintGL();

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QOpenGLDebugLogger* debugLogger;

  QVector<QVector2D> netCoords, subCoords;

  QOpenGLShaderProgram* mainShaderProg, *subdivShader;

  GLuint netVAO, netCoordsBO,subdevideVAO, subdivideCoordsBO, affectedVertsBO;
  QVector<float> affectedVertices;

  void createShaderPrograms();
  void createBuffers();
  void updateUniforms();

  bool updateUniformsRequired = true;
  int curvVisType; // What kind of curvature visualization to show (0 = none, 1 = curvature combs, 2 = colors, 3 = centers of osculating circles)
  int steps; // Subdivision steps to perform
  int visScale; // Curvature visualization modifier, e.g. how large the curvature combs should be

  QVector<short int> subdivMask, firstStencil, secondStencil;
  float normalizeValue;

private slots:
  void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
