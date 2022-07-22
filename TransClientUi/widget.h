#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCore/QString>
#include <QtWidgets/QPushButton>

#include "network.h"
#include "tradewidget.h"
#include "revwidget.h"
#include "subwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;


    network _netWidget;
    TradeWidget _tradeWidget;
    RevWidget _revWidget;
    SubWidget _subWidget;

    void getSupportSec();

private slots:
    void hasFindHost();
    void hasConnected();
    void disConnected();
    void netError();

    void delNetConnect();
    void delBuy();
    void delSell();
    void delRevoke();
    void delUserSubPub();

    void _setPushBtnEnable();
    void _setPushBtnDisable();

    bool _userNameChange();
    bool _userNameChangeVerify();

    void _closeWidget();
};
#endif // WIDGET_H
