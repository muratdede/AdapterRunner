#ifndef HEADERDEF_H
#define HEADERDEF_H

#include <QVector>

#include "fielddef.h"

struct HeaderDef
{
    QString type;

    QByteArray startBytes;

    QVector<FieldDef> fields;

    int headerSize = 0;
};

#endif // HEADERDEF_H
