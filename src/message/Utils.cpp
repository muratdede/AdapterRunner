#include "Utils.h"
#include "checksumfactory.h"

#include <QDebug>

QVariant Utils::readField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian parentEndian, int arrayLen, int baseOffset)
{
    QVariant value;

    if (arrayLen > 0)
    {
        value = readArrayField(data, field, parentEndian, arrayLen, baseOffset);
    }
    else
    {
        value = readSingleField(data, field, parentEndian, baseOffset);
    }

    return value;
}

void Utils::writeField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian parentEndian, int baseOffset)
{
    if (field->arrayLength != 0 || !field->arrayLengthField.isEmpty())
        writeArrayField(buffer, field, value, parentEndian, baseOffset);
    else
        writeSingleField(buffer, field, value, parentEndian, baseOffset);
}

uint64_t Utils::extractBits(const QByteArray &data, int byteOffset, int bitOffset, int bitLength)
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

QVariant Utils::readArrayField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian parentEndian, int length, int baseOffset)
{
    QVariantList list;

    int elementSize = field->getSize();

    for (int i = 0; i < length; i++)
    {
        QVariant value = readSingleField(data, field, parentEndian, baseOffset + i * elementSize);
        if (value.isNull())
            return {};

        list.push_back(value);
    }

    return list;
}

QVariant Utils::readSingleField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian parentEndian, int baseOffset)
{
    int absoluteOffset = baseOffset + field->byteOffset;

    if (field->isMessage())
    {
        auto msg = static_cast<const MessageDef*>(field);
        QVariantMap map;
        for (const auto& child : msg->fields)
        {
            int length = child->arrayLength;
            if (!child->arrayLengthField.isEmpty())
                length = map[child->arrayLengthField].toInt();

            QVariant value = readField(data, child.get(), msg->endian, length, absoluteOffset);
            if (value.isNull())
                return {};

            map[child->name] = value;
        }
        return map;
    }

    auto fieldDef = static_cast<const FieldDef*>(field);

    if (fieldDef->bitOffset >= 0)
    {
        uint64_t v = extractBits(data, absoluteOffset, fieldDef->bitOffset, fieldDef->bitLength);

        if (fieldDef->type == FieldType::BOOL) {
            return QVariant::fromValue(bool(v));
        }

        return QVariant::fromValue(v);
    }

    QVariant value;
    QSysInfo::Endian endian = fieldDef->hasEndianOverride ? fieldDef->endian : parentEndian;
    switch (fieldDef->type)
    {
    case FieldType::UINT:
    {
        if (fieldDef->size == 1)
            value = readValue<uint8_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 2)
            value = readValue<uint16_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 4)
            value = readValue<uint32_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 8)
            value = readValue<uint64_t>(data, absoluteOffset, endian);
    }
    break;

    case FieldType::INT:
    {
        if (fieldDef->size == 1)
            value = readValue<int8_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 2)
            value = readValue<int16_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 4)
            value = readValue<int32_t>(data, absoluteOffset, endian);
        if (fieldDef->size == 8)
            value = readValue<int64_t>(data, absoluteOffset, endian);
    }
    break;

    case FieldType::FLOAT:
        value = readValue<float>(data, absoluteOffset, endian);
    case FieldType::DOUBLE:
        value = readValue<double>(data, absoluteOffset, endian);
    case FieldType::BOOL:
        value = bool(readValue<uint8_t>(data, absoluteOffset, endian));
    }

    if (fieldDef->compute.hasCompute())
    {
        const IChecksumAlgorithm* algo = ChecksumFactory::create(fieldDef->compute.algorithm);
        if (algo)
        {
            auto expected = algo->compute(data, baseOffset, *fieldDef);
            if (expected != value)
            {
                qWarning() << "MessageParser: compute mismatch for" << fieldDef->name
                           << "- expected:" << expected << "received:" << value;

                if (fieldDef->compute.onMismatch == "drop")
                    return {};
            }
        }
    }

    return value;
}

void Utils::writeArrayField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian parentEndian, int baseOffset)
{
    QVariantList list = value.toList();

    int elementSize = field->getSize();

    for (int i = 0; i < list.size(); i++)
    {
        writeSingleField(buffer, field, list[i], parentEndian, baseOffset + i * elementSize);
    }
}

void Utils::writeSingleField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian parentEndian, int baseOffset)
{
    int absoluteOffset = baseOffset + field->byteOffset;

    if (field->isMessage())
    {
        auto messageDef = static_cast<const MessageDef*>(field);
        QVariantMap valueMap = value.toMap();
        for (const auto& child : messageDef->fields)
        {
            writeField(buffer, child.get(), valueMap.value(child->name), messageDef->endian, absoluteOffset);
        }
        return;
    }

    auto fieldDef = static_cast<const FieldDef*>(field);

    // Ensure buffer is large enough
    int needed = absoluteOffset + fieldDef->size;
    if (buffer.size() < needed)
        buffer.resize(needed);

    QSysInfo::Endian endian = fieldDef->hasEndianOverride ? fieldDef->endian : parentEndian;
    switch (fieldDef->type)
    {
    case FieldType::UINT:
    {
        if (fieldDef->size == 1) writeValue<uint8_t>(buffer, absoluteOffset, value.toUInt(), endian);
        else if (fieldDef->size == 2) writeValue<uint16_t>(buffer, absoluteOffset, value.toUInt(), endian);
        else if (fieldDef->size == 4) writeValue<uint32_t>(buffer, absoluteOffset, value.toUInt(), endian);
        else if (fieldDef->size == 8) writeValue<uint64_t>(buffer, absoluteOffset, value.toULongLong(), endian);
        break;
    }

    case FieldType::INT:
    {
        if (fieldDef->size == 1) writeValue<int8_t>(buffer, absoluteOffset, value.toInt(), endian);
        else if (fieldDef->size == 2) writeValue<int16_t>(buffer, absoluteOffset, value.toInt(), endian);
        else if (fieldDef->size == 4) writeValue<int32_t>(buffer, absoluteOffset, value.toInt(), endian);
        else if (fieldDef->size == 8) writeValue<int64_t>(buffer, absoluteOffset, value.toLongLong(), endian);
        break;
    }

    case FieldType::FLOAT:
        writeValue<float>(buffer, absoluteOffset, value.toFloat(), endian);
        break;

    case FieldType::DOUBLE:
        writeValue<double>(buffer, absoluteOffset, value.toDouble(), endian);
        break;

    case FieldType::BOOL:
        writeValue<uint8_t>(buffer, absoluteOffset, value.toBool() ? 1 : 0, endian);
        break;
    }
}
