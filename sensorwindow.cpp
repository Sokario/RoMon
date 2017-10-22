#include "sensorwindow.h"
#include "ui_sensorwindow.h"

SensorWindow::SensorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SensorWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Icon/Icon/Evo.png"));

    locked = false;

    QPixmap lockImage(":/Icon/Icon/lock.png");
    lock.addPixmap(lockImage);

    QPixmap unlockImage(":/Icon/Icon/unlock.png");
    unlock.addPixmap(unlockImage);

    ui->pushButton->setIcon(lock);
    ui->pushButton->setIconSize(QSize(20,20));
}

SensorWindow::~SensorWindow()
{
    delete ui;
}

void SensorWindow::on_pushButton_clicked()
{
    if (locked) {
        locked = false;
        ui->pushButton->setIcon(lock);
    } else {
        locked = true;
        ui->pushButton->setIcon(unlock);
    }
}
