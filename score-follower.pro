TEMPLATE = subdirs

include(common.pri)

SUBDIRS += \
    core \
    core-tests \
    gui

gui.depends = core
core-tests.depends = core

OTHER_FILES += \
    common.pri


