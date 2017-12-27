#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "objfile.h"
#include <QFileDialog>
#include "mesh.h"
#include "meshtools.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  //changed into a matrix of meshes, first element indicates
  // number of sharp subdivisions and second number of catmull clark subdivs
  QVector<Mesh> Meshes;
  void importOBJ();

private slots:
  void on_ImportOBJ_clicked();
  void on_RotationDial_valueChanged(int value);
  void on_subdivSteps_valueChanged(int value);
  void on_edgeSharpnesses_valueChanged(double value);
  void on_edgeSharpness_valueChanged(double value);

  void on_dispControlMesh_toggled(bool checked);

  void on_dispSubSurf_toggled(bool checked);

  void on_renderMode_currentIndexChanged(int index);

  void on_saveButton_pressed();

  void on_saveCrease_clicked();

  void on_addCrease_clicked();

  void on_displayCrease_toggled(bool checked);

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
