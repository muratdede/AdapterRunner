#include "protocolschema.h"

#include "Utils.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

bool ProtocolSchema::load(const QString &path) {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly))
    return false;

  auto doc = QJsonDocument::fromJson(file.readAll());

  if (!doc.isObject())
    return false;

  auto root = doc.object();

  /*
      HEADERS
  */
  for (auto headerVal : root["headers"].toArray()) {
    auto obj = headerVal.toObject();

    HeaderDef header;

    header.type = obj["type"].toString();

    header.headerSize = obj["header_size"].toInt();

    if (obj.contains("endianness"))
      header.endian = parseEndianness(obj["endianness"].toString());
    else
      header.endian = QSysInfo::LittleEndian;

    for (auto fieldVal : obj["fields"].toArray()) {
      auto fieldObj = fieldVal.toObject();

      FieldDef field = parseField(fieldObj);

      header.fields.push_back(field);
    }

    mHeaders.insert(header.type, header);
  }

  /*
      MESSAGES
  */
  for (auto msgVal : root["messages"].toArray()) {
    auto obj = msgVal.toObject();

    MessageDef msg;

    msg.headerType = obj["header"].toString();
    msg.messageId = obj["message_id"].toInt();
    msg.name = obj["name"].toString();

    msg.endian = parseEndianness(obj["endianness"].toString());

    for (auto fieldVal : obj["fields"].toArray()) {
      FieldDef field = parseField(fieldVal.toObject());

      msg.fields.push_back(field);
    }

    mMessages[msg.headerType].insert(msg.messageId, msg);
  }

  // Build name lookup index
  for (auto &headerMap : mMessages) {
    for (auto it = headerMap.begin(); it != headerMap.end(); ++it) {
      mMessagesByName.insert(it.value().name, &it.value());
    }
  }

  return true;
}

const HeaderDef *ProtocolSchema::findHeader(const QByteArray &data) const {
  for (auto &header : mHeaders) {
    if (matchesHeader(data, header))
      return &header;
  }

  return nullptr;
}

const MessageDef *ProtocolSchema::getMessage(const QString &headerType,
                                             int msgId) const {
  auto headerIt = mMessages.find(headerType);

  if (headerIt == mMessages.end())
    return nullptr;

  const auto &map = headerIt.value();

  auto msgIt = map.find(msgId);

  if (msgIt == map.end())
    return nullptr;

  return &msgIt.value();
}

const MessageDef *ProtocolSchema::getMessageByName(const QString &name) const {
  return mMessagesByName.value(name, nullptr);
}

const HeaderDef *ProtocolSchema::getHeader(const QString &type) const {
  auto it = mHeaders.find(type);

  if (it == mHeaders.end())
    return nullptr;

  return &it.value();
}

bool ProtocolSchema::matchesHeader(const QByteArray &data, const HeaderDef &header) const {
    if (data.size() < header.headerSize)
        return false;

    for (const FieldDef &field : header.fields)
    {
        if (!field.isIdentifier)
            continue;

        if (data.size() < field.byteOffset + field.size)
            return false;

        QVariant value = Utils::readField(data, field, header.endian);

        if (!field.matchValues.contains(value.toInt())) // TODO may produce bug if field is not integer type
            return false;
    }

    return true;
}

QSysInfo::Endian ProtocolSchema::parseEndianness(const QString &str) {
  if (str == "big")
    return QSysInfo::BigEndian;

  return QSysInfo::LittleEndian;
}

FieldType ProtocolSchema::parseFieldType(const QString &type) {
    if (type.startsWith("uint"))
        return FieldType::UINT;

    if (type.startsWith("int"))
        return FieldType::INT;

    if (type == "float")
        return FieldType::FLOAT;

    if (type == "double")
        return FieldType::DOUBLE;

    if (type == "bool")
        return FieldType::BOOL;

    throw std::runtime_error("Unknown field type");
}

int ProtocolSchema::parseTypeSize(const QString &type) {
    if (type.contains("8"))
        return 1;
    if (type.contains("16"))
        return 2;
    if (type.contains("32"))
        return 4;
    if (type.contains("64"))
        return 8;

    if (type == "float")
        return 4;
    if (type == "double")
        return 8;
    if (type == "bool")
        return 1;

  return 0;
}

FieldDef ProtocolSchema::parseField(const QJsonObject &f) {
  FieldDef field;

  field.name = f["name"].toString();

  QString type = f["type"].toString();

  field.type = parseFieldType(type);
  field.size = parseTypeSize(type);

  field.byteOffset = f["byte_offset"].toInt();

  if (f.contains("bit_offset")) {
    field.bitOffset = f["bit_offset"].toInt();
    field.bitLength = f["bit_length"].toInt();
  }

  if (f.contains("endianness")) {
    field.hasEndianOverride = true;
    field.endian = parseEndianness(f["endianness"].toString());
  }

  if (f.contains("array_length")) {
    field.arrayLength = f["array_length"].toInt();
  }

  if (f.contains("array_length_field")) {
    field.arrayLengthField = f["array_length_field"].toString();
  }

  if (f.contains("compute")) {
    auto cObj = f["compute"].toObject();
    field.compute.algorithm = cObj["algorithm"].toString();
    field.compute.onMismatch = cObj["onMismatch"].toString();
    field.compute.scope = cObj.contains("scope") ? cObj["scope"].toString() : "payload";
    if (cObj.contains("range")) {
      auto rangeObj = cObj["range"].toObject();
      field.compute.rangeStart = rangeObj.contains("start") ? rangeObj["start"].toInt() : 0;
      field.compute.rangeEnd = rangeObj.contains("end") ? rangeObj["end"].toInt() : -1;
    }
  }

  if (f.contains("identifier") && f["identifier"].toBool()) {
    field.isIdentifier = true;
    if (f.contains("match_values")) {
      auto arr = f["match_values"].toArray();
      for (const auto &val : arr) {
        if (val.isString()) {
          bool ok;
          int parsed = val.toString().toInt(
              &ok, 16); // support hex string like "07" or "AA55"
          if (ok)
            field.matchValues.push_back(parsed);
        } else {
          field.matchValues.push_back(val.toInt());
        }
      }
    }
  }

  return field;
}
