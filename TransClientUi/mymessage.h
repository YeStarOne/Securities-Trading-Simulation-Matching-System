#ifndef MYMESSAGE_H
#define MYMESSAGE_H

#include <QWidget>

namespace Ui {
class MyMessage;
}

class MyMessage : public QWidget
{
    Q_OBJECT

public:
    explicit MyMessage(QWidget *parent = nullptr);
    ~MyMessage();

    bool _setText(QString& Qstr);

private:
    Ui::MyMessage *ui;
};

#endif // MYMESSAGE_H
