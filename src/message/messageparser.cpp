#include "messageparser.h"

#include "Utils.h"

#include <QDebug>

ParsedMessage MessageParser::parseFrame(const QByteArray& frame)
{
    std::shared_ptr<HeaderDef> header = mSchema->findHeader(frame);
    if (!header)
        return {};

    ParsedHeader parsedHeader = parseHeader(frame, *header);

    int msgId = parsedHeader.values["msg_id"].toInt();
    std::shared_ptr<MessageDef> msgDef = mSchema->getMessageById(msgId);
    if (!msgDef)
        return {};

    ParsedMessage result;
    result.name = msgDef->name;

    for (const auto& fieldPtr : std::as_const(msgDef->fields))
    {
        int length = fieldPtr->arrayLength;
        if (!fieldPtr->arrayLengthField.isEmpty())
            length = result.values[fieldPtr->arrayLengthField].toInt();

        QVariant value = Utils::readField(frame, fieldPtr.get(), msgDef->endian, length, 0);
        if (value.isNull())
            return {};

        result.values[fieldPtr->name] = value;
    }
    return result;
}

ParsedHeader MessageParser::parseHeader(const QByteArray &frame, const HeaderDef &header)
{
    ParsedHeader result;

    result.type = header.name;

    for (const auto& fieldPtr : header.fields)
    {
        int length = fieldPtr->arrayLength;
        if (!fieldPtr->arrayLengthField.isEmpty())
            length = result.values[fieldPtr->arrayLengthField].toInt();

        QVariant value = Utils::readField(frame, fieldPtr.get(), header.endian, length, 0);
        if (value.isNull())
            return {};

        result.values[fieldPtr->name] = value;
    }

    return result;
}
