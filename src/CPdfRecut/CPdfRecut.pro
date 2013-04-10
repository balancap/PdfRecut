##################################################################
##                           CPdfRecut                          ##
##################################################################

QT       += core
QT       += gui

TARGET = CPdfRecut
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app

### Sources !
SOURCES += main.cpp

include( $$PWD/../3rdparty/QsLog/QsLog.pri )

### libPoDoFoExtended
INCLUDEPATH += $$PWD/../libPoDoFoExtended
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/release/ -llibPdfRecut
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/debug/ -llibPdfRecut
else:unix: LIBS += -L$$OUT_PWD/../libPdfRecut/ -llibPdfRecut

DEPENDPATH += $$PWD/../libPoDoFoExtended
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/release/libPdfRecut.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/debug/libPdfRecut.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/liblibPdfRecut.a

### libPdfRecut
INCLUDEPATH += $$PWD/../libPdfRecut
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/release/ -llibPoDoFoExtended
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/debug/ -llibPoDoFoExtended
else:unix:!symbian: LIBS += -L$$OUT_PWD/../libPoDoFoExtended/ -llibPoDoFoExtended

DEPENDPATH += $$PWD/../libPdfRecut
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/release/libPoDoFoExtended.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/debug/libPoDoFoExtended.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/liblibPoDoFoExtended.a

### PdfRecut Dependencies (include+libs)
### Caution: order matters for compilation !
include( $$PWD/../PRDependencies.pri )
