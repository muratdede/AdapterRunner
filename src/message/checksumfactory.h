#ifndef CHECKSUMFACTORY_H
#define CHECKSUMFACTORY_H

#include "ichecksumalgorithm.h"
#include "checksum2c.h"

#include <QString>

class ChecksumFactory
{
public:
    /// Returns a pointer to a singleton algorithm instance, or nullptr if unknown.
    static const IChecksumAlgorithm* create(const QString& algorithm)
    {
        static Checksum2C checksum2c;

        if (algorithm == "checksum_2c")
            return &checksum2c;

        return nullptr;
    }
};

#endif // CHECKSUMFACTORY_H
