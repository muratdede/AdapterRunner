#include "itransport.h"

QByteArray ITransport::consume(size_t maxBytes)
{
    QMutexLocker lock(&mMutex);

    if (mBuffer.isEmpty())
        return {};

    size_t n = std::min(maxBytes, (size_t)mBuffer.size());

    QByteArray out = mBuffer.left(n);

    mBuffer.remove(0, n);

    return out;
}

size_t ITransport::available() const
{
    QMutexLocker lock(&mMutex);

    return mBuffer.size();
}

void ITransport::appendData(const QByteArray& data)
{
    QMutexLocker lock(&mMutex);

    mBuffer.append(data);

    emit dataArrived();
}
