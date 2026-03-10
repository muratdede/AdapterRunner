#ifndef RESPONDWITHLASTBEHAVIOUR_H
#define RESPONDWITHLASTBEHAVIOUR_H

#include "ibehaviour.h"
#include "sourcetracker.h"

#include <QTimer>
#include <QJsonObject>

#include "src/conn/transport/isender.h"
#include "src/message/messageserializer.h"

class RespondWithLastBehaviour : public IBehaviour
{
    Q_OBJECT
public:
    RespondWithLastBehaviour(const QJsonObject& config, ISender* sender, MessageSerializer* serializer, QObject* parent = nullptr);

    void onMessageReceived(const QString& receiverName, const QString& messageName, const QMap<QString, QVariant>& values) override;

    QStringList requiredReceivers() const override;

private slots:
    void onWatchdogTimeout();

private:
    void sendResponse();

    // Trigger config
    QString mTriggerMessage;
    QString mReceiverName;

    // Response
    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;

    // Source tracking (delegated)
    SourceTracker* mSourceTracker;

    // Watchdog for trigger
    QTimer* mWatchdog;
    int mPeriodMs;
    QString mOnTimeout;
    bool mActive;
};

#endif // RESPONDWITHLASTBEHAVIOUR_H
