#include "messageparser.h"

uint64_t MessageParser::extractBits(const QByteArray &data, int byteOffset, int bitOffset, int bitLength)
{
    uint64_t value = 0;

    int totalBits = bitOffset + bitLength;

    int bytes = (totalBits + 7) / 8;

    for (int i = 0; i < bytes; i++)
    {
        value |= (uint64_t(uint8_t(data[byteOffset + i])) << (8 * i));
    }

    value >>= bitOffset;

    uint64_t mask = (1ULL << bitLength) - 1;

    return value & mask;
}

QVariant MessageParser::readField(const QByteArray &data, const FieldDef &field)
{
    if (field.bitOffset >= 0)
    {
        uint64_t v = extractBits(data,
                                 field.byteOffset,
                                 field.bitOffset,
                                 field.bitLength);

        return QVariant::fromValue(v);
    }

    const char* ptr = data.constData() + field.byteOffset;

    switch (field.type)
    {
    case FieldType::UINT:
    {
        if (field.size == 1)
            return *(uint8_t*)ptr;

        if (field.size == 2)
            return *(uint16_t*)ptr;

        if (field.size == 4)
            return *(uint32_t*)ptr;

        if (field.size == 8)
            return *(uint64_t*)ptr;
    } break;

    case FieldType::INT:
    {
        if (field.size == 1)
            return *(int8_t*)ptr;

        if (field.size == 2)
            return *(int16_t*)ptr;

        if (field.size == 4)
            return *(int32_t*)ptr;

        if (field.size == 8)
            return *(int64_t*)ptr;
    } break;

    case FieldType::FLOAT:
        return *(float*)ptr;

    case FieldType::DOUBLE:
        return *(double*)ptr;

    case FieldType::BOOL:
        return bool(*(uint8_t*)ptr);
    }

    return {};
}

ParsedMessage MessageParser::MessageParser::parseFrame(const QByteArray &frame)
{
    ParsedMessage result;

    const HeaderDef* header = mSchema->findHeader(frame);

    if (!header)
        return result;

    ParsedHeader h = parseHeader(frame, *header);

    int msgId = h.values["msg_id"].toInt();

    const MessageDef* msgDef = mSchema->getMessage(header->type, msgId);

    if (!msgDef)
        return result;

    QByteArray payload = frame.mid(header->headerSize);

    result.name = msgDef->name;

    for (const FieldDef& field : msgDef->fields)
    {
        QVariant v = readField(payload, field);

        result.values[field.name] = v;
    }

    return result;
}

ParsedHeader MessageParser::parseHeader(const QByteArray &frame, const HeaderDef &header)
{
    ParsedHeader result;

    result.type = header.type;

    for (const FieldDef& field : header.fields)
    {
        QVariant v = readField(frame, field);

        result.values[field.name] = v;
    }

    return result;
}
