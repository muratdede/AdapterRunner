#include "protocolschema.h"

#include "Utils.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

bool ProtocolSchema::load(const QString &path)
{
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

        auto header = std::make_shared<HeaderDef>();
        header->name = obj["name"].toString();
        header->headerSize = obj["header_size"].toInt();

        if (obj.contains("endianness"))
            header->endian = parseEndianness(obj["endianness"].toString());
        else
            header->endian = QSysInfo::LittleEndian;

        for (auto fieldVal : obj["fields"].toArray()) {
            auto fieldObj = fieldVal.toObject();

            auto field = parseField(fieldObj, header->endian);

            header->fields.push_back(field);
        }

        mHeaders.insert(header->name, header);
        mTypesByName.insert(header->name, header);
    }

    /*
      TYPES
    */
    for (auto typeVal : root["types"].toArray()) {
        auto obj = typeVal.toObject();

        auto msg = std::make_shared<MessageDef>();

        msg->id = obj["id"].toInt(-1);
        msg->name = obj["name"].toString();

        if (obj.contains("endianness"))
            msg->endian = parseEndianness(obj["endianness"].toString());
        else
            msg->endian = QSysInfo::LittleEndian;

        for (auto fieldVal : obj["fields"].toArray()) {
            auto field = parseField(fieldVal.toObject(), msg->endian);

            msg->fields.push_back(field);
        }

        mTypesByName.insert(msg->name, msg);
    }

    /*
      MESSAGES
    */
    for (auto msgVal : root["messages"].toArray()) {
        auto obj = msgVal.toObject();

        auto msg = std::make_shared<MessageDef>();

        msg->id = obj["id"].toInt();
        msg->name = obj["name"].toString();

        if (obj.contains("endianness"))
            msg->endian = parseEndianness(obj["endianness"].toString());
        else
            msg->endian = QSysInfo::LittleEndian;

        for (auto fieldVal : obj["fields"].toArray()) {
            auto field = parseField(fieldVal.toObject(), msg->endian);

            msg->fields.push_back(field);
        }

        mMessagesByName.insert(msg->name, msg);
        mMessagesById.insert(msg->id, msg);

        resolveCompositeFields(msg);
    }

    return true;
}

std::shared_ptr<HeaderDef> ProtocolSchema::findHeader(const QByteArray &data) const
{
    for (auto &header : mHeaders) {
        if (matchesHeader(data, header))
            return header;
    }

    return nullptr;
}

std::shared_ptr<MessageDef> ProtocolSchema::getMessageById(int msgId) const
{
    auto it = mMessagesById.find(msgId);

    if (it == mMessagesById.end())
        return nullptr;

    return it.value();
}

std::shared_ptr<MessageDef> ProtocolSchema::getMessageByName(const QString& name) const
{
    auto it = mMessagesByName.find(name);

    if (it == mMessagesByName.end())
        return nullptr;

    return it.value();
}

std::shared_ptr<HeaderDef> ProtocolSchema::getHeader(const QString& name) const
{
    auto it = mHeaders.find(name);

    if (it == mHeaders.end())
        return nullptr;

    return it.value();
}

bool ProtocolSchema::matchesHeader(const QByteArray &data, std::shared_ptr<HeaderDef> header) const
{
    if (data.size() < header->headerSize)
        return false;

    // Check all identifier fields
    for (const auto& fieldPtr : header->fields)
    {
        if (fieldPtr->isMessage()) // TODO: child fields' identifier in header ignored
            continue;

        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);

        if (field->matchValues.empty())
            continue;

        if (data.size() < field->byteOffset + field->size)
            return false;

        QVariant value = Utils::readField(data, fieldPtr.get(), header->endian);

        if (!field->matchValues.contains(value.toInt())) // BUG: may produce bug if field is not integer type
            return false;
    }

    return true;
}

QSysInfo::Endian ProtocolSchema::parseEndianness(const QString &str)
{
    if (str == "big")
        return QSysInfo::BigEndian;

    return QSysInfo::LittleEndian;
}

FieldType ProtocolSchema::parseFieldType(const QString &type)
{
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

int ProtocolSchema::parseTypeSize(const QString &type)
{
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
    QString typeName = f["type"].toString();

    // Check if primitive
    FieldType pType;
    bool isPrimitive = true;
    try {
        pType = parseFieldType(typeName);
    } catch (...) {
        isPrimitive = false;
    }

    if (!isPrimitive)
    {
        auto child = std::make_shared<MessageDef>();
        child->name = f["name"].toString();
        child->typeName = typeName;
        child->byteOffset = f["byte_offset"].toInt();
        child->arrayLength = f["array_length"].toInt(child->arrayLength);
        child->arrayLengthField = f["array_length_field"].toString(child->arrayLengthField);

        child->endian = defaultEndian;
        if (f.contains("endianness"))
        {
            child->endian = parseEndianness(f["endianness"].toString());
        }

        return child;
    }

    auto field = std::make_shared<FieldDef>();
    field->name = f["name"].toString();
    field->type = pType;
    field->size = parseTypeSize(typeName);
    field->byteOffset = f["byte_offset"].toInt();
    field->bitOffset = f["bit_offset"].toInt(field->bitOffset);
    field->bitLength = f["bit_length"].toInt(field->bitLength);
    field->arrayLength = f["array_length"].toInt(field->arrayLength);
    field->arrayLengthField = f["array_length_field"].toString();

    field->endian = defaultEndian;
    if (f.contains("endianness"))
    {
        field->endian = parseEndianness(f["endianness"].toString());
    }

    if (f.contains("compute"))
    {
        auto cObj = f["compute"].toObject();
        field->compute.algorithm = cObj["algorithm"].toString();
        field->compute.onMismatch = cObj["onMismatch"].toString();
    }

    if (f.contains("identifier") && f["identifier"].toBool())
    {
        field->isIdentifier = true;
    }

    if (f.contains("match_values"))
    {
        auto arr = f["match_values"].toArray();
        for (const auto& val : qAsConst(arr))
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

    return field;
}

void ProtocolSchema::resolveCompositeFields(std::shared_ptr<MessageDef> message)
{
    for (auto& field : message.get()->fields)
    {
        if (!field->isMessage())
            return;

        auto comp = std::static_pointer_cast<MessageDef>(field);

        if (comp->typeName.isEmpty() || !mTypesByName.contains(comp->typeName)) {
            qWarning("Unknown composite type: %s", comp->typeName.toLocal8Bit().data());
            return;
        }

        auto tmpl = mTypesByName[comp->typeName];
        comp->fields = tmpl->fields;

        resolveCompositeFields(comp);
    }
}
