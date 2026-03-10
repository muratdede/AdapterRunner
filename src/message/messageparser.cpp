#include "messageparser.h"
#include "checksumfactory.h"

#include "Utils.h"

#include <QDebug>

// TODO 4 bytelık chunk parcalama da eklenmeli
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

    const MessageDef* msgDef = mSchema->getMessage(header->type, msgId);

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

    // Verify computed fields (e.g. checksum)
    for (const FieldDef& field : msgDef->fields)
    {
        if (!field.compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field.compute.algorithm);
        if (!algo)
            continue;

        uint8_t received = static_cast<uint8_t>(parsedFields[field.name].toUInt());

        // Build the buffer based on scope
        QByteArray computeBuffer;
        int fieldAbsoluteOffset;

        if (field.compute.scope == "frame")
        {
            computeBuffer = frame;
            fieldAbsoluteOffset = header->headerSize + field.byteOffset;
        }
        else if (field.compute.scope == "header")
        {
            computeBuffer = frame.left(header->headerSize);
            fieldAbsoluteOffset = field.byteOffset;
        }
        else  // "payload" (default)
        {
            computeBuffer = payload;
            fieldAbsoluteOffset = field.byteOffset;
        }

        int rangeStart = field.compute.rangeStart;
        int rangeEnd = field.compute.rangeEnd;
        int exclOffset = field.compute.excludeSelf ? fieldAbsoluteOffset : -1;
        int exclSize = field.compute.excludeSelf ? field.size : 0;

        if (!algo->verify(computeBuffer, rangeStart, rangeEnd, exclOffset, exclSize, received))
        {
            uint8_t expected = algo->compute(computeBuffer, rangeStart, rangeEnd, exclOffset, exclSize);
            qWarning() << "MessageParser: checksum mismatch for" << result.name
                       << "- expected:" << expected << "received:" << received;

            if (field.compute.onMismatch == "drop")
                return ParsedMessage();
        }
    }

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
    if (field.bitOffset >= 0)
    {
        uint64_t v = extractBits(data,
                                 field.byteOffset,
                                 field.bitOffset,
                                 field.bitLength);

        if (field.type == FieldType::BOOL) {
            return QVariant::fromValue(bool(v));
        }

        return QVariant::fromValue(v);
    }

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
