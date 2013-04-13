##################################################################
##                      PdfRecut Dependencies                   ##
##################################################################

### 3rd party librairies (QsLog, vmmlib, ...)
INCLUDEPATH += $$PWD/3rdparty

### PoDoFo and its dependencies
win32 {
    ### PoDoFo path
    debug {
        INCLUDEPATH += $$PWD/../../../PoDoFo/podofo-win64-debug/include
        LIBS += -L$$PWD/../../../PoDoFo/podofo-win64-debug/lib
    }
    release {
        INCLUDEPATH += $$PWD/../../../PoDoFo/podofo-win64-release/include
        LIBS += -L$$PWD/../../../PoDoFo/podofo-win64-release/lib
    }
    # INCLUDEPATH += "C:/Program Files/GnuWin32/include"
    LIBS += -L"C:/Program Files/GnuWin32/lib"

    LIBS += -lpodofod -lfreetype -ljpeg -lz
    LIBS += -lpthread -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -lWS2_32
    # LIBS += -lpoppler-qt4
}
unix {
    ### PoDoFo path
    debug {
        INCLUDEPATH += $$PWD/../../../PoDoFo/podofo-linux64-debug/include
        LIBS += -L$$PWD/../../../PoDoFo/podofo-linux64-debug/lib
    }
    release {
        INCLUDEPATH += $$PWD/../../../PoDoFo/podofo-linux64-release/include
        LIBS += -L$$PWD/../../../PoDoFo/podofo-linux64-release/lib
    }

    INCLUDEPATH += /usr/include/freetype2
    LIBS += -lpodofo -lfreetype -lfontconfig -ljpeg -lz -lssl -lcrypto -lidn
    # LIBS += -lpoppler-qt4
}
