#ifndef CHECKSUM2C_H
#define CHECKSUM2C_H

#include "ichecksumalgorithm.h"
#include "src/message/defs.h"

class Checksum2C : public IChecksumAlgorithm
{
public:
    uint8_t compute(const QByteArray& data, const FieldDef& field) const override
    {
        int rangeStart = field.compute.rangeStart;
        int rangeEnd = field.compute.rangeEnd;
        int excludeOffset = field.byteOffset;
        int excludeSize = field.size;

        int end = (rangeEnd < 0) ? data.size() : rangeEnd;
        uint8_t sum = 0;
        for (int i = rangeStart; i < end; ++i)
        {
            if (excludeOffset >= 0 && i >= excludeOffset && i < excludeOffset + excludeSize)
                continue;
            sum += static_cast<uint8_t>(data[i]);
        }
        return static_cast<uint8_t>(~sum + 1);
    }
};

#endif // CHECKSUM2C_H
