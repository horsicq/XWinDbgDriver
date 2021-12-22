INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xwindbgdriver.h

SOURCES += \
    $$PWD/xwindbgdriver.cpp

win32 {
    LIBS += Advapi32.lib
}
