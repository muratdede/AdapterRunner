#ifndef IBEHAVIOUR_H
#define IBEHAVIOUR_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QStringList>

class IBehaviour : public QObject
{
    Q_OBJECT
public:
    explicit IBehaviour(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IBehaviour() = default;

    /// Called by ReactionEngine when a message is received on any connected receiver
    virtual void onMessageReceived(const QString& receiverName,
                                   const QString& messageName,
                                   const QMap<QString, QVariant>& values) = 0;

    /// Returns all receiver names this behaviour needs to be connected to
    virtual QStringList requiredReceivers() const = 0;
};

#endif // IBEHAVIOUR_H
