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
  int isophotesNumStripes; // Number of isophote stripes to display.
  int isophotesDistance; // Width of black isophote stripes.
  int isophotesSmoothness; // Indicates how sharp the isophote stripes must be.
  int visMode; // Curvature visualization mode (0 = none, 1 = isophotes).

  float FoV;
  float dispRatio;
  float rotAngle;

  bool uniformUpdateRequired;

  Mesh* curDispMesh; // Pointer to the mesh which is currently displayed

  void updateMatrices();
  void updateUniforms();
  void updateMeshBuffers(Mesh* currentMesh);

  void updateSelectionBuffers();

protected:
  void initializeGL();
  void resizeGL(int newWidth, int newHeight);
  void paintGL();

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
        uniINumIsophotes,  uniIsophotesDistance,uniIsophotesSmoothness, uniVismode,
        uniSelModelViewMatrix, uniSelProjectionMatrix;

  // ---

  QOpenGLShaderProgram* mainShaderProg,* selectionShader;
  GLuint meshVAO, meshCoordsBO, meshNormalsBO, meshIndexBO;
  GLuint selectionVAO, selectionCoordsBO; // Storage for coordinates of selected edge or vertex

  unsigned int meshIBOSize;

  // ---

  int selectedPt; // Selected vertex.
  int selectedEdge; // Selected edge.
  QVector<QVector3D> selectedPoints; // Coordinates of selected edge or vertex point(s)

  QVector<QVector3D> vertexCoords;
  QVector<QVector3D> vertexNormals;
  QVector<unsigned int> polyIndices;

  void createShaderPrograms();
  void createBuffers();  

  void selectVertex(QVector4D nearpos, QVector4D farpos);
  void selectEdge(QVector4D nearpos, QVector4D farpos);
  
private slots:
  void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
