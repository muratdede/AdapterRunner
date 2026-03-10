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

    for (const auto& fieldPtr : msgDef->fields)
    {
        int length = fieldPtr->arrayLength;
        if (!fieldPtr->arrayLengthField.isEmpty())
            length = parsedFields[fieldPtr->arrayLengthField].toInt();

        parsedFields[fieldPtr->name] = Utils::readField(payload, fieldPtr.get(), msgDef->endian, length, 0);
    }

    result.values = parsedFields;

    // Verify computed fields (e.g. checksum)
    for (const auto& fieldPtr : msgDef->fields)
    {
        if (fieldPtr->isMessage()) continue; // TODO: make this recursive function

        auto fieldObj = std::static_pointer_cast<FieldDef>(fieldPtr);
        const FieldDef& field = *fieldObj;

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

    for (const auto& fieldPtr : header.fields)
    {
        // TODO: -> asagıdakiler
        int length = fieldPtr->arrayLength;
        if (!fieldPtr->arrayLengthField.isEmpty())
            length = result.values[fieldPtr->arrayLengthField].toInt();

        result.values[fieldPtr->name] = Utils::readField(frame, fieldPtr.get(), header.endian, length, 0);
    }

    return result;
}
