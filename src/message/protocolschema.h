#ifndef PROTOCOLSCHEMA_H
#define PROTOCOLSCHEMA_H

#include "defs.h"

#include <QHash>
#include <QJsonObject>

class ProtocolSchema
{
public:

    bool load(const QString &path);

    const HeaderDef* findHeader(const QByteArray& data) const;
    const MessageDef* getMessage(const QString& headerType, int msgId) const;

private:
    QSysInfo::Endian parseEndianness(const QString& str);
    FieldType parseFieldType(const QString& type);
    int parseTypeSize(const QString& type);
    FieldDef parseField(const QJsonObject& f);

    QHash<QString, HeaderDef> mHeaders;
    QHash<QString, QHash<int, MessageDef>> mMessages;
};

#endif // PROTOCOLSCHEMA_H
