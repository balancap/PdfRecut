##################################################################
##                       libPoDoFoExtended                      ##
##################################################################

DEFINES += LIBPODOFOEXTENDED_LIBRARY
TARGET = libPoDoFoExtended
TEMPLATE = lib
CONFIG += staticlib
QT += gui

### PdfRecut Dependencies (include+libs)
include( $$PWD/../PRDependencies.pri )

### Sources files...
SOURCES += \
    PdfeTypes.cpp \
    PdfeFontDescriptor.cpp \
    PdfeEncoding.cpp \
    PdfeFont.cpp \
    PdfeFontType0.cpp \
    PdfeFontType1.cpp \
    PdfeFontType3.cpp \
    PdfeFontTrueType.cpp \
    PdfeCMap.cpp \
    PdfeGraphicsState.cpp \
    PdfeResources.cpp \
    PdfeStreamTokenizer.cpp \
    PdfeCanvasAnalysis.cpp \
    PdfeUtils.cpp \
    PdfeContentsStream.cpp \
    PdfeContentsAnalysis.cpp \
    PdfeGElement.cpp \
    PdfePath.cpp \
    PdfeTextElement.cpp \
    PdfeData.cpp

HEADERS += \
    PdfeTypes.h \
    PdfeFontDescriptor.h \
    PdfeEncoding.h \
    PdfeFont.h \
    PdfeFontType0.h \
    PdfeFontType1.h \
    PdfeFontType3.h \
    PdfeFontTrueType.h \
    PdfeCMap.h \
    PdfeGraphicsState.h \
    PdfeGraphicsOperators.h \
    PdfeMisc.h \
    PdfeSemaphore.h \
    PdfeResources.h \
    PdfeStreamTokenizer.h \
    PdfeCanvasAnalysis.h \
    PdfeUtils.h \
    PdfeContentsStream.h \
    PdfeContentsAnalysis.h \
    PdfeGElement.h \
    PdfePath.h \
    PdfeTextElement.h \
    PdfeData.h
