LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core/include
DEPENDPATH += $$PWD/../core/include

PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a

