#ifndef ICHECKSUMNALGORITHM_H
#define ICHECKSUMNALGORITHM_H

#include <QByteArray>
#include <QVariant>

#include "src/protocol/types/defs.h"

class IChecksumAlgorithm
{
public:
    virtual ~IChecksumAlgorithm() = default;

    virtual QVariant compute(const QByteArray& data, int offset, const FieldDef& field) const = 0;
};

#endif // ICHECKSUMNALGORITHM_H
