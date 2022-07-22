#ifndef TRADEWIDGET_H
#define TRADEWIDGET_H

#include <QWidget>

namespace Ui {
class TradeWidget;
}

class TradeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TradeWidget(QWidget *parent = nullptr);
    ~TradeWidget();

    void createSecButton(const QString secName, const qint16 row, const qint16 col);

    bool _trade(QString tradeType);

    void _clearSecButton();


    QVector<QRadioButton*> _Vec_Button;

    QString _secName;
    float _price = 0.0;
    quint32 _quantity = 0;

private:
    Ui::TradeWidget *ui;

    QString _tradeType;

    void verifyButtonEv();

    void _set_secName();
    void _set_price();
    void _set_quantity();
};

#endif // TRADEWIDGET_H
