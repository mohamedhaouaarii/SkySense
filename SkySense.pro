QT += core gui widgets printsupport sql
CONFIG += c++17

win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8

TEMPLATE = app
TARGET = SkySense

SOURCES += \
    main.cpp \
    skysensewindow.cpp \
    serviceui.cpp \
    servicemodel.cpp \
    connection.cpp \
    apptheme.cpp

HEADERS += \
    skysensewindow.h \
    serviceui.h \
    servicemodel.h \
    connection.h \
    apptheme.h

FORMS += skysensewindow.ui
RESOURCES += resources.qrc
