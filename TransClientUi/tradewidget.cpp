#include "tradewidget.h"
#include "ui_tradewidget.h"

TradeWidget::TradeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TradeWidget)
{
    ui->setupUi(this);

    QRegExp regExpPrice;
    QRegExp regExpQuantity;
    // 匹配浮点数
    regExpPrice.setPattern("^([0]|[1-9]\\d*)(?:\\.\\d{0,2})?$|(^\\t?$)");
    // 匹配正整数
    regExpQuantity.setPattern("^[1-9]\\d*$|(^\\t?$)");
    QValidator * editPrice = new QRegExpValidator(regExpPrice,ui->price);
    QValidator * editQuantity = new QRegExpValidator(regExpQuantity,ui->quantity);
    ui->price->setValidator(editPrice);
    ui->quantity->setValidator(editQuantity);

    connect(ui->verifyButton, &QPushButton::clicked, this, &TradeWidget::verifyButtonEv);

    connect(ui->price, &QLineEdit::textChanged, this, &TradeWidget::_set_price);
    connect(ui->quantity, &QLineEdit::textChanged, this, &TradeWidget::_set_quantity);

    // 除当前窗口以外的所有窗口禁止被操作
    setWindowModality(Qt::ApplicationModal);
}

TradeWidget::~TradeWidget()
{
    for(auto& SecButton : _Vec_Button) {
        delete SecButton;
    }
    delete ui;
}

void TradeWidget::createSecButton(const QString secName, const qint16 row, const qint16 col) {
    _Vec_Button.push_back(new QRadioButton(this));
    _Vec_Button.back()->setFixedSize(160, 30);
    _Vec_Button.back()->setText(secName);
    _Vec_Button.back()->move(row, col);
    _Vec_Button.back()->show();
    connect(_Vec_Button.back(), &QRadioButton::clicked, this, &TradeWidget::_set_secName);
}

void TradeWidget::_clearSecButton() {
    _Vec_Button.clear();
}

void TradeWidget::_set_secName() {
    for(auto& SecButton : _Vec_Button) {
        if(SecButton->isChecked()) {
            _secName = SecButton->text();
            if(ui->price->text().length() > 0 && ui->quantity->text().length() > 0) {
                ui->verifyButton->setEnabled(true);
            }
            else {
                ui->verifyButton->setDisabled(true);
            }

            break;
        }
    }
}

void TradeWidget::_set_price() {
    if(ui->price->text().length() > 0) {
        _price = ui->price->text().toFloat();
        if(ui->quantity->text().length() > 0 && _secName.length() > 0) {
            ui->verifyButton->setEnabled(true);
        }
    }
    else {
        ui->verifyButton->setDisabled(true);
    }
}

void TradeWidget::_set_quantity() {
    if(ui->quantity->text().length() > 0) {
        _quantity = ui->quantity->text().toUInt();
        if(ui->price->text().length() > 0 && _secName.length() > 0) {
            ui->verifyButton->setEnabled(true);
        }
    }
    else {
        ui->verifyButton->setDisabled(true);
    }
}

void TradeWidget::verifyButtonEv() {
    qDebug() << "secName:" << _secName << "price:" << _price <<"quantity:" << _quantity;

    network::_str = network::_userName.toStdString() + "\r\n" + _tradeType.toStdString() + "\r\n";
    network::_str += _secName.toStdString() + ';' + std::to_string(_price) + ';' + std::to_string(_quantity) + "\r\n";

    network::_send();

    network::_receive();

    QString text = "日期:" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                    '\n' + network::_buffer + '\n';

    if(_tradeType == "buy") {
        QMessageBox::about(nullptr, "买入结果", text);
    }
    else {
        QMessageBox::about(nullptr, "卖出结果", text);
    }

}

bool TradeWidget::_trade(QString tradeType) {
    _tradeType = tradeType;
    if(_tradeType == "buy") {
        ui->label1->setText("买入价格");
        ui->label2->setText("买入数量");
    }
    else {
        ui->label1->setText("卖出价格");
        ui->label2->setText("卖出数量");
    }
    ui->price->clear();
    ui->quantity->clear();
    for(auto& SecButton : _Vec_Button) {
        SecButton->setAutoExclusive(false);
        SecButton->setChecked(false);
        SecButton->setAutoExclusive(true);
    }
    ui->verifyButton->setDisabled(true);
    _secName.clear();

    return true;
}
