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
    QString scope;        // "payload" (default), "header", "frame"
    int rangeStart = 0;   // byte offset within scope
    int rangeEnd = -1;    // -1 = end of scope

    bool hasCompute() const { return !algorithm.isEmpty(); }
};

struct AbstractField
{
    QString name;

    int byteOffset = 0;

    int arrayLength = 0;
    QString arrayLengthField;

    bool hasEndianOverride = false;
    QSysInfo::Endian endian;

    virtual ~AbstractField() = default;

    virtual bool isMessage() const = 0;
    virtual int getSize() const = 0;
};

struct FieldDef : public AbstractField
{
    FieldType type;

    int bitOffset = -1;
    int bitLength = 0;

    int size = 0;

    ComputeDef compute;

    bool isIdentifier = false;
    QVector<int> matchValues;

    bool isMessage() const override { return false; }
    int getSize() const override { return size; }
};

struct MessageDef : public AbstractField
{
    QString headerType;

    int messageId = -1;
    QString templateName;

    QVector<std::shared_ptr<AbstractField>> fields;

    bool isMessage() const override { return true; }
    int getSize() const override {
        int max = 0;
        for (const auto& f : fields) {
            int len = f->arrayLength > 0 ? f->arrayLength : 1;
            int s = f->byteOffset + len * f->getSize();
            if (s > max) max = s;
        }
        return max;
    }
};

struct HeaderDef
{
    QString type;

    QSysInfo::Endian endian;

    QVector<std::shared_ptr<AbstractField>> fields;

    int headerSize = 0;
};

#endif // DEFS_H
