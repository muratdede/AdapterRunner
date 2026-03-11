#include "src/engine/core/reactionengine.h"

#include "src/behavior/impls/periodicsenderbehaviour.h"
#include "src/behavior/impls/respondbehaviour.h"
#include "src/behavior/impls/respondwithlastbehaviour.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ReactionEngine::ReactionEngine(TransportManager *tm, MessageParser *parser, MessageSerializer *serializer, QObject *parent)
    : QObject(parent)
    , mTransportManager(tm)
    , mParser(parser)
    , mSerializer(serializer)
{

}

void ReactionEngine::loadFromFile(const QString &messagesJsonPath)
{
    QFile file(messagesJsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ReactionEngine: cannot open" << messagesJsonPath;
        return;
    }

    auto doc = QJsonDocument::fromJson(file.readAll());
    auto reactions = doc.object()["reactions"].toArray();

    for (const auto &val : reactions) {
        IBehaviour *behaviour = createBehaviour(val.toObject());
        if (!behaviour)
            continue;

        // Connect all receivers this behaviour needs
        for (const auto &receiverName : behaviour->requiredReceivers())
            connectReceiver(receiverName);

        mBehaviours.append(behaviour);
    }
}

IBehaviour *ReactionEngine::createBehaviour(const QJsonObject &config) {
    QString type = config["type"].toString();

    // Resolve sender
    QString senderName = config["sender"].toString();
    ISender *sender = mTransportManager->getSender(senderName);
    if (!sender) {
        qWarning() << "ReactionEngine: sender not found:" << senderName;
        return nullptr;
    }

    // Create behaviour by type
    if (type == "respond")
        return new RespondBehaviour(config, sender, mSerializer, this);

    if (type == "respond_with_last")
        return new RespondWithLastBehaviour(config, sender, mSerializer, this);

    if (type == "periodic_sender")
        return new PeriodicSenderBehaviour(config, sender, mSerializer, this);

    qWarning() << "ReactionEngine: unknown behaviour type:" << type;
    return nullptr;
}

void ReactionEngine::connectReceiver(const QString &receiverName)
{
    if (receiverName.isEmpty() || mConnectedReceivers.contains(receiverName))
        return;

    ITransport *receiver = mTransportManager->getReceiver(receiverName);
    if (!receiver) {
        qWarning() << "ReactionEngine: receiver not found:" << receiverName;
        return;
    }

    connect(receiver, &ITransport::newMessageFromRemote, this, std::bind(&ReactionEngine::onReceiverData, this, std::placeholders::_1, receiverName));

    mConnectedReceivers.insert(receiverName);
}

void ReactionEngine::onReceiverData(const QByteArray &data, const QString &receiverName)
{
    ParsedMessage message = mParser->parseFrame(data);

    if (message.name.isEmpty())
        return;

    for (IBehaviour *behaviour : mBehaviours)
        behaviour->onMessageReceived(receiverName, message.name, message.values);
}
