#include <QCoreApplication>

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "src/conn/transport/transportmanager.h"

#include "src/message/messageparser.h"
#include "src/message/messageserializer.h"
#include "src/message/protocolschema.h"

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

    // Load reactions from messages.json
    MessageParser parser(&schema);
    MessageSerializer serializer(&schema);
    ReactionEngine engine(&tm, &parser, &serializer);

    QFile file("../exampleConfigs/messages.json");
    QJsonArray reactionsArray;
    if (file.open(QIODevice::ReadOnly))
    {
        auto doc = QJsonDocument::fromJson(file.readAll());
        reactionsArray = doc.object()["reactions"].toArray();
        file.close();
    }
    engine.loadReactions(reactionsArray);

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

            auto valueNames = message.values.keys();
            qDebug() << "////" << name << "////" << message.name << "////";
            for (const auto &valueName : valueNames)
            {
                qDebug() << "  " << valueName << ":" << message.values.value(valueName);
            }
        });
    }

    /*
    AA 55 01 05 01 02 03 04 03
    AA 55 02 06 01 00 02 00 03 00
    AA 55 03 07 03 00 01 00 02 00 03
    */

    return a.exec();
}
