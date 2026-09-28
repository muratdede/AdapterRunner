#include "messageserializer.h"

#include <QDebug>

#include "src/protocol/checksum/factory/checksumfactory.h"
#include "src/core/utils.h"

MessageSerializer::MessageSerializer(const ProtocolSchema *schema)
    : mSchema(schema)
{

}

QByteArray MessageSerializer::buildPayload(const AbstractMessage &msgDef, const QMap<QString, QVariant> &values)
{
    QByteArray payload;
    int currentOffset = 0;

    for (const auto& fieldPtr : msgDef.fields)
    {
        QSysInfo::Endian endian = fieldPtr->hasEndianOverride ? fieldPtr->endian : msgDef.endian;

        if (!fieldPtr->isMessage())
        {
            auto field = std::static_pointer_cast<FieldDef>(fieldPtr);
            if (field->compute.hasCompute())
            {
                Utils::writeField(payload, field.get(), 0, endian, 0, &currentOffset); // Placeholder to advance offset

                continue;
            }
        }

        QVariant value = values.value(fieldPtr->name);
        Utils::writeField(payload, fieldPtr.get(), value, endian, 0, &currentOffset);
    }

    for (const auto& fieldPtr : msgDef.fields)
    {
        if (fieldPtr->isMessage())
            continue;

        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);
        if (!field->compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
        if (algo)
        {
            QSysInfo::Endian endian = fieldPtr->hasEndianOverride ? fieldPtr->endian : msgDef.endian;
            QVariant computedValue = algo->compute(payload, 0, *field);
            int off = field->byteOffset;
            Utils::writeField(payload, field.get(), computedValue, endian, 0, &off);
        }
        else
        {
            qWarning() << "Unsupported checksum algorithm:" << field->compute.algorithm;
        }
    }

    return payload;
}

QByteArray MessageSerializer::buildFrame(const QString &messageName, const QMap<QString, QVariant> &values)
{
    auto msgDef = mSchema->getMessageByName(messageName);

    if (!msgDef)
    {
        qWarning() << "MessageSerializer::buildFrame: unknown message" << messageName;
        return {};
    }

    return buildPayload(*msgDef, values);
}
