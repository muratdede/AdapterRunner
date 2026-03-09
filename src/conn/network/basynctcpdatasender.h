#ifndef BASYNCTCPDATASENDER_H
#define BASYNCTCPDATASENDER_H

#include "src/thread/bathread.h"
#include <QTcpSocket>
#include <QHostAddress>

class BAsyncTCPDataSender : public BAThread
{
    Q_OBJECT
public:
    explicit BAsyncTCPDataSender(const QHostAddress &pDestinationHost, uint16_t pDestinationPort);
    virtual ~BAsyncTCPDataSender();

public slots:
    void send(const QByteArray &pBuffer);

signals:
    void errorOccurred(QAbstractSocket::SocketError error);

private:
    void mainLoop() override;
    bool sendToRemote(const char *data, uint len);
    void sendRawData(const char *data, uint size);

private:
    QTcpSocket *mSocket;
    QHostAddress mDestinationHost;
    uint16_t mDestinationPort;

private:
    const static uint BYTE_SIZE = 1024;
};

#endif // BASYNCTCPDATASENDER_H
