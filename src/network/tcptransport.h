#ifndef TCPTRANSPORT_H
#define TCPTRANSPORT_H

#include "itransport.h"

#include <QTcpSocket>

class TcpTransport : public ITransport
{
public:
    TcpTransport(QString host, int port);

    bool start() override;
    void stop() override;

private:
    void readData();

    QString mHost;
    int mPort;

    QTcpSocket* mSocket = nullptr;
};

#endif // TCPTRANSPORT_H
