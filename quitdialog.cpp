#include "quitdialog.h"
#include "ui_quitdialog.h"

Quitdialog::Quitdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Quitdialog)
{
    ui->setupUi(this);
}

Quitdialog::~Quitdialog()
{
    delete ui;
}
