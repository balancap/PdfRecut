#-------------------------------------------------
#
# Project created by QtCreator 2012-08-10T17:01:32
#
#-------------------------------------------------

QT       -= gui

TARGET = libPoDoFoExtended
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPODOFOEXTENDED_LIBRARY

SOURCES +=

HEADERS +=

INCLUDEPATH += $$PWD/../3rdparty
win32 {
    INCLUDEPATH += ../PoDoFo/win32/include
    # INCLUDEPATH += "C:/Program Files/GnuWin32/include"
}
unix {
    INCLUDEPATH += $$PWD/../../../PoDoFo/linux64/include
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
