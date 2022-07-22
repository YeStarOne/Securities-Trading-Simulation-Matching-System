#include "revwidget.h"
#include "ui_revwidget.h"

RevWidget::RevWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RevWidget)
{
    ui->setupUi(this);

    connect(ui->verifyButton, &QPushButton::clicked, this, &RevWidget::verifyButtonEv);

    connect(ui->continueRev, &QPushButton::clicked, this, &RevWidget::_revoke);

    setWindowModality(Qt::ApplicationModal);
}

RevWidget::~RevWidget()
{
    for(auto& SecButton : _Vec_Button) {
        delete SecButton;
        delete _OrderLableMap[SecButton->text()];
        delete _QuanEditMap[SecButton->text()];
    }
    delete ui;
}

void RevWidget::_createOrderButton() {
    QString str(network::_buffer);
    QStringList orderList = str.split(";");
    orderList.pop_back();

    QRegExp regExpQuantity;
    // 匹配正整数
    regExpQuantity.setPattern("^[1-9]\\d*$|(^\\t?$)");

    qint16 i = 0;
    for(auto& order : orderList) {
        qint16 row = 50 + i++ * 30;

        _Vec_Button.push_back(new QLabel(this));
        _Vec_Button.back()->setFixedSize(600, 25);
        _Vec_Button.back()->setText(order);
        _Vec_Button.back()->move(200, row);
        _Vec_Button.back()->show();

        _OrderLableMap[order] = new QLabel(this);
        _OrderLableMap[order]->setFixedSize(80, 25);
        _OrderLableMap[order]->setText("撤销数量");
        _OrderLableMap[order]->move(10, row);
        _OrderLableMap[order]->show();

        _QuanEditMap[order] = new QLineEdit(this);
        _QuanEditMap[order]->setFixedSize(100, 25);
        _QuanEditMap[order]->move(85, row);
        _QuanEditMap[order]->show();

        QValidator* editQuantity = new QRegExpValidator(regExpQuantity, _QuanEditMap[order]);
        _QuanEditMap[order]->setValidator(editQuantity);

    }

}

void RevWidget::_clearOrderButton() {
    if(_Vec_Button.size() > 0) {
        for(auto& SecButton : _Vec_Button) {
            delete _OrderLableMap[SecButton->text()];
            delete _QuanEditMap[SecButton->text()];
            delete SecButton;
        }

        _OrderLableMap.clear();
        _QuanEditMap.clear();
        _Vec_Button.clear();
    }
}

void RevWidget::verifyButtonEv() {
    ui->verifyButton->setDisabled(true);
    int i = 0;
    network::_str = network::_userName.toStdString() + "\r\nrevoke\r\n";
    for(auto& orderLabel : _Vec_Button) {
        if(_QuanEditMap[orderLabel->text()]->text().length() > 0) {
            network::_str += std::to_string(i) + ',' + _QuanEditMap[orderLabel->text()]->text().toStdString() + ';';
        }
        _QuanEditMap[orderLabel->text()]->setDisabled(true);
        ++i;
    }
    network::_str += "\r\n";

    network::_send();

    network::_receive();

    qDebug() << network::_buffer;

    QString text = "日期:" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                    '\n' + network::_buffer + '\n';

    QMessageBox::about(nullptr, "撤销结果", text);

    ui->continueRev->setEnabled(true);

}

bool RevWidget::_revoke() {
    ui->continueRev->setDisabled(true);
    _clearOrderButton();

    network::_str = network::_userName.toStdString() + "\r\nrevoke\r\nstart\r\n";
    network::_send();

    network::_receive();

    if(network::_buffer[0] == '#') {
        QMessageBox::about(nullptr, "撤销结果", "没有可撤销的订单!!!");
        close();
        return false;
    }
    else {
        ui->label->setText("还未成交的可撤销订单有:\n");
        _createOrderButton();
        ui->verifyButton->setEnabled(true);
    }

    return true;
}
