#ifndef REVWIDGET_H
#define REVWIDGET_H

#include <QWidget>

namespace Ui {
class RevWidget;
}

class RevWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RevWidget(QWidget *parent = nullptr);
    ~RevWidget();

    void _createOrderButton();

    void _clearOrderButton();

    bool _revoke();

    QVector<QLabel*> _Vec_Button;

    QMap<QString, QLabel*> _OrderLableMap;

    QMap<QString, QLineEdit*> _QuanEditMap;


private:
    Ui::RevWidget *ui;

    void verifyButtonEv();
};

#endif // REVWIDGET_H
