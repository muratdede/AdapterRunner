#ifndef DEFS_H
#define DEFS_H

#include <QString>
#include <QVector>

enum class FieldType
{
    UINT,
    INT,
    FLOAT,
    DOUBLE,
    BOOL
};

struct ComputeDef
{
    QString algorithm;    // e.g. "checksum_2c"
    QString onMismatch;   // e.g. "drop", "warn" (receive-side only)
    QString scope;        // "payload" (default), "header", "frame"
    int rangeStart = 0;   // byte offset within scope
    int rangeEnd = -1;    // -1 = end of scope
    bool excludeSelf = true;

    bool hasCompute() const { return !algorithm.isEmpty(); }
};

struct FieldDef
{
    QString name;

    FieldType type;

    int byteOffset = 0;

    int bitOffset = -1;
    int bitLength = 0;

    int size = 0;

    bool hasEndianOverride = false;
    QSysInfo::Endian endian;

    int arrayLength = 0;
    QString arrayLengthField;

    ComputeDef compute;

    bool isIdentifier = false;
    QVector<int> matchValues;
};

struct HeaderDef
{
    QString type;

    QSysInfo::Endian endian;

    QVector<FieldDef> fields;

    int headerSize = 0;
};

struct MessageDef
{
    QString headerType;

    int messageId;

    QString name;

    QSysInfo::Endian endian;

    QVector<FieldDef> fields;
};

#endif // DEFS_H
