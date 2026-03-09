#include "periodicsenderbehaviour.h"
#include "src/common/jsonutils.h"

#include <QJsonArray>
#include <QDebug>

PeriodicSenderBehaviour::PeriodicSenderBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mPeriodicTimer(nullptr)
{
    int periodMs = config["period_ms"].toInt(100);

    // Create trackers and mappings from sources array
    auto sourcesArray = config["sources"].toArray();
    for (int i = 0; i < sourcesArray.size(); ++i)
    {
        auto srcObj = sourcesArray[i].toObject();

        auto defaults = JsonUtils::parseValues(srcObj["default_values"].toObject());

        auto* tracker = new SourceTracker(
            srcObj["message"].toString(),
            srcObj["receiver"].toString(),
            srcObj["timeout_ms"].toInt(0),
            defaults,
            this
        );

        mTrackers.append(tracker);
        mMappings.append(FieldMapping::fromJsonArray(srcObj["mappings"].toArray()));

        qDebug() << "PeriodicSenderBehaviour: source[" << i << "]"
                 << tracker->messageName() << "on" << tracker->receiverName()
                 << "mappings:" << mMappings.last().size();
    }

    // Start periodic send timer
    mPeriodicTimer = new QTimer(this);
    mPeriodicTimer->setInterval(periodMs);
    connect(mPeriodicTimer, &QTimer::timeout, this, &PeriodicSenderBehaviour::onTimerTick);
    mPeriodicTimer->start();

    qDebug() << "PeriodicSenderBehaviour: response=" << mResponseMessage
             << "period=" << periodMs << "ms"
             << "sources:" << mTrackers.size();
}

void PeriodicSenderBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    for (SourceTracker* tracker : qAsConst(mTrackers))
    {
        if (receiverName == tracker->receiverName() && messageName == tracker->messageName())
            tracker->feed(values);
    }
}

void PeriodicSenderBehaviour::onTimerTick()
{
    if (!mSender || !hasAnyAliveSource())
        return;

    auto merged = buildMergedValues();

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, merged);
    if (!frame.isEmpty())
        mSender->send(frame);
}

QMap<QString, QVariant> PeriodicSenderBehaviour::buildMergedValues()
{
    QMap<QString, QVariant> merged;

    for (int i = 0; i < mTrackers.size(); ++i)
    {
        const auto& values = mTrackers[i]->currentValues();
        const auto& mappings = mMappings[i];

        if (mappings.isEmpty())
        {
            // No mappings defined → direct pass-through by field name
            for (auto it = values.begin(); it != values.end(); ++it)
                merged.insert(it.key(), it.value());
        }
        else
        {
            // Apply expression-based mappings
            mEvaluator.setVariables(values);

            for (const FieldMapping& mapping : mappings)
            {
                QVariant result = mEvaluator.evaluate(mapping.expression);
                merged.insert(mapping.targetField, result);
            }
        }
    }

    return merged;
}

bool PeriodicSenderBehaviour::hasAnyAliveSource() const
{
    for (const SourceTracker* tracker : mTrackers)
    {
        if (tracker->isAlive())
            return true;
    }
    return false;
}

QStringList PeriodicSenderBehaviour::requiredReceivers() const
{
    QStringList list;
    for (const SourceTracker* tracker : mTrackers)
    {
        QString r = tracker->receiverName();
        if (!r.isEmpty() && !list.contains(r))
            list.append(r);
    }
    return list;
}
