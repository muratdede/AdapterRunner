#include "Utils.h"


QVariant Utils::readField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian msgEndian, int arrayLen, int baseOffset)
{
    QVariant v;

    if (arrayLen > 0)
    {
        v = readArrayField(data, field, msgEndian, arrayLen, baseOffset);
    }
    else
    {
        v = readSingleField(data, field, msgEndian, baseOffset);
    }

    return v;
}

void Utils::writeField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian endian, int baseOffset)
{
    if (field->arrayLength != 0 || !field->arrayLengthField.isEmpty())
        writeArrayField(buffer, field, value, endian, baseOffset);
    else
        writeSingleField(buffer, field, value, endian, baseOffset);
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

QVariant Utils::readSingleField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian msgEndian, int baseOffset)
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

            map[child->name] = readField(data, child.get(), msg->endian, length, absoluteOffset);
        }
        return map;
    }

    auto fieldDef = static_cast<const FieldDef*>(field);

    if (fieldDef->bitOffset >= 0)
    {
        uint64_t v = extractBits(data, absoluteOffset, fieldDef->bitOffset, ffieldDef->bitLength);

        if (fieldDef->type == FieldType::BOOL) {
            return QVariant::fromValue(bool(v));
        }

        return QVariant::fromValue(v);
    }

    QSysInfo::Endian e = fieldDef->hasEndianOverride ? fieldDef->endian : msgEndian;

    switch (fieldDef->type)
    {
    case FieldType::UINT:
    {
        if (fieldDef->size == 1)
            return readValue<uint8_t>(data, absoluteOffset, e);

        if (fieldDef->size == 2)
            return readValue<uint16_t>(data, absoluteOffset, e);

        if (fieldDef->size == 4)
            return readValue<uint32_t>(data, absoluteOffset, e);

        if (fieldDef->size == 8)
            return readValue<uint64_t>(data, absoluteOffset, e);
    }
    break;

    case FieldType::INT:
    {
        if (fieldDef.size == 1)
            return readValue<int8_t>(data, absoluteOffset, e);

        if (fieldDef.size == 2)
            return readValue<int16_t>(data, absoluteOffset, e);

        if (fieldDef.size == 4)
            return readValue<int32_t>(data, absoluteOffset, e);

        if (fieldDef.size == 8)
            return readValue<int64_t>(data, absoluteOffset, e);
    }
    break;

    case FieldType::FLOAT:
        return readValue<float>(data, absoluteOffset, e);

    case FieldType::DOUBLE:
        return readValue<double>(data, absoluteOffset, e);

    case FieldType::BOOL:
        return bool(readValue<uint8_t>(data, absoluteOffset, e));
    }

    return {};
}

QVariant Utils::readArrayField(const QByteArray &data, const AbstractField *field, QSysInfo::Endian msgEndian, int length, int baseOffset)
{
    QVariantList list;

    int elementSize = field->getSize();

    for (int i = 0; i < length; i++)
    {
        list.push_back(readSingleField(data, field, msgEndian, baseOffset + i * elementSize));
    }

    return list;
}


void Utils::writeSingleField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian endian, int baseOffset)
{
    int absoluteOffset = baseOffset + field->byteOffset;

    if (field->isMessage())
    {
        auto msg = static_cast<const MessageDef*>(field);
        QVariantMap map = value.toMap();
        for (const auto& child : msg->fields)
        {
            writeField(buffer, child.get(), map.value(child->name), msg->endian, absoluteOffset);
        }
        return;
    }

    auto f = static_cast<const FieldDef*>(field);

    QSysInfo::Endian e = f->hasEndianOverride ? f->endian : endian;

    // Ensure buffer is large enough
    int needed = absoluteOffset + f->size;
    if (buffer.size() < needed)
        buffer.resize(needed);

    switch (f->type)
    {
    case FieldType::UINT:
    {
        if (f->size == 1) writeValue<uint8_t>(buffer, absoluteOffset, value.toUInt(), e);
        else if (f->size == 2) writeValue<uint16_t>(buffer, absoluteOffset, value.toUInt(), e);
        else if (f->size == 4) writeValue<uint32_t>(buffer, absoluteOffset, value.toUInt(), e);
        else if (f->size == 8) writeValue<uint64_t>(buffer, absoluteOffset, value.toULongLong(), e);
        break;
    }

    case FieldType::INT:
    {
        if (f->size == 1) writeValue<int8_t>(buffer, absoluteOffset, value.toInt(), e);
        else if (f->size == 2) writeValue<int16_t>(buffer, absoluteOffset, value.toInt(), e);
        else if (f->size == 4) writeValue<int32_t>(buffer, absoluteOffset, value.toInt(), e);
        else if (f->size == 8) writeValue<int64_t>(buffer, absoluteOffset, value.toLongLong(), e);
        break;
    }

    case FieldType::FLOAT:
        writeValue<float>(buffer, absoluteOffset, value.toFloat(), e);
        break;

    case FieldType::DOUBLE:
        writeValue<double>(buffer, absoluteOffset, value.toDouble(), e);
        break;

    case FieldType::BOOL:
        writeValue<uint8_t>(buffer, absoluteOffset, value.toBool() ? 1 : 0, e);
        break;
    }
}

void Utils::writeArrayField(QByteArray &buffer, const AbstractField *field, const QVariant &value, QSysInfo::Endian endian, int baseOffset)
{
    QVariantList list = value.toList();

    int elementSize = field->getSize();

    for (int i = 0; i < list.size(); i++)
    {
        writeSingleField(buffer, field, list[i], endian, baseOffset + i * elementSize);
    }
}
