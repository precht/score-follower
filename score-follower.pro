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
    include/devicesmodel.h \
    include/scorereader.h

SOURCES += \
    src/main.cpp \
    src/controller.cpp \
    src/lilypond.cpp \
    src/recorder.cpp \
    src/devicesmodel.cpp \
    src/scorereader.cpp

RESOURCES += \
    resources.qrc


LIBS += -L$$PWD/../../../../../usr/local/lib/ -lessentia -lessentia  -lfftw3f -lavformat -lavcodec -lavutil -lavresample -lsamplerate -ltag -lyaml -lchromaprint
INCLUDEPATH += $$PWD/../../../../../usr/local/include/essentia
DEPENDPATH += $$PWD/../../../../../usr/local/include/essentia
PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/libessentia.a

unix:!macx: LIBS += -L$$PWD/../../../../../opt/midifile/lib/ -lmidifile

INCLUDEPATH += $$PWD/../../../../../opt/midifile/include
DEPENDPATH += $$PWD/../../../../../opt/midifile/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../opt/midifile/lib/libmidifile.a
