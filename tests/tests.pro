include(gtest_dependency.pri)

#TEMPLATE = app
#CONFIG += console
CONFIG += thread

include(../core/core.pri)
include(../common.pri)

HEADERS += \
    sample_data.h \
    unit_tests.h \
    performance_tests.h \
    acceptance_tests.h \
    integration_tests.h

SOURCES += \
        main.cpp

# copy settings.json to build direcotry
COPIES += settings
settings.files = $$files(other/settings.json)
settings.path = $$OUT_PWD
