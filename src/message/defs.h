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
};

struct HeaderDef
{
    QString type;

    QByteArray startBytes;

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
