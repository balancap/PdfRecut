#-------------------------------------------------
#
# Project created by QtCreator 2012-01-29T17:45:24
#
#-------------------------------------------------

QT       += core
QT       += gui

TARGET = CPdfRecut
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp


INCLUDEPATH += $$PWD/../3rdparty
INCLUDEPATH += $$PWD/../libPdfRecut
DEPENDPATH += $$PWD/../libPdfRecut

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/release/ -llibPdfRecut
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libPdfRecut/debug/ -llibPdfRecut
else:symbian: LIBS += -llibPdfRecut
else:unix: LIBS += -L$$OUT_PWD/../libPdfRecut/ -llibPdfRecut


win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/release/libPdfRecut.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/debug/libPdfRecut.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../libPdfRecut/liblibPdfRecut.a

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
    INCLUDEPATH += $$PWD/../../../PoDoFo/linux64/include

    LIBS += -L$$PWD/../../../PoDoFo/linux64/lib64
    LIBS +=  -lpodofod -lfreetype -lfontconfig -ljpeg -lz
    LIBS +=  -lpoppler-qt4
}
