#include "mymessage.h"
#include "ui_mymessage.h"

MyMessage::MyMessage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyMessage)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
}

MyMessage::~MyMessage()
{
    delete ui;
}

bool MyMessage::_setText(QString& Qstr) {
    ui->textBrowser->setText(Qstr);

    return true;
}
