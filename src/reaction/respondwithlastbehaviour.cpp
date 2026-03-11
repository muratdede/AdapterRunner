#include "respondwithlastbehaviour.h"
#include "src/common/jsonutils.h"

#include <QDebug>

RespondWithLastBehaviour::RespondWithLastBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mTriggerMessage(config["trigger"].toString())
    , mReceiverName(config["receiver"].toString())
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mSourceTracker(nullptr)
    , mWatchdog(nullptr)
    , mPeriodMs(config["period_ms"].toInt(0))
    , mOnTimeout(config["on_timeout"].toString("stop"))
    , mActive(true)
{
    // Create source tracker
    auto defaults = JsonUtils::parseValues(config["default_values"].toObject());
    mSourceTracker = new SourceTracker(
        config["message"].toString(),
        config["receiver"].toString(),
        config["timeout_ms"].toInt(0),
        config["mappings"].toArray(),
        defaults,
        this
    );

    // Watchdog for trigger message
    if (mPeriodMs > 0)
    {
        mWatchdog = new QTimer(this);
        mWatchdog->setSingleShot(true);
        mWatchdog->setInterval(mPeriodMs);
        mWatchdog->setTimerType(Qt::PreciseTimer);
        connect(mWatchdog, &QTimer::timeout, this, &RespondWithLastBehaviour::onWatchdogTimeout);
        mWatchdog->start();
    }

    qDebug() << "RespondWithLastBehaviour: trigger=" << mTriggerMessage
             << "response=" << mResponseMessage
             << "source=" << mSourceTracker->messageName()
             << "period=" << mPeriodMs << "ms";
}

void RespondWithLastBehaviour::onMessageReceived(const QString &receiverName, const QString &messageName, const QMap<QString, QVariant> &values)
{
    // Feed source tracker
    if (receiverName == mSourceTracker->receiverName() && messageName == mSourceTracker->messageName())
        mSourceTracker->feed(values);

    // Check trigger
    if (receiverName == mReceiverName && messageName == mTriggerMessage)
    {
        mActive = true;
        if (mWatchdog) mWatchdog->start();
        sendResponse();
    }
}

void RespondWithLastBehaviour::onWatchdogTimeout()
{
    if (mOnTimeout == "send_defaults")
        sendResponse();
    else
        mActive = false;
}

void RespondWithLastBehaviour::sendResponse()
{
    if (!mActive || !mSender)
        return;

    const auto& values = mSourceTracker->currentValues();
    QByteArray frame = mSerializer->buildFrame(mResponseMessage, values);
    if (!frame.isEmpty())
        mSender->send(frame);
}

QStringList RespondWithLastBehaviour::requiredReceivers() const
{
    QStringList list = {mReceiverName};
    QString sourceReceiver = mSourceTracker->receiverName();
    if (!sourceReceiver.isEmpty() && !list.contains(sourceReceiver))
        list.append(sourceReceiver);
    return list;
}
