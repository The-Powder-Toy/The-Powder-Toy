QMAKE_CXXFLAGS_RELEASE += -fkeep-inline-functions
QMAKE_CXXFLAGS_DEBUG += -fkeep-inline-functions

LIBS += -lSDL -lm -lbz2

INCLUDEPATH += data/ \
               src/

SOURCES += $$files(src/*.cpp) \
           $$files(src/client/*.cpp) \
           $$files(src/elements/*.cpp) \
           $$files(src/game/*.cpp) \
           $$files(src/interface/*.cpp) \
           $$files(src/search/*.cpp) \
           $$files(src/simulation/*.cpp) \
           $$files(src/cajun/*.cpp)

HEADERS += src/*.h \
           src/client/*.h \
           src/elements/*.h \
           src/game/*.h \
           src/interface/*.h \
           src/search/*.h \
           src/simulation/*.h \
           src/cajun/*.h


