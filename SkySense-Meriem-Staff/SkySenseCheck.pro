include(SkySense.pro)
TARGET = SkySenseIntegrationCheck
QT += testlib
CONFIG += console
SOURCES -= main.cpp
SOURCES += tests/integration.cpp

