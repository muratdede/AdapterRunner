#ifndef REACTIONENGINE_H
#define REACTIONENGINE_H

#include <QObject>
#include <QVector>
#include <QSet>

#include "ibehaviour.h"
#include "src/conn/transport/transportmanager.h"
#include "src/message/messageparser.h"
#include "src/message/messageserializer.h"

class ReactionEngine : public QObject
{
    Q_OBJECT
public:
    ReactionEngine(TransportManager* tm,
                   MessageParser* parser,
                   MessageSerializer* serializer,
                   QObject* parent = nullptr);

    void loadReactions(const QJsonArray& reactions);

private slots:
    void onReceiverData(const QByteArray& data, const QString& receiverName);

private:
    IBehaviour* createBehaviour(const QJsonObject& config);

    TransportManager* mTransportManager;
    MessageParser* mParser;
    MessageSerializer* mSerializer;
    QVector<IBehaviour*> mBehaviours;
    QSet<QString> mConnectedReceivers;
};

#endif // REACTIONENGINE_H
