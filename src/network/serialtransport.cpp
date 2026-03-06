#include "serialtransport.h"

SerialTransport::SerialTransport(QString port, int baud)
    : mPort(port)
    , mBaud(baud)
{

}

bool SerialTransport::start()
{
    mSerial = new QSerialPort(this);

    mSerial->setPortName(mPort);
    mSerial->setBaudRate(mBaud);

    if (!mSerial->open(QIODevice::ReadOnly))
        return false;

    connect(mSerial, &QSerialPort::readyRead,
            this, &SerialTransport::readData);

    return true;
}

void SerialTransport::stop()
{
    if (mSerial)
        mSerial->close();
}

void SerialTransport::readData()
{
    QByteArray data = mSerial->readAll();

    appendData(data);
}
