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
#define MAX_DATA 16777216

#define ADD_DISTANCE 10
#define ADD_ANGLE 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Icon/Icon/Evo.png"));

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
    run_cmd = false;
    sending = false;

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

    QString cmd = parserSendHandler(command.last());
    if (!cmd.isEmpty())
        commandSending(cmd);
    else
        ui->textEdit_2->append("UNKWNON COMMAND: " + command.last() + "\n>>>: ");
}

QString MainWindow::parserSendHandler(QString command)
{
    /****************************************************
     * | XX | XX XX XX | 8 Hexa caracters
     * |CMD type| Data | 8 Hexa caracters
     *
     * CMD type : 1 HEX
     * 0000: -      ->      |       |       |
     * 0001: CMD    -> SET  | GET   | RUN   | STOP
     * 0010: INFO   -> CAPT | DIST  | ANGLE | RUNNING
     * 0011: ACK    -> OK   | END   | ERROR | RESEND
     * 0100: +1     ->      |       |       |
     * 0101: +1     ->      |       |       |
     * 0110: +1     ->      |       |       |
     * 0111: +1     ->      |       |       |
     * 1000: +1     ->      |       |       |
     * 1001: +1     ->      |       |       |
     * 1010: +1     ->      |       |       |
     * 1011: +1     ->      |       |       |
     * 1100: +1     ->      |       |       |
     * 1101: +1     ->      |       |       |
     * 1110: +1     ->      |       |       |
     * 1111: QUIT   ->      |       |       |
     * -------------------------------------------------
     * CMD type : 2 HEX (1 HEX = CMD)
     * 00|XX: STOP
     * 01|01: SET Angle
     * 01|10: SET Distance
     * 10|01: GET Angle
     * 10|10: GET Distance
     * 11|00: RUN ALL
     * 11|01: RUN ANGLE
     * 11|10: RUN DISTANCE
     * 11|11: RUN -
     *
     * CMD type : 2 HEX (1 HEX = INFO)
     * 00|XX: RUNNING
     * 01|01: POS 1: ANGLE
     * 01|10: POS 1: X
     * 10|01: POS 2: Distance
     * 10|10: POS 2: Y
     * 11|00: CAPT ToR
     * 11|01: CAPT GP2
     * 11|10: CAPT -
     * 11|11: CAPT Color
     *
     * CMD type : 2 HEX (1 HEX = ACK)
     * 00|00: ERROR unknow
     * 00|01: ERROR cmd type
     * 00|10: ERROR data
     * 00|11: ERROR -
     * 01|XX: RESEND
     * 10|XX: OK    -> | XX : CMD type 2 HEX (1 HEX = CMD)
     * 11|XX: END   -> | XX : CMD type 2 HEX (1 HEX = CMD)
     *
    ****************************************************/
    sending = true;
    monitorWindow->cmd_acquisition(false);

    QStringList parser = command.split(" ");
    QString verbose, cmd;
    int cmdHEX = 0, dataHEX = 0;

    QList<QString>::iterator it;
    for (it = parser.begin(); it != parser.end(); it++) {
        if (it->isEmpty())
            parser.erase(it);
        else
            verbose.append(*it + " ");
    }

    if ((parser.size() < MIN_CMD_ARG) || (parser.size() > MAX_CMD_ARG))
        return NULL;

    // CMD Type: CMD
    if (parser[0].toCaseFolded() == "set") {
        if (parser.size() == 3) {
            cmdHEX |= (1 << 4); // CMD TYPE
            cmdHEX |= (1 << 2); // CMD SET
            if (parser[1].toCaseFolded() == "angle")
                cmdHEX |= (1 << 0); // SET type ANGLE
            else if (parser[1].toCaseFolded() == "distance")
                cmdHEX |= (1 << 1); // SET type DISTANCE
            else
                return NULL;
            bool ok;
            parser[2].toInt(&ok);
            if ((ok) && (parser[2].toInt() <= MAX_DATA)) {
                dataHEX = parser[2].toInt();
            } else
                return NULL;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "get") {
        if (parser.size() == 2) {
            cmdHEX |= (1 << 4); // CMD TYPE
            cmdHEX |= (1 << 3); // CMD GET
            if (parser[1].toCaseFolded() == "angle")
                cmdHEX |= (1 << 0); // GET type ANGLE
            else if (parser[1].toCaseFolded() == "distance")
                cmdHEX |= (1 << 1); // GET type DISTANCE
            else
                return NULL;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "run") {
        if ((parser.size() >= 1) && (parser.size() <= 3)) {
            cmdHEX |= (1 << 4); // CMD TYPE
            cmdHEX |= ((1 << 3) | (1 << 2)); // CMD RUN
            if (parser.size() >= 2) {
                if (parser[1].toCaseFolded() == "angle")
                    cmdHEX |= (1 << 0); // RUN type ANGLE
                else if (parser[1].toCaseFolded() == "distance")
                    cmdHEX |= (1 << 1); // RUN type DISTANCE
                else
                    return NULL;
                if (parser.size() == 3) {
                    bool ok;
                    parser[2].toInt(&ok);
                    if ((ok) && (parser[2].toInt() <= MAX_DATA)) {
                        dataHEX = parser[2].toInt();
                    } else
                        return NULL;
                } else
                    dataHEX = 0;
            } else
                dataHEX = 0;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "stop") {
        if (parser.size() == 1)
            cmdHEX |= (1 << 4); // CMD TYPE
        else
            return NULL;
    // CMD Type: ACK
    } else if (parser[0].toCaseFolded() == "error") {
        if (parser.size() == 2) {
            cmdHEX |= ((1 << 5) | (1 << 4)); // ACK TYPE
            cmdHEX |= (1 << 3); // ERROR
            if (parser[1].toCaseFolded() == "unknow")
                cmdHEX |= (0 << 0); // ERROR type UNKNOW
            else if (parser[1].toCaseFolded() == "command")
                cmdHEX |= (1 << 0); // ERROR type CMD
            else if (parser[1].toCaseFolded() == "data")
                cmdHEX |= (1 << 1); // ERROR type DATA
            else
                return NULL;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "resend") {
        if (parser.size() == 1) {
            cmdHEX |= ((1 << 5) | (1 << 4)); // ACK TYPE
            cmdHEX |= (1 << 1); // RESEND
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "ok") {
        if (parser.size() == 2) {
            cmdHEX |= ((1 << 5) | (1 << 4)); // ACK TYPE
            cmdHEX |= (1 << 3); // OK
            if (parser[1].toCaseFolded() == "stop")
                cmdHEX |= (0 << 0); // OK type STOP
            else if (parser[1].toCaseFolded() == "set")
                cmdHEX |= (1 << 0); // OK type SET
            else if (parser[1].toCaseFolded() == "get")
                cmdHEX |= (1 << 1); // OK type GET
            else if (parser[1].toCaseFolded() == "run")
                cmdHEX |= ((1 << 1) | (1 << 0)); // OK type RUN
            else
                return NULL;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "end") {
        if (parser.size() == 2) {
            cmdHEX |= ((1 << 5) | (1 << 4)); // ACK TYPE
            cmdHEX |= ((1 << 3) | (1 << 2)); // ENDED
            if (parser[1].toCaseFolded() == "stop")
                cmdHEX |= (0 << 0); // OK type STOP
            else if (parser[1].toCaseFolded() == "set")
                cmdHEX |= (1 << 0); // OK type SET
            else if (parser[1].toCaseFolded() == "get")
                cmdHEX |= (1 << 1); // OK type GET
            else if (parser[1].toCaseFolded() == "run")
                cmdHEX |= ((1 << 1) | (1 << 0)); // OK type RUN
            else
                return NULL;
        } else
            return NULL;
    } else if (parser[0].toCaseFolded() == "quit") {
        if (parser.size() == 1) {
            cmdHEX = 0xFF; // QUIT
            dataHEX = 0x000000;
        } else
            return NULL;
    } else
        return NULL;

    char str[8];
    itoa((cmdHEX << 24) | (dataHEX), str, 16);
    cmd.append(str);

    return cmd;
}

void MainWindow::commandRun()
{
    run_cmd = true;
    if (!distance_ack)
        commandSending(parserSendHandler("set distance " + QString::number(monitorWindow->getConsDistance())));
    else if (!angle_ack)
        commandSending(parserSendHandler("set angle " + QString::number(monitorWindow->getConsAngle())));
    else
        commandSending(parserSendHandler("run"));
}

void MainWindow::commandStop()
{
    QString cmd = parserSendHandler("stop");
    commandSending(cmd);
}

void MainWindow::commandHome()
{
    commandSending("Home command!");
}

void MainWindow::commandUp()
{
    QString cmd = parserSendHandler("run distance " + QString::number(monitorWindow->getAddDistance()));
    commandSending(cmd);
}

void MainWindow::commandDoubleUp()
{
    QString cmd = parserSendHandler("run distance " + QString::number(monitorWindow->getAddDoubleDistance()));
    commandSending(cmd);
}

void MainWindow::commandRight()
{
    QString cmd = parserSendHandler("run angle " + QString::number(monitorWindow->getAddAngle()));
    commandSending(cmd);
}

void MainWindow::commandDoubleRight()
{
    QString cmd = parserSendHandler("run angle " + QString::number(monitorWindow->getAddDoubleAngle()));
    commandSending(cmd);
}

void MainWindow::commandDown()
{
    QString cmd = parserSendHandler("run distance -" + QString::number(monitorWindow->getAddDistance()));
    commandSending(cmd);
}

void MainWindow::commandDoubleDown()
{
    QString cmd = parserSendHandler("run distance -" + QString::number(monitorWindow->getAddDoubleDistance()));
    commandSending(cmd);
}

void MainWindow::commandLeft()
{
    QString cmd = parserSendHandler("run angle -" + QString::number(monitorWindow->getAddAngle()));
    commandSending(cmd);
}

void MainWindow::commandDoubleLeft()
{
    QString cmd = parserSendHandler("run angle -" + QString::number(monitorWindow->getAddDoubleAngle()));
    commandSending(cmd);
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
            sending = false;
            monitorWindow->cmd_acquisition(true);
        }
    } else {
        sending = false;
        monitorWindow->cmd_acquisition(true);
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
        if (port_com[i].portName() == "COM1"){
            if (i == 0)
                MainWindow::on_actionCOM_1_triggered();
            ui->actionCOM_1->setEnabled(true);
        }else if (port_com[i].portName() == "COM2"){
            if (i == 0)
                MainWindow::on_actionCOM_2_triggered();
            ui->actionCOM_2->setEnabled(true);
        }else if (port_com[i].portName() == "COM3"){
                if (i == 0)
                    MainWindow::on_actionCOM_3_triggered();
            ui->actionCOM_3->setEnabled(true);
        }else if (port_com[i].portName() == "COM4"){
                if (i == 0)
                    MainWindow::on_actionCOM_4_triggered();
            ui->actionCOM_4->setEnabled(true);
        }else if (port_com[i].portName() == "COM5"){
                if (i == 0)
                    MainWindow::on_actionCOM_5_triggered();
            ui->actionCOM_5->setEnabled(true);
        }else if (port_com[i].portName() == "COM6"){
                if (i == 0)
                    MainWindow::on_actionCOM_6_triggered();
            ui->actionCOM_6->setEnabled(true);
        }else if (port_com[i].portName() == "COM7"){
                if (i == 0)
                    MainWindow::on_actionCOM_7_triggered();
            ui->actionCOM_7->setEnabled(true);
        }else if (port_com[i].portName() == "COM8"){
                if (i == 0)
                    MainWindow::on_actionCOM_8_triggered();
            ui->actionCOM_8->setEnabled(true);
        }else if (port_com[i].portName() == "COM9"){
                if (i == 0)
                    MainWindow::on_actionCOM_9_triggered();
            ui->actionCOM_9->setEnabled(true);
        }else if (port_com[i].portName() == "COM10"){
                if (i == 0)
                    MainWindow::on_actionCOM_10_triggered();
            ui->actionCOM_10->setEnabled(true);
        }else if (port_com[i].portName() == "COM11"){
                if (i == 0)
                    MainWindow::on_actionCOM_11_triggered();
            ui->actionCOM_11->setEnabled(true);
        }else if (port_com[i].portName() == "COM12"){
                if (i == 0)
                    MainWindow::on_actionCOM_12_triggered();
            ui->actionCOM_12->setEnabled(true);
        }else if (port_com[i].portName() == "COM13"){
                if (i == 0)
                    MainWindow::on_actionCOM_13_triggered();
            ui->actionCOM_13->setEnabled(true);
        }else if (port_com[i].portName() == "COM14"){
                if (i == 0)
                    MainWindow::on_actionCOM_14_triggered();
            ui->actionCOM_14->setEnabled(true);
        }else if (port_com[i].portName() == "COM15"){
                if (i == 0)
                    MainWindow::on_actionCOM_15_triggered();
            ui->actionCOM_15->setEnabled(true);
        }else if (port_com[i].portName() == "COM16"){
                if (i == 0)
                    MainWindow::on_actionCOM_16_triggered();
            ui->actionCOM_16->setEnabled(true);
        }else{
            ui->textEdit->append("No communication port detected!");
            MainWindow::on_actionConfiguration_triggered();
        }
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
        serialPort->setBaudRate(comBaudRate);
        serialPort->setDataBits(comDataBits);
        serialPort->setFlowControl(comFlowControl);
        serialPort->setParity(comParity);
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
