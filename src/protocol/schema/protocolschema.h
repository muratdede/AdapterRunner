#ifndef PROTOCOLSCHEMA_H
#define PROTOCOLSCHEMA_H

#include <QHash>
#include <QJsonObject>
#include <memory>

#include "src/protocol/types/defs.h"

class ProtocolSchema
{
public:

    bool load(const QString &path);

    std::shared_ptr<HeaderDef> findHeader(const QByteArray& data) const;
    std::shared_ptr<HeaderDef> getHeader(const QString& name) const;

    std::shared_ptr<MessageDef> getMessageById(int msgId) const;
    std::shared_ptr<MessageDef> getMessageByName(const QString& name) const;

private:
    bool matchesHeader(const QByteArray& data, std::shared_ptr<HeaderDef> header) const;

    QSysInfo::Endian parseEndianness(const QString& str);
    FieldType parseFieldType(const QString& type);
    int parseTypeSize(const QString& type);
    
    // Parses a basic primitive field, or creates a placeholder MessageDef for nested types
    std::shared_ptr<AbstractField> parseField(const QJsonObject& f, QSysInfo::Endian defaultEndian);

    void resolveCompositeFields(std::shared_ptr<MessageDef> message);

    QHash<QString, std::shared_ptr<HeaderDef>> mHeaders;
    QHash<QString, std::shared_ptr<AbstractMessage>> mTypesByName;
    QHash<int, std::shared_ptr<MessageDef>> mMessagesById;
    QHash<QString, std::shared_ptr<MessageDef>> mMessagesByName;
};

#endif // PROTOCOLSCHEMA_H
