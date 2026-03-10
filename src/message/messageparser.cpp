#include "messageparser.h"
#include "checksumfactory.h"

#include "Utils.h"

#include <QDebug>


// TODO refactor contains compute algorithm
ParsedMessage MessageParser::parseFrame(const QByteArray& frame)
{
    ParsedMessage result;

    const HeaderDef* header = mSchema->findHeader(frame);

    if (!header)
        return result;

    ParsedHeader h = parseHeader(frame, *header);

    int msgId = h.values["msg_id"].toInt();

    const MessageDef* msgDef = mSchema->getMessage(header->type, msgId);

    if (!msgDef)
        return result;

    QByteArray payload = frame.mid(header->headerSize);

    result.name = msgDef->name;

    QMap<QString, QVariant> parsedFields;

    for (const FieldDef& field : msgDef->fields)
    {
        int length = field.arrayLength;
        if (!field.arrayLengthField.isEmpty())
            length = parsedFields[field.arrayLengthField].toInt();

        parsedFields[field.name] = Utils::readField(payload, field, msgDef->endian, length);
    }

    result.values = parsedFields;

    // Verify computed fields (e.g. checksum)
    for (const FieldDef& field : msgDef->fields)
    {
        if (!field.compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field.compute.algorithm);
        if (!algo)
            continue;

        uint8_t received = static_cast<uint8_t>(parsedFields[field.name].toUInt());

        // Build the buffer based on scope
        QByteArray computeBuffer;

        FieldDef tmp = field;
        if (field.compute.scope == "frame")
        {
            computeBuffer = frame;
            tmp.byteOffset = header->headerSize + field.byteOffset;
        }
        else if (field.compute.scope == "header")
        {
            computeBuffer = payload;
        }
        else  // "payload" (default)
        {
            computeBuffer = payload;
        }

        if (!algo->verify(computeBuffer, tmp, received))
        {
            // TODO: calculating two times of computed value, verify can return this
            uint8_t expected = algo->compute(computeBuffer, field);
            qWarning() << "MessageParser: checksum mismatch for" << result.name
                       << "- expected:" << expected << "received:" << received;

            if (field.compute.onMismatch == "drop")
                return ParsedMessage();
        }
    }

    return result;
}

ParsedHeader MessageParser::parseHeader(const QByteArray &frame, const HeaderDef &header)
{
    ParsedHeader result;

    result.type = header.type;

    for (const FieldDef& field : header.fields)
    {
        // TODO: -> asagıdakiler
        int length = field.arrayLength;
        if (!field.arrayLengthField.isEmpty())
            length = result.values[field.arrayLengthField].toInt();

        result.values[field.name] = Utils::readField(frame, field, header.endian, length);
    }

    return result;
}
