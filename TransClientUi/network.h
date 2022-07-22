#ifndef NETWORK_H
#define NETWORK_H

#include <QWidget>

namespace Ui {
class network;
}

class network : public QWidget
{
    Q_OBJECT

public:
    explicit network(QWidget *parent = nullptr);
    ~network();

    bool Connect();

    bool disConnect();


    static QTcpSocket *_socket;

    static QString _host_ip;
    static quint16 _host_port;

    static std::string _str;
    static QByteArray _buffer;
    static QString _userName;  // 用户名
    static QStringList supportSec; // 支持的证券

    static void _send();
    static void _receive();

    void _hostIp();
    void _hostPort();
    void _set_userName();
    void _verifyButtonEv();

private slots:
    void hasConnected();

private:
    Ui::network *ui;
};

#endif // NETWORK_H
