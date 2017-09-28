#ifndef QUITDIALOG_H
#define QUITDIALOG_H

#include <QDialog>

namespace Ui {
class Quitdialog;
}

class Quitdialog : public QDialog
{
    Q_OBJECT

public:
    explicit Quitdialog(QWidget *parent = 0);
    ~Quitdialog();

private:
    Ui::Quitdialog *ui;
};

#endif // QUITDIALOG_H
