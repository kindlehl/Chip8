######################################################################
# Automatically generated by qmake (3.1) Sun Dec 10 20:49:06 2017
######################################################################
QT += core gui

QT += widgets


TEMPLATE = app
TARGET = Chip8
QMAKE_CXXFLAGS += -I/usr/include -std=c++17
LIBS += -L/usr/lib -lSDL2


# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += cpu.h  
SOURCES += cpu.cpp main.cpp
