#ifndef MESSAGESERIALIZER_H
#define MESSAGESERIALIZER_H

#include <QMap>
#include <QVariant>

#include "src/protocol/schema/protocolschema.h"

class MessageSerializer
{
public:
    MessageSerializer(const ProtocolSchema* schema);

    QByteArray buildFrame(const QString& messageName, const QMap<QString, QVariant>& values);

private:
    QByteArray buildPayload(const AbstractMessage &msgDef, const QMap<QString, QVariant> &values);

    const ProtocolSchema* mSchema;
};

#endif // MESSAGESERIALIZER_H
