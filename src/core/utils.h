#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QVariant>
#include <cstdint>

#include "src/protocol/types/defs.h"

class Utils {
public:
    static QVariant readField(const QByteArray& data, const AbstractField* field, QSysInfo::Endian parentEndian, int arrayLen = 0, int baseOffset = 0);
    static void writeField(QByteArray& buffer, const AbstractField* field, const QVariant& value, QSysInfo::Endian endian, int baseOffset = 0);

    template<typename T>
    static QVariant toQVariant(const FieldDef& field, T value)
    {
        switch (field.type)
        {
        case FieldType::UINT:
        {
            if (field.size == 1)
                return static_cast<uint8_t>(value);
            if (field.size == 2)
                return static_cast<uint16_t>(value);
            if (field.size == 4)
                return static_cast<uint32_t>(value);
            if (field.size == 8)
                return static_cast<uint64_t>(value);
        }
        break;

        case FieldType::INT:
        {
            if (field.size == 1)
                return static_cast<int8_t>(value);
            if (field.size == 2)
                return static_cast<int16_t>(value);
            if (field.size == 4)
                return static_cast<int32_t>(value);
            if (field.size == 8)
                return static_cast<int64_t>(value);
        }
        break;

        case FieldType::FLOAT:
            return static_cast<float>(value);
        case FieldType::DOUBLE:
            return static_cast<double>(value);
        case FieldType::BOOL:
            return bool(static_cast<uint8_t>(value));
        }
    }

private:
    static QVariant readArrayField(const QByteArray &data, const AbstractField* field, QSysInfo::Endian parentEndian, int length, int baseOffset);
    static QVariant readSingleField(const QByteArray &data, const AbstractField* field, QSysInfo::Endian parentEndian, int baseOffset);

    static uint64_t extractBits(const QByteArray &data, int byteOffset, int bitOffset, int bitLength);

    template<typename T>
    static T readValue(const QByteArray& data, int offset, QSysInfo::Endian endian)
    {
        T val = readRaw<T>(data, offset);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        bool systemLittle = true;
#else
        bool systemLittle = false;
#endif

        if ((endian == QSysInfo::LittleEndian && !systemLittle) ||
            (endian == QSysInfo::BigEndian && systemLittle))
        {
            val = swapEndian(val);
        }

        return val;
    }

    template<typename T>
    static T swapEndian(T val)
    {
        union
        {
            T val;
            uint8_t bytes[sizeof(T)];
        } src, dst;

        src.val = val;

        for (size_t i = 0; i < sizeof(T); i++)
            dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];

        return dst.val;
    }

    template<typename T>
    static T readRaw(const QByteArray& data, int offset)
    {
        T val;

        memcpy(&val, data.constData() + offset, sizeof(T));

        return val;
    }

    //----------------------------------------------------------//

    static void writeArrayField(QByteArray& buffer, const AbstractField* field, const QVariant& value, QSysInfo::Endian parentEndian, int baseOffset);
    static void writeSingleField(QByteArray& buffer, const AbstractField* field, const QVariant& value, QSysInfo::Endian parentEndian, int baseOffset);

    template<typename T>
    static void writeValue(QByteArray &buffer, int offset, T value, QSysInfo::Endian endian)
    {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        bool systemLittle = true;
#else
        bool systemLittle = false;
#endif

        if ((endian == QSysInfo::LittleEndian && !systemLittle) ||
            (endian == QSysInfo::BigEndian && systemLittle))
        {
            value = swapEndian(value);
        }

        memcpy(buffer.data() + offset, &value, sizeof(T));
    }
};


#endif // UTILS_H
