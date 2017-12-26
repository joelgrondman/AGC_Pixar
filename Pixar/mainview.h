#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <QOpenGLShaderProgram>

#include <QMouseEvent>
#include "mesh.h"

#include <QGroupBox>
#include <QDoubleSpinBox>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *Parent = 0);
  ~MainView();

  bool modelLoaded;
  bool wireframeMode;

  float FoV;
  float dispRatio;
  float rotAngle;

  bool uniformUpdateRequired;
  int sharpSubdivSteps = 0;

  void updateMatrices();
  void updateUniforms();
  void updateMeshBuffers(Mesh* currentMesh, Mesh* orgMesh);
  void updateSelectionBuffers();
  QGroupBox* selectionUIBox;
  QDoubleSpinBox* selectionUIValue;
  int selectedEdge;
  bool dispControlMesh;
  bool dispSubdivSurface;

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
  GLint uniModelViewMatrix, uniProjectionMatrix, uniNormalMatrix,
        uniSelModelViewMatrix, uniSelProjectionMatrix;
  // ---

 QOpenGLShaderProgram* mainShaderProg,* selectionShader;

  GLuint meshVAO, meshCoordsBO, meshNormalsBO, meshIndexBO;
  GLuint selectionVAO, selectionCoordsBO,selectionSharpnessBO; // Storage for coordinates of selected edge or vertex

  unsigned int meshIBOSize;

  // ---

  QVector<QVector3D> vertexCoords;
  QVector<QVector3D> vertexNormals;
  QVector<unsigned int> polyIndices;

  QVector<float> selectionSharpness;
  QVector<QVector3D> controlMeshLines;
  Mesh* origMesh; // Pointer to the control mesh
  Mesh* curDispMesh; // Pointer to the mesh which is currently displayed

  void createShaderPrograms();
  void createBuffers();  


  void selectEdge(QVector4D nearpos, QVector4D farpos);

private slots:
  void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
