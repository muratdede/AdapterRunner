#ifndef ICHECKSUMNALGORITHM_H
#define ICHECKSUMNALGORITHM_H

#include <QByteArray>
#include <cstdint>

class IChecksumAlgorithm
{
public:
    virtual ~IChecksumAlgorithm() = default;

    /// Compute over data[rangeStart..rangeEnd), optionally excluding [excludeOffset, excludeOffset+excludeSize)
    /// rangeEnd = -1 means end of data
    virtual uint8_t compute(const QByteArray& data, int rangeStart, int rangeEnd,
                            int excludeOffset, int excludeSize) const = 0;

    /// Verify the checksum: returns true if the received value matches the expected
    bool verify(const QByteArray& data, int rangeStart, int rangeEnd,
                int excludeOffset, int excludeSize, uint8_t received) const
    {
        return compute(data, rangeStart, rangeEnd, excludeOffset, excludeSize) == received;
    }
};

#endif // ICHECKSUMNALGORITHM_H
