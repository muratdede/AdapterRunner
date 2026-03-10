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

    for (const FieldDef& field : header.fields)
    {
        QSysInfo::Endian e = field.hasEndianOverride ? field.endian : header.endian;

        if (field.compute.hasCompute())
        {
            // Skip frame-scope computes — they are deferred to buildFrame()
            if (field.compute.scope == "frame")
                continue;

            const IChecksumAlgorithm* algo = ChecksumFactory::create(field.compute.algorithm);
            if (algo)
            {
                uint8_t computedValue = algo->compute(headerData, field);
                Utils::writeField(headerData, field, computedValue, header.endian);
            }
            else
            {
                qWarning() << "MessageSerializer: unknown compute algorithm:" << field.compute.algorithm
                           << "for field" << field.name;
            }
        }
        else
        {
            QVariant value = values.value(field.name);
            Utils::writeField(headerData, field, value, e);
        }
    }

    return headerData;
}

QByteArray MessageSerializer::buildPayload(const MessageDef &msgDef, const QMap<QString, QVariant> &values)
{
    QByteArray payload;

    for (const FieldDef& field : msgDef.fields)
    {
        int needed = field.byteOffset + field.size;
        if (payload.size() < needed)
            payload.resize(needed);

        if (field.compute.hasCompute())
            continue;

        QVariant v = values.value(field.name);
        Utils::writeField(payload, field, v, msgDef.endian);
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
    for (const FieldDef& field : header->fields)
    {
        FieldDef tmp = field;

        if (!field.compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field.compute.algorithm);
        if (!algo) continue;

        // Build the buffer based on scope
        QByteArray computeBuffer;
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
        QVariant computedValue = algo->compute(computeBuffer, tmp);

        Utils::writeField(frame, tmp, computedValue, header->endian);
    }

    // compute payload fields
    for (const FieldDef& field : msgDef->fields)
    {
        FieldDef tmp = field;

        if (!field.compute.hasCompute())
            continue;

        const IChecksumAlgorithm* algo = ChecksumFactory::create(field.compute.algorithm);
        if (!algo) continue;

        // Build the buffer based on scope
        QByteArray computeBuffer;
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
        QVariant computedValue = algo->compute(computeBuffer, tmp);

        Utils::writeField(frame, tmp, computedValue, header->endian);
    }

    return frame;
}
