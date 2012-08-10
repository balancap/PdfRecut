##################################################################
##                       libPoDoFoExtended                      ##
##################################################################

QT       += gui

TARGET = libPoDoFoExtended
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPODOFOEXTENDED_LIBRARY

SOURCES += \
    PdfCMap.cpp \
    PdfStreamTokenizer.cpp \
    PdfResources.cpp \
    PdfPath.cpp \
    PdfGraphicsState.cpp \
    PdfFontMetricsType3.cpp \
    PdfFontMetricsType0.cpp \
    PdfFontMetricsCache.cpp \
    PdfFontMetrics14.cpp

HEADERS += \
    PdfTypes.h \
    PdfStreamTokenizer.h \
    PdfSemaphore.h \
    PdfResources.h \
    PdfPath.h \
    PdfMisc.h \
    PdfGraphicsState.h \
    PdfGraphicsOperators.h \
    PdfFontMetricsType3.h \
    PdfFontMetricsType0.h \
    PdfFontMetricsCache.h \
    PdfFontMetrics14.h \
    PdfCMap.h

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
