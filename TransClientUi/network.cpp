#include "network.h"
#include "ui_network.h"

QTcpSocket *network::_socket = new QTcpSocket();//创建套接字
std::string network::_str = "";
QByteArray network::_buffer = static_cast<char *>(malloc(4096));
QString network::_host_ip = "127.0.0.1";
quint16 network::_host_port = 1316;
QString network::_userName = ""; // 用户名
// 支持的证券
QStringList network::supportSec = {"00000000", "00000000", "00000000", "00000000",
                                   "00000000", "00000000", "00000000", "00000000",
                                   "00000000", "00000000", "00000000", "00000000",
                                   "00000000", "00000000", "00000000", "00000000"};

network::network(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::network)
{
    ui->setupUi(this);

    QRegExp regExpIp;
    QRegExp regExpPort;
    regExpIp.setPattern("^(?:(?:1[0-9][0-9]\\.)|(?:2[0-4][0-9]\\.)|(?:25[0-5]\\.)|"
                        "(?:[1-9][0-9]\\.)|(?:[0-9]\\.)){3}(?:(?:1[0-9][0-9])|"
                        "(?:2[0-4][0-9])|(?:25[0-5])|(?:[1-9][0-9])|(?:[0-9]))$");
    regExpPort.setPattern("^([1-9][0-9]{0,3}|[1-5][0-9]{0,4}|[1-6][0-4][0-9]{0,3}|"
          "[1-6][0-5][0-4][0-9]{0,2}|[1-6][0-5][0-5][0-2][0-9]{0,1}|[1-6][0-5][0-5][0-3][0-5])$|(^\\t?$)");
    QValidator * editPrice = new QRegExpValidator(regExpIp,ui->hostIp);
    QValidator * editQuantity = new QRegExpValidator(regExpPort,ui->port);
    ui->hostIp->setValidator(editPrice);
    ui->port->setValidator(editQuantity);

    connect(_socket,SIGNAL(connected()),this,SLOT(hasConnected()));//判断如果连接上

    connect(ui->hostIp, &QLineEdit::textChanged, this, &network::_hostIp);
    connect(ui->port, &QLineEdit::textChanged, this, &network::_hostPort);
    connect(ui->userName, &QLineEdit::textChanged, this, &network::_set_userName);
    connect(ui->verifyButton, &QPushButton::clicked, this, &network::_verifyButtonEv);

    setWindowModality(Qt::ApplicationModal);
}

network::~network()
{
    delete _socket;
    delete ui;
}

void network::_hostIp() {
    if(ui->hostIp->text().length() > 0) {
        _host_ip = ui->hostIp->text();
        if(ui->port->text().length() > 0 && ui->userName->text().length() > 0) {
            ui->verifyButton->setEnabled(true);
        }
    }
    else {
        ui->verifyButton->setDisabled(true);
    }

}

void network::_hostPort() {
    if(ui->port->text().length() > 0) {
        _host_port = ui->port->text().toUShort();
        if(ui->hostIp->text().length() > 0 && ui->userName->text().length() > 0) {
            ui->verifyButton->setEnabled(true);
        }
    }
    else {
        ui->verifyButton->setDisabled(true);
    }
}

void network::_set_userName() {
    if(ui->userName->text().length() > 0) {
        _userName = ui->userName->text();
        if(ui->hostIp->text().length() > 0 && ui->port->text().length() > 0) {
            ui->verifyButton->setEnabled(true);
        }
    }
    else {
        ui->verifyButton->setDisabled(true);
    }
}

void network::_verifyButtonEv() {
    qDebug() << "IP:" << _host_ip;
    qDebug() << "port:" << _host_port;
    qDebug() << "userName:" << _userName;
    _socket->connectToHost(_host_ip,_host_port);//连接服务器

}

void network::hasConnected() {
    qDebug() << "has connected";
}

bool network::Connect() {
    ui->hostIp->setText("127.0.0.1");
    ui->port->setText("1316");
    ui->userName->setText("lr");
//    ui->userName->clear();

    ui->verifyButton->setDisabled(true);

    if(ui->hostIp->text().length() > 0 && ui->port->text().length() > 0 && ui->userName->text().length() > 0) {
        ui->verifyButton->setEnabled(true);
    }

    return true;
}

bool network::disConnect() {
    _socket->disconnectFromHost();
    return true;
}

void network::_send()
{
    _socket->write(_str.c_str(), static_cast<qint64>(_str.length()));
    _socket->flush();
    while(_socket->bytesAvailable() < 1){
        _socket->waitForReadyRead(10);
    }

    _str.clear();
}

void network::_receive()
{
    _buffer.clear();
//    _socket->waitForReadyRead();
    _buffer = _socket->readAll();
}
