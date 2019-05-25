TEMPLATE = app
CONFIG += console c++98
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++98
SOURCES += \
        ../src/tdd.cpp
HEADERS += \
    ../../include/iobuffer/my_cpp98_compat.hpp \
    ../../include/iobuffer/my_iobuffer.hpp \
    ./../include/iobuffer/iobuffer.hpp
