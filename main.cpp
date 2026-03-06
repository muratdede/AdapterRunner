#include <QCoreApplication>

#include <QDebug>

#include "src/conn/transport/transportmanager.h"

#include "src/message/messageparser.h"
#include "src/message/protocolschema.h"


// TODO: circular buffer

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    TransportManager tm;

    tm.load("transports.json");

    ITransport* udp = tm.get("telemetry_udp");


    ProtocolSchema schema;

    schema.load("messages.json");

    MessageParser parser(&schema);

    QObject::connect(udp, &ITransport::newMessageFromRemote, &a, [&](QByteArray data){
        ParsedMessage message = parser.parseFrame(data);

        qDebug() << "//////////////////////////////" << message.name << "////";
        for (const auto &valueName : message.values.keys())
        {
            qDebug() << valueName << " : " << message.values.value(valueName);
        }
    });

    /*
    QByteArray array;
    // header
    array.append(0xAA);
    array.append(0x55);

    array.append(0x01);
    array.append(0x05);
    // data
    array.append(0x01); // timestamp : 0x01020304(big endian) = 16.909.060
    array.append(0x02);
    array.append(0x03);
    array.append(0x04);

    array.append(0x03);    // status    : 0x03 = 3
    ParsedMessage message = parser.parseFrame(array);

    qDebug() << "//////////////////////////////" << message.name << "////";
    for (const auto &valueName : message.values.keys())
    {
        qDebug() << valueName << " : " << message.values.value(valueName);
    }



    // array.clear();
    // array.append(4, 0x01); // timestamp : 0x01010101 = 16.843.009
    // array.append(0x03);    // status    : 0x03 = 3
    // message = parser.parse(2, array);
    */

    return a.exec();
}
