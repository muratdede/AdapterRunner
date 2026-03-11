#include "src/behavior/impls/respondbehaviour.h"
#include "src/core/jsonutils.h"

#include <QDebug>

RespondBehaviour::RespondBehaviour(const QJsonObject &config, ISender *sender, MessageSerializer *serializer, QObject *parent)
    : IBehaviour(parent)
    , mTriggerMessage(config["trigger"].toString())
    , mReceiverName(config["receiver"].toString())
    , mResponseMessage(config["response"].toString())
    , mSender(sender)
    , mSerializer(serializer)
    , mDefaultValues(JsonUtils::parseValues(config["default_values"].toObject()))
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
        mWatchdog->setTimerType(Qt::PreciseTimer);
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
    if (mWatchdog) mWatchdog->start();
    sendResponse();
}

void RespondBehaviour::onWatchdogTimeout()
{
    if (mOnTimeout == "send_defaults")
        sendResponse();
    else
        mActive = false;
}

void RespondBehaviour::sendResponse()
{
    if (!mActive || !mSender)
        return;

    QByteArray frame = mSerializer->buildFrame(mResponseMessage, mDefaultValues);
    if (!frame.isEmpty())
        mSender->send(frame);
}

QStringList RespondBehaviour::requiredReceivers() const
{
    return {mReceiverName};
}
