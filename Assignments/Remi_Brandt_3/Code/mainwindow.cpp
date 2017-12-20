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


void MainWindow::on_TessMode_toggled(bool boolVal){
  ui->MainDisplay->tessMode = boolVal;
  renderScene();
}

void MainWindow::on_SubdivSteps_valueChanged(int value) {
  renderScene();
}

void MainWindow::on_MoveToLimPos_toggled(bool boolVal){
  renderScene();
}

void MainWindow::on_Oa_valueChanged(double value) {
    ui->MainDisplay->tessO1 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
void MainWindow::on_Ob_valueChanged(double value) {
    ui->MainDisplay->tessO2 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
void MainWindow::on_Oc_valueChanged(double value) {
    ui->MainDisplay->tessO3 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
void MainWindow::on_Od_valueChanged(double value) {
    ui->MainDisplay->tessO4 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
void MainWindow::on_I1_valueChanged(double value) {
    ui->MainDisplay->tessi1 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}
void MainWindow::on_I2_valueChanged(double value) {
    ui->MainDisplay->tessi2 = value;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->update();
}

void MainWindow::renderScene(){
    // Subdivide if needed.

    unsigned short value = ui->SubdivSteps->value();
    for (unsigned short k=Meshes.size(); k<value+1; k++) {
      Meshes.append(Mesh());
      subdivideCatmullClark(&Meshes[k-1], &Meshes[k]);
    }
    if(ui->MoveToLimPos->isChecked()){
        // Move vertices to their limit position, then render.
        Mesh limitPosMesh;
        findLimitPos(&Meshes[value], &limitPosMesh);
        ui->MainDisplay->updateMeshBuffers( &limitPosMesh );
    }else{
        // Render scene.
        ui->MainDisplay->updateMeshBuffers( &Meshes[value] );
    }
}
