#ifndef CHECKSUM2C_H
#define CHECKSUM2C_H

#include "src/core/utils.h"
#include "src/protocol/checksum/interfaces/ichecksumalgorithm.h"
#include "src/protocol/types/defs.h"

class Checksum2C : public IChecksumAlgorithm
{
public:
    QVariant compute(const QByteArray& data, int offset, const FieldDef& field) const override
    {
        int endOffset = std::min(data.size(), offset + field.byteOffset);

        uint64_t sum = 0;
        for (int i = offset; i < endOffset; ++i)
        {
            sum += static_cast<uint8_t>(data[i]);
        }

        return Utils::toQVariant(field, ~sum + 1);
    }
};

#endif // CHECKSUM2C_H
