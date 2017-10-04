#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quitdialog.h"

#include "iostream"
#include "unistd.h"
#include "string"

#define COMM_TIMEOUT 2000
#define SERIAL_TIMEOUT 20

#define MIN_CMD_ARG 1
#define MAX_CMD_ARG 3

#define ADD_DISTANCE 10
#define ADD_ANGLE 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    monitorWindow = new MonitorWindow();
    sensorWindow = new SensorWindow();
    enterKey = new EnterKeyHandler(this);
    backspaceEater = new BackspaceEater(this);

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

    distance_ack = false;
    angle_ack = false;

    connect(enterKey, SIGNAL(enterKeyPressed()), this, SLOT(commandHandler()));
    connect(&comTimer, &QTimer::timeout, this, &MainWindow::serialTimeout);

    monitorWindow->setAddDistance(ADD_DISTANCE);
    monitorWindow->setAddDoubleDistance(10 * ADD_DISTANCE);
    monitorWindow->setAddAngle(ADD_ANGLE);
    monitorWindow->setAddDoubleAngle(10 * ADD_ANGLE);
    connect(monitorWindow, SIGNAL(runButton()), this, SLOT(commandRun()));
    connect(monitorWindow, SIGNAL(stopButton()), this, SLOT(commandStop()));
    connect(monitorWindow, SIGNAL(up()), this, SLOT(commandUp()));
    connect(monitorWindow, SIGNAL(home()), this, SLOT(commandHome()));
    connect(monitorWindow, SIGNAL(doubleUp()), this, SLOT(commandDoubleUp()));
    connect(monitorWindow, SIGNAL(right()), this, SLOT(commandRight()));
    connect(monitorWindow, SIGNAL(doubleRight()), this, SLOT(commandDoubleRight()));
    connect(monitorWindow, SIGNAL(down()), this, SLOT(commandDown()));
    connect(monitorWindow, SIGNAL(doubleDown()), this, SLOT(commandDoubleDown()));
    connect(monitorWindow, SIGNAL(left()), this, SLOT(commandLeft()));
    connect(monitorWindow, SIGNAL(doubleLeft()), this, SLOT(commandDoubleLeft()));

    ui->textEdit_2->installEventFilter(backspaceEater);
    ui->textEdit_2->installEventFilter(enterKey);
    ui->textEdit_2->append(">>>: ");
    ui->lineEdit->setReadOnly(true);
    ui->lineEdit_2->setReadOnly(true);
    ui->lineEdit_3->setReadOnly(true);
    ui->lineEdit_4->setReadOnly(true);
    ui->lineEdit_5->setReadOnly(true);
    ui->progressBar->setStyleSheet("QProgressBar::chunk{background-color:Red}" "QProgressBar{color:White}");
    ui->progressBar->setTextVisible(true);
    ui->progressBar->setAlignment(Qt::AlignCenter);
    ui->progressBar->setFormat("Disconnected");
    ui->pushButton->setText("Connect");

    MainWindow::on_actionRefresh_triggered();
    MainWindow::on_actionConfiguration_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::commandHandler()
{
    QString str = "\n";
    str.append(ui->textEdit_2->toPlainText());
    QStringList command = str.split("\n>>>: ");

    QString cmd = parserHandler(command.last());
    if (!cmd.isEmpty())
        commandSending(cmd);
    else
        ui->textEdit_2->append("UNKWNON COMMAND: " + command.last() + "\n>>>: ");
}

QString MainWindow::parserHandler(QString command)
{
    /****************************************************
     * |00|00000000000000| 16 caracters
    ****************************************************/
    QStringList parser = command.split(" ");
    QString verbose, cmd;

    QList<QString>::iterator it;
    for (it = parser.begin(); it != parser.end(); it++)
    {
        if (it->isEmpty())
            parser.erase(it);
        else
            verbose.append(*it + " ");
    }

    if ((parser.size() < MIN_CMD_ARG) || (parser.size() > MAX_CMD_ARG))
        return NULL;

    if (parser[0].toCaseFolded() == "ok")
        cmd = "OK0000000000000";
    else if (parser[0].toCaseFolded() == "error")
        cmd = "ER0000000000000";
    else if (parser[0].toCaseFolded() == "resend")
        cmd = "RS0000000000000";
    else if (parser[0].toCaseFolded() == "set")
        cmd = "SX0000000000000";
    else if (parser[0].toCaseFolded() == "get")
        cmd = "GX0000000000000";
    else if (parser[0].toCaseFolded() == "run")
        cmd = "RN0000000000000";
    else if (parser[0].toCaseFolded() == "stop")
        cmd = "ST0000000000000";
    else
        return NULL;

    return cmd;
}

