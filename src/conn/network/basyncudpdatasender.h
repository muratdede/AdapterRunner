#ifndef BASYNCUDPDATASENDER_H
#define BASYNCUDPDATASENDER_H

#include "src/thread/bathread.h"
#include <QUdpSocket>

class BAsyncUDPDataSender : public BAThread
{
    Q_OBJECT
public:
    explicit BAsyncUDPDataSender(const QHostAddress &pDestinationHost, uint16_t pDestinationPort, const QHostAddress &pEthernetInterfaceAddress = QHostAddress::AnyIPv4, uint16_t pSourcePort = 0);
    virtual ~BAsyncUDPDataSender();

public slots:
    void send(const QByteArray &pBuffer);

signals:
    void errorOccurred(QAbstractSocket::SocketError error);

private:
    void sendRawData(const char *data, uint size);
    void mainLoop() override;
    bool sendToRemote(const char *data, uint len);

private:
    QUdpSocket *mSocket;
    QHostAddress mDestinationHost;
    QHostAddress mEthernetInterfaceAddress;
    uint16_t mDestinationPort;
    uint16_t mSourcePort;

private:
    //referanced by https://stackoverflow.com/questions/14993000/the-most-reliable-and-efficient-udp-packet-size 1472
    const static uint BYTE_SIZE = 1024;
};

#endif // BASYNCUDPMESSAGESENDER_H
