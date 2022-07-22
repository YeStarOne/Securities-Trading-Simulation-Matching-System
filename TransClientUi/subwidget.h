#ifndef SUBWIDGET_H
#define SUBWIDGET_H

#include <QWidget>
#include "mymessage.h"

namespace Ui {
class SubWidget;
}

class SubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SubWidget(QWidget *parent = nullptr);
    ~SubWidget();

    void _clearSecButton();

    void createSecButton(const QString secName, const qint16 row, const qint16 col);

    QVector<QCheckBox*> _Vec_Button;

private:
    Ui::SubWidget *ui;

    MyMessage _pubMessage;

    void _selectSec();

    void verifyButtonEv();

    void selectAllButtonEv();

    QSet<QString> _hasSubSec;
};

#endif // SUBWIDGET_H
