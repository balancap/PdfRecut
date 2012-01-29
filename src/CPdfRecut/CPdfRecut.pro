#-------------------------------------------------
#
# Project created by QtCreator 2012-01-29T17:45:24
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CPdfRecut
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/release/ -llibPdfRecut
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/debug/ -llibPdfRecut
else:symbian: LIBS += -llibPdfRecut
else:unix: LIBS += -L$$OUT_PWD/../libPdfRecut/ -llibPdfRecut

INCLUDEPATH += $$PWD/../libPdfRecut
DEPENDPATH += $$PWD/../libPdfRecut

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/release/libPdfRecut.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/debug/libPdfRecut.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/liblibPdfRecut.a
