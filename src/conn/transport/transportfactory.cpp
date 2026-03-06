#include "transportfactory.h"

// #include "udptransport.h"
// #include "tcptransport.h"
// #include "serialtransport.h"
#include "src/conn/network/basyncudpdatareceiver.h"
#include "src/conn/serial/bserialsenderreceiver.h"

ITransport *createTransport(const QJsonObject &obj)
{
    QString type = obj["type"].toString();

    if (type == "udp")
    {
        return new BAsyncUDPDataReceiver(
            obj["port"].toInt(),
            QHostAddress(obj["bind_address"].toString()));
    }

    // if (type == "tcp")
    // {
    //     return new TcpTransport(
    //         obj["host"].toString(),
    //         obj["port"].toInt());
    // }

    if (type == "serial")
    {
        return new BSerialSenderReceiver(
            obj["port"].toString(),
            static_cast<QSerialPort::BaudRate>(obj["baudrate"].toInt()),
            static_cast<QSerialPort::DataBits>(obj["databits"].toInt()),
            static_cast<QSerialPort::Parity>(obj["parity"].toInt()),
            static_cast<QSerialPort::FlowControl>(obj["flowcontrol"].toInt()),
            static_cast<QSerialPort::StopBits>(obj["stopbits"].toInt()));
    }

    return nullptr;
}
