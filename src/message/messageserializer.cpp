#include "messageserializer.h"
#include "Utils.h"

#include <QDebug>
#include <cstring>

MessageSerializer::MessageSerializer(const ProtocolSchema *schema)
    : mSchema(schema)
{
}

template<typename T>
void MessageSerializer::writeValue(QByteArray &buffer, int offset, T value, QSysInfo::Endian msgEndian)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    bool systemLittle = true;
#else
    bool systemLittle = false;
#endif

    if ((msgEndian == QSysInfo::LittleEndian && !systemLittle) ||
        (msgEndian == QSysInfo::BigEndian && systemLittle))
    {
        value = swapEndian(value);
    }

    memcpy(buffer.data() + offset, &value, sizeof(T));
}

QByteArray MessageSerializer::buildHeader(const HeaderDef &header, int msgId, int payloadLength)
{
    QByteArray headerData(header.headerSize, '\0');

    // Write start bytes
    memcpy(headerData.data(), header.startBytes.constData(), header.startBytes.size());

    // Write header fields
    for (const FieldDef& field : header.fields)
    {
        QSysInfo::Endian e = field.hasEndianOverride ? field.endian : header.endian;

        if (field.name == "msg_id")
        {
            switch (field.size)
            {
            case 1: writeValue<uint8_t>(headerData, field.byteOffset, msgId, e); break;
            case 2: writeValue<uint16_t>(headerData, field.byteOffset, msgId, e); break;
            case 4: writeValue<uint32_t>(headerData, field.byteOffset, msgId, e); break;
            }
        }
        else if (field.name == "length")
        {
            switch (field.size)
            {
            case 1: writeValue<uint8_t>(headerData, field.byteOffset, payloadLength, e); break;
            case 2: writeValue<uint16_t>(headerData, field.byteOffset, payloadLength, e); break;
            case 4: writeValue<uint32_t>(headerData, field.byteOffset, payloadLength, e); break;
            }
        }
    }

    return headerData;
}

void MessageSerializer::writeSingleField(QByteArray &buffer, const FieldDef &field, const QVariant &value, QSysInfo::Endian endian)
{
    QSysInfo::Endian e = field.hasEndianOverride ? field.endian : endian;

    // Ensure buffer is large enough
    int needed = field.byteOffset + field.size;
    if (buffer.size() < needed)
        buffer.resize(needed);

    switch (field.type)
    {
    case FieldType::UINT:
    {
        if (field.size == 1) writeValue<uint8_t>(buffer, field.byteOffset, value.toUInt(), e);
        else if (field.size == 2) writeValue<uint16_t>(buffer, field.byteOffset, value.toUInt(), e);
        else if (field.size == 4) writeValue<uint32_t>(buffer, field.byteOffset, value.toUInt(), e);
        else if (field.size == 8) writeValue<uint64_t>(buffer, field.byteOffset, value.toULongLong(), e);
        break;
    }

    case FieldType::INT:
    {
        if (field.size == 1) writeValue<int8_t>(buffer, field.byteOffset, value.toInt(), e);
        else if (field.size == 2) writeValue<int16_t>(buffer, field.byteOffset, value.toInt(), e);
        else if (field.size == 4) writeValue<int32_t>(buffer, field.byteOffset, value.toInt(), e);
        else if (field.size == 8) writeValue<int64_t>(buffer, field.byteOffset, value.toLongLong(), e);
        break;
    }

    case FieldType::FLOAT:
        writeValue<float>(buffer, field.byteOffset, value.toFloat(), e);
        break;

    case FieldType::DOUBLE:
        writeValue<double>(buffer, field.byteOffset, value.toDouble(), e);
        break;

    case FieldType::BOOL:
        writeValue<uint8_t>(buffer, field.byteOffset, value.toBool() ? 1 : 0, e);
        break;
    }
}

void MessageSerializer::writeArrayField(QByteArray &buffer, const FieldDef &field, const QVariant &value, QSysInfo::Endian endian)
{
    QVariantList list = value.toList();

    int elementSize = field.size;

    for (int i = 0; i < list.size(); i++)
    {
        FieldDef tmp = field;
        tmp.byteOffset = field.byteOffset + i * elementSize;

        writeSingleField(buffer, tmp, list[i], endian);
    }
}

QByteArray MessageSerializer::buildFrame(const QString &messageName, const QMap<QString, QVariant> &values)
{
    const MessageDef* msgDef = mSchema->getMessageByName(messageName);

    if (!msgDef)
    {
        qWarning() << "MessageSerializer::buildFrame: unknown message" << messageName;
        return {};
    }

    const HeaderDef* header = mSchema->getHeader(msgDef->headerType);

    if (!header)
    {
        qWarning() << "MessageSerializer::buildFrame: unknown header type" << msgDef->headerType;
        return {};
    }

    // Build payload
    QByteArray payload;

    for (const FieldDef& field : msgDef->fields)
    {
        QVariant v = values.value(field.name);

        int arrayLen = field.arrayLength;

        if (!field.arrayLengthField.isEmpty())
            arrayLen = values.value(field.arrayLengthField).toInt();

        if (arrayLen > 0)
        {
            writeArrayField(payload, field, v, msgDef->endian);
        }
        else
        {
            writeSingleField(payload, field, v, msgDef->endian);
        }
    }

    // Build header with msg_id and payload length
    QByteArray headerData = buildHeader(*header, msgDef->messageId, payload.size());

    return headerData + payload;
}
