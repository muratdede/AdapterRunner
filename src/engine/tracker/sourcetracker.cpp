#include "sourcetracker.h"

#include <QDebug>

SourceTracker::SourceTracker(const QString &message, const QString &receiver,
                             int timeoutMs, const QJsonArray &jsonArray, const QMap<QString, QVariant> &defaults,
                             QObject *parent)
    : QObject(parent)
    , mMessage(message)
    , mReceiver(receiver)
    , mDefaults(defaults)
    , mLastValues(defaults)
    , mMappings(FieldMapping::fromJsonArray(jsonArray))
    , mWatchdog(nullptr)
    , mAlive(false)
{
    if (timeoutMs > 0)
    {
        mWatchdog = new QTimer(this);
        mWatchdog->setSingleShot(true);
        mWatchdog->setInterval(timeoutMs);
        mWatchdog->setTimerType(Qt::PreciseTimer);
        connect(mWatchdog, &QTimer::timeout, this, &SourceTracker::onWatchdogTimeout);
    }
}

QString SourceTracker::messageName() const
{
    return mMessage;
}

QString SourceTracker::receiverName() const
{
    return mReceiver;
}

bool SourceTracker::isAlive() const
{
    return mAlive;
}

void SourceTracker::feed(const QMap<QString, QVariant> &values)
{
    mLastValues = values;
    mAlive = true;

    if (mWatchdog)
        mWatchdog->start();
}

const QMap<QString, QVariant>& SourceTracker::currentValues() const
{
    return mAlive ? mLastValues : mDefaults;
}

const QVector<FieldMapping> &SourceTracker::mappings() const
{
    return mMappings;
}

void SourceTracker::onWatchdogTimeout()
{
    mAlive = false;
    emit timedOut();

    qDebug() << "SourceTracker: timeout -" << mMessage << "(falling back to defaults)";
}
