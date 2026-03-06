// #ifndef GENERICMESSAGEPARSER_H
// #define GENERICMESSAGEPARSER_H

// class GenericMessageParser
// {
// public:
//     GenericMessageParser();

//     ParsedMessage parse(QByteArray frame)
//     {
//         int msgId = readUInt16(frame, HEADER_MSG_ID_OFFSET);

//         const MessageDef& def = protocol.get(msgId);

//         ParsedMessage msg;

//         for (auto& field : def.fields)
//         {
//             QVariant value = readField(frame, field);

//             msg.values[field.name] = value;
//         }

//         return msg;
//     }
// };

// #endif // GENERICMESSAGEPARSER_H
