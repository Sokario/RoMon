#ifndef SENSORWINDOW_H
#define SENSORWINDOW_H

#include <QMainWindow>

namespace Ui {
class SensorWindow;
}

class SensorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SensorWindow(QWidget *parent = 0);
    ~SensorWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SensorWindow *ui;

    bool locked;
    QIcon lock;
    QIcon unlock;
};

#endif // SENSORWINDOW_H
