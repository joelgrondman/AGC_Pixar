#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <QOpenGLShaderProgram>

#include <QMouseEvent>
#include "mesh.h"

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *Parent = 0);
  ~MainView();

  bool modelLoaded;
  bool wireframeMode;
  bool tessMode; // Render bezier patches if tessMode = true.

  float FoV;
  float dispRatio;
  float rotAngle;
  float tessO1,tessO2,tessO3,tessO4,tessi1,tessi2; // The tessellation levels: first the four outer levels and then the two inner levels.
  bool uniformUpdateRequired;

  void updateMatrices();
  void updateUniforms();
  void updateMeshBuffers(Mesh* currentMesh);

protected:
  void initializeGL();
  void resizeGL(int newWidth, int newHeight);
  void paintGL();

  unsigned int maxInt;

  void renderMesh();

  void mousePressEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* event);

private:
  QOpenGLDebugLogger* debugLogger;

  QMatrix4x4 modelViewMatrix, projectionMatrix;
  QMatrix3x3 normalMatrix;

  // Uniforms
  GLint uniModelViewMatrix, uniProjectionMatrix, uniNormalMatrix;
  GLint tesUniModelViewMatrix, tesUniProjectionMatrix, tesUniNormalMatrix;

  // ---

  QOpenGLShaderProgram* mainShaderProg;
  QOpenGLShaderProgram* tessShaderProg; // Shader program which renders using tessellation.

  GLuint tessVAO, tessMeshCoordsBO, tessMeshNormalsBO, tessMeshIndexBO;
  GLuint mainVAO, mainMeshCoordsBO, mainMeshNormalsBO, mainMeshIndexBO;

  GLuint uniTesO1,uniTesO2,uniTesO3,uniTesO4,uniTesi1,uniTesi2; // The uniforms for the tessellation levels.


  unsigned int meshIBOSize;

  // ---

  QVector<QVector3D> vertexCoords;
  QVector<QVector3D> vertexNormals;
  QVector<unsigned int> polyIndices;

  void createShaderPrograms();
  void createBuffers();  

private slots:
  void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
