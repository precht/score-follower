# Author: Jakub Precht

# set according to your setup
ESSENTIA_PATH = /opt/essentia
MIDIFILE_PATH = /opt/midifile

CONFIG += c++14 file_copies
QMAKE_CXXFLAGS += -O2 -Wall -Wshadow -Wpedantic -Wextra

QT += quick core multimedia widgets quickcontrols2
#QT += quick core multimedia widgets
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += include

HEADERS += \
    include/controller.h \
    include/lilypond.h \
    include/recorder.h \
    include/scorereader.h \
    include/settings.h

SOURCES += \
    src/main.cpp \
    src/controller.cpp \
    src/lilypond.cpp \
    src/recorder.cpp \
    src/scorereader.cpp \
    src/settings.cpp

RESOURCES += \
    resources.qrc

# essentia
LIBS += -L$$ESSENTIA_PATH/build/src/ -lessentia -lfftw3f -lavformat -lavcodec -lavutil -lavresample -lsamplerate -ltag -lyaml -lchromaprint
INCLUDEPATH += $$ESSENTIA_PATH/src/essentia/
DEPENDPATH += $$ESSENTIA_PATH/src/essentia/
PRE_TARGETDEPS += $$ESSENTIA_PATH/build/src/libessentia.a

# midifile
LIBS += -L$$MIDIFILE_PATH/lib/ -lmidifile
INCLUDEPATH += $$MIDIFILE_PATH/include
DEPENDPATH += $$MIDIFILE_PATH/include
PRE_TARGETDEPS += $$MIDIFILE_PATH/lib/libmidifile.a

# copy settings.json to build direcotry
COPIES += settings
settings.files = $$files(other/settings.json)
settings.path = $$OUT_PWD
