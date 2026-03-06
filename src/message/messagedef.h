#ifndef MESSAGEDEF_H
#define MESSAGEDEF_H

#include "fielddef.h"

#include <QVector>

struct MessageDef
{
    QString headerType;

    int messageId;

    QString name;

    QSysInfo::Endian endian;

    QVector<FieldDef> fields;
};

#endif // MESSAGEDEF_H
