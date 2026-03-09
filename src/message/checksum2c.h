#ifndef CHECKSUM2C_H
#define CHECKSUM2C_H

#include "ichecksumalgorithm.h"

class Checksum2C : public IChecksumAlgorithm
{
public:
    uint8_t compute(const QByteArray& payload, int excludeOffset, int excludeSize) const override
    {
        uint8_t sum = 0;
        for (int i = 0; i < payload.size(); ++i)
        {
            if (i >= excludeOffset && i < excludeOffset + excludeSize)
                continue;
            sum += static_cast<uint8_t>(payload[i]);
        }
        return static_cast<uint8_t>(~sum + 1);
    }
};

#endif // CHECKSUM2C_H
