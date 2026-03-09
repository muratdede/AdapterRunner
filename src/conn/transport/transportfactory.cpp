#include "transportfactory.h"


#include "src/conn/network/basynctcpdatareceiver.h"
#include "src/conn/network/basyncudpdatareceiver.h"
#include "src/conn/network/basyncudpdatasender.h"
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

    if (type == "tcp")
    {
        return new BAsyncTCPDataReceiver(
            obj["port"].toInt(),
            QHostAddress(obj["host"].toString()));
    }

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

ISender *createSender(const QJsonObject &obj)
{
    QString type = obj["type"].toString();

    if (type == "udp")
    {
        return new BAsyncUDPDataSender(
            QHostAddress(obj["destination_host"].toString()),
            obj["destination_port"].toInt(),
            QHostAddress::AnyIPv4,
            obj["source_port"].toInt(0));
    }

    return nullptr;
}

