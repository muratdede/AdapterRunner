#include "respondwithlastbehaviour.h"

#include <QJsonArray>
#include <QDebug>

static QMap<QString, QVariant> parseDefaults(const QJsonObject& obj)
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

RespondWithLastBehaviour::RespondWithLastBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mTriggerMessage(config["trigger"].toString())
    , mReceiverName(config["receiver"].toString())
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mSourceMessage(config["source_message"].toString())
    , mSourceReceiver(config["source_receiver"].toString())
    , mDefaultValues(parseDefaults(config["default_values"].toObject()))
    , mHasReceivedSource(false)
    , mWatchdog(nullptr)
    , mPeriodMs(config["period_ms"].toInt(0))
    , mOnTimeout(config["on_timeout"].toString("stop"))
    , mActive(true)
{
    if (mPeriodMs > 0)
    {
        mWatchdog = new QTimer(this);
        mWatchdog->setSingleShot(true);
        mWatchdog->setInterval(mPeriodMs);
        connect(mWatchdog, &QTimer::timeout, this, &RespondWithLastBehaviour::onWatchdogTimeout);
        mWatchdog->start();
    }

    qDebug() << "RespondWithLastBehaviour: trigger=" << mTriggerMessage
             << "response=" << mResponseMessage
             << "source=" << mSourceMessage << "on" << mSourceReceiver
             << "period=" << mPeriodMs << "ms";
}

void RespondWithLastBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    // Track source message
    if (receiverName == mSourceReceiver && messageName == mSourceMessage)
    {
        mLastReceivedValues = values;
        mHasReceivedSource = true;
    }

    // Check trigger
    if (receiverName == mReceiverName && messageName == mTriggerMessage)
    {
        mActive = true;
        resetWatchdog();
        sendResponse();
    }
}

void RespondWithLastBehaviour::onWatchdogTimeout()
{
    qDebug() << "RespondWithLastBehaviour: timeout" << mTriggerMessage << "->" << mResponseMessage
             << "on_timeout:" << mOnTimeout;

    if (mOnTimeout == "send_defaults")
    {
        sendResponse();
    }
    else
    {
        mActive = false;
    }
}

void RespondWithLastBehaviour::sendResponse()
{
    if (!mActive || !mSender)
        return;

    const auto& values = mHasReceivedSource ? mLastReceivedValues : mDefaultValues;

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, values);
    if (frame.isEmpty())
        return;

    mSender->send(frame);
    qDebug() << "RespondWithLastBehaviour: sent" << mResponseMessage
             << "(" << frame.size() << "bytes)"
             << (mHasReceivedSource ? "[last received]" : "[defaults]");
}

void RespondWithLastBehaviour::resetWatchdog()
{
    if (mWatchdog)
        mWatchdog->start();
}
