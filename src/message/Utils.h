#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QVariant>
#include <cstdint>

#include "defs.h"

class Utils {
public:
    static QVariant readField(const QByteArray& data, const FieldDef& field, QSysInfo::Endian msgEndian, int arrayLen = 0);
    static void writeField(QByteArray& buffer, const FieldDef& field, const QVariant& value, QSysInfo::Endian endian);

private:
    static QVariant readSingleField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian);
    static QVariant readArrayField(const QByteArray &data, const FieldDef &field, QSysInfo::Endian msgEndian, int length);

    // TODO 4 bytelık chunk parcalama da eklenmeli
    static uint64_t extractBits(const QByteArray &data, int byteOffset, int bitOffset, int bitLength);

    template<typename T>
    static T readValue(const QByteArray& data, int offset, QSysInfo::Endian msgEndian)
    {
        T val = readRaw<T>(data, offset);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        bool systemLittle = true;
#else
        bool systemLittle = false;
#endif

        if ((msgEndian == QSysInfo::LittleEndian && !systemLittle) ||
            (msgEndian == QSysInfo::BigEndian && systemLittle))
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

    static void writeSingleField(QByteArray& buffer, const FieldDef& field, const QVariant& value, QSysInfo::Endian endian);
    static void writeArrayField(QByteArray& buffer, const FieldDef& field, const QVariant& value, QSysInfo::Endian endian);

    template<typename T>
    static void writeValue(QByteArray &buffer, int offset, T value, QSysInfo::Endian msgEndian)
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
};


#endif // UTILS_H
