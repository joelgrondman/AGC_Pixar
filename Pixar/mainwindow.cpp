#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;

  Meshes.clear();
  Meshes.squeeze();
}

void MainWindow::importOBJ() {
  OBJFile newModel = OBJFile(QFileDialog::getOpenFileName(this, "Import OBJ File", "models/", tr("Obj Files (*.obj)")));
  Meshes.clear();
  Meshes.squeeze();
  Meshes.append(QVector<Mesh>(1,Mesh(&newModel)));

  ui->MainDisplay->updateMeshBuffers( &Meshes[0][0] );
  ui->MainDisplay->modelLoaded = true;

  ui->MainDisplay->update();
}

void MainWindow::on_ImportOBJ_clicked() {
  importOBJ();
  ui->ImportOBJ->setEnabled(false);
  ui->SubdivSteps->setEnabled(true);
}

void MainWindow::on_RotationDial_valueChanged(int value) {
  ui->MainDisplay->rotAngle = value;
  ui->MainDisplay->updateMatrices();
}

void MainWindow::on_SubdivSteps_valueChanged(int value) {
  unsigned short k;
  unsigned short sharpSteps = ui->spinBox->value();
  int smoothSteps = std::max(value - sharpSteps,0);

  for (k=Meshes[sharpSteps].size(); k<smoothSteps + 1; k++) {
    Meshes[sharpSteps].append(Mesh());
    subdivideCatmullClark(&Meshes[sharpSteps][k-1], &Meshes[sharpSteps][k]);
  }
  ui->MainDisplay->updateMeshBuffers( &Meshes[sharpSteps][smoothSteps] );
}

void MainWindow::on_spinBox_valueChanged(int value)
{
    unsigned short k;

    unsigned short smoothSteps = ui->SubdivSteps->value();
    unsigned short sharpSteps = value;

    qDebug() << Meshes.size() << Meshes[Meshes.size()-1].size();

    for (k=Meshes.size(); k<sharpSteps + 1; k++) {
      Meshes.append(QVector<Mesh>(1,Mesh()));
      sharpSubdivideCatmullClark(&Meshes[k-1][0], &Meshes[k][0]);
    }

    qDebug() << Meshes.size() << Meshes[Meshes.size()-1].size();
    //call on this function to fill in the smooth steps (if needed)
    on_SubdivSteps_valueChanged(smoothSteps);

    //when sharpness value changes whole subdivision needs to be repeated
    //if subdivsteps changes then we can simply subdivide from the previous mesh
    //store smooth meshes and sharp meshes

    // clamp to maximum subdivide steps
    //unsigned short steps = min(arg1,ui->MainWindow.SubdivSteps->value());

}
