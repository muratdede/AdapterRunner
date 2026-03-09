#ifndef PERIODICSENDERBEHAVIOUR_H
#define PERIODICSENDERBEHAVIOUR_H

#include "ibehaviour.h"

#include <QTimer>
#include <QVector>
#include <QJsonObject>

#include "src/conn/transport/isender.h"
#include "src/message/messageserializer.h"

struct SourceEntry
{
    QString message;
    QString receiver;
    int timeoutMs = 0;
    QMap<QString, QVariant> defaultValues;

    // Runtime state
    QMap<QString, QVariant> lastReceivedValues;
    QTimer* watchdog = nullptr;
    bool alive = false;
};

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

    /// Returns the list of receiver names this behaviour needs
    QStringList requiredReceivers() const;

private slots:
    void onTimerTick();
    void onSourceTimeout(int sourceIndex);

private:
    QMap<QString, QVariant> buildMergedValues() const;
    bool hasAnyAliveSource() const;

    QString mResponseMessage;
    ISender* mSender;
    MessageSerializer* mSerializer;
    QTimer* mPeriodicTimer;
    QVector<SourceEntry> mSources;
};

#endif // PERIODICSENDERBEHAVIOUR_H
