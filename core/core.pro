TARGET = core
TEMPLATE = lib

DEFINES += CORE_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

QT += widgets qml quick multimedia
QT -= gui

CONFIG += staticlib
include(../common.pri)

INCLUDEPATH += include

HEADERS += \
    include/controller.h \
    include/lilypond.h \
    include/recorder.h \
    include/scorereader.h \
    include/settings.h

SOURCES += \
    src/controller.cpp \
    src/lilypond.cpp \
    src/recorder.cpp \
    src/scorereader.cpp \
    src/settings.cpp

DISTFILES += \
    core.pri

