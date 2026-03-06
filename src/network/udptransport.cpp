#include "udptransport.h"

UdpTransport::UdpTransport(QString bindAddr, int port)
    : mAddr(bindAddr)
    , mPort(port)
{

}

bool UdpTransport::start()
{
    mSocket = new QUdpSocket(this);

    if (!mSocket->bind(QHostAddress(mAddr), mPort, QAbstractSocket::BindFlag::ShareAddress)) {
        return false;
    }

    connect(mSocket, &QUdpSocket::readyRead, this, &UdpTransport::readPending);

    return true;
}

void UdpTransport::stop()
{
    if (mSocket)
        mSocket->close();
}

void UdpTransport::readPending()
{
    while (mSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(mSocket->pendingDatagramSize());

        mSocket->readDatagram(datagram.data(), datagram.size());

        appendData(datagram);
    }
}
