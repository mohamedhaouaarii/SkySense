QT += core gui widgets sql printsupport multimedia
CONFIG += c++17
TEMPLATE = app
TARGET = SkySense_Equipment
SOURCES += main.cpp skysensewindow.cpp equipmentui.cpp equipmentdata.cpp equipmentreport.cpp databaseconnection.cpp \
    gestionstation/stationui.cpp gestionstation/stationdata.cpp gestionstation/connection.cpp gestionstation/apptheme.cpp
HEADERS += skysensewindow.h equipmentui.h equipmentdata.h databaseconnection.h \
    gestionstation/stationui.h gestionstation/stationdata.h gestionstation/connection.h gestionstation/apptheme.h
FORMS += skysensewindow.ui
RESOURCES += resources.qrc gestionstation/gestionstation.qrc
