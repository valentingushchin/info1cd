QT += core
QT -= gui

CONFIG += c++11

TARGET = info1cd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

PROJECTDIR = "C:/dev/vvg/Project/info1cd"
ZLIB_INCLUDE = $$PROJECTDIR/zlib/include
ZLIB_LIB = $$PROJECTDIR/zlib/lib

INCLUDEPATH += $${ZLIB_INCLUDE}

CONFIG(debug, debug|release) {
        LIBS += -L$${ZLIB_LIB} -lzlibd
} else {
        LIBS += -L$${ZLIB_LIB} -lzlib
}

SOURCES += main.cpp \
    vl/generic.cpp \
    vl/info1cd.cpp \
    vl/zlibwrap.cpp \
    vl/cache.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    vl/generic.h \
    vl/info1cd.h \
    vl/zlibwrap.h \
    zlib/include/zconf.h \
    zlib/include/zlib.h \
    vl/cache.h

DISTFILES += \
    zlib/lib/zlib.lib \
    zlib/lib/zlibd.lib \
    zlib/lib/zlib.dll \
    zlib/lib/zlibd.dll \
    zlib/lib/zlib.dll \
    zlib/lib/zlibd.dll
