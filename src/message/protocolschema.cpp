#include "protocolschema.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

bool ProtocolSchema::load(QString path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // HEADERS

    for (auto headerVal : root["headers"].toArray())
    {
        QJsonObject obj = headerVal.toObject();

        HeaderDef header;

        header.type = obj["type"].toString();

        header.startBytes = QByteArray::fromHex(obj["start_bytes"].toString().toUtf8());

        header.headerSize = obj["header_size"].toInt();

        for (auto fieldVal : obj["fields"].toArray())
        {
            QJsonObject f = fieldVal.toObject();

            FieldDef field;

            field.name = f["name"].toString();

            QString type = f["type"].toString();

            if (type.startsWith("uint")) field.type = FieldType::UINT;
            else if (type.startsWith("int")) field.type = FieldType::INT;
            else if (type == "float") field.type = FieldType::FLOAT;
            else if (type == "double") field.type = FieldType::DOUBLE;
            else if (type == "bool") field.type = FieldType::BOOL;

            if (type.contains("8")) field.size = 1;
            if (type.contains("16")) field.size = 2;
            if (type.contains("32")) field.size = 4;
            if (type.contains("64")) field.size = 8;

            field.byteOffset = f["byte_offset"].toInt();

            if (f.contains("bit_offset"))
            {
                field.bitOffset = f["bit_offset"].toInt();
                field.bitLength = f["bit_length"].toInt();
            }

            header.fields.push_back(field);
        }

        mHeaders[header.type] = header;
    }

    // MESSAGES

    for (auto msgVal : root["messages"].toArray())
    {
        QJsonObject obj = msgVal.toObject();

        MessageDef msg;

        msg.headerType = obj["header"].toString();
        msg.messageId = obj["message_id"].toInt();
        msg.name = obj["name"].toString();

        for (auto fieldVal : obj["fields"].toArray())
        {
            QJsonObject f = fieldVal.toObject();

            FieldDef field;

            field.name = f["name"].toString();

            QString type = f["type"].toString();

            if (type.startsWith("uint")) field.type = FieldType::UINT;
            else if (type.startsWith("int")) field.type = FieldType::INT;
            else if (type == "float") field.type = FieldType::FLOAT;
            else if (type == "double") field.type = FieldType::DOUBLE;
            else if (type == "bool") field.type = FieldType::BOOL;

            if (type.contains("8")) field.size = 1;
            if (type.contains("16")) field.size = 2;
            if (type.contains("32")) field.size = 4;
            if (type.contains("64")) field.size = 8;

            field.byteOffset = f["byte_offset"].toInt();

            if (f.contains("bit_offset"))
            {
                field.bitOffset = f["bit_offset"].toInt();
                field.bitLength = f["bit_length"].toInt();
            }

            msg.fields.push_back(field);
        }

        mMessages[msg.headerType][msg.messageId] = msg;
    }

    return true;
}

const HeaderDef* ProtocolSchema::findHeader(const QByteArray& data) const
{
    for (auto& header : mHeaders)
    {
        if (data.startsWith(header.startBytes))
            return &header;
    }

    return nullptr;
}

const MessageDef* ProtocolSchema::getMessage(const QString& headerType, int msgId) const
{
    auto headerIt = mMessages.find(headerType);

    if (headerIt == mMessages.end())
        return nullptr;

    const auto& map = headerIt.value();

    auto msgIt = map.find(msgId);

    if (msgIt == map.end())
        return nullptr;

    return &msgIt.value();
}
