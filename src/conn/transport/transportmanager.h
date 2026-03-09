#ifndef TRANSPORTMANAGER_H
#define TRANSPORTMANAGER_H

#include "itransport.h"
#include "isender.h"

#include <QHash>

class TransportManager
{
public:
    bool load(const QString& path);

    ITransport* getReceiver(const QString& name);
    ISender* getSender(const QString& name);

    QHash<QString, ITransport*>& receivers();

private:
    QHash<QString, ITransport*> mReceivers;
    QHash<QString, ISender*> mSenders;
};

#endif // TRANSPORTMANAGER_H
