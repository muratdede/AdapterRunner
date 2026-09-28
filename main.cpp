#include <QCoreApplication>

#include <QDebug>

#include "src/transport/manager/transportmanager.h"
#include "src/protocol/parser/messageparser.h"
#include "src/protocol/serializer/messageserializer.h"
#include "src/protocol/schema/protocolschema.h"
#include "src/engine/core/reactionengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // TODO: refactor needed here, maybe a controller

    // Load transports (receivers + senders)
    TransportManager tm;
    tm.load("../exampleConfigs/akkor_transports.json");

    // Load protocol schema
    ProtocolSchema schema;
    schema.load("../exampleConfigs/akkor_messages.json");

    MessageParser parser(&schema);
    MessageSerializer serializer(&schema);

    // Load and start reaction engine
    ReactionEngine engine(&tm, &parser, &serializer);
    engine.loadFromFile("../exampleConfigs/akkor_messages.json");

    // Debug: log all received messages on all receivers
    auto& receivers = tm.receivers();
    for (auto it = receivers.begin(); it != receivers.end(); ++it)
    {
        QString name = it.key();
        ITransport* receiver = it.value();

        QObject::connect(receiver, &ITransport::newMessageFromRemote, &a, [&parser, name](QByteArray data) {
            ParsedMessage message = parser.parseFrame(data);

            if (message.name.isEmpty())
            {
                qDebug() << "[" << name << "] unknown frame (" << data.size() << "bytes)";
                return;
            }

            qDebug() << "////" << name << "////" << message.name << "////";
            for (auto it = message.values.begin(); it != message.values.end(); ++it)
                qDebug() << "  " << it.key() << ":" << it.value();
        });
    }

    /*
    AA 55 01 05 01 02 03 04 03
    AA 55 02 06 01 00 02 00 03 00
    AA 55 03 07 03 00 01 00 02 00 03

    AA 55 02 0c a4 70 9d 3f a4 70 9d 3f a4 70 9d 3f // Vector float 1.23, 1.23, 1.23
    */

    return a.exec();
}