void MainWindow::commandRun()
{
    run_cmd = true;
    if (!distance_ack)
        commandSending(parserHandler("set distance 1234"));
    else if (!angle_ack)
        commandSending(parserHandler("set angle 1234"));
    else
        commandSending(parserHandler("run"));
}

void MainWindow::commandStop()
{
    QString cmd = parserHandler("stop");
    commandSending(cmd);
}

void MainWindow::commandHome()
{
    commandSending("Home command!");
}

void MainWindow::commandUp()
{
    commandSending("Distance: " + QString::number(monitorWindow->getAddDistance()));
}

void MainWindow::commandDoubleUp()
{
    commandSending("Distance: " + QString::number(monitorWindow->getAddDoubleDistance()));
}

void MainWindow::commandRight()
{
    commandSending("Angle: " + QString::number(monitorWindow->getAddAngle()));
}

void MainWindow::commandDoubleRight()
{
    commandSending("Agnle: " + QString::number(monitorWindow->getAddDoubleAngle()));
}

void MainWindow::commandDown()
{
    commandSending("Distance: -" + QString::number(monitorWindow->getAddDistance()));
}

void MainWindow::commandDoubleDown()
{
    commandSending("Distance: -" + QString::number(monitorWindow->getAddDoubleDistance()));
}

void MainWindow::commandLeft()
{
    commandSending("Angle: -" + QString::number(monitorWindow->getAddAngle()));
}

void MainWindow::commandDoubleLeft()
{
    commandSending("Agnle: -" + QString::number(monitorWindow->getAddDoubleAngle()));
}

void MainWindow::commandSending(QString command)
{
    QByteArray serialCommand = command.toLatin1();
    ui->textEdit_2->append("HOST: " + command);
    serialPort->write(serialCommand);
    ui->textEdit_2->setReadOnly(true);
    comTimer.start(COMM_TIMEOUT);
}

void MainWindow::serialRead()
{
    serialData.append(serialPort->readAll());
    comTimer.start(SERIAL_TIMEOUT);
}

void MainWindow::serialTimeout()
{
    if (serialData.isEmpty())
        ui->textEdit_2->append("TIMEOUT: No data currently available!\n>>>: ");
    else {
        ui->textEdit_2->append( serialPort->objectName() + ": " + serialData + "\n>>>: ");
        serialData.clear();
    }
    ui->textEdit_2->setReadOnly(false);
    comTimer.stop();

    if (run_cmd) {
        if (!distance_ack) {
            distance_ack = true;
            emit monitorWindow->runButton();
        }
        else if (!angle_ack) {
            angle_ack = true;
            emit monitorWindow->runButton();
        }
        else {
            distance_ack = false;
            angle_ack = false;
            run_cmd = false;
        }
    }
}

