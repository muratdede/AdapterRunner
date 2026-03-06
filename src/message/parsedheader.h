#ifndef PARSEDHEADER_H
#define PARSEDHEADER_H

#include <QVariant>

struct ParsedHeader
{
    QString type;

    QMap<QString, QVariant> values;
};

#endif // PARSEDHEADER_H
