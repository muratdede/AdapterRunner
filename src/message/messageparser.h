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

    const ProtocolSchema* mSchema;
};

#endif // MESSAGEPARSER_H
