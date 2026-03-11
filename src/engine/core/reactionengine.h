#ifndef REACTIONENGINE_H
#define REACTIONENGINE_H

#include <QObject>
#include <QSet>
#include <QVector>

#include "src/behavior/interfaces/ibehaviour.h"
#include "src/transport/manager/transportmanager.h"
#include "src/protocol/parser/messageparser.h"
#include "src/protocol/serializer/messageserializer.h"

class ReactionEngine : public QObject {
  Q_OBJECT
public:
  ReactionEngine(TransportManager *tm, MessageParser *parser, MessageSerializer *serializer, QObject *parent = nullptr);

  void loadFromFile(const QString &messagesJsonPath);

private slots:
  void onReceiverData(const QByteArray &data, const QString &receiverName);

private:
  IBehaviour *createBehaviour(const QJsonObject &config);
  void connectReceiver(const QString &receiverName);

  TransportManager *mTransportManager;
  MessageParser *mParser;
  MessageSerializer *mSerializer;
  QVector<IBehaviour *> mBehaviours;
  QSet<QString> mConnectedReceivers;
};

#endif // REACTIONENGINE_H
