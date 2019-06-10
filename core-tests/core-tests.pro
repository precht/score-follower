include(gtest_dependency.pri)

#TEMPLATE = app
#CONFIG += console
CONFIG += thread

include(../core/core.pri)
include(../common.pri)

HEADERS += \
    unit_tests.h

SOURCES += \
        main.cpp
