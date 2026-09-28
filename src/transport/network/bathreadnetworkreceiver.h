#ifndef BATHREADNETWORKRECEIVER_H
#define BATHREADNETWORKRECEIVER_H

#include "src/transport/interfaces/itransport.h"

//referances by https://forum.qt.io/topic/40653/qobject-movetothread-cannot-move-objects-with-a-parent/8
//don't assign parent that is working main thread
class BAThreadNetworkReceiver : public ITransport
{
    Q_OBJECT
public:
    explicit BAThreadNetworkReceiver() : ITransport() {}
    virtual ~BAThreadNetworkReceiver(){}

};

#endif // BATHREADNETWORKRECEIVER_H
