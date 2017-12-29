#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
    qDebug() << "✓✓ MainWindow constructor";
    ui->setupUi(this);

    // Initiaize Ege selection UI elements
    ui->MainDisplay->selectionUIBox = ui->edgeSharpnessBox;
    ui->MainDisplay->selectionUIBox->hide();
    ui->MainDisplay->selectionUIValue = ui->edgeSharpness;

    ui->addCrease->show();
    ui->saveCrease->hide();
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
    Meshes.append(Mesh(&newModel));

    ui->MainDisplay->updateMeshBuffers( &Meshes[0],&Meshes[0]);
    ui->MainDisplay->updateSelectionBuffers();
    ui->MainDisplay->modelLoaded = true;

    ui->MainDisplay->update();
}

void MainWindow::on_ImportOBJ_clicked() {
    importOBJ();
    ui->ImportOBJ->setEnabled(false);
    ui->subdivSteps->setEnabled(true);
}

void MainWindow::on_RotationDial_valueChanged(int value) {
    ui->MainDisplay->rotAngle = value;
    ui->MainDisplay->updateMatrices();
}

void MainWindow::on_subdivSteps_valueChanged(int value) {
    unsigned short k;
    unsigned short sharpSteps = ui->edgeSharpness->value();

    for (k=Meshes.size(); k<value + 1; k++) {
        Meshes.append(Mesh());
        subdivideCatmullClark(&Meshes[k-1], &Meshes[k]);
    }
    ui->MainDisplay->updateMeshBuffers( &Meshes[value], &Meshes[0]);
}

void MainWindow::on_edgeSharpnesses_valueChanged(double value)
{

    unsigned short smoothSteps = ui->subdivSteps->value();

    Meshes.resize(1);
    Meshes.squeeze();
    for (int i = 0; i < Meshes[0].HalfEdges.size(); ++i) {
        Meshes[0].HalfEdges[i].sharpness = value;
    }

    //call on this function to fill in the smooth steps (if needed)
    on_subdivSteps_valueChanged(smoothSteps);


    //making ready for different sharp values everywhere
    //subdivide as usual with sharp rules (inefficient)
    // for additional smooth subdivision start from previous step, throw these away when sharpness changes
    // for the first sharp subdivisions create two subdivision meshes one floor, one ceil of local sharpness, interpolate at last step

}

void MainWindow::on_edgeSharpness_valueChanged(double value){
    if(ui->MainDisplay->selectedEdges.size() == 1){
        unsigned int selectedEdge = ui->MainDisplay->selectedEdges[0];

        Meshes[0].HalfEdges[selectedEdge].sharpness = value;
        Meshes[0].HalfEdges[selectedEdge].twin->sharpness = value;

        Meshes.resize(1);
        Meshes.squeeze();

        unsigned short smoothSteps = ui->subdivSteps->value();
        on_subdivSteps_valueChanged(smoothSteps);
    }
}

void MainWindow::on_dispControlMesh_toggled(bool checked)
{
    ui->MainDisplay->dispControlMesh = checked;
    ui->MainDisplay->update();
}

void MainWindow::on_dispSubSurf_toggled(bool checked)
{
    ui->MainDisplay->dispSubdivSurface = checked;
    ui->MainDisplay->update();
}

void MainWindow::on_renderMode_currentIndexChanged(int index)
{
    ui->MainDisplay->wireframeMode = index==0;
    ui->MainDisplay->update();
}

void MainWindow::on_saveButton_pressed()
{
    QString imagePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save File"),
                    "",
                    tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" )
                    );

    ui->MainDisplay->grabFramebuffer().save(imagePath);
}

void MainWindow::on_saveCrease_clicked()
{
    ui->addCrease->show();
    ui->saveCrease->hide();
    ui->MainDisplay->creaseIsBeingSelected = false;
    ui->MainDisplay->creases.append(ui->MainDisplay->selectedEdges);
    ui->MainDisplay->selectedEdges.clear();
    ui->MainDisplay->updateSelectionBuffers();

    //when creases are updated mesh needs to be updated to incorporate finer creases
    QVector<QVector<unsigned int>> creases = ui->MainDisplay->creases;
    Meshes.resize(1);
    Meshes.squeeze();
    for (int c = 0; c < creases.size(); ++c) {
        for (int e = 0; e < creases[c].size(); ++ e) {
            Meshes[0].HalfEdges[creases[c][e]].crease = c;
            Meshes[0].HalfEdges[creases[c][e]].twin->crease = c;
        }
    }

    //call on this function to fill in the smooth steps (if needed)
    on_subdivSteps_valueChanged(ui->subdivSteps->value());
}

void MainWindow::on_addCrease_clicked()
{
    ui->MainDisplay->selectedEdges.clear();
    ui->addCrease->hide();
    ui->saveCrease->show();
    ui->edgeSharpnessBox->hide();
    ui->MainDisplay->selectedEdges.clear();
    ui->MainDisplay->creaseIsBeingSelected = true;
    ui->MainDisplay->updateSelectionBuffers();
    ui->MainDisplay->update();
}

void MainWindow::on_displayCrease_toggled(bool checked)
{
    ui->MainDisplay->displayCreases = checked?1:0;
    ui->MainDisplay->uniformUpdateRequired = true;
    ui->MainDisplay->updateSelectionBuffers();
    ui->MainDisplay->update();
}
