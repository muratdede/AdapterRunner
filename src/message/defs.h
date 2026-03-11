#ifndef DEFS_H
#define DEFS_H

#include <QString>
#include <QVector>
#include <memory>

enum class FieldType
{
    UINT,
    INT,
    FLOAT,
    DOUBLE,
    BOOL
};

struct ComputeDef
{
    QString algorithm;    // e.g. "checksum_2c"
    QString onMismatch;   // e.g. "drop", "warn" (receive-side only)

    bool hasCompute() const { return !algorithm.isEmpty(); }
};

struct AbstractField
{
    QString name;
    QString typeName;
    FieldType type;

    bool hasEndianOverride = false;
    QSysInfo::Endian endian;

    int byteOffset = 0;

    int arrayLength = 0;
    QString arrayLengthField;

    virtual ~AbstractField() = default;

    virtual bool isMessage() const = 0;
    virtual int getSize() const = 0;
};

struct AbstractMessage: AbstractField
{
    QVector<std::shared_ptr<AbstractField>> fields;

    bool isMessage() const override { return true; }
};

struct FieldDef : public AbstractField
{
    int bitOffset = -1;
    int bitLength = 0;

    int size = 0;

    ComputeDef compute;

    bool isIdentifier = false;
    QVector<int> matchValues;

    bool isMessage() const override { return false; }
    int getSize() const override { return size; }
};

struct HeaderDef: AbstractMessage
{
    int headerSize = 0;

    int getSize() const override { return headerSize; }
};

struct MessageDef: AbstractMessage
{
    int id = -1;

    int getSize() const override { // TODO: smells
        int max = 0;
        for (const auto& f : fields) {
            int len = f->arrayLength > 0 ? f->arrayLength : 1;
            int s = f->byteOffset + len * f->getSize();
            if (s > max) max = s;
        }
        return max;
    }
};

#endif // DEFS_H
