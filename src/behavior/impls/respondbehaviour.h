#ifndef RESPONDBEHAVIOUR_H
#define RESPONDBEHAVIOUR_H

#include "src/behavior/interfaces/ibehaviour.h"

#include <QTimer>
#include <QJsonObject>

#include "src/transport/interfaces/isender.h"
#include "src/protocol/serializer/messageserializer.h"

class RespondBehaviour : public IBehaviour
{
    Q_OBJECT
public:
    RespondBehaviour(const QJsonObject& config, ISender* sender, MessageSerializer* serializer, QObject* parent = nullptr);

    void onMessageReceived(const QString& receiverName, const QString& messageName, const QMap<QString, QVariant>& values) override;

    QStringList requiredReceivers() const override;

private slots:
    void onWatchdogTimeout();

private:
    void sendResponse();

    QString mTriggerMessage;
    QString mReceiverName;
    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;
    QMap<QString, QVariant> mDefaultValues;
    QTimer* mWatchdog;
    int mPeriodMs;
    QString mOnTimeout;
    bool mActive;
};

#endif // RESPONDBEHAVIOUR_H
