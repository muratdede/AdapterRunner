#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <QMutex>
#include <QObject>

class ITransport : public QObject
{
    Q_OBJECT
public:
    virtual bool start() = 0;
    virtual void stop() = 0;

    QByteArray consume(size_t maxBytes = SIZE_MAX);

    size_t available() const;

signals:

    void dataArrived();

protected:

    void appendData(const QByteArray& data);

private:

    QByteArray mBuffer;
    mutable QMutex mMutex;
};

#endif // ITRANSPORT_H
