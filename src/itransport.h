#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <QByteArray>

class ITransport
{
public:
    virtual QByteArray read() = 0;
};

#endif // ITRANSPORT_H
