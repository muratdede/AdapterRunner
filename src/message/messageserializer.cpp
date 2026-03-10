#include "messageserializer.h"
#include "checksumfactory.h"
#include "Utils.h"

#include <QDebug>

MessageSerializer::MessageSerializer(const ProtocolSchema *schema)
    : mSchema(schema)
{
}

QByteArray MessageSerializer::buildHeader(const HeaderDef &header, const QMap<QString, QVariant> &values)
{
    QByteArray headerData(header.headerSize, '\0');

    for (const auto& fieldPtr : header.fields)
    {
        QSysInfo::Endian e = fieldPtr->hasEndianOverride ? fieldPtr->endian : header.endian;
        bool wroteCompute = false;

        if (!fieldPtr->isMessage())
        {
            auto field = std::static_pointer_cast<FieldDef>(fieldPtr);
            if (field->compute.hasCompute())
            {
                if (field->compute.scope == "frame")
                    continue;

                const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
                if (algo)
                {
                    uint8_t computedValue = algo->compute(headerData, *field);
                    Utils::writeField(headerData, field.get(), computedValue, header.endian, 0);
                }
                else
                {
                    qWarning() << "Unsupported checksum algorithm:" << field->compute.algorithm;
                }
                wroteCompute = true;
            }
        }

        if (!wroteCompute)
        {
            QVariant value = values.value(fieldPtr->name);
            Utils::writeField(headerData, fieldPtr.get(), value, e, 0);
        }
    }

    return headerData;
}

QByteArray MessageSerializer::buildPayload(const MessageDef &msgDef, const QMap<QString, QVariant> &values)
{
    QByteArray payload;

    for (const auto& fieldPtr : msgDef.fields)
    {
        int needed = fieldPtr->byteOffset + fieldPtr->getSize();
        if (payload.size() < needed)
            payload.resize(needed);

        if (!fieldPtr->isMessage())
        {
            auto field = std::static_pointer_cast<FieldDef>(fieldPtr);
            if (field->compute.hasCompute())
                continue; // Compute fields are calculated at frame level
        }

        QVariant v = values.value(fieldPtr->name);
        Utils::writeField(payload, fieldPtr.get(), v, msgDef.endian, 0);
    }

    return payload;
}


QByteArray MessageSerializer::buildFrame(const QString &messageName, const QMap<QString, QVariant> &values)
{
    const MessageDef* msgDef = mSchema->getMessageByName(messageName);

    if (!msgDef)
    {
        qWarning() << "MessageSerializer::buildFrame: unknown message" << messageName;
        return {};
    }

    const HeaderDef* header = mSchema->getHeader(msgDef->headerType);

    if (!header)
    {
        qWarning() << "MessageSerializer::buildFrame: unknown header type" << msgDef->headerType;
        return {};
    }

    QByteArray payload = buildPayload(*msgDef, values);
    QByteArray headerData = buildHeader(*header, values);

    QByteArray frame = headerData + payload;

    // compute header fields
    for (const auto& fieldPtr : header->fields)
    {
        if (fieldPtr->isMessage()) continue;
        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);

        if (!field->compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
        if (!algo) continue;

        QByteArray computeBuffer;
        FieldDef tmp = *field;

        if (field->compute.scope == "frame")
        {
            computeBuffer = frame;
        }
        else if (field->compute.scope == "header")
        {
            computeBuffer = headerData;
        }
        else  // "payload"
        {
            computeBuffer = payload;
        }

        uint8_t computedValue = algo->compute(computeBuffer, tmp);
        Utils::writeField(frame, field.get(), computedValue, header->endian, 0);
    }

    // compute payload fields
    for (const auto& fieldPtr : msgDef->fields)
    {
        if (fieldPtr->isMessage()) continue;
        auto field = std::static_pointer_cast<FieldDef>(fieldPtr);

        if (!field->compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field->compute.algorithm);
        if (!algo) continue;

        QByteArray computeBuffer;
        FieldDef tmp = *field;

        if (field->compute.scope == "frame")
        {
            computeBuffer = frame;
            tmp.byteOffset = header->headerSize + field->byteOffset;
        }
        else if (field->compute.scope == "header")
        {
            computeBuffer = headerData;
        }
        else  // "payload"
        {
            computeBuffer = payload;
        }

        uint32_t computedValue = algo->compute(computeBuffer, tmp);
        Utils::writeField(frame, field.get(), computedValue, msgDef->endian, header->headerSize);
    }

    return frame;
}
