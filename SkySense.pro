QT += core gui widgets multimedia
CONFIG += c++17
win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8
TEMPLATE = app
TARGET = SmartWeather_Station
SOURCES += main.cpp skysensewindow.cpp stationui.cpp
HEADERS += skysensewindow.h stationui.h
FORMS += skysensewindow.ui
RESOURCES += resources.qrc

