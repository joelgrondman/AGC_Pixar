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
  Meshes.append( Mesh(&newModel) );

  ui->MainDisplay->updateMeshBuffers( &Meshes[0] );
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

  for (k=Meshes.size(); k<value+1; k++) {
    Meshes.append(Mesh());
    subdivideCatmullClark(&Meshes[k-1], &Meshes[k]);
  }

  if (limitStencil) {
    //save current coordinates
    QVector<QVector3D> oldCoords;
    unsigned int numVerts = Meshes[value].Vertices.size();
    oldCoords.reserve(numVerts);
    unsigned int k;
    for(k=0;k<numVerts;++k)
        oldCoords.append(Meshes[value].Vertices[k].coords);

    //apply new coordinates
    applyLimitStencils(&Meshes[value]);
    ui->MainDisplay->updateMeshBuffers(&Meshes[value]);

    //reapply old coordinates
    for(k=0;k<numVerts;++k)
        Meshes[value].Vertices[k].coords = oldCoords[k];


  } else {
    ui->MainDisplay->updateMeshBuffers(&Meshes[value] );
  }


}

void MainWindow::on_LimitStencil_toggled(bool checked)
{
    limitStencil = checked;
    int steps = ui->SubdivSteps->value();
    if (limitStencil) {
      //save current coordinates
      QVector<QVector3D> oldCoords;

      unsigned int numVerts = Meshes[steps].Vertices.size();
      oldCoords.reserve(numVerts);
      unsigned int k;
      for(k=0;k<numVerts;++k)
          oldCoords.append(Meshes[steps].Vertices[k].coords);

      //apply new coordinates
      applyLimitStencils(&Meshes[steps]);

      ui->MainDisplay->updateMeshBuffers(&Meshes[steps]);

      //reapply old coordinates
      for(k=0;k<numVerts;++k)
          Meshes[steps].Vertices[k].coords = oldCoords[k];


    } else {
      ui->MainDisplay->updateMeshBuffers(&Meshes[steps] );
    }
}

void MainWindow::on_SurfacePatches_toggled(bool checked)
{

    ui->MainDisplay->surfacePatches = checked;
    ui->MainDisplay->updateMeshBuffers(&Meshes[ui->SubdivSteps->value()] );
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}

void MainWindow::on_Inner1Slider_valueChanged(int value)
{
    ui->MainDisplay->inner1=value/10.0;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}

void MainWindow::on_Inner2Slider_valueChanged(int value)
{
    ui->MainDisplay->inner2=value/10.0;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}

void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    ui->MainDisplay->outer=value/10.0;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
