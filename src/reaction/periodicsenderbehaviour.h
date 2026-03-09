#ifndef PERIODICSENDERBEHAVIOUR_H
#define PERIODICSENDERBEHAVIOUR_H

#include "ibehaviour.h"

#include <QTimer>
#include <QJsonObject>

#include "src/conn/transport/isender.h"
#include "src/message/messageserializer.h"

class PeriodicSenderBehaviour : public IBehaviour
{
    Q_OBJECT
public:
    PeriodicSenderBehaviour(const QJsonObject& config,
                            ISender* sender,
                            MessageSerializer* serializer,
                            QObject* parent = nullptr);

    void onMessageReceived(const QString& receiverName,
                           const QString& messageName,
                           const QMap<QString, QVariant>& values) override;

private slots:
    void onTimerTick();

private:
    // Source tracking
    QString mSourceMessage;
    QString mSourceReceiver;
    QString mResponseMessage;
    QMap<QString, QVariant> mLastReceivedValues;
    QMap<QString, QVariant> mDefaultValues;
    bool mHasReceivedSource;

    ISender* mSender;
    MessageSerializer* mSerializer;
    QTimer* mPeriodicTimer;
};

#endif // PERIODICSENDERBEHAVIOUR_H