void MainWindow::errorMessage()
{
    qDebug("Error: %d", serialPort->error());
    if (serialPort->error() == QSerialPort::ResourceError)
        on_actionStop_triggered();
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
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action2400_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud2400;
    ui->action2400->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action4800_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud4800;
    ui->action4800->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action9600_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud9600;
    ui->action9600->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action19200_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud19200;
    ui->action19200->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action38400_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud38400;
    ui->action38400->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action57600_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud57600;
    ui->action57600->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action115200_triggered()
{
    clearCheckedMenu(ui->menuBaudrate);
    comBaudRate = QSerialPort::Baud115200;
    ui->action115200->setChecked(true);
    ui->textEdit->append(QString(ui->menuBaudrate->title()) + " selected: " + QString::number(comBaudRate));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action5_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data5;
    ui->action5->setChecked(true);
    ui->textEdit->append(QString(ui->menuData_Bits->title()) + " selected: " + QString::number(comDataBits));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action6_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data6;
    ui->action6->setChecked(true);
    ui->textEdit->append(QString(ui->menuData_Bits->title()) + " selected: " + QString::number(comDataBits));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action7_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data7;
    ui->action7->setChecked(true);
    ui->textEdit->append(QString(ui->menuData_Bits->title()) + " selected: " + QString::number(comDataBits));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action8_triggered()
{
    clearCheckedMenu(ui->menuData_Bits);
    comDataBits = QSerialPort::Data8;
    ui->action8->setChecked(true);
    ui->textEdit->append(QString(ui->menuData_Bits->title()) + " selected: " + QString::number(comDataBits));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionnone_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::NoParity;
    ui->actionnone->setChecked(true);
    ui->textEdit->append(QString(ui->menuParity->title()) + " selected: " + QString::number(comParity));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actioneven_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::EvenParity;
    ui->actioneven->setChecked(true);
    ui->textEdit->append(QString(ui->menuParity->title()) + " selected: " + QString::number(comParity));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionodd_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::OddParity;
    ui->actionodd->setChecked(true);
    ui->textEdit->append(QString(ui->menuParity->title()) + " selected: " + QString::number(comParity));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionspace_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::SpaceParity;
    ui->actionspace->setChecked(true);
    ui->textEdit->append(QString(ui->menuParity->title()) + " selected: " + QString::number(comParity));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionmark_triggered()
{
    clearCheckedMenu(ui->menuParity);
    comParity = QSerialPort::MarkParity;
    ui->actionmark->setChecked(true);
    ui->textEdit->append(QString(ui->menuParity->title()) + " selected: " + QString::number(comParity));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action1_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::OneStop;
    ui->action1_bits->setChecked(true);
    ui->textEdit->append(QString(ui->menuStop_Bits->title()) + " selected: " + QString::number(comStopBit));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action1_5_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::OneAndHalfStop;
    ui->action1_5_bits->setChecked(true);
    ui->textEdit->append(QString(ui->menuStop_Bits->title()) + " selected: " + QString::number(comStopBit));
    on_actionConfiguration_triggered();
}

void MainWindow::on_action2_bits_triggered()
{
    clearCheckedMenu(ui->menuStop_Bits);
    comStopBit = QSerialPort::TwoStop;
    ui->action2_bits->setChecked(true);
    ui->textEdit->append(QString(ui->menuStop_Bits->title()) + " selected: " + QString::number(comStopBit));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionConfiguration_triggered()
{
    ui->lineEdit->clear();
    ui->lineEdit->setText(QString(comPortName));
    ui->lineEdit_2->clear();
    ui->lineEdit_2->setText(QString::number(comBaudRate));
    ui->lineEdit_3->clear();
    ui->lineEdit_3->setText(QString::number(comDataBits));
    ui->lineEdit_4->clear();
    ui->lineEdit_4->setText(QString::number(comParity));
    ui->lineEdit_5->clear();
    ui->lineEdit_5->setText(QString::number(comStopBit));
    ui->textEdit->append("Serial configuration is up to date");
}

void MainWindow::on_actionCOM_1_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM1";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_1->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_2_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM2";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_2->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_3_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM3";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_3->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_4_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM4";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_4->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_5_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM5";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_5->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_6_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM6";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_6->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_7_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM7";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_7->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_8_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM8";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_8->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_9_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM9";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_9->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_10_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM10";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_10->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_11_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM11";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_11->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_12_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM12";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_12->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_13_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM13";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_13->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_14_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM14";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_14->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_15_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM15";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_15->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionCOM_16_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "COM16";
    comPort = QSerialPortInfo(comPortName);
    ui->actionCOM_16->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
}

void MainWindow::on_actionNo_Port_triggered()
{
    clearCheckedMenu(ui->menuPort_Com);
    comPortName = "None";
    comPort = QSerialPortInfo(comPortName);
    ui->actionNo_Port->setChecked(true);
    ui->textEdit->append(QString(ui->menuPort_Com->title()) + " selected: " + QString(comPortName));
    on_actionConfiguration_triggered();
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
        connect(serialPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::errorMessage);

        ui->textEdit->append("Port " + serialPort->portName() + " Opened!");
        ui->textEdit_2->setEnabled(true);
        ui->textEdit_2->setFocus();
        ui->progressBar->setStyleSheet("QProgressBar::chunk{background-color:Green}" "QProgressBar{color:White}");
        ui->progressBar->setFormat("Connected");
        ui->pushButton->setText("Disconnect");
        ui->actionMonitor->setEnabled(true);
        ui->actionSensor->setEnabled(true);
    }
}

void MainWindow::on_actionStop_triggered()
{
    if (serialPort->isOpen() == true) {
        serialPort->close();
        ui->textEdit->append("Port " + serialPort->portName() + " Closed!");
    }

    ui->textEdit_2->setEnabled(false);
    ui->progressBar->setStyleSheet("QProgressBar::chunk{background-color:Red}" "QProgressBar{color:White}");
    ui->progressBar->setFormat("Disconnected");
    ui->pushButton->setText("Connect");
    ui->actionMonitor->setEnabled(false);
    ui->actionSensor->setEnabled(false);

    if(monitorWindow)
        monitorWindow->close();
    if (sensorWindow)
        sensorWindow->close();
}

void MainWindow::on_actionMonitor_triggered()
{
    monitorWindow->show();
}

void MainWindow::on_actionSensor_triggered()
{
    sensorWindow->show();
}

void MainWindow::on_pushButton_clicked()
{
    if (serialPort->isOpen())
        on_actionStop_triggered();
    else
        on_actionStart_triggered();
}
