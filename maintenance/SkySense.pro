QT       += core gui widgets printsupport sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += SKYSENSE_HAS_MULTIMEDIA
}
CONFIG   += c++17
TARGET    = SkySense
TEMPLATE  = app

SOURCES += main.cpp \
           mainwindow.cpp \
           connection.cpp \
           maintenancemodel.cpp

HEADERS += mainwindow.h \
           connection.h \
           intervention.h \
           maintenancemodel.h

RESOURCES += resources.qrc
DISTFILES += README.md \
             LISEZ-MOI.txt \
             maintenance_schema.sql

win32:LIBS += -lwinmm
