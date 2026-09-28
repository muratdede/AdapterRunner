#include "basynctcpdatareceiver.h"

#include <QDebug>
#include <QAbstractEventDispatcher>

BAsyncTCPDataReceiver::BAsyncTCPDataReceiver(const uint16_t &pPort, const QHostAddress &pHost)
    : BAThreadNetworkReceiver()
    , mServer(nullptr)
    , mHost(pHost)
    , mPort(pPort)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

BAsyncTCPDataReceiver::~BAsyncTCPDataReceiver()
{
    deleteSockets();
}

void BAsyncTCPDataReceiver::mainLoop()
{
    mServer = new QTcpServer;

    if(!mServer->listen(mHost, mPort)){
        qDebug() << this->objectName() << "BAsyncTCPDataReceiver::mainLoop QTcpServer listen error " << mServer->serverError()  << " mPort:" << mPort << " mHost:" << mHost;
    } else {
        qDebug() << this->objectName() << "BAsyncTCPDataReceiver::mainLoop listen successfully on port" << mPort;
    }

    QObject::connect(mServer, &QTcpServer::newConnection, this, &BAsyncTCPDataReceiver::onNewConnection);
    QObject::connect(mServer, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError socketError){
        emit errorOccurred(socketError);
    });

    while(!getShouldStop()) {
        QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
    }

    QObject::disconnect(mServer, &QTcpServer::newConnection, this, &BAsyncTCPDataReceiver::onNewConnection);

    deleteSockets();
}

void BAsyncTCPDataReceiver::onNewConnection()
{
    while (mServer->hasPendingConnections()) {
        QTcpSocket *clientConnection = mServer->nextPendingConnection();
        connect(clientConnection, &QTcpSocket::readyRead, this, &BAsyncTCPDataReceiver::onReadyRead);
        connect(clientConnection, &QTcpSocket::disconnected, this, &BAsyncTCPDataReceiver::onClientDisconnected);
        connect(clientConnection, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &BAsyncTCPDataReceiver::error);
        mClients.append(clientConnection);
    }
}

void BAsyncTCPDataReceiver::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QByteArray tBuffer = clientSocket->readAll();
    if(!tBuffer.isEmpty()) {
        emit newMessageFromRemote(tBuffer);
    }
}

void BAsyncTCPDataReceiver::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;
        
    mClients.removeOne(clientSocket);
    clientSocket->deleteLater();
}

void BAsyncTCPDataReceiver::error(QAbstractSocket::SocketError error)
{
    qDebug() << this->objectName() << "BAsyncTCPDataReceiver::error error " << error;
    emit errorOccurred(error);
}

void BAsyncTCPDataReceiver::deleteSockets()
{
    for(QTcpSocket *client : qAsConst(mClients)){
        client->close();
        client->deleteLater();
    }
    mClients.clear();

    if(mServer){
        mServer->close();
        mServer->deleteLater();
        mServer = nullptr;
    }
}
