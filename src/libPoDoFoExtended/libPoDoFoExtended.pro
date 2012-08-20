##################################################################
##                       libPoDoFoExtended                      ##
##################################################################

QT       += gui

TARGET = libPoDoFoExtended
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPODOFOEXTENDED_LIBRARY

SOURCES += \
    PdfStreamTokenizer.cpp \
    PdfResources.cpp \
    PdfPath.cpp \
    PdfGraphicsState.cpp \
    PdfeFont.cpp \
    PdfeFontDescriptor.cpp \
    PdfeFontType0.cpp \
    PdfeFontType1.cpp \
    PdfeFontType3.cpp \
    PdfeFontTrueType.cpp \
    PdfeCMap.cpp

HEADERS += \
    PdfStreamTokenizer.h \
    PdfSemaphore.h \
    PdfResources.h \
    PdfPath.h \
    PdfMisc.h \
    PdfGraphicsState.h \
    PdfGraphicsOperators.h \
    PdfeFont.h \
    PdfeTypes.h \
    PdfeFontDescriptor.h \
    PdfeFontType0.h \
    PdfeFontType1.h \
    PdfeFontType3.h \
    PdfeFontTrueType.h \
    PdfeCMap.h

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
