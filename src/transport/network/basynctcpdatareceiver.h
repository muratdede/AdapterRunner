#ifndef BASYNCTCPDATARECEIVER_H
#define BASYNCTCPDATARECEIVER_H

#include <QTcpServer>
#include <QTcpSocket>

#include "src/transport/network/bathreadnetworkreceiver.h"

class BAsyncTCPDataReceiver : public BAThreadNetworkReceiver
{
    Q_OBJECT
public:
    BAsyncTCPDataReceiver(const uint16_t &pPort, const QHostAddress &pHost = QHostAddress::AnyIPv4);
    ~BAsyncTCPDataReceiver();

signals:
    void errorOccurred(QAbstractSocket::SocketError error);

protected:
    void mainLoop() override;

private slots:
    void error(QAbstractSocket::SocketError error);
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    void deleteSockets();

private:
    QTcpServer *mServer;
    QList<QTcpSocket*> mClients;
    QHostAddress mHost;
    uint16_t mPort;
};

#endif // BASYNCTCPDATARECEIVER_H
