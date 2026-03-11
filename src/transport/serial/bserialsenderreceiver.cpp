#include "src/transport/serial/bserialsenderreceiver.h"

#include <QCoreApplication>
#include <QAbstractEventDispatcher>

BSerialSenderReceiver::BSerialSenderReceiver(QString pPortName, QSerialPort::BaudRate pBaudRate, QSerialPort::DataBits pDataBits, QSerialPort::Parity pParity, QSerialPort::FlowControl pFlowControl, QSerialPort::StopBits pStopBits)
    : BAThreadNetworkReceiver()
    , mPortName(pPortName)
    , mBaudRate(pBaudRate)
    , mDataBits(pDataBits)
    , mParity(pParity)
    , mFlowControl(pFlowControl)
    , mStopBits(pStopBits)

{

}

void BSerialSenderReceiver::send(QByteArray data)
{
    if(!makeMethodRunOnThisThread("send", Q_ARG(QByteArray, data))){
        return;
    }

    if(mSerialPort->write(data) == -1){
        qDebug() << this->objectName() << "BSerialSenderReceiver::send QByteArray error while sending data " << mSerialPort->error();
    }
}

void BSerialSenderReceiver::mainLoop()
{
    mSerialPort = new QSerialPort(this);
    mSerialPort->setPortName(mPortName);

    if(!mSerialPort->setBaudRate(mBaudRate)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind setBaudRate error " <<  mSerialPort->errorString();
        return;
    }
    if(!mSerialPort->setDataBits(mDataBits)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind setDataBits error " <<  mSerialPort->errorString();
        return;
    }
    if(!mSerialPort->setParity(mParity)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind setParity error " <<  mSerialPort->errorString();
        return;
    }
    if(!mSerialPort->setFlowControl(mFlowControl)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind setFlowControl error " <<  mSerialPort->errorString();
        return;
    }
    if(!mSerialPort->setStopBits(mStopBits)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind setStopBits error " <<  mSerialPort->errorString();
        return;
    }
    if(!mSerialPort->open(QIODevice::ReadWrite)){
        qDebug() << this->objectName() << "BSerialSenderReceiver::bind open error " <<  mSerialPort->errorString();
        return;
    }

    QObject::connect(mSerialPort, &QSerialPort::errorOccurred, this, &BSerialSenderReceiver::error);

    while(!getShouldStop()) {
        QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
        if(mSerialPort->waitForReadyRead(TIME_OUT_MS)){
            emit newMessageFromRemote(mSerialPort->readAll());
        }
    }
    mSerialPort->close();
    mSerialPort->deleteLater();
}

void BSerialSenderReceiver::error(QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::TimeoutError){
        qDebug() << this->objectName() << "BSerialReceiver::errorOccurred error " << error;
        emit errorOccurred(error);
    }
}

