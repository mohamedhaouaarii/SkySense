QT += core gui widgets sql
CONFIG += c++17
TEMPLATE = app
TARGET = SkySense_Equipment
SOURCES += main.cpp skysensewindow.cpp equipmentui.cpp equipmentdata.cpp equipmentreport.cpp databaseconnection.cpp
HEADERS += skysensewindow.h equipmentui.h equipmentdata.h databaseconnection.h
FORMS += skysensewindow.ui
RESOURCES += resources.qrc
