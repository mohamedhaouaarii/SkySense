QT += core gui widgets
CONFIG += c++17
win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8
TEMPLATE = app
TARGET = SkySense_Services
SOURCES += main.cpp skysensewindow.cpp serviceui.cpp
HEADERS += skysensewindow.h serviceui.h
FORMS += skysensewindow.ui
RESOURCES += resources.qrc

