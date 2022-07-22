#include "subwidget.h"
#include "ui_subwidget.h"

SubWidget::SubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubWidget)
{
    ui->setupUi(this);

    connect(ui->verifyButton, &QPushButton::clicked, this, &SubWidget::verifyButtonEv);

    connect(ui->selectAllButton, &QPushButton::clicked, this, &SubWidget::selectAllButtonEv);

    setWindowModality(Qt::ApplicationModal);
}

SubWidget::~SubWidget()
{
    for(auto& SecButton : _Vec_Button) {
        delete SecButton;
    }
    delete ui;
}

void SubWidget::createSecButton(const QString secName, const qint16 row, const qint16 col) {
    _Vec_Button.push_back(new QCheckBox(this));
    _Vec_Button.back()->setFixedSize(160, 30);
    _Vec_Button.back()->setText(secName);
    _Vec_Button.back()->move(row, col);
    _Vec_Button.back()->show();
    connect(_Vec_Button.back(), &QCheckBox::clicked, this, &SubWidget::_selectSec);
}

void SubWidget::_clearSecButton() {
    _Vec_Button.clear();
}

void SubWidget::_selectSec() {

}

void SubWidget::selectAllButtonEv() {
    bool flag = false;
    for(auto& btn : _Vec_Button) {
        if(btn->checkState() == Qt::CheckState::Unchecked) {
            flag = true;
            break;
        }
    }
    if(flag) {
        for(auto& btn : _Vec_Button) {
            btn->setCheckState(Qt::CheckState::Checked);
        }
    }
    else {
        for(auto& btn : _Vec_Button) {
            btn->setCheckState(Qt::CheckState::Unchecked);
        }
    }

}

void SubWidget::verifyButtonEv() {
    bool flag = true;
    network::_str = network::_userName.toStdString() + "\r\nuserPublish\r\n";

    for(auto& btn : _Vec_Button) {
        if(btn->isChecked()) {
            network::_str += btn->text().toStdString() + ';';
            flag = false;
        }
    }

    if(flag) {
        QMessageBox::about(nullptr, "提示", "没有任何订阅证券!!!");
        return;
    }

    network::_str += "\r\n";

    network::_send();

    network::_receive();

    qDebug() << network::_buffer;

    QString text;
    if(network::_buffer[0] == '#') {
        text = "日期:" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                        '\n';
    }
    else {
        text = "日期:" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                        '\n' + network::_buffer + '\n';
    }

//    QMessageBox::about(nullptr, "行情信息", text);
    _pubMessage.setWindowTitle("行情信息");
    _pubMessage._setText(text);
    _pubMessage.show();

}
