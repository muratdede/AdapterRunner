QT -= gui
QT += network serialport qml

CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/core/utils.cpp \
    src/core/bathread.cpp \
    src/transport/network/basynctcpdatareceiver.cpp \
    src/transport/network/basynctcpdatasender.cpp \
    src/transport/network/basyncudpdatareceiver.cpp \
    src/transport/network/basyncudpdatasender.cpp \
    src/transport/serial/bserialsenderreceiver.cpp \
    src/transport/factory/transportfactory.cpp \
    src/transport/manager/transportmanager.cpp \
    src/protocol/parser/messageparser.cpp \
    src/protocol/serializer/messageserializer.cpp \
    src/protocol/schema/protocolschema.cpp \
    src/engine/evaluator/expressioneval.cpp \
    src/engine/core/reactionengine.cpp \
    src/engine/tracker/sourcetracker.cpp \
    src/behavior/impls/respondbehaviour.cpp \
    src/behavior/impls/respondwithlastbehaviour.cpp \
    src/behavior/impls/periodicsenderbehaviour.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/core/jsonutils.h \
    src/core/utils.h \
    src/core/bathread.h \
    src/transport/network/basynctcpdatareceiver.h \
    src/transport/network/basynctcpdatasender.h \
    src/transport/network/basyncudpdatareceiver.h \
    src/transport/network/basyncudpdatasender.h \
    src/transport/network/bathreadnetworkreceiver.h \
    src/transport/serial/bserialsenderreceiver.h \
    src/transport/interfaces/isender.h \
    src/transport/interfaces/itransport.h \
    src/transport/factory/transportfactory.h \
    src/transport/manager/transportmanager.h \
    src/protocol/types/defs.h \
    src/protocol/checksum/interfaces/ichecksumalgorithm.h \
    src/protocol/checksum/impls/checksum2c.h \
    src/protocol/checksum/factory/checksumfactory.h \
    src/protocol/parser/messageparser.h \
    src/protocol/serializer/messageserializer.h \
    src/protocol/types/parsedheader.h \
    src/protocol/types/parsedmessage.h \
    src/protocol/schema/protocolschema.h \
    src/protocol/schema/fieldmapping.h \
    src/engine/evaluator/expressioneval.h \
    src/engine/core/reactionengine.h \
    src/engine/tracker/sourcetracker.h \
    src/behavior/interfaces/ibehaviour.h \
    src/behavior/impls/respondbehaviour.h \
    src/behavior/impls/respondwithlastbehaviour.h \
    src/behavior/impls/periodicsenderbehaviour.h
