INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xwindbgdriver.h

SOURCES += \
    $$PWD/xwindbgdriver.cpp

!contains(XCONFIG, xprocess) {
    XCONFIG += xprocess
    include(../XProcess/xprocess.pri)
}

win32-msvc* {
    LIBS += ntdll.lib
}
