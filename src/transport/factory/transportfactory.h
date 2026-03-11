#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include <QJsonObject>

#include "src/transport/interfaces/itransport.h"
#include "src/transport/interfaces/isender.h"

ITransport* createTransport(const QJsonObject& obj);
ISender* createSender(const QJsonObject& obj);

#endif // TRANSPORTFACTORY_H
