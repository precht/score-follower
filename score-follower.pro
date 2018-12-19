CONFIG += c++14
QMAKE_CXXFLAGS += -O2 -Wall -Wshadow -Wpedantic -Wextra

QT += quick core multimedia widgets quickcontrols2
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += include

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

## Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


HEADERS += \
    include/controller.h \
    include/lilypond.h \
    include/recorder.h \
    include/devicesmodel.h

SOURCES += \
    src/main.cpp \
    src/controller.cpp \
    src/lilypond.cpp \
    src/recorder.cpp \
    src/devicesmodel.cpp

RESOURCES += \
    resources.qrc


LIBS += -L$$PWD/../../../../../usr/lib/ -lessentia -lessentia  -lfftw3f -lavformat -lavcodec -lavutil -lavresample -lsamplerate -ltag -lyaml -lchromaprint
INCLUDEPATH += $$PWD/../../../../../usr/include/essentia
DEPENDPATH += $$PWD/../../../../../usr/include/essentia
PRE_TARGETDEPS += $$PWD/../../../../../usr/lib/libessentia.a
