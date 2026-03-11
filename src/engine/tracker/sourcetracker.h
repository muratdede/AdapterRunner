#ifndef SOURCETRACKER_H
#define SOURCETRACKER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVariant>

#include "src/protocol/schema/fieldmapping.h"

class SourceTracker : public QObject
{
    Q_OBJECT
public:
    SourceTracker(const QString& message, const QString& receiver,
                  int timeoutMs, const QJsonArray& jsonArray, const QMap<QString, QVariant>& defaults,
                  QObject* parent = nullptr);

    QString messageName() const;
    QString receiverName() const;
    bool isAlive() const;

    /// Called by the owning behaviour when a matching message arrives
    void feed(const QMap<QString, QVariant>& values);

    /// Returns last received values if alive, defaults if timed out
    const QMap<QString, QVariant>& currentValues() const;

    const QVector<FieldMapping>& mappings() const;

signals:
    void timedOut();

private slots:
    void onWatchdogTimeout();

private:
    QString mMessage;
    QString mReceiver;
    QMap<QString, QVariant> mDefaults;
    QMap<QString, QVariant> mLastValues;
    QVector<FieldMapping> mMappings;
    QTimer* mWatchdog;
    bool mAlive;
};

#endif // SOURCETRACKER_H
