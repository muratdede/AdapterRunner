#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include <QJsonObject>

#include "itransport.h"

ITransport* createTransport(const QJsonObject& obj);

#endif // TRANSPORTFACTORY_H
