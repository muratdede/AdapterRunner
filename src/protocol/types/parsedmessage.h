#ifndef PARSEDMESSAGE_H
#define PARSEDMESSAGE_H

#include <QVariant>

class ParsedMessage
{
public:

    QString name;

    QMap<QString, QVariant> values;
};

#endif // PARSEDMESSAGE_H
