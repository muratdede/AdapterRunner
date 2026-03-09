#ifndef ICHECKSUMNALGORITHM_H
#define ICHECKSUMNALGORITHM_H

#include <QByteArray>
#include <cstdint>

class IChecksumAlgorithm
{
public:
    virtual ~IChecksumAlgorithm() = default;

    /// Compute the checksum over the payload, excluding bytes at [offset, offset+size)
    virtual uint8_t compute(const QByteArray& payload, int excludeOffset, int excludeSize) const = 0;

    /// Verify the checksum: returns true if the received value matches the expected
    bool verify(const QByteArray& payload, int excludeOffset, int excludeSize, uint8_t received) const
    {
        return compute(payload, excludeOffset, excludeSize) == received;
    }
};

#endif // ICHECKSUMNALGORITHM_H
