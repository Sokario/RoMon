#include "monitorwindow.h"
#include "ui_monitorwindow.h"

MonitorWindow::MonitorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MonitorWindow)
{
    ui->setupUi(this);
    consDistance = 0;
    consAngle = 0;
    posDistance = 0;
    posAngle = 0;
    addDistance = 0;
    addDoubleDistance = 0;
    addAngle = 0;
    addDoubleAngle = 0;

    QPixmap homeImage(":/Icon/Icon/home.png");
    QIcon home;
    home.addPixmap(homeImage);
    ui->pushButton->setIcon(home);
    ui->pushButton->setIconSize(QSize(20,20));

    QPixmap arrowUpImage(":/Icon/Icon/arrow_up.png");
    QIcon arrowUp;
    arrowUp.addPixmap(arrowUpImage);
    ui->pushButton_2->setIcon(arrowUp);
    ui->pushButton_2->setIconSize(QSize(20,20));

    QPixmap doubleArrowUpImage(":/Icon/Icon/double_arrow_up.png");
    QIcon doubleArrowUp;
    doubleArrowUp.addPixmap(doubleArrowUpImage);
    ui->pushButton_3->setIcon(doubleArrowUp);
    ui->pushButton_3->setIconSize(QSize(20,40));

    QPixmap arrowRightImage(":/Icon/Icon/arrow_right.png");
    QIcon arrowRight;
    arrowRight.addPixmap(arrowRightImage);
    ui->pushButton_4->setIcon(arrowRight);
    ui->pushButton_4->setIconSize(QSize(20,20));

    QPixmap doubleArrowRightImage(":/Icon/Icon/double_arrow_right.png");
    QIcon doubleArrowRight;
    doubleArrowRight.addPixmap(doubleArrowRightImage);
    ui->pushButton_5->setIcon(doubleArrowRight);
    ui->pushButton_5->setIconSize(QSize(40,20));

    QPixmap arrowDownImage(":/Icon/Icon/arrow_down.png");
    QIcon arrowDown;
    arrowDown.addPixmap(arrowDownImage);
    ui->pushButton_6->setIcon(arrowDown);
    ui->pushButton_6->setIconSize(QSize(20,20));

    QPixmap doubleArrowDownImage(":/Icon/Icon/double_arrow_down.png");
    QIcon doubleArrowDown;
    doubleArrowDown.addPixmap(doubleArrowDownImage);
    ui->pushButton_7->setIcon(doubleArrowDown);
    ui->pushButton_7->setIconSize(QSize(20,40));

    QPixmap arrowLeftImage(":/Icon/Icon/arrow_left.png");
    QIcon arrowLeft;
    arrowLeft.addPixmap(arrowLeftImage);
    ui->pushButton_8->setIcon(arrowLeft);
    ui->pushButton_8->setIconSize(QSize(20,20));

    QPixmap doubleArrowLeftImage(":/Icon/Icon/double_arrow_left.png");
    QIcon doubleArrowLeft;
    doubleArrowLeft.addPixmap(doubleArrowLeftImage);
    ui->pushButton_9->setIcon(doubleArrowLeft);
    ui->pushButton_9->setIconSize(QSize(40,20));

    shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(quit()));
}

MonitorWindow::~MonitorWindow()
{
    delete ui;
}

void MonitorWindow::setAddDistance(int value)
{
    addDistance = value;
}

int MonitorWindow::getAddDistance()
{
    return addDistance;
}

void MonitorWindow::setAddDoubleDistance(int value)
{
    addDoubleDistance = value;
}

int MonitorWindow::getAddDoubleDistance()
{
    return addDoubleDistance;
}

void MonitorWindow::setAddAngle(int value)
{
    addAngle = value;
}

int MonitorWindow::getAddAngle()
{
    return addAngle;
}

void MonitorWindow::setAddDoubleAngle(int value)
{
    addDoubleAngle = value;
}

int MonitorWindow::getAddDoubleAngle()
{
    return addDoubleAngle;
}

int MonitorWindow::getConsDistance()
{
    return consDistance;
}

int MonitorWindow::getConsAngle()
{
    return consAngle;
}

void MonitorWindow::quit()
{
    this->close();
}

void MonitorWindow::lcd_update()
{
    ui->lcdNumber->display(consDistance);
    ui->lcdNumber_2->display(consAngle);
    ui->lcdNumber_3->display(posDistance);
    ui->lcdNumber_4->display(posAngle);
}

void MonitorWindow::on_pushButton_clicked()
{
    if (ui->checkBox->isChecked())
        emit home();
    else {
        consDistance = 0;
        consAngle = 0;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_2_clicked()
{
    if (ui->checkBox->isChecked())
        emit up();
    else {
        consDistance += addDistance;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_3_clicked()
{
    if (ui->checkBox->isChecked())
        emit doubleUp();
    else {
        consDistance += addDoubleDistance;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_4_clicked()
{
    if (ui->checkBox->isChecked())
        emit right();
    else {
        consAngle += addAngle;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_5_clicked()
{
    if (ui->checkBox->isChecked())
        emit doubleRight();
    else {
        consAngle += addDoubleAngle;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_6_clicked()
{
    if (ui->checkBox->isChecked())
        emit down();
    else {
        consDistance -= addDistance;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_7_clicked()
{
    if (ui->checkBox->isChecked())
        emit doubleDown();
    else {
        consDistance -= addDoubleDistance;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_8_clicked()
{
    if (ui->checkBox->isChecked())
        emit left();
    else {
        consAngle -= addAngle;
        lcd_update();
    }
}

void MonitorWindow::on_pushButton_9_clicked()
{
    if (ui->checkBox->isChecked())
        emit doubleLeft();
    else {
        consAngle -= addDoubleAngle;
        lcd_update();
    }
}

void MonitorWindow::on_pushRun_clicked()
{
    emit runButton();
}

void MonitorWindow::on_checkBox_clicked()
{
    consDistance = 0;
    consAngle = 0;
    lcd_update();
}

void MonitorWindow::on_pushStop_clicked()
{
    emit stopButton();
}
