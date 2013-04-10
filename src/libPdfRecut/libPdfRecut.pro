##################################################################
##                           libPdfRecut                        ##
##################################################################

DEFINES += LIBPDFRECUT_LIBRARY
TARGET = libPdfRecut
TEMPLATE = lib
CONFIG += staticlib

### PRGeometry and PRStructure sources files.
include( $$PWD/PRGeometry/PRGeometry.pri )
include( $$PWD/PRStructure/PRStructure.pri )

### PdfRecut Dependencies (include+libs)
include( $$PWD/../PRDependencies.pri )

SOURCES += \
    PRDocument.cpp \
    PRException.cpp \
    PRDocumentLayout.cpp \
    PRDocumentTools.cpp \
    PRStreamLayoutZone.cpp \
    PRRenderPage.cpp \
    PRUtils.cpp \
    PRPage.cpp

HEADERS +=\
    PRDocument.h \
    PRException.h \
    PRDocumentLayout.h \
    PRDocumentTools.h \
    PRStreamLayoutZone.h \
    PRRenderPage.h \
    PRUtils.h \
    PRPage.h

### libPoDoFoExtended
INCLUDEPATH += $$PWD/../libPoDoFoExtended
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/release/ -llibPoDoFoExtended
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/debug/ -llibPoDoFoExtended
else:unix:!symbian: LIBS += -L$$OUT_PWD/../libPoDoFoExtended/ -llibPoDoFoExtended

DEPENDPATH += $$PWD/../libPoDoFoExtended
win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/release/libPoDoFoExtended.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/debug/libPoDoFoExtended.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/liblibPoDoFoExtended.a
