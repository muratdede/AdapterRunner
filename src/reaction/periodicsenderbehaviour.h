#ifndef PERIODICSENDERBEHAVIOUR_H
#define PERIODICSENDERBEHAVIOUR_H

#include "ibehaviour.h"
#include "sourcetracker.h"
#include "fieldmapping.h"
#include "expressioneval.h"

#include <QTimer>
#include <QVector>
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

    QStringList requiredReceivers() const override;

private slots:
    void onTimerTick();

private:
    QMap<QString, QVariant> buildMergedValues();
    bool hasAnyAliveSource() const;

    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;
    QTimer* mPeriodicTimer;

    QVector<SourceTracker*> mTrackers;
    QVector<QVector<FieldMapping>> mMappings;  // per-tracker mappings
    ExpressionEvaluator mEvaluator;
};

#endif // PERIODICSENDERBEHAVIOUR_H
