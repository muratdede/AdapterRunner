#ifndef PERIODICSENDERBEHAVIOUR_H
#define PERIODICSENDERBEHAVIOUR_H

#include <QTimer>
#include <QVector>
#include <QJsonObject>

#include "src/transport/interfaces/isender.h"
#include "src/protocol/serializer/messageserializer.h"
#include "src/behavior/interfaces/ibehaviour.h"
#include "src/engine/tracker/sourcetracker.h"
#include "src/protocol/schema/fieldmapping.h"
#include "src/engine/evaluator/expressioneval.h"

class PeriodicSenderBehaviour : public IBehaviour
{
    Q_OBJECT
public:
    PeriodicSenderBehaviour(const QJsonObject& config, ISender* sender, MessageSerializer* serializer, QObject* parent = nullptr);

    void onMessageReceived(const QString& receiverName, const QString& messageName, const QMap<QString, QVariant>& values) override;

    QStringList requiredReceivers() const override;

private slots:
    void onTimerTick();

private:
    QMap<QString, QVariant> buildMergedValues();
    void applyMappings(const QVector<FieldMapping>& mappings, const QMap<QString, QVariant>& baseValues, QMap<QString, QVariant>& targetMap);
    bool hasAnyAliveSource() const;

    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;
    QTimer* mPeriodicTimer;

    QVector<SourceTracker*> mTrackers;
    QMap<QString, SourceTracker*> mTrackerByName;  // lookup by message name
    QVector<FieldMapping> mMappings;
    ExpressionEvaluator mEvaluator;
};

#endif // PERIODICSENDERBEHAVIOUR_H
