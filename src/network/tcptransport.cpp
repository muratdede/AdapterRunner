#include "tcptransport.h"

TcpTransport::TcpTransport(QString host, int port)
    : mHost(host)
    , mPort(port)
{

}

bool TcpTransport::start()
{
    mSocket = new QTcpSocket(this);

    connect(mSocket, &QTcpSocket::readyRead,
            this, &TcpTransport::readData);

    mSocket->connectToHost(mHost, mPort);

    return true;
}

void TcpTransport::stop()
{
    if (mSocket)
        mSocket->disconnectFromHost();
}

void TcpTransport::readData()
{
    QByteArray data = mSocket->readAll();

    appendData(data);
}
