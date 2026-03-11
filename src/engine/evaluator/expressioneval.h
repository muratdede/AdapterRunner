#ifndef EXPRESSIONEVAL_H
#define EXPRESSIONEVAL_H

#include <QJSEngine>
#include <QMap>
#include <QVariant>

class ExpressionEvaluator
{
public:
    ExpressionEvaluator();

    /// Load all source field values as JS variables
    void setVariables(const QMap<QString, QVariant>& vars);

    /// Load a single field value as a JS variable
    void setVariable(const QString& name, const QVariant& value);

    /// Evaluate a math expression and return result
    QVariant evaluate(const QString& expression);

private:
    QJSEngine mEngine;
};

#endif // EXPRESSIONEVAL_H
