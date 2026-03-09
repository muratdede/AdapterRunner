#include "respondbehaviour.h"

#include <QJsonArray>
#include <QDebug>

static QMap<QString, QVariant> parseDefaultValues(const QJsonObject& obj)
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

RespondBehaviour::RespondBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mTriggerMessage(config["trigger"].toString())
    , mReceiverName(config["receiver"].toString())
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mDefaultValues(parseDefaultValues(config["default_values"].toObject()))
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
        connect(mWatchdog, &QTimer::timeout, this, &RespondBehaviour::onWatchdogTimeout);
        mWatchdog->start();
    }

    qDebug() << "RespondBehaviour: trigger=" << mTriggerMessage
             << "response=" << mResponseMessage
             << "period=" << mPeriodMs << "ms";
}

void RespondBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    Q_UNUSED(values)

    if (receiverName != mReceiverName || messageName != mTriggerMessage)
        return;

    mActive = true;
    resetWatchdog();
    sendResponse();
}

void RespondBehaviour::onWatchdogTimeout()
{
    qDebug() << "RespondBehaviour: timeout" << mTriggerMessage << "->" << mResponseMessage
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

void RespondBehaviour::sendResponse()
{
    if (!mActive || !mSender)
        return;

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, mDefaultValues);
    if (frame.isEmpty())
        return;

    mSender->send(frame);
    qDebug() << "RespondBehaviour: sent" << mResponseMessage << "(" << frame.size() << "bytes)";
}

void RespondBehaviour::resetWatchdog()
{
    if (mWatchdog)
        mWatchdog->start();
}
