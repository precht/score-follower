DEFINES += QT_DEPRECATED_WARNINGS

include(../core/core.pri)
include(../common.pri)

SOURCES += \
        src/main.cpp

RESOURCES += resources.qrc

# copy settings.json to build direcotry
COPIES += settings
settings.files = $$files(other/settings.json)
settings.path = $$OUT_PWD
