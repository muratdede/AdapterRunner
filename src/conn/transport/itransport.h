#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include "src/thread/bathread.h"

class ITransport : public BAThread
{
    Q_OBJECT
signals:
    void newMessageFromRemote(QByteArray pBuffer);
};

#endif // ITRANSPORT_H
