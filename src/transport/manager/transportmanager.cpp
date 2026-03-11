#include "src/transport/manager/transportmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "src/transport/factory/transportfactory.h"

bool TransportManager::load(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    auto doc = QJsonDocument::fromJson(file.readAll());

    auto root = doc.object();

    // Receivers
    for (auto val : root["receivers"].toArray())
    {
        auto obj = val.toObject();

        QString name = obj["name"].toString();

        auto transport = createTransport(obj);

        if (!transport)
            continue;

        transport->start();

        mReceivers.insert(name, transport);
    }

    // Senders
    for (auto val : root["senders"].toArray())
    {
        auto obj = val.toObject();

        QString name = obj["name"].toString();

        auto sender = createSender(obj);

        if (!sender)
            continue;

        sender->start();

        mSenders.insert(name, sender);
    }

    return true;
}

ITransport* TransportManager::getReceiver(const QString& name)
{
    return mReceivers.value(name, nullptr);
}

ISender* TransportManager::getSender(const QString& name)
{
    return mSenders.value(name, nullptr);
}

QHash<QString, ITransport*>& TransportManager::receivers()
{
    return mReceivers;
}
