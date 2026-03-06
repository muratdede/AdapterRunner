#ifndef HEADERDEF_H
#define HEADERDEF_H

#include <QVector>

#include "fielddef.h"

struct HeaderDef
{
    QString type;

    QByteArray startBytes;

    QSysInfo::Endian endian;

    QVector<FieldDef> fields;

    int headerSize = 0;
};

#endif // HEADERDEF_H
