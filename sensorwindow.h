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

private:
    Ui::SensorWindow *ui;
};

#endif // SENSORWINDOW_H
