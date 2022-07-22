#include "widget.h"
#include "ui_widget.h"

using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置槽函数
    connect(network::_socket, SIGNAL(hostFound()), this, SLOT(hasFindHost()));//先查看host是否存在
    connect(network::_socket, SIGNAL(connected()), this, SLOT(hasConnected()));//判断如果连接上
    connect(network::_socket, SIGNAL(disconnected()), this, SLOT(disConnected()));//如果断开连接
    connect(network::_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(netError()));//显示错误


    connect( ui->netButton , &QPushButton::clicked , this , &Widget::delNetConnect);
    connect( ui->buyButton , &QPushButton::clicked , this , &Widget::delBuy);
    connect( ui->sellButton , &QPushButton::clicked , this , &Widget::delSell);
    connect( ui->revButton , &QPushButton::clicked , this , &Widget::delRevoke);
    connect( ui->subPubButton , &QPushButton::clicked , this , &Widget::delUserSubPub);

    connect( ui->userNameChangeBtn , &QPushButton::clicked , this , &Widget::_userNameChange);
    connect( ui->verifyUseNameBtn , &QPushButton::clicked , this , &Widget::_userNameChangeVerify);

    connect( ui->closeBtn , &QPushButton::clicked , this , &Widget::_closeWidget);

    ui->buyButton->setDisabled(true);
    ui->sellButton->setDisabled(true);
    ui->revButton->setDisabled(true);
    ui->subPubButton->setDisabled(true);

    ui->userNameChangeBtn->setDisabled(true);
    ui->userNameEdit->setDisabled(true);
    ui->verifyUseNameBtn->setDisabled(true);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::hasFindHost() {
    qDebug()<<"found host!!!";
}

void Widget::_setPushBtnEnable() {
    ui->buyButton->setEnabled(true);
    ui->sellButton->setEnabled(true);
    ui->revButton->setEnabled(true);
    ui->subPubButton->setEnabled(true);
    ui->netButton->setEnabled(true);
    ui->userNameChangeBtn->setEnabled(true);

}

void Widget::_setPushBtnDisable() {
    ui->buyButton->setDisabled(true);
    ui->sellButton->setDisabled(true);
    ui->revButton->setDisabled(true);
    ui->subPubButton->setDisabled(true);
    ui->netButton->setDisabled(true);
    ui->userNameChangeBtn->setDisabled(true);

}

void Widget::hasConnected() {
    qDebug()<<"Network has connected!!!";
    getSupportSec();
    _setPushBtnEnable();
    ui->netButton->setDisabled(true);

    _netWidget.close();

    ui->userNameEdit->setText(network::_userName);

    QMessageBox::about(nullptr, "网络连接", "连接成功\n服务端ip:" + network::_host_ip + "\n服务端端口号:" +
                       QString::number(network::_host_port) + "\n用户名:" + network::_userName);

}

void Widget::disConnected() {
    qDebug() << "Network disconnected!!!";
    _setPushBtnDisable();
    ui->netButton->setEnabled(true);

    QMessageBox::about(nullptr, "网络连接", "网络连接已断开,请尝试重新连接!");

}

void Widget::netError() {
    QString string = network::_socket->errorString();
    // 不报该错误
    if(string == "Network operation timed out") {
        return;
    }
    qDebug() << "Network error:" << string;

    QMessageBox::about(nullptr, "网络错误", string);
}

void Widget::getSupportSec() {
    network::_str = network::_userName.toStdString() + "\r\ngetSupportSec\r\n\r\n";
    network::_send();

    network::_receive();
    QString str(network::_buffer);
    network::supportSec = str.split(";");
    network::supportSec.pop_back();

    _tradeWidget._clearSecButton();
    _subWidget._clearSecButton();

    qint16 i = 0, j = 0;
    for(auto& supSec : network::supportSec) {
        qint16 row = 20 + i * 150;
        qint16 col = 50 + j * 30;
        _tradeWidget.createSecButton(supSec, row, col);
        _subWidget.createSecButton(supSec, row, col);
        if(++j % 4 == 0) {
            ++i;
            j = 0;
        }
    }
}

void Widget::delNetConnect() {
    _netWidget.setWindowTitle("网络连接");
    _netWidget.show();
    auto Qpoint = this->mapToGlobal(QPoint(0, 0));
    _netWidget.move(Qpoint.x() + (width() - _netWidget.width()) / 2,
                    Qpoint.y() + (height() - _netWidget.height()) /2);

    _netWidget.Connect();

}

bool Widget::_userNameChange() {
    _setPushBtnDisable();
    ui->verifyUseNameBtn->setEnabled(true);

    ui->userNameEdit->setText(network::_userName);
    ui->userNameEdit->setEnabled(true);

    return true;
}

bool Widget::_userNameChangeVerify() {
    if(ui->userNameEdit->text().length() > 0) {
        _setPushBtnEnable();
        ui->netButton->setDisabled(true);
        ui->userNameEdit->setDisabled(true);
        ui->verifyUseNameBtn->setDisabled(true);
        network::_userName = ui->userNameEdit->text();
        QMessageBox::about(nullptr, "提示", "用户更改为:" + network::_userName);
    }
    else {
        QMessageBox::about(nullptr, "错误", "用户名不能为空!!!");
    }

    return true;
}

void Widget::delBuy() {
    _tradeWidget.setWindowTitle("买入界面");
    _tradeWidget.show();
    auto Qpoint = this->mapToGlobal(QPoint(0, 0));
    _tradeWidget.move(Qpoint.x() + (width() - _tradeWidget.width()) / 2,
                    Qpoint.y() + (height() - _tradeWidget.height()) /2);

    _tradeWidget._trade("buy");

}

void Widget::delSell() {
    _tradeWidget.setWindowTitle("卖出界面");
    _tradeWidget.show();
    auto Qpoint = this->mapToGlobal(QPoint(0, 0));
    _tradeWidget.move(Qpoint.x() + (width() - _tradeWidget.width()) / 2,
                    Qpoint.y() + (height() - _tradeWidget.height()) /2);

    _tradeWidget._trade("sell");

}
void Widget::delRevoke() {
    _revWidget.setWindowTitle("撤销界面");
    _revWidget.show();
    auto Qpoint = this->mapToGlobal(QPoint(0, 0));
    _revWidget.move(Qpoint.x() + (width() - _revWidget.width()) / 2,
                    Qpoint.y() + (height() - _revWidget.height()) /2);

    _revWidget._revoke();

}
void Widget::delUserSubPub() {
    _subWidget.setWindowTitle("订阅发布界面");
    _subWidget.show();
    auto Qpoint = this->mapToGlobal(QPoint(0, 0));
    _subWidget.move(Qpoint.x() + (width() - _subWidget.width()) / 2,
                    Qpoint.y() + (height() - _subWidget.height()) /2);

}

void Widget::_closeWidget() {
    disconnect(network::_socket, SIGNAL(disconnected()), this, SLOT(disConnected()));
    _netWidget.disConnect();
    close();
}
