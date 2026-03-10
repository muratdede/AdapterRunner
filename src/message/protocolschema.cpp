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

      auto field = parseField(fieldObj, header.endian);

      header.fields.push_back(field);
    }

    mHeaders.insert(header.type, header);
  }

  /*
      MESSAGES
  */
  for (auto msgVal : root["messages"].toArray()) {
    auto obj = msgVal.toObject();

    auto msg = std::make_shared<MessageDef>();

    msg->headerType = obj["header"].toString();
    msg->messageId = obj["message_id"].toInt();
    msg->name = obj["name"].toString();

    msg->endian = parseEndianness(obj["endianness"].toString());

    for (auto fieldVal : obj["fields"].toArray()) {
      auto field = parseField(fieldVal.toObject(), msg->endian);

      msg->fields.push_back(field);
    }

    mMessages[msg->headerType].insert(msg->messageId, msg);
  }

  // Build name lookup index
  for (auto &headerMap : mMessages) {
    for (auto it = headerMap.begin(); it != headerMap.end(); ++it) {
      mMessagesByName.insert(it.value()->name, it.value());
    }
  }

  resolveCompositeFields();

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

    return msgIt.value().get();
}

const MessageDef* ProtocolSchema::getMessageByName(const QString& name) const
{
    auto it = mMessagesByName.find(name);
    if (it == mMessagesByName.end()) return nullptr;
    return it.value().get();
}

const HeaderDef* ProtocolSchema::getHeader(const QString& type) const
{
    auto it = mHeaders.find(type);

    if (it == mHeaders.end())
        return nullptr;

    return &it.value();
}

bool ProtocolSchema::matchesHeader(const QByteArray &data, const HeaderDef &header) const
{
    if (data.size() < header.headerSize)
        return false;

    // Check all identifier fields
    for (const auto& fieldPtr : header.fields)
    {
        if (fieldPtr->isMessage()) continue;
        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);

        if (!field->isIdentifier)
            continue;

        if (data.size() < field->byteOffset + field->size)
            return false;

        QVariant value = Utils::readField(data, fieldPtr.get(), header.endian);

        if (!field->matchValues.contains(value.toInt())) // TODO may produce bug if field is not integer type
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

std::shared_ptr<AbstractField> ProtocolSchema::parseField(const QJsonObject& f, QSysInfo::Endian defaultEndian)
{
    QString type = f["type"].toString();

    // Check if primitive
    FieldType pType;
    bool isPrimitive = true;
    try {
        pType = parseFieldType(type);
    } catch (...) {
        isPrimitive = false;
    }

    if (!isPrimitive)
    {
        auto comp = std::make_shared<MessageDef>();
        comp->name = f["name"].toString();
        comp->templateName = type;
        comp->byteOffset = f["byte_offset"].toInt();
        comp->endian = defaultEndian;
        if (f.contains("endianness"))
        {
            comp->hasEndianOverride = true;
            comp->endian = parseEndianness(f["endianness"].toString());
        }
        if (f.contains("array_length"))
            comp->arrayLength = f["array_length"].toInt();
        if (f.contains("array_length_field"))
            comp->arrayLengthField = f["array_length_field"].toString();
            
        return comp;
    }

    auto field = std::make_shared<FieldDef>();
    field->name = f["name"].toString();
    field->type = pType;
    field->size = parseTypeSize(type);
    field->byteOffset = f["byte_offset"].toInt();

    if (f.contains("bit_offset"))
    {
        field->bitOffset = f["bit_offset"].toInt();
        field->bitLength = f["bit_length"].toInt();
    }

    if (f.contains("endianness"))
    {
        field->hasEndianOverride = true;
        field->endian = parseEndianness(f["endianness"].toString());
    }

    if (f.contains("array_length"))
    {
        field->arrayLength = f["array_length"].toInt();
    }

    if (f.contains("array_length_field"))
    {
        field->arrayLengthField = f["array_length_field"].toString();
    }

    if (f.contains("compute"))
    {
        auto cObj = f["compute"].toObject();
        field->compute.algorithm = cObj["algorithm"].toString();
        field->compute.onMismatch = cObj["onMismatch"].toString();
        field->compute.scope = cObj.contains("scope") ? cObj["scope"].toString() : "payload";
        if (cObj.contains("range"))
        {
            auto rangeObj = cObj["range"].toObject();
            field->compute.rangeStart = rangeObj.contains("start") ? rangeObj["start"].toInt() : 0;
            field->compute.rangeEnd = rangeObj.contains("end") ? rangeObj["end"].toInt() : -1;
        }
    }

    if (f.contains("identifier") && f["identifier"].toBool())
    {
        field->isIdentifier = true;
        if (f.contains("match_values"))
        {
            auto arr = f["match_values"].toArray();
            for (const auto& val : arr)
            {
                if (val.isString())
                {
                    bool ok;
                    int parsed = val.toString().toInt(&ok, 16); 
                    if (ok) field->matchValues.push_back(parsed);
                }
                else
                {
                    field->matchValues.push_back(val.toInt());
                }
            }
        }
    }

    return field;
}

void ProtocolSchema::resolveCompositeFields()
{
    // Iterate all messages and templates
    for (auto& headerMap : mMessages)
    {
        for (auto& msg : headerMap)
        {
            for (int i = 0; i < msg->fields.size(); ++i)
            {
                auto& f = msg->fields[i];
                if (f->isMessage())
                {
                    auto comp = std::static_pointer_cast<MessageDef>(f);
                    if (!comp->templateName.isEmpty() && mMessagesByName.contains(comp->templateName))
                    {
                        auto tmpl = mMessagesByName[comp->templateName];
                        comp->fields = tmpl->fields; 
                    }
                    else
                    {
                        qWarning("Unknown composite type: %s", comp->templateName.toLocal8Bit().data());
                    }
                }
            }
        }
    }
    
    // Resolve in HeaderDefs
    for (auto& header : mHeaders)
    {
        for (int i = 0; i < header.fields.size(); ++i)
        {
            auto& f = header.fields[i];
            if (f->isMessage())
            {
                auto comp = std::static_pointer_cast<MessageDef>(f);
                if (!comp->templateName.isEmpty() && mMessagesByName.contains(comp->templateName))
                    comp->fields = mMessagesByName[comp->templateName]->fields;
            }
        }
    }
}
