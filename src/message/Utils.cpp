#include "Utils.h"


QVariant Utils::readField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian, int arrayLen)
{
    QVariant v;

    if (arrayLen > 0)
    {
        v = readArrayField(data, field, field.endian, arrayLen);
    }
    else
    {
        v = readSingleField(data, field, field.endian);
    }

    return v;
}

void Utils::writeField(QByteArray &buffer, const FieldDef &field, const QVariant &value, QSysInfo::Endian endian)
{
    if (field.arrayLength != 0 || !field.arrayLengthField.isEmpty())
        writeArrayField(buffer, field, value, endian);
    else
        writeSingleField(buffer, field, value, endian);
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

QVariant Utils::readSingleField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian)
{
    if (field.bitOffset >= 0)
    {
        uint64_t v = extractBits(data, field.byteOffset, field.bitOffset, field.bitLength);

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

QVariant Utils::readArrayField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian, int length)
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


void Utils::writeSingleField(QByteArray &buffer, const FieldDef &field, const QVariant &value, QSysInfo::Endian endian)
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

void Utils::writeArrayField(QByteArray &buffer, const FieldDef &field, const QVariant &value, QSysInfo::Endian endian)
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
