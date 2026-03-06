#ifndef FIELDDEF_H
#define FIELDDEF_H

#include <QString>

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

    bool littleEndian = true;
};

#endif // FIELDDEF_H
