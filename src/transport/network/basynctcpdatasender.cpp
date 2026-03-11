#include "src/transport/network/basynctcpdatasender.h"

#include <QDebug>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>

BAsyncTCPDataSender::BAsyncTCPDataSender(const QHostAddress &pDestinationHost, uint16_t pDestinationPort)
    : BAThread()
    , mSocket(nullptr)
    , mDestinationHost(pDestinationHost)
    , mDestinationPort(pDestinationPort)
{

}

BAsyncTCPDataSender::~BAsyncTCPDataSender()
{

}

void BAsyncTCPDataSender::mainLoop()
{
    mSocket = new QTcpSocket;
    connect(mSocket, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &BAsyncTCPDataSender::errorOccurred);
    
    mSocket->connectToHost(mDestinationHost, mDestinationPort);
    mSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    while(!getShouldStop()) {
        QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
    }

    if (mSocket->state() == QAbstractSocket::ConnectedState) {
        mSocket->disconnectFromHost();
        if (mSocket->state() != QAbstractSocket::UnconnectedState) {
            mSocket->waitForDisconnected();
        }
    }

    mSocket->close();
    disconnect(mSocket, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &BAsyncTCPDataSender::errorOccurred);
    mSocket->deleteLater();
    mSocket = nullptr;
}

void BAsyncTCPDataSender::sendRawData(const char *data, uint size)
{
    uint index = 0;
    while(size != index) {
        uint sendSize = qMin(BYTE_SIZE, size-index);
        sendToRemote(data+index,  sendSize);
        index += sendSize;
    }
}

void BAsyncTCPDataSender::send(const QByteArray &pBuffer)
{
    if(!this->isRunning())
        return;

    if(!makeMethodRunOnThisThread("send", Q_ARG(QByteArray, pBuffer))){
        return;
    }

    sendRawData(pBuffer.constData(), pBuffer.size());
}

bool BAsyncTCPDataSender::sendToRemote(const char *data, uint len)
{
    if(mSocket == nullptr)
        return false;

    if(mSocket->state() != QAbstractSocket::ConnectedState)
        return false;

    if(mSocket->write(data, len) == -1){
        qDebug() << this->objectName() << "BAsyncTCPDataSender::sendToRemote error while sending data " << mSocket->error();
        return false;
    }
    return true;
}
