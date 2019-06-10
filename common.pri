CONFIG += c++14
QMAKE_CXXFLAGS += -O2 -Wall -Wshadow -Wpedantic -Wextra
QT += quick core multimedia widgets quickcontrols2

# paths
ESSENTIA_PATH = /opt/essentia
MIDIFILE_PATH = /opt/midifile
GOOGLETEST_DIR = /opt/googletest

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
