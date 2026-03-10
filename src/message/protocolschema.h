#ifndef PROTOCOLSCHEMA_H
#define PROTOCOLSCHEMA_H

#include "defs.h"

#include <QHash>
#include <QJsonObject>
#include <memory>

class ProtocolSchema
{
public:

    bool load(const QString &path);

    const HeaderDef* findHeader(const QByteArray& data) const;
    const MessageDef* getMessage(const QString& headerType, int msgId) const;

    const MessageDef* getMessageByName(const QString& name) const;
    const HeaderDef* getHeader(const QString& type) const;

    bool matchesHeader(const QByteArray& data, const HeaderDef& header) const;

private:
    QSysInfo::Endian parseEndianness(const QString& str);
    FieldType parseFieldType(const QString& type);
    int parseTypeSize(const QString& type);
    
    // Parses a basic primitive field, or creates a placeholder MessageDef for nested types
    std::shared_ptr<AbstractField> parseField(const QJsonObject& f, QSysInfo::Endian defaultEndian);

    // Resolves all composite fields (MessageDefs used as fields) using initialized templates
    void resolveCompositeFields();

    QHash<QString, HeaderDef> mHeaders;
    QHash<QString, QHash<int, std::shared_ptr<MessageDef>>> mMessages;
    QHash<QString, std::shared_ptr<MessageDef>> mMessagesByName;
};

#endif // PROTOCOLSCHEMA_H
