#include "periodicsenderbehaviour.h"

#include <QJsonArray>
#include <QDebug>

static QMap<QString, QVariant> parseValues(const QJsonObject& obj)
{
    QMap<QString, QVariant> result;
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        if (it.value().isArray())
        {
            QVariantList list;
            for (auto v : it.value().toArray())
                list.append(v.toDouble());
            result[it.key()] = list;
        }
        else if (it.value().isDouble())
        {
            result[it.key()] = it.value().toDouble();
        }
        else
        {
            result[it.key()] = it.value().toVariant();
        }
    }
    return result;
}

PeriodicSenderBehaviour::PeriodicSenderBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mPeriodicTimer(nullptr)
{
    int periodMs = config["period_ms"].toInt(100);

    qDebug() << "PeriodicSenderBehaviour" << mResponseMessage << ">> ";

    // Parse sources array
    auto sourcesArray = config["sources"].toArray();
    for (int i = 0; i < sourcesArray.size(); ++i)
    {
        auto srcObj = sourcesArray[i].toObject();

        SourceEntry entry;
        entry.message = srcObj["message"].toString();
        entry.receiver = srcObj["receiver"].toString();
        entry.timeoutMs = srcObj["timeout_ms"].toInt(0);
        entry.defaultValues = parseValues(srcObj["default_values"].toObject());
        entry.alive = false;

        // Create per-source watchdog timer
        if (entry.timeoutMs > 0)
        {
            entry.watchdog = new QTimer(this);
            entry.watchdog->setSingleShot(true);
            entry.watchdog->setInterval(entry.timeoutMs);

            int idx = i;
            connect(entry.watchdog, &QTimer::timeout, this, [this, idx]() {
                onSourceTimeout(idx);
            });
        }

        mSources.append(entry);

        qDebug() << ">> source[" << i << "]"
                 << entry.message << "on" << entry.receiver
                 << "timeout:" << entry.timeoutMs << "ms";
    }

    // Start periodic send timer
    mPeriodicTimer = new QTimer(this);
    mPeriodicTimer->setInterval(periodMs);
    connect(mPeriodicTimer, &QTimer::timeout, this, &PeriodicSenderBehaviour::onTimerTick);
    mPeriodicTimer->start();

    qDebug() << "PeriodicSenderBehaviour: response=" << mResponseMessage
             << "period=" << periodMs << "ms"
             << "sources:" << mSources.size();
}

void PeriodicSenderBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    for (int i = 0; i < mSources.size(); ++i)
    {
        SourceEntry& src = mSources[i];

        if (receiverName == src.receiver && messageName == src.message)
        {
            src.lastReceivedValues = values;
            src.alive = true;

            // Reset watchdog
            if (src.watchdog)
                src.watchdog->start();
        }
    }
}

void PeriodicSenderBehaviour::onSourceTimeout(int sourceIndex)
{
    if (sourceIndex < 0 || sourceIndex >= mSources.size())
        return;

    SourceEntry& src = mSources[sourceIndex];
    src.alive = false;

    qDebug() << "PeriodicSenderBehaviour: source timeout -" << src.message
             << "(falling back to defaults)";
}

void PeriodicSenderBehaviour::onTimerTick()
{
    if (!mSender)
        return;

    // If ALL sources timed out → don't send
    if (!hasAnyAliveSource())
        return;

    auto merged = buildMergedValues();

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, merged);
    if (frame.isEmpty())
        return;

    mSender->send(frame);
}

QMap<QString, QVariant> PeriodicSenderBehaviour::buildMergedValues() const
{
    QMap<QString, QVariant> merged;

    for (const SourceEntry& src : mSources)
    {
        // Use last received values if alive, otherwise defaults
        const auto& values = src.alive ? src.lastReceivedValues : src.defaultValues;

        for (auto it = values.begin(); it != values.end(); ++it)
        {
            merged.insert(it.key(), it.value());
        }
    }

    return merged;
}

bool PeriodicSenderBehaviour::hasAnyAliveSource() const
{
    for (const SourceEntry& src : mSources)
    {
        if (src.alive)
            return true;
    }
    return false;
}

QStringList PeriodicSenderBehaviour::requiredReceivers() const
{
    QStringList list;
    for (const SourceEntry& src : mSources)
    {
        if (!list.contains(src.receiver))
            list.append(src.receiver);
    }
    return list;
}
