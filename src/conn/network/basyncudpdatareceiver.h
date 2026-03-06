#ifndef BASYNCUDPDATARECEIVER_H
#define BASYNCUDPDATARECEIVER_H

#include "bathreadnetworkreceiver.h"

#include <QUdpSocket>

class BAsyncUDPDataReceiver : public BAThreadNetworkReceiver
{
    Q_OBJECT
public:
    //unicast listen host must be 127.0.0.1 or local ip
    BAsyncUDPDataReceiver(const uint16_t &pPort, const QHostAddress &pHost = QHostAddress::AnyIPv4, const QHostAddress &pEthernetInterfaceAddress = QHostAddress::AnyIPv4);
    ~BAsyncUDPDataReceiver();

signals:
    void errorOccurred(QAbstractSocket::SocketError error);

protected:
    void mainLoop() override;

private slots:
    void error(QAbstractSocket::SocketError error);

private:
    bool bindMulticast();
    bool bindUnicast();

    void closeMulticast();
    void closeUnicast();

    void deleteSocket();

private:
    QUdpSocket *mSocket;
    QHostAddress mHost;
    QHostAddress mEthernetInterfaceAddress;
    uint16_t mPort;

private:
    const static int TIME_OUT = 20;
};

#endif // BASYNCUDPDATARECEIVER_H
