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
        src/network/itransport.cpp \
        src/message/messageparser.cpp \
        src/message/protocolschema.cpp \
        src/network/serialtransport.cpp \
        src/network/tcptransport.cpp \
        src/network/transportfactory.cpp \
        src/network/transportmanager.cpp \
        src/network/udptransport.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/message/Utils.h \
    src/message/fielddef.h \
    src/network/itransport.h \
    src/message/headerdef.h \
    src/message/messagedef.h \
    src/message/messageparser.h \
    src/message/parsedheader.h \
    src/message/parsedmessage.h \
    src/message/protocolschema.h \
    src/network/serialtransport.h \
    src/network/tcptransport.h \
    src/network/transportfactory.h \
    src/network/transportmanager.h \
    src/network/udptransport.h
