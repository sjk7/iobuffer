TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11
SOURCES += \
        ../src/tdd.cpp
HEADERS += \
    ../../include/iobuffer/my_cpp98_compat.hpp \
    ../../include/iobuffer/my_iobuffer.hpp \
    ./../include/iobuffer/iobuffer.hpp
