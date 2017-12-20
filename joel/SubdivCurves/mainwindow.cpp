#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);

  ui->controlNet->setChecked(true);

  ui->curvePoints->setChecked(false);

  ui->netPresets->addItem("Pentagon");
  ui->netPresets->addItem("Basis");

  // Input restrictions for the Mask
  ui->subdivMask->setValidator(new QRegularExpressionValidator(QRegularExpression("(-?\\d+\\s)+(-?\\d+\\s?)")));

  // Initialise mask
  ui->mainView->setMask(ui->subdivMask->text());

  ui->curvePoints->setChecked(true);

}

MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;
}

void MainWindow::on_controlNet_toggled(bool checked) {
  ui->mainView->showNet = checked;
  ui->mainView->update();
}

void MainWindow::on_curvePoints_toggled(bool checked) {
  ui->mainView->showCurvePts = checked;
  ui->mainView->update();
}

void MainWindow::on_netPresets_currentIndexChanged(int index) {
  if (ui->mainView->isValid()) {
    ui->mainView->presetNet(index);
  }
}

void MainWindow::on_subdivMask_returnPressed() {
  ui->mainView->setMask(ui->subdivMask->text());
  ui->mainView->generateSpline();
  ui->mainView->update();
}

void MainWindow::on_subdivSteps_valueChanged(int arg1) {
  ui->mainView->setSubdivSteps(arg1);
  ui->mainView->update();
}


void MainWindow::on_CurvatureCircle_toggled(bool checked)
{
  ui->mainView->showCircles = checked;
  ui->mainView->update();
}

void MainWindow::on_CurvatureNormal_toggled(bool checked)
{
  ui->mainView->showCurvatureNormals = checked;
  ui->mainView->update();
}

void MainWindow::on_CurvatureColor_toggled(bool checked)
{
  ui->mainView->showCurvatureColors = checked;
  ui->mainView->update();
}

// 1000.0 is an arbitrary number chosen for the resolution of the slider
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->mainView->updateCirclePosition(value/1000.0);
    ui->mainView->update();
}

void MainWindow::on_Highlight_toggled(bool checked)
{
    ui->mainView->highlight = checked;
    ui->mainView->update();
}

void MainWindow::on_allCirclesBox_toggled(bool checked)
{
    ui->mainView->updateAllCirclesUniform(checked);
    ui->mainView->update();
}
