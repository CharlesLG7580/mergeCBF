TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    bitmap.cpp \
    countingbloomfilter.cpp \
    murmur.c

DISTFILES +=

HEADERS += \
    bitmap.h \
    countingbloomfilter.h \
    murmur.h

