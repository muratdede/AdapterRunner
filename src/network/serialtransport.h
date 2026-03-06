#ifndef SERIALTRANSPORT_H
#define SERIALTRANSPORT_H

#include "itransport.h"

#include <QSerialPort>

class SerialTransport : public ITransport
{
public:

    SerialTransport(QString port, int baud);

    bool start() override;
    void stop() override;

private:
    void readData();

    QString mPort;
    int mBaud;

    QSerialPort* mSerial = nullptr;
};

#endif // SERIALTRANSPORT_H
