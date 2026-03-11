#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

namespace JsonUtils
{

inline QMap<QString, QVariant> parseValues(const QJsonObject& obj)
{
    QMap<QString, QVariant> result;
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        if (it.value().isArray())
        {
            QVariantList list;
            for (auto v : it.value().toArray())
                list.append(v.toDouble());
            result[it.key()] = list;
        }
        else if (it.value().isDouble())
        {
            result[it.key()] = it.value().toDouble();
        }
        else
        {
            result[it.key()] = it.value().toVariant();
        }
    }
    return result;
}

} // namespace JsonUtils

#endif // JSONUTILS_H
