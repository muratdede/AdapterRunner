#ifndef ICHECKSUMNALGORITHM_H
#define ICHECKSUMNALGORITHM_H

#include <QByteArray>

#include "src/message/defs.h"

class IChecksumAlgorithm
{
public:
    virtual ~IChecksumAlgorithm() = default;

    /// Compute over data[rangeStart..rangeEnd), optionally excluding [excludeOffset, excludeOffset+excludeSize)
    /// rangeEnd = -1 means end of data
    virtual uint8_t compute(const QByteArray& data, const FieldDef& field) const = 0;

    /// Verify the checksum: returns true if the received value matches the expected
    bool verify(const QByteArray& data, const FieldDef& field, uint8_t received) const
    {
        return compute(data, field) == received;
    }
};

#endif // ICHECKSUMNALGORITHM_H
