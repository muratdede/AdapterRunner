#include "reactionengine.h"

#include "respondbehaviour.h"
#include "respondwithlastbehaviour.h"
#include "periodicsenderbehaviour.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ReactionEngine::ReactionEngine(TransportManager *tm, MessageParser *parser, MessageSerializer *serializer, QObject *parent)
    : QObject(parent)
    , mTransportManager(tm)
    , mParser(parser)
    , mSerializer(serializer)
{
}

void ReactionEngine::loadReactions(const QJsonArray &reactions)
{
    for (const auto& val : reactions)
    {
        auto config = val.toObject();

        IBehaviour* behaviour = createBehaviour(config);
        if (!behaviour)
            continue;

        mBehaviours.append(behaviour);
    }
}

IBehaviour* ReactionEngine::createBehaviour(const QJsonObject &config)
{
    QString type = config["type"].toString();

    // Resolve sender
    QString senderName = config["sender"].toString();
    ISender* sender = mTransportManager->getSender(senderName);
    if (!sender)
    {
        qWarning() << "ReactionEngine: sender not found:" << senderName;
        return nullptr;
    }

    // Connect relevant receivers (avoid duplicates)
    QStringList receiverKeys = {"receiver", "source_receiver"};
    for (const auto& key : receiverKeys)
    {
        QString receiverName = config[key].toString();
        if (receiverName.isEmpty() || mConnectedReceivers.contains(receiverName))
            continue;

        ITransport* receiver = mTransportManager->getReceiver(receiverName);
        if (!receiver)
        {
            qWarning() << "ReactionEngine: receiver not found:" << receiverName;
            continue;
        }

        connect(receiver, &ITransport::newMessageFromRemote, this,
                [this, receiverName](const QByteArray& data) {
                    onReceiverData(data, receiverName);
                });
        mConnectedReceivers.insert(receiverName);
    }

    // Create behaviour by type
    if (type == "respond")
    {
        return new RespondBehaviour(config, sender, mSerializer, this);
    }

    if (type == "respond_with_last")
    {
        return new RespondWithLastBehaviour(config, sender, mSerializer, this);
    }

    if (type == "periodic_sender")
    {
        return new PeriodicSenderBehaviour(config, sender, mSerializer, this);
    }

    qWarning() << "ReactionEngine: unknown behaviour type:" << type;
    return nullptr;
}

void ReactionEngine::onReceiverData(const QByteArray &data, const QString &receiverName)
{
    // Parse ONCE
    ParsedMessage message = mParser->parseFrame(data);

    if (message.name.isEmpty())
        return;

    // Dispatch to ALL behaviours
    for (IBehaviour* behaviour : mBehaviours)
    {
        behaviour->onMessageReceived(receiverName, message.name, message.values);
    }
}
