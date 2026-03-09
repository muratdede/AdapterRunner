#include <QCoreApplication>

#include <QDebug>

#include "src/conn/transport/transportmanager.h"

#include "src/message/messageparser.h"
#include "src/message/messageserializer.h"
#include "src/message/protocolschema.h"

// TODO: reactionları runtimeda sürekli if name equals ile kıyaslamak yerine gerekli sender ve reaction handlerların
//   connectionlarıyla bu iş çözülebilir
#include "src/reaction/reactionengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Load transports (receivers + senders)
    TransportManager tm;
    tm.load("../exampleConfigs/transports.json");

    // Load protocol schema
    ProtocolSchema schema;
    schema.load("../exampleConfigs/messages.json");

    MessageParser parser(&schema);
    MessageSerializer serializer(&schema);

    // Load and start reaction engine
    ReactionEngine engine(&tm, &parser, &serializer);
    engine.loadFromFile("../exampleConfigs/messages.json");

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
