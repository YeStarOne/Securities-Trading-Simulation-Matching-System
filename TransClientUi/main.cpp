#include "widget.h"

#include "network.h"

#include <QApplication>

#include <qdesktopwidget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    //设置窗口的标题
    w.setWindowTitle("证券交易系统用户端");

    QDesktopWidget *desktop = QApplication::desktop();
    w.move((desktop->width() - w.width())/ 2, (desktop->height() - w.height()) /2);
    w.show();

    return a.exec();
}
