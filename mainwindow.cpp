#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quitdialog.h"

#include "iostream"
#include "unistd.h"
#include "string"

#define TIMEOUT 20

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    enterKey = new EnterKeyHandler(this);

    comBaudRate = QSerialPort::Baud9600;
    comBreakEnable = true;
    comDataBits = QSerialPort::Data8;
    comDataTermReady = false;
    comFlowControl = QSerialPort::NoFlowControl;
    comParity = QSerialPort::NoParity;
    comPortName = "None";
    comPort = QSerialPortInfo(comPortName);
    comReadBufferSize = 0;
    comRequestToSend = false;
    comStopBit = QSerialPort::OneStop;
    serialData.clear();
    serialPort = new QSerialPort(comPort);

    connect(enterKey, SIGNAL(enterKeyPressed()), this, SLOT(commandHandler()));
    connect(&comTimer, &QTimer::timeout, this, &MainWindow::serialTimeout);
//    connect(m_serialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::handleError);

    ui->textEdit_3->installEventFilter(enterKey);

    MainWindow::on_actionRefresh_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::commandHandler()
{
    QString str = ui->textEdit_3->toPlainText();
    QStringList command = str.split("\n");
    ui->textEdit_2->append("Sending data: |" + command.last() + "|");
    QByteArray serialCommand = command.last().toLatin1();
    serialPort->write(serialCommand);
}

void MainWindow::serialRead()
{
    serialData.append(serialPort->readAll());
//    ui->textEdit_2->append("Reading data: |" + serialData + "|");

//    if (!comTimer.isActive())
      comTimer.start(TIMEOUT);
}

void MainWindow::serialTimeout()
{
    if (serialData.isEmpty())
        ui->textEdit_2->append("No data currently available!");
    else {
        ui->textEdit_2->append( serialPort->objectName() + ": " + serialData);
        serialData.clear();
    }
    comTimer.stop();
}

void MainWindow::errorMessage()
{

}

void MainWindow::clearCheckedMenu(QMenu *menu)
{
    QList<QAction *> list = menu->actions();
//    ui->textEdit_2->append(menu->activeAction()->objectName());
    for (int i = 0; i < list.size(); i++) {
//        ui->textEdit_2->append(list[i]->objectName());
        list[i]->setChecked(false);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    Quitdialog qDialog;
    qDialog.setModal(true);
    qDialog.exec();
    if (qDialog.result() == 1) {
        MainWindow::on_actionStop_triggered();
        QApplication::quit();
    }
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->textEdit->append("Communication ports listing!");
    for(int i = 0; i < actions.size(); i++) {
        ui->menuPort_Com->removeAction(actions[i]);
    }
    actions.clear();
    QList<QSerialPortInfo> port_com = QSerialPortInfo::availablePorts();

    for(int i = 0; i < port_com.size(); i++) {
        ui->textEdit->append("-" + port_com[i].portName());
        if (port_com[i].portName() == "COM1")
            ui->actionCOM_1->setEnabled(true);
        else if (port_com[i].portName() == "COM2")
            ui->actionCOM_2->setEnabled(true);
        else if (port_com[i].portName() == "COM3")
            ui->actionCOM_3->setEnabled(true);
        else if (port_com[i].portName() == "COM4")
            ui->actionCOM_4->setEnabled(true);
        else if (port_com[i].portName() == "COM5")
            ui->actionCOM_5->setEnabled(true);
        else if (port_com[i].portName() == "COM6")
            ui->actionCOM_6->setEnabled(true);
        else if (port_com[i].portName() == "COM7")
            ui->actionCOM_7->setEnabled(true);
        else if (port_com[i].portName() == "COM8")
            ui->actionCOM_8->setEnabled(true);
        else if (port_com[i].portName() == "COM9")
            ui->actionCOM_9->setEnabled(true);
        else if (port_com[i].portName() == "COM10")
            ui->actionCOM_10->setEnabled(true);
        else if (port_com[i].portName() == "COM11")
            ui->actionCOM_11->setEnabled(true);
        else if (port_com[i].portName() == "COM12")
            ui->actionCOM_12->setEnabled(true);
        else if (port_com[i].portName() == "COM13")
            ui->actionCOM_13->setEnabled(true);
        else if (port_com[i].portName() == "COM14")
            ui->actionCOM_14->setEnabled(true);
        else if (port_com[i].portName() == "COM15")
            ui->actionCOM_15->setEnabled(true);
        else if (port_com[i].portName() == "COM16")
            ui->actionCOM_16->setEnabled(true);
        else
            ui->textEdit->append("No communication port detected!");
    }

/*    QAction *action = new QAction("NO PORT");
    action->setObjectName("NoPort");
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(errorMessage()));
    action->setChecked(true);
    actions.append(action);
    if (port_com.size() == 0) {
        ui->textEdit->append("No communication port detected!");
        ui->menuPort_Com->addActions(actions);
    } else {
        ui->textEdit->append("Ports detected!");
        for(int i = 0; i < port_com.size(); i++) {
            ui->textEdit->append(port_com[i].portName());
            QAction *action = new QAction(port_com[i].portName());
            action->setObjectName(port_com[i].portName());
            action->setCheckable(true);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(errorMessage()));
            actions.append(action);
        }
        ui->menuPort_Com->addActions(actions);
    }*/
}

void MainWindow::on_action1200_triggered()
{
    ui->menuBaudrate->setActiveAction(ui->action1200);
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud1200;
    ui->action1200->setChecked(true);
}

void MainWindow::on_action2400_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud2400;
    ui->action2400->setChecked(true);
}

