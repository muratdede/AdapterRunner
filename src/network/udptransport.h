#ifndef UDPTRANSPORT_H
#define UDPTRANSPORT_H

#include "itransport.h"

#include <QUdpSocket>

class UdpTransport : public ITransport
{
public:
    UdpTransport(QString bindAddr, int port);

    bool start() override;
    void stop() override;

private:
    void readPending();

    QString mAddr;
    int mPort;

    QUdpSocket* mSocket = nullptr;
};

#endif // UDPTRANSPORT_H
