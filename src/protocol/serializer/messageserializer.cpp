#include "src/protocol/serializer/messageserializer.h"
#include "src/protocol/checksum/factory/checksumfactory.h"
#include "src/core/utils.h"

#include <QDebug>

MessageSerializer::MessageSerializer(const ProtocolSchema *schema)
    : mSchema(schema)
{

}

QByteArray MessageSerializer::buildPayload(const AbstractMessage &msgDef, const QMap<QString, QVariant> &values)
{
    QByteArray payload;

    for (const auto& fieldPtr : msgDef.fields)
    {
        int needed = fieldPtr->byteOffset + fieldPtr->getSize();
        if (payload.size() < needed)
            payload.resize(needed);

        QSysInfo::Endian endian = fieldPtr->hasEndianOverride ? fieldPtr->endian : msgDef.endian;

        if (!fieldPtr->isMessage())
        {
            auto field = std::static_pointer_cast<FieldDef>(fieldPtr);
            if (field->compute.hasCompute())
            {
                const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
                if (algo)
                {
                    QVariant computedValue = algo->compute(payload, 0, *field);
                    Utils::writeField(payload, field.get(), computedValue, endian, 0);
                }
                else
                {
                    qWarning() << "Unsupported checksum algorithm:" << field->compute.algorithm;
                }

                continue;
            }
        }

        QVariant value = values.value(fieldPtr->name);
        Utils::writeField(payload, fieldPtr.get(), value, endian, 0);
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

    QByteArray payload = buildPayload(*msgDef, values);

    for (const auto& fieldPtr : msgDef->fields)
    {
        if (fieldPtr->isMessage())
            continue;

        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);

        if (!field->compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
        if (!algo) continue;

        FieldDef tmp = *field;

        QVariant computedValue = algo->compute(payload, 0, tmp);
        Utils::writeField(payload, field.get(), computedValue, msgDef->endian, 0);
    }

    return payload;
}
