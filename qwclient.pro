#-------------------------------------------------
#
# Project created by QtCreator 2012-08-16T04:30:39
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = qwclient
TEMPLATE = lib

DEFINES += QWCLIENT_LIBRARY

SOURCES += QWClient.cpp \
    QWClientPrivate.cpp \
    QWPack.cpp \
    QWTables.cc

HEADERS += QWClient.h\
        qwclient_global.h \
    QWClientPrivate.h \
    quakedef.h \
    QWPack.h \
    QWTables.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE3787C8C
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = qwclient.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
