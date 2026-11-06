QT += core gui widgets multimedia printsupport sql
CONFIG += c++17

win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8

TEMPLATE = app
TARGET = SkySense

contains(CONFIG, skysense_tests): DEFINES += SKYSENSE_UI_TESTS

# Version integree : Connexion + Tableau de bord + Stations + Equipement.
# Architecture Modele-Vue de l'atelier CRUD : un dossier par gestion.
#   Modele : classe purement C++ (attributs, constructeurs, getters/setters, requetes preparees).
#   Vue    : widgets ; aucune requete SQL, appels via l'objet du modele.
# Integration : voir INTEGRATION.md.
INCLUDEPATH += commun fenetre dashboard equipement station compte

SOURCES += main.cpp

# Commun : connexion unique a la BD (Singleton), session, theme
SOURCES += commun/connection.cpp commun/appsession.cpp commun/apptheme.cpp
HEADERS += commun/connection.h commun/appsession.h commun/apptheme.h

# Fenetre principale : page de connexion, en-tete, barre de navigation
SOURCES += fenetre/skysensewindow.cpp fenetre/sidebar.cpp
HEADERS += fenetre/skysensewindow.h fenetre/loginatmosphere.h fenetre/sidebar.h
FORMS   += fenetre/skysensewindow.ui

# Connexion : comptes (colonnes de PERSONNEL) et creation du premier administrateur
SOURCES += compte/compte.cpp compte/compteui.cpp
HEADERS += compte/compte.h compte/compteui.h

# Tableau de bord des equipements (lecture seule, via Equipement::statistiques())
SOURCES += dashboard/dashboarddata.cpp dashboard/dashboardui.cpp
HEADERS += dashboard/dashboarddata.h dashboard/dashboardui.h

# Gestion des equipements (CRUD complet)
SOURCES += equipement/equipement.cpp equipement/equipmentdata.cpp equipement/equipmentreport.cpp equipement/equipmentui.cpp
HEADERS += equipement/equipement.h equipement/equipmentdata.h equipement/equipmentui.h

# Gestion des stations (CRUD complet, exports PDF) - meme connexion Oracle, meme fenetre, meme en-tete
SOURCES += station/stationdata.cpp station/stationui.cpp
HEADERS += station/stationdata.h station/stationui.h

RESOURCES += resources.qrc
