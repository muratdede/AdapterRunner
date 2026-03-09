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

    // Create behaviour by type
    IBehaviour* behaviour = nullptr;

    if (type == "respond")
    {
        behaviour = new RespondBehaviour(config, sender, mSerializer, this);
    }
    else if (type == "respond_with_last")
    {
        behaviour = new RespondWithLastBehaviour(config, sender, mSerializer, this);
    }
    else if (type == "periodic_sender")
    {
        behaviour = new PeriodicSenderBehaviour(config, sender, mSerializer, this);
    }
    else
    {
        qWarning() << "ReactionEngine: unknown behaviour type:" << type;
        return nullptr;
    }

    // Collect all receiver names this behaviour needs
    QStringList receiverNames;

    // Top-level keys
    for (const auto& key : {"receiver", "source_receiver"})
    {
        QString name = config[key].toString();
        if (!name.isEmpty())
            receiverNames.append(name);
    }

    // PeriodicSender sources
    auto* periodic = qobject_cast<PeriodicSenderBehaviour*>(behaviour);
    if (periodic)
    {
        receiverNames.append(periodic->requiredReceivers());
    }

    // Connect receivers (avoid duplicates)
    for (const auto& receiverName : receiverNames)
    {
        if (mConnectedReceivers.contains(receiverName))
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

    return behaviour;
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
