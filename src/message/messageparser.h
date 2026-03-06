#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include "parsedmessage.h"
#include "protocolschema.h"
#include "parsedheader.h"

class MessageParser
{
public:

    MessageParser(const ProtocolSchema* schema)
        : mSchema(schema)
    {}

    ParsedMessage parseFrame(const QByteArray& frame);

private:
    ParsedHeader parseHeader(const QByteArray& frame, const HeaderDef& header);

    uint64_t extractBits(const QByteArray& data, int byteOffset, int bitOffset, int bitLength);
    QVariant readField(const QByteArray& data, const FieldDef& field);

    const ProtocolSchema* mSchema;
};

#endif // MESSAGEPARSER_H
