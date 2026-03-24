#ifndef RESPONDWITHLASTBEHAVIOUR_H
#define RESPONDWITHLASTBEHAVIOUR_H

#include <QTimer>
#include <QJsonObject>

#include "src/transport/interfaces/isender.h"
#include "src/protocol/serializer/messageserializer.h"
#include "src/behavior/interfaces/ibehaviour.h"
#include "src/engine/tracker/sourcetracker.h"

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
