#ifndef RESPONDWITHLASTBEHAVIOUR_H
#define RESPONDWITHLASTBEHAVIOUR_H

#include "ibehaviour.h"

#include <QTimer>
#include <QJsonObject>

#include "src/conn/transport/isender.h"
#include "src/message/messageserializer.h"

class RespondWithLastBehaviour : public IBehaviour
{
    Q_OBJECT
public:
    RespondWithLastBehaviour(const QJsonObject& config,
                             ISender* sender,
                             MessageSerializer* serializer,
                             QObject* parent = nullptr);

    void onMessageReceived(const QString& receiverName,
                           const QString& messageName,
                           const QMap<QString, QVariant>& values) override;

private slots:
    void onWatchdogTimeout();

private:
    void sendResponse();
    void resetWatchdog();

    // Trigger config
    QString mTriggerMessage;
    QString mReceiverName;

    // Response config
    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;

    // Source tracking
    QString mSourceMessage;
    QString mSourceReceiver;
    QMap<QString, QVariant> mLastReceivedValues;
    QMap<QString, QVariant> mDefaultValues;
    bool mHasReceivedSource;

    // Watchdog
    QTimer* mWatchdog;
    int mPeriodMs;
    QString mOnTimeout;
    bool mActive;
};

#endif // RESPONDWITHLASTBEHAVIOUR_H
