#include "periodicsenderbehaviour.h"

#include <QJsonArray>
#include <QDebug>

static QMap<QString, QVariant> parsePeriodicDefaults(const QJsonObject& obj)
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
    , mSourceMessage(config["source_message"].toString(""))
    , mSourceReceiver(config["source_receiver"].toString(""))
    , mResponseMessage(config["response"].toString())
    , mDefaultValues(parsePeriodicDefaults(config["default_values"].toObject()))
    , mHasReceivedSource(false)
    , mSender(sender)
    , mSerializer(serializer)
    , mPeriodicTimer(nullptr)
{
    int periodMs = config["period_ms"].toInt(100);

    mPeriodicTimer = new QTimer(this);
    mPeriodicTimer->setInterval(periodMs);
    connect(mPeriodicTimer, &QTimer::timeout, this, &PeriodicSenderBehaviour::onTimerTick);
    mPeriodicTimer->start();

    qDebug() << "PeriodicSenderBehaviour: source=" << mSourceMessage
             << "on" << mSourceReceiver
             << "response=" << mResponseMessage
             << "period=" << periodMs << "ms";
}

void PeriodicSenderBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    if (mSourceReceiver.isEmpty() || mSourceMessage.isEmpty())
        return;

    if (receiverName != mSourceReceiver)
        return;

    if (receiverName != mSourceMessage)
        return;

    mLastReceivedValues = values;
    mHasReceivedSource = true;
}

void PeriodicSenderBehaviour::onTimerTick()
{
    if (!mSender)
        return;

    const auto& values = mHasReceivedSource ? mLastReceivedValues : mDefaultValues;

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, values);
    if (frame.isEmpty())
        return;

    mSender->send(frame);
}
