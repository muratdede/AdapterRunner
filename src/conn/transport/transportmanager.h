#ifndef TRANSPORTMANAGER_H
#define TRANSPORTMANAGER_H

#include "itransport.h"

#include <QHash>

class TransportManager
{
public:
    bool load(const QString& path);

    ITransport* get(const QString& name);

private:
    QHash<QString, ITransport*> mTransports;
};

#endif // TRANSPORTMANAGER_H
