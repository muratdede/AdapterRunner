#ifndef PROTOCOLSCHEMA_H
#define PROTOCOLSCHEMA_H

#include "headerdef.h"
#include "messagedef.h"

#include <QHash>

class ProtocolSchema
{
public:

    bool load(QString path);

    const HeaderDef* findHeader(const QByteArray& data) const;

    const MessageDef* getMessage(const QString& headerType, int msgId) const;

private:

    QHash<QString, HeaderDef> mHeaders;

    QHash<QString, QHash<int, MessageDef>> mMessages;
};

#endif // PROTOCOLSCHEMA_H
