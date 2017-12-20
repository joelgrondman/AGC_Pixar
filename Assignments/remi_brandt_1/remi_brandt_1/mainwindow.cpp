#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);

  ui->controlNet->setChecked(true);
  ui->curvePoints->setChecked(true);

  ui->netPresets->addItem("Pentagon");
  ui->netPresets->addItem("Basis");

  // Input restrictions for the Mask
  ui->subdivMask->setValidator(new QRegularExpressionValidator(QRegularExpression("(-?\\d+\\s)+(-?\\d+\\s?)")));

  // Initialise mask
  ui->mainView->setMask(ui->subdivMask->text());

  // Initially hide visualization scale slider as default curvature visualization type is 'none'
  ui->presetLabel_3->hide();
  ui->scaleSlider->hide();
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
  if(checked){
      ui->mainView->subdivide();
  }else{
      ui->mainView->update();
  }
}

void MainWindow::on_curveVis_currentIndexChanged(int index) {
    ui->mainView->setCurveVis(index);

    // Show or hide the curvature modification slider.

    if(index ==0 || index == 3){
        ui->presetLabel_3->hide();
        ui->scaleSlider->hide();
        ui->curveVisBox->resize(ui->curveVisBox->width(),80);
    }else{
        ui->presetLabel_3->show();
        ui->scaleSlider->show();
        ui->curveVisBox->resize(ui->curveVisBox->width(),150);
    }
}

void MainWindow::on_netPresets_currentIndexChanged(int index) {
  if (ui->mainView->isValid()) {
    ui->mainView->presetNet(index);
  }
}

void MainWindow::on_subdivMask_returnPressed() {
  ui->mainView->setMask(ui->subdivMask->text());
  ui->mainView->update();
}

void MainWindow::on_subdivSteps_valueChanged(int arg1) {
  ui->mainView->setSteps(arg1);
}

void MainWindow::on_scaleSlider_valueChanged(int value){
    ui->mainView->setVisScale(value);
}
