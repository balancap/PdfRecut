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
    PdfPath.cpp \
    PdfGraphicsState.cpp \
    PdfFontMetricsType3.cpp \
    PdfFontMetricsCache.cpp \
    PdfFontMetrics14.cpp \
    PRDocument.cpp \
    PRException.cpp \
    PRDocumentLayout.cpp \
    PRDocumentStructure.cpp \
    PRDocumentTools.cpp \
    PRStreamAnalysis.cpp \
    PRStreamLayoutZone.cpp \
    PRRenderPage.cpp \
    PdfResources.cpp \
    PRPageStatistics.cpp

HEADERS +=\
    PdfTypes.h \
    PdfStreamTokenizer.h \
    PdfSemaphore.h \
    PdfPath.h \
    PdfMisc.h \
    PdfGraphicsState.h \
    PdfGraphicsOperators.h \
    PdfFontMetricsType3.h \
    PdfFontMetricsCache.h \
    PdfFontMetrics14.h \
    PRDocument.h \
    PRException.h \
    PRDocumentLayout.h \
    PRDocumentStructure.h \
    PRDocumentTools.h \
    PRRenderPage.h \
    PRStreamAnalysis.h \
    PRStreamLayoutZone.h \
    PdfResources.h \
    PRPageStatistics.h

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