void MainWindow::on_action4800_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud4800;
    ui->action4800->setChecked(true);
}

void MainWindow::on_action9600_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud9600;
    ui->action9600->setChecked(true);
}

void MainWindow::on_action19200_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud19200;
    ui->action19200->setChecked(true);
}

void MainWindow::on_action38400_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud38400;
    ui->action38400->setChecked(true);
}

void MainWindow::on_action57600_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud57600;
    ui->action57600->setChecked(true);
}

void MainWindow::on_action115200_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud115200;
    ui->action115200->setChecked(true);
}

void MainWindow::on_action5_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data5;
    ui->action5->setChecked(true);
}

void MainWindow::on_action6_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data6;
    ui->action6->setChecked(true);
}

void MainWindow::on_action7_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data7;
    ui->action7->setChecked(true);
}

void MainWindow::on_action8_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data8;
    ui->action8->setChecked(true);
}

void MainWindow::on_actionnone_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::NoParity;
    ui->actionnone->setChecked(true);
}

void MainWindow::on_actioneven_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::EvenParity;
    ui->actioneven->setChecked(true);
}

void MainWindow::on_actionodd_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::OddParity;
    ui->actionodd->setChecked(true);
}

void MainWindow::on_actionspace_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::SpaceParity;
    ui->actionspace->setChecked(true);
}

void MainWindow::on_actionmark_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::MarkParity;
    ui->actionmark->setChecked(true);
}

void MainWindow::on_action1_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::OneStop;
    ui->action1_bits->setChecked(true);
}

void MainWindow::on_action1_5_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::OneAndHalfStop;
    ui->action1_5_bits->setChecked(true);
}

void MainWindow::on_action2_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::TwoStop;
    ui->action2_bits->setChecked(true);
}

void MainWindow::on_actionConfiguration_triggered()
{
    ui->textEdit_2->append("Actual Configuration --------------------------");
    ui->textEdit_2->append("-Port Name:\t " + QString(comPortName));
    ui->textEdit_2->append("-BaudeRate:\t " + QString::number(comBaudRate));
    ui->textEdit_2->append("-DataBits:\t " + QString::number(comDataBits));
    ui->textEdit_2->append("-Parity:\t " + QString::number(comParity));
    ui->textEdit_2->append("-StopBit:\t " + QString::number(comStopBit));
}

void MainWindow::on_actionCOM_1_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM1";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_1->setChecked(true);
}

void MainWindow::on_actionCOM_2_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM2";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_2->setChecked(true);
}

void MainWindow::on_actionCOM_3_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM3";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_3->setChecked(true);
}

void MainWindow::on_actionCOM_4_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM4";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_4->setChecked(true);
}

void MainWindow::on_actionCOM_5_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM5";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_5->setChecked(true);
}

void MainWindow::on_actionCOM_6_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM6";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_6->setChecked(true);
}

void MainWindow::on_actionCOM_7_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM7";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_7->setChecked(true);
}

void MainWindow::on_actionCOM_8_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM8";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_8->setChecked(true);
}

void MainWindow::on_actionCOM_9_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM9";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_9->setChecked(true);
}

void MainWindow::on_actionCOM_10_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM10";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_10->setChecked(true);
}

void MainWindow::on_actionCOM_11_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM11";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_11->setChecked(true);
}

void MainWindow::on_actionCOM_12_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM12";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_12->setChecked(true);
}

void MainWindow::on_actionCOM_13_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM13";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_13->setChecked(true);
}

void MainWindow::on_actionCOM_14_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM14";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_14->setChecked(true);
}

void MainWindow::on_actionCOM_15_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM15";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_15->setChecked(true);
}

void MainWindow::on_actionCOM_16_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM16";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(true);
    ui->actionCOM_16->setChecked(true);
}

void MainWindow::on_actionNo_Port_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "None";
    comPort = QSerialPortInfo(comPortName);
    ui->actionTest->setEnabled(false);
    ui->actionNo_Port->setChecked(true);
}

void MainWindow::on_actionTest_triggered()
{
    if (comPortName == "None")
        ui->textEdit->append("No port specified!");
    else {
        serialPort = new QSerialPort(comPort);
        serialPort->setObjectName(comPortName);
        serialPort->open(QIODevice::ReadWrite);

        ui->textEdit->append("Serial port " + serialPort->objectName() + " opened!");
        serialPort->waitForReadyRead();
        serialData = serialPort->readAll();
        ui->textEdit_2->append("Serial port " + serialPort->objectName() + " data: |" + serialData + "|");

        serialPort->close();
    }
}


void MainWindow::on_actionStart_triggered()
{
    if (comPortName == "None")
        ui->textEdit->append("No port specified!");
    else {
        serialPort = new QSerialPort(comPort);
        serialPort->setObjectName(comPortName);
        serialPort->open(QIODevice::ReadWrite);
        connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::serialRead);

        ui->textEdit->append("Port " + serialPort->portName() + " Opened!");
        ui->textEdit_3->setEnabled(true);

    }
}

void MainWindow::on_actionStop_triggered()
{
    if (serialPort->isOpen() == true) {
        serialPort->close();
        ui->textEdit->append("Port " + serialPort->portName() + " Closed!");
    }
    ui->textEdit_3->setEnabled(false);
}

void MainWindow::on_actionSensor_triggered()
{
    sensorWindow = new SensorWindow();
    sensorWindow->show();
}
