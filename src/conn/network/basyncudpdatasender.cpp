#include "basyncudpdatasender.h"

#include <QDebug>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

BAsyncUDPDataSender::BAsyncUDPDataSender(const QHostAddress &pDestinationHost, uint16_t pDestinationPort, const QHostAddress &pEthernetInterfaceAddress, uint16_t pSourcePort)
    : ISender()
    , mSocket(nullptr)
    , mDestinationHost(pDestinationHost)
    , mDestinationPort(pDestinationPort)
    , mEthernetInterfaceAddress(pEthernetInterfaceAddress)
    , mSourcePort(pSourcePort)
{

}

BAsyncUDPDataSender::~BAsyncUDPDataSender()
{

}

void BAsyncUDPDataSender::mainLoop()
{
    //TODO :  if you send multicast set mHost("224.0.0.2") or releated ip
    mSocket = new QUdpSocket;
    connect(mSocket, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &BAsyncUDPDataSender::errorOccurred);
    mSocket->open(QIODevice::WriteOnly);

    if(!mSocket->bind(mEthernetInterfaceAddress, mSourcePort, (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))){
        qDebug() << "BAsyncUDPDataSender::mainLoop bind error " << mSocket->error() << " mEthernetInterfaceAddress:" << mEthernetInterfaceAddress;
    }

    mSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    while(!getShouldStop()) {
        QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
    }

    mSocket->close();
    disconnect(mSocket, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &BAsyncUDPDataSender::errorOccurred);
    mSocket->deleteLater();
    mSocket = nullptr;
}

void BAsyncUDPDataSender::sendRawData(const char *data, uint size)
{
    uint index = 0;
    while(size != index) {
        uint sendSize = qMin(BYTE_SIZE, size-index);
        sendToRemote(data+index,  sendSize);
        index += sendSize;
    }
}

void BAsyncUDPDataSender::send(const QByteArray &pBuffer)
{
    if(!this->isRunning())
        return;

    if(!makeMethodRunOnThisThread("send", Q_ARG(QByteArray, pBuffer))){
        return;
    }

    sendRawData(pBuffer.constData(), pBuffer.size());
}

bool BAsyncUDPDataSender::sendToRemote(const char *data, uint len)
{
    if(mSocket == nullptr)
        return false; // already closed or was not opened at all

    if(mSocket->writeDatagram(data, len, mDestinationHost, mDestinationPort) == -1){
        qDebug() << this->objectName() << "BAsyncUDPDataSender::sendToRemote char* error while sending data " << mSocket->error();
        return false;
    }
    return true;
}
