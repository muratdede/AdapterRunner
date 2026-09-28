#ifndef FIELDMAPPING_H
#define FIELDMAPPING_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

struct FieldMapping
{
    QString targetField;
    QString expression;
    QStringList dependents;

    static QVector<FieldMapping> fromJsonArray(const QJsonArray& arr)
    {
        QVector<FieldMapping> result;
        for (const auto& val : arr)
        {
            auto obj = val.toObject();
            FieldMapping m;
            m.targetField = obj["to"].toString();
            m.expression = obj["expr"].toString();

            auto depArr = obj["dependents"].toArray();
            for (const auto& d : qAsConst(depArr))
                m.dependents.append(d.toString());

            result.append(m);
        }
        return result;
    }
};

#endif // FIELDMAPPING_H
