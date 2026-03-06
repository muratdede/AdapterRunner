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

        auto valueNames = message.values.keys();
        qDebug() << "//////////////////////////////" << message.name << "////";
        for (const auto &valueName : valueNames)
        {
            qDebug() << valueName << " : " << message.values.value(valueName);
        }
    });

    /*
    AA 55 03 07 03 00 01 00 02 00 03
    AA 55 01 05 01 02 03 04 03
    */

    return a.exec();
}
