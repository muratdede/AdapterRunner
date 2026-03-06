#include "transportfactory.h"

#include "udptransport.h"
#include "tcptransport.h"
#include "serialtransport.h"

ITransport *createTransport(const QJsonObject &obj)
{
    QString type = obj["type"].toString();

    if (type == "udp")
    {
        return new UdpTransport(
            obj["bind_address"].toString(),
            obj["port"].toInt());
    }

    if (type == "tcp")
    {
        return new TcpTransport(
            obj["host"].toString(),
            obj["port"].toInt());
    }

    if (type == "serial")
    {
        return new SerialTransport(
            obj["port"].toString(),
            obj["baudrate"].toInt());
    }

    return nullptr;
}
