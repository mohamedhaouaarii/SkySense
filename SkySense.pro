QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = SkySense_Equipment
SOURCES += main.cpp skysensewindow.cpp equipmentui.cpp equipmentdata.cpp equipmentreport.cpp
HEADERS += skysensewindow.h equipmentui.h equipmentdata.h
FORMS += skysensewindow.ui
RESOURCES += resources.qrc
