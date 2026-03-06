#include "transportmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "transportfactory.h"

bool TransportManager::load(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    auto doc = QJsonDocument::fromJson(file.readAll());

    auto root = doc.object();

    for (auto val : root["transports"].toArray())
    {
        auto obj = val.toObject();

        QString name = obj["name"].toString();

        auto transport = createTransport(obj);

        if (!transport)
            continue;

        transport->start();

        mTransports.insert(name, transport);
    }

    return true;
}

ITransport* TransportManager::get(const QString& name)
{
    return mTransports.value(name, nullptr);
}
