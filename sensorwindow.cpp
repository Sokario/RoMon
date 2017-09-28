#include "sensorwindow.h"
#include "ui_sensorwindow.h"

SensorWindow::SensorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SensorWindow)
{
    ui->setupUi(this);
}

SensorWindow::~SensorWindow()
{
    delete ui;
}
