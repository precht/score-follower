TEMPLATE = subdirs

SUBDIRS += \
    core \
    tests \
    gui

gui.depends = core
tests.depends = core

OTHER_FILES += \
    common.pri


