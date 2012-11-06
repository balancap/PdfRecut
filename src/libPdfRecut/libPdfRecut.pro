##################################################################
##                           libPdfRecut                        ##
##################################################################

TARGET = libPdfRecut
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPDFRECUT_LIBRARY

SOURCES += \
    PRDocument.cpp \
    PRSubDocument.cpp \
    PRPage.cpp \
    PRException.cpp \
    PRDocumentLayout.cpp \
    PRDocumentStructure.cpp \
    PRDocumentTools.cpp \
    PRStreamLayoutZone.cpp \
    PRRenderPage.cpp \
    PRTextWords.cpp \
    PRTextLine.cpp \
    PRTextStructure.cpp \
    PRTextPage.cpp

HEADERS +=\
    PRDocument.h \
    PRSubDocument.h \
    PRPage.h \
    PRException.h \
    PRDocumentLayout.h \
    PRDocumentStructure.h \
    PRDocumentTools.h \
    PRStreamLayoutZone.h \
    PRRenderPage.h \
    PRTextLine.h \
    PRTextStructure.h \
    PRTextWords.h \
    PRTextPage.h

INCLUDEPATH += $$PWD/../3rdparty
win32 {
    INCLUDEPATH += ../PoDoFo/win32/include
    # INCLUDEPATH += "C:/Program Files/GnuWin32/include"
}
unix {
    INCLUDEPATH += $$PWD/../../../PoDoFo/linux64/include
    INCLUDEPATH += /usr/include/freetype2
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/release/ -llibPoDoFoExtended
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPoDoFoExtended/debug/ -llibPoDoFoExtended
else:unix:!symbian: LIBS += -L$$OUT_PWD/../libPoDoFoExtended/ -llibPoDoFoExtended

INCLUDEPATH += $$PWD/../libPoDoFoExtended
INCLUDEPATH += $$PWD/../3rdparty

DEPENDPATH += $$PWD/../libPoDoFoExtended

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/release/libPoDoFoExtended.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/debug/libPoDoFoExtended.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPoDoFoExtended/liblibPoDoFoExtended.a
