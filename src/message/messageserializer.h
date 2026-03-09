#ifndef MESSAGESERIALIZER_H
#define MESSAGESERIALIZER_H

#include "protocolschema.h"

#include <QMap>
#include <QVariant>

class MessageSerializer
{
public:
    MessageSerializer(const ProtocolSchema* schema);

    QByteArray buildFrame(const QString& messageName,
                          const QMap<QString, QVariant>& values);

private:
    void writeSingleField(QByteArray& buffer, const FieldDef& field,
                          const QVariant& value, QSysInfo::Endian endian);

    void writeArrayField(QByteArray& buffer, const FieldDef& field,
                         const QVariant& value, QSysInfo::Endian endian);

    QByteArray buildHeader(const HeaderDef& header, int msgId, int payloadLength);

    template<typename T>
    void writeValue(QByteArray& buffer, int offset, T value, QSysInfo::Endian msgEndian);

    const ProtocolSchema* mSchema;
};

#endif // MESSAGESERIALIZER_H
