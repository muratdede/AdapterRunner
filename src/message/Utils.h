#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <cstdint>

template<typename T>
T swapEndian(T val)
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
T readRaw(const QByteArray& data, int offset)
{
    T val;

    memcpy(&val, data.constData() + offset, sizeof(T));

    return val;
}

template<typename T>
T readValue(const QByteArray& data, int offset, QSysInfo::Endian msgEndian)
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

#endif // UTILS_H
