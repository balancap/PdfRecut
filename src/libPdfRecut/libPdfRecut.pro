#-------------------------------------------------
#
# Project created by QtCreator 2012-01-29T17:30:02
#
#-------------------------------------------------


TARGET = libPdfRecut
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPDFRECUT_LIBRARY

SOURCES += \
    PdfStreamTokenizer.cpp \
    PdfStreamLayoutZone.cpp \
    PdfStreamAnalysis.cpp \
    PdfRenderPage.cpp \
    PdfPath.cpp \
    PdfGraphicsState.cpp \
    PdfFontMetricsType3.cpp \
    PdfFontMetricsCache.cpp \
    PdfFontMetrics14.cpp \
    PdfDocumentTools.cpp \
    PdfDocumentStructure.cpp \
    PdfDocumentLayout.cpp \
    PdfDocumentHandle.cpp \
    PdfDocException.cpp \
    PDSTwoColumns.cpp \
    PDSBook.cpp

HEADERS +=\
    PdfTypes.h \
    PdfStreamTokenizer.h \
    PdfStreamLayoutZone.h \
    PdfStreamAnalysis.h \
    PdfSemaphore.h \
    PdfRenderPage.h \
    PdfPath.h \
    PdfMisc.h \
    PdfGraphicsState.h \
    PdfGraphicsOperators.h \
    PdfFontMetricsType3.h \
    PdfFontMetricsCache.h \
    PdfFontMetrics14.h \
    PdfDocumentTools.h \
    PdfDocumentStructure.h \
    PdfDocumentLayout.h \
    PdfDocumentHandle.h \
    PdfDocException.h \
    PDSTwoColumns.h \
    PDSBook.h

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

