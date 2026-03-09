#ifndef ISENDER_H
#define ISENDER_H

#include "src/thread/bathread.h"

class ISender : public BAThread
{
    Q_OBJECT
public slots:
    virtual void send(const QByteArray& data) = 0;
};

#endif // ISENDER_H
