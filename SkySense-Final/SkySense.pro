QT += core gui widgets printsupport sql network
CONFIG += c++17

win32-g++: QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc*: QMAKE_CXXFLAGS += /utf-8

TEMPLATE = app
TARGET = SkySense

contains(CONFIG, skysense_tests): DEFINES += SKYSENSE_UI_TESTS

# Version integree : Connexion + Tableau de bord + Stations + Equipement + Maintenance + Services.
# Architecture Modele-Vue de l'atelier CRUD : un dossier par gestion.
#   Modele : classe purement C++ (attributs, constructeurs, getters/setters, requetes preparees).
#   Vue    : widgets ; aucune requete SQL, appels via l'objet du modele.
# Integration : voir INTEGRATION.md.
INCLUDEPATH += commun fenetre dashboard equipement station maintenance service staff

SOURCES += main.cpp

# Commun : connexion unique a la BD (Singleton), session, theme
SOURCES += commun/connection.cpp commun/appsession.cpp commun/apptheme.cpp
HEADERS += commun/connection.h commun/appsession.h commun/apptheme.h

# Fenetre principale : page de connexion, en-tete, barre de navigation
SOURCES += fenetre/skysensewindow.cpp fenetre/sidebar.cpp
HEADERS += fenetre/skysensewindow.h fenetre/loginatmosphere.h fenetre/sidebar.h
FORMS   += fenetre/skysensewindow.ui

# Tableau de bord des equipements (lecture seule, via Equipement::statistiques())
SOURCES += dashboard/dashboarddata.cpp dashboard/dashboardui.cpp
HEADERS += dashboard/dashboarddata.h dashboard/dashboardui.h

# Gestion des equipements (CRUD complet)
SOURCES += equipement/equipement.cpp equipement/equipmentdata.cpp equipement/equipmentreport.cpp equipement/equipmentui.cpp
HEADERS += equipement/equipement.h equipement/equipmentdata.h equipement/equipmentui.h

# Gestion des stations (CRUD complet, exports PDF) - meme connexion Oracle, meme fenetre, meme en-tete
SOURCES += station/stationdata.cpp station/stationui.cpp
HEADERS += station/stationdata.h station/stationui.h

# Gestion Maintenance (module repense pour les tables Oracle MAINTENANCE + REALISER)
SOURCES += maintenance/maintenancemodel.cpp maintenance/mainwindow.cpp maintenance/maintenanceui.cpp
HEADERS += maintenance/intervention.h maintenance/maintenancemodel.h maintenance/mainwindow.h maintenance/maintenanceui.h

# Gestion Services (CRUD complet sur les tables Oracle SERVICE + AFFECTER + PERSONNEL, PDF, statistiques)
SOURCES += service/servicemodel.cpp service/serviceui.cpp
HEADERS += service/servicemodel.h service/serviceui.h

RESOURCES += resources.qrc

# Staff management and the verified workspace accounts
SOURCES += staff/personnel.cpp staff/planningpersonnel.cpp staff/staffaccountpanel.cpp staff/staffaccounts.cpp staff/staffdata.cpp staff/staffprofile.cpp staff/stafftraining.cpp staff/stafftrainingdialog.cpp staff/staffui.cpp
HEADERS += staff/personnel.h staff/planningpersonnel.h staff/staffaccountpanel.h staff/staffaccounts.h staff/staffdata.h staff/staffdesign.h staff/staffprofile.h staff/staffstatisticscurve.h staff/stafftraining.h staff/staffui.h
SOURCES += commun/clicksound.cpp
HEADERS += commun/clicksound.h
qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += SKYSENSE_MULTIMEDIA
} else:win32 {
    LIBS += -lwinmm
}
