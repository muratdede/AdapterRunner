#include "src/transport/network/basyncudpdatareceiver.h"

#include <QDebug>
#include <QAbstractEventDispatcher>

BAsyncUDPDataReceiver::BAsyncUDPDataReceiver(const uint16_t &pPort, const QHostAddress &pHost, const QHostAddress &pEthernetInterfaceAddress)
    : BAThreadNetworkReceiver()
    , mHost(pHost)
    , mPort(pPort)
    , mEthernetInterfaceAddress(pEthernetInterfaceAddress)
    , mSocket(nullptr)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

BAsyncUDPDataReceiver::~BAsyncUDPDataReceiver()
{
    if(mHost.isMulticast()){
        closeMulticast();
    }else {
        closeUnicast();
    }
}

void BAsyncUDPDataReceiver::mainLoop()
{
    mSocket = new QUdpSocket;
    mSocket->open(QIODevice::ReadOnly);

    if(mHost.isMulticast()){
        bindMulticast();
    }else {
        bindUnicast();
    }

    QObject::connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    QObject::connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(errorOccurred(QAbstractSocket::SocketError)));
    QObject::connect(mSocket, &QIODevice::readyRead, this, [this]{
        QByteArray tBuffer;
        tBuffer.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(tBuffer.data(), tBuffer.size(), &sender, &senderPort);
        emit newMessageFromRemote(tBuffer);
    });

    while(!getShouldStop()) {
        QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
    }

    QObject::disconnect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    if(mHost.isMulticast()){
        closeMulticast();
    }else {
        closeUnicast();
    }
}

void BAsyncUDPDataReceiver::error(QAbstractSocket::SocketError error)
{
    //if(error != QAbstractSocket::SocketTimeoutError){
    qDebug() << this->objectName() << "BAsyncUDPDataReceiver::error error " << error;
    //}
}

bool BAsyncUDPDataReceiver::bindMulticast()
{
    if(!mSocket->bind(mEthernetInterfaceAddress, mPort, (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))){
        qDebug() << this->objectName() << "BAsyncUDPDataReceiver::bindMulticast QUDPSocket bind error " << mSocket->error()  << " mPort:" << mPort << " mHost:" << mHost;
        return false;
    }

    if(!mSocket->joinMulticastGroup(mHost)){
        qDebug() << this->objectName() << "BAsyncUDPDataReceiver::bindMulticast QUDPSocket joinMulticastGroup error " << mSocket->error() << " mPort:" << mPort << " mHost:" << mHost;
        return false;
    }
    qDebug() << this->objectName() << "BAsyncUDPDataReceiver::bindMulticast successfully";
    return true;
}

bool BAsyncUDPDataReceiver::bindUnicast()
{
    if(!mSocket->bind(mEthernetInterfaceAddress, mPort, (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))){
        qDebug() << this->objectName() << "BAsyncUDPDataReceiver::bindUnicast QUDPSocket bind error " << mSocket->error() << " mPort:" << mPort << " mHost:" << mHost;
        return false;
    }
    qDebug() << this->objectName() << "BAsyncUDPDataReceiver::bindUnicast successfully";
    return true;
}

void BAsyncUDPDataReceiver::closeMulticast()
{
    if(mSocket){
        mSocket->leaveMulticastGroup(mHost);
       deleteSocket();
    }
}

void BAsyncUDPDataReceiver::closeUnicast()
{
    deleteSocket();
}

void BAsyncUDPDataReceiver::deleteSocket()
{
    if(mSocket){
        mSocket->close();
        mSocket->deleteLater();
        mSocket = nullptr;
    }
}
