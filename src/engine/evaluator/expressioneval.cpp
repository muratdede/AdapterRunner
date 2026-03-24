#include "expressioneval.h"

#include <QJSValue>
#include <QDebug>

ExpressionEvaluator::ExpressionEvaluator()
{
    // Add helper functions to the global scope
    mEngine.evaluate(
        "function int(x)   { return Math.trunc(x); }\n"
        "function float(x) { return Number(x); }\n"
        "function abs(x)   { return Math.abs(x); }\n"
        "function sqrt(x)  { return Math.sqrt(x); }\n"
        "function min(a,b) { return Math.min(a,b); }\n"
        "function max(a,b) { return Math.max(a,b); }\n"
        "function pow(a,b) { return Math.pow(a,b); }\n"
        "function round(x) { return Math.round(x); }\n"
        "function floor(x) { return Math.floor(x); }\n"
        "function ceil(x)  { return Math.ceil(x); }\n"
        "function toDegree(rad) { return rad * (180.0 / Math.PI); }\n"
        "function toRadian(deg) { return deg * (Math.PI / 180.0); }\n"
    );
}

void ExpressionEvaluator::setVariables(const QMap<QString, QVariant> &vars)
{
    for (auto it = vars.begin(); it != vars.end(); ++it)
        setVariable(it.key(), it.value());
}

void ExpressionEvaluator::setVariable(const QString &name, const QVariant &value)
{
    QJSValue val;

    if (value.type() == QVariant::Double || value.type() == QVariant::Int
        || value.type() == QVariant::UInt || value.type() == QVariant::LongLong
        || value.type() == QVariant::ULongLong)
    {
        val = QJSValue(value.toDouble());
    }
    else if (value.type() == QVariant::Bool)
    {
        val = QJSValue(value.toBool());
    }
    else if (value.type() == QVariant::List)
    {
        QJSValue arr = mEngine.newArray(value.toList().size());
        auto list = value.toList();
        for (int i = 0; i < list.size(); ++i)
            arr.setProperty(i, QJSValue(list[i].toDouble()));
        val = arr;
    }
    else
    {
        val = QJSValue(value.toDouble());
    }

    mEngine.globalObject().setProperty(name, val);
}

QVariant ExpressionEvaluator::evaluate(const QString &expression)
{
    QJSValue result = mEngine.evaluate(expression);

    if (result.isError())
    {
        qWarning() << "ExpressionEvaluator: error in" << expression << ":" << result.toString();
        return QVariant(0);
    }

    if (result.isNumber())
        return QVariant(result.toNumber());

    if (result.isBool())
        return QVariant(result.toBool());

    if (result.isArray())
    {
        QVariantList list;
        int len = result.property("length").toInt();
        for (int i = 0; i < len; ++i)
            list.append(result.property(i).toNumber());
        return list;
    }

    if (result.isObject())
        return result.toVariant();

    qDebug() << result.toNumber();
    return QVariant(result.toNumber());
}
