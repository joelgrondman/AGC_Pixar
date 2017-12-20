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

  QVector<Mesh> Meshes;
  void importOBJ();

private slots:
  void on_ImportOBJ_clicked();
  void on_RotationDial_valueChanged(int value);
  void on_SubdivSteps_valueChanged(int value);
  void on_MoveToLimPos_toggled(bool val);
  void on_TessMode_toggled(bool boolVal);

  // The following 6 are called when tessellation levels are changed.
  void on_Oa_valueChanged(double value);
  void on_Ob_valueChanged(double value);
  void on_Oc_valueChanged(double value);
  void on_Od_valueChanged(double value);
  void on_I1_valueChanged(double value);
  void on_I2_valueChanged(double value);


  private:
  Ui::MainWindow *ui;
  void renderScene();
};

#endif // MAINWINDOW_H
