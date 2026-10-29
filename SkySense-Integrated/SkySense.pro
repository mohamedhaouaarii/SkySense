QT += core gui widgets printsupport sql
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
    stafftraining.cpp \
    stafftrainingdialog.cpp \
    personnel.cpp \
    appsession.cpp \
    connection.cpp \
    maintenancemodel.cpp \
    servicemodel.cpp

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
    stafftraining.h \
    personnel.h \
    appsession.h \
    connection.h \
    intervention.h \
    maintenancemodel.h \
    servicemodel.h

FORMS += skysensewindow.ui
RESOURCES += resources.qrc


contains(CONFIG, skysense_tests): DEFINES += SKYSENSE_UI_TESTS

HEADERS += loginatmosphere.h

SOURCES += apptheme.cpp
HEADERS += apptheme.h

qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += SKYSENSE_MULTIMEDIA
} else:win32 {
    LIBS += -lwinmm
}
SOURCES += clicksound.cpp
HEADERS += clicksound.h
