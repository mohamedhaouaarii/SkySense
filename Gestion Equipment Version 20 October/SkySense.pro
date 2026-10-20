QT += core gui widgets multimedia printsupport sql
CONFIG += c++17

win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8

TEMPLATE = app
TARGET = SkySense

SOURCES += \
    main.cpp \
    skysensewindow.cpp \
    equipmentui.cpp \
    equipmentdata.cpp \
    equipment.cpp \
    equipmentreport.cpp \
    serviceui.cpp \
    stationui.cpp \
    maintenanceui.cpp \
    mainwindow.cpp \
    staffui.cpp \
    staffdata.cpp \
    appsession.cpp \
    connection.cpp

HEADERS += \
    skysensewindow.h \
    equipmentui.h \
    equipmentdata.h \
    equipment.h \
    serviceui.h \
    stationui.h \
    maintenanceui.h \
    mainwindow.h \
    staffui.h \
    staffdata.h \
    appsession.h \
    connection.h

FORMS += skysensewindow.ui
RESOURCES += resources.qrc


contains(CONFIG, skysense_tests): DEFINES += SKYSENSE_UI_TESTS

HEADERS += loginatmosphere.h

SOURCES += apptheme.cpp
HEADERS += apptheme.h
