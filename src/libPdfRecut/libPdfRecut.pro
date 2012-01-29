#-------------------------------------------------
#
# Project created by QtCreator 2012-01-29T17:30:02
#
#-------------------------------------------------


TARGET = libPdfRecut
TEMPLATE = lib
CONFIG += staticlib

DEFINES += LIBPDFRECUT_LIBRARY

INCLUDEPATH += $$PWD/../3rdparty
win32 {
    INCLUDEPATH += ../PoDoFo/win32/include
    # INCLUDEPATH += "C:/Program Files/GnuWin32/include"
    LIBS += -L../PoDoFo/win32/lib
    LIBS += -L"C:/Program Files/GnuWin32/lib"
    LIBS += -lpodofo -lfreetype -ljpeg -lz
    LIBS += -lpthread -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -lWS2_32
    LIBS +=  -lpoppler-qt4
}
unix {
    # INCLUDEPATH += ../PoDoFo/linux32/include
    # LIBS += -L../PoDoFo/linux32/lib
    # INCLUDEPATH += /usr/include/c++/4.6
    INCLUDEPATH += $$PWD/../../../PoDoFo/linux64/include
    LIBS += -L$$PWD/../../../PoDoFo/linux64/lib64
    LIBS +=  -lpodofod -lfreetype -lfontconfig -ljpeg -lz
    LIBS +=  -lpoppler-qt4
}

SOURCES += LibPdfRecut.cpp \
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

HEADERS += LibPdfRecut.h\
        libPdfRecut_global.h \
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

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE8A37148
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = libPdfRecut.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}


