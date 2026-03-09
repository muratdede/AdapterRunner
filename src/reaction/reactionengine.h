#ifndef REACTIONENGINE_H
#define REACTIONENGINE_H

#include <QObject>
#include <QSet>
#include <QVector>

#include "ibehaviour.h"
#include "src/conn/transport/transportmanager.h"
#include "src/message/messageparser.h"
#include "src/message/messageserializer.h"

class ReactionEngine : public QObject {
  Q_OBJECT
public:
  ReactionEngine(TransportManager *tm, MessageParser *parser,
                 MessageSerializer *serializer, QObject *parent = nullptr);

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
