#ifndef SIDEBAR_H
#define SIDEBAR_H
#include <functional>
#include <QString>
#include <QStringList>
class QWidget;

// Barre de navigation de gauche, commune a toutes les gestions.
// sections : les gestions presentes dans cette version (ex. {"Dashboard","Equipment"}).
// A l'integration, il suffit d'ajouter le nom de la gestion ("Stations", "Services"...).
QWidget *createSkySenseSidebar(std::function<void(const QString &)> navigate,
                               const QString &initialSection,
                               const QStringList &sections);
#endif
