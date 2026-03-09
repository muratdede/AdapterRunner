#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include <QJsonObject>

#include "itransport.h"
#include "isender.h"

ITransport* createTransport(const QJsonObject& obj);
ISender* createSender(const QJsonObject& obj);

#endif // TRANSPORTFACTORY_H
