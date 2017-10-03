#ifndef MONITORWINDOW_H
#define MONITORWINDOW_H

#include <QMainWindow>
#include "QShortcut"

namespace Ui {
class MonitorWindow;
}

class MonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorWindow(QWidget *parent = 0);
    ~MonitorWindow();

    void setAddDistance(int value);
    int getAddDistance();
    void setAddDoubleDistance(int value);
    int getAddDoubleDistance();
    void setAddAngle(int value);
    int getAddAngle();
    void setAddDoubleAngle(int value);
    int getAddDoubleAngle();

    int getConsDistance();
    int getConsAngle();

signals:
    void runButton();

    void home();

    void up();

    void doubleUp();

    void right();

    void doubleRight();

    void down();

    void doubleDown();

    void left();

    void doubleLeft();

private slots:
    void quit();

    void lcd_update();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushRun_clicked();

    void on_checkBox_clicked();

private:
    Ui::MonitorWindow *ui;
    QShortcut *shortcut;

    int addDistance;
    int addDoubleDistance;
    int addAngle;
    int addDoubleAngle;

    int consDistance;
    int consAngle;
    int posDistance;
    int posAngle;
};

#endif // MONITORWINDOW_H
