#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sensorwindow.h"
#include "enterkeyhandler.h"

#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void commandHandler();

    void serialRead();

    void serialTimeout();

    void errorMessage();

    void clearCheckedMenu(QMenu *menu);

    void on_actionQuit_triggered();

    void on_actionRefresh_triggered();

    void on_action1200_triggered();

    void on_action2400_triggered();

    void on_action4800_triggered();

    void on_action9600_triggered();

    void on_action19200_triggered();

    void on_action38400_triggered();

    void on_action57600_triggered();

    void on_action115200_triggered();

    void on_action5_triggered();

    void on_action6_triggered();

    void on_action7_triggered();

    void on_action8_triggered();

    void on_actionnone_triggered();

    void on_actioneven_triggered();

    void on_actionodd_triggered();

    void on_actionspace_triggered();

    void on_actionmark_triggered();

    void on_action1_bits_triggered();

    void on_action1_5_bits_triggered();

    void on_action2_bits_triggered();

    void on_actionConfiguration_triggered();

    void on_actionCOM_1_triggered();

    void on_actionCOM_2_triggered();

    void on_actionCOM_3_triggered();

    void on_actionCOM_4_triggered();

    void on_actionCOM_5_triggered();

    void on_actionCOM_6_triggered();

    void on_actionCOM_7_triggered();

    void on_actionCOM_8_triggered();

    void on_actionCOM_9_triggered();

    void on_actionCOM_10_triggered();

    void on_actionCOM_11_triggered();

    void on_actionCOM_12_triggered();

    void on_actionCOM_13_triggered();

    void on_actionCOM_14_triggered();

    void on_actionCOM_15_triggered();

    void on_actionCOM_16_triggered();

    void on_actionNo_Port_triggered();

    void on_actionTest_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionSensor_triggered();

private:
    Ui::MainWindow *ui;
    SensorWindow *sensorWindow;
    EnterKeyHandler *enterKey;
    QList<QAction*> actions;
    QSerialPort *serialPort;

    qint32 comBaudRate;
    bool comBreakEnable;
    QSerialPort::DataBits comDataBits;
    bool comDataTermReady;
    bool comFlowControl;
    bool comParity;
    QString comPortName;
    QSerialPortInfo comPort;
    qint64 comReadBufferSize;
    bool comRequestToSend;
    QSerialPort::StopBits comStopBit;
    QByteArray serialData;
    QTimer comTimer;
};

#endif // MAINWINDOW_H
