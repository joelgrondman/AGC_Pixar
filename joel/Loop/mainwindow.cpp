#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;

  Meshes.clear();
  Meshes.squeeze();
}

void MainWindow::loadOBJ() {
  OBJFile newModel = OBJFile(QFileDialog::getOpenFileName(this, "Import OBJ File", "models/", tr("Obj Files (*.obj)")));
  Meshes.clear();
  Meshes.squeeze();
  Meshes.append( Mesh(&newModel) );

  ui->MainDisplay->updateMeshBuffers( &Meshes[0] );
  ui->MainDisplay->modelLoaded = true;

  ui->MainDisplay->update();
}

void MainWindow::on_RotateDial_valueChanged(int value) {
  ui->MainDisplay->rotAngle = value;
  ui->MainDisplay->updateMatrices();
}

void MainWindow::on_SubdivSteps_valueChanged(int value) {
  unsigned short k;

  for (k=Meshes.size(); k<value+1; k++) {
    Meshes.append(Mesh());
    subdivideLoop(&Meshes[k-1], &Meshes[k]);
  }

  ui->MainDisplay->updateMeshBuffers( &Meshes[value] );
}

void MainWindow::on_LoadOBJ_clicked() {
  loadOBJ();
  //ui->LoadOBJ->setEnabled(false);
  ui->SubdivSteps->setValue(0);
  ui->SubdivSteps->setEnabled(true);
}

void MainWindow::on_Isophotes_toggled(bool checked)
{
    ui->MainDisplay->updateIsophotes(checked);
    ui->MainDisplay->update();
}

void MainWindow::on_IsophotesRange_sliderMoved(int position)
{
    ui->MainDisplay->updateIsophotesRange(position);
    ui->MainDisplay->update();
}
