QT -= gui
QT = network serialport

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
    src/conn/network/basyncudpdatareceiver.cpp \
    src/conn/network/basyncudpdatasender.cpp \
    src/conn/serial/bserialsenderreceiver.cpp \
    src/conn/transport/transportfactory.cpp \
    src/conn/transport/transportmanager.cpp \
    src/message/messageparser.cpp \
    src/message/protocolschema.cpp \
    src/thread/bathread.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/conn/network/basyncudpdatareceiver.h \
    src/conn/network/basyncudpdatasender.h \
    src/conn/network/bathreadnetworkreceiver.h \
    src/conn/serial/bserialsenderreceiver.h \
    src/conn/transport/itransport.h \
    src/conn/transport/transportfactory.h \
    src/conn/transport/transportmanager.h \
    src/message/Utils.h \
    src/message/fielddef.h \
    src/message/headerdef.h \
    src/message/messagedef.h \
    src/message/messageparser.h \
    src/message/parsedheader.h \
    src/message/parsedmessage.h \
    src/message/protocolschema.h \
    src/thread/bathread.h
