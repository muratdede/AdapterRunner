#include "messageparser.h"

#include "Utils.h"

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

ParsedMessage MessageParser::parseFrame(const QByteArray& frame)
{
    ParsedMessage result;

    const HeaderDef* header = mSchema->findHeader(frame);

    if (!header)
        return result;

    ParsedHeader h = parseHeader(frame, *header);

    int msgId = h.values["msg_id"].toInt();

    const MessageDef* msgDef =
        mSchema->getMessage(header->type, msgId);

    if (!msgDef)
        return result;

    QByteArray payload = frame.mid(header->headerSize);

    result.name = msgDef->name;

    QMap<QString, QVariant> parsedFields;

    for (const FieldDef& field : msgDef->fields)
    {
        int arrayLen = field.arrayLength;

        if (!field.arrayLengthField.isEmpty())
            arrayLen = parsedFields[field.arrayLengthField].toInt();

        QVariant v;

        if (arrayLen > 0)
        {
            v = readArrayField(payload, field, msgDef->endian, arrayLen);
        }
        else
        {
            v = readSingleField(payload, field, msgDef->endian);
        }

        parsedFields[field.name] = v;
    }

    result.values = parsedFields;

    return result;
}

ParsedHeader MessageParser::parseHeader(const QByteArray &frame, const HeaderDef &header)
{
    ParsedHeader result;

    result.type = header.type;

    for (const FieldDef& field : header.fields)
    {
        QVariant v;

        int arrayLen = field.arrayLength;
        if (arrayLen > 0)
        {
            v = readArrayField(frame, field, header.endian, arrayLen);
        }
        else
        {
            v = readSingleField(frame, field, header.endian);
        }

        result.values[field.name] = v;
    }

    return result;
}

QVariant MessageParser::readSingleField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian)
{
    QSysInfo::Endian e = field.hasEndianOverride ? field.endian : msgEndian;

    switch (field.type)
    {
    case FieldType::UINT:
    {
        if (field.size == 1)
            return readValue<uint8_t>(data, field.byteOffset, e);

        if (field.size == 2)
            return readValue<uint16_t>(data, field.byteOffset, e);

        if (field.size == 4)
            return readValue<uint32_t>(data, field.byteOffset, e);

        if (field.size == 8)
            return readValue<uint64_t>(data, field.byteOffset, e);
    }

    case FieldType::INT:
    {
        if (field.size == 1)
            return readValue<int8_t>(data, field.byteOffset, e);

        if (field.size == 2)
            return readValue<int16_t>(data, field.byteOffset, e);

        if (field.size == 4)
            return readValue<int32_t>(data, field.byteOffset, e);

        if (field.size == 8)
            return readValue<int64_t>(data, field.byteOffset, e);
    }

    case FieldType::FLOAT:
        return readValue<float>(data, field.byteOffset, e);

    case FieldType::DOUBLE:
        return readValue<double>(data, field.byteOffset, e);

    case FieldType::BOOL:
        return bool(readValue<uint8_t>(data, field.byteOffset, e));
    }

    return {};
}

QVariant MessageParser::readArrayField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian, int length)
{
    QVariantList list;

    int elementSize = field.size;

    for (int i = 0; i < length; i++)
    {
        FieldDef tmp = field;

        tmp.byteOffset = field.byteOffset + i * elementSize;

        list.push_back(readSingleField(data, tmp, msgEndian));
    }

    return list;
}
