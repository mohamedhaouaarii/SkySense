#ifndef STAFFMODEL_H
#define STAFFMODEL_H

#include "staffdata.h"

#include <QList>
#include <QString>

// Pure C++ model for the Staff entity (table SKYSENSE_STAFF).
// All SQL lives here: prepared queries + bound values, never behind a button.
class StaffModel final {
public:
    static bool isConnected();

    // Controle de saisie (done in C++ before any query is sent).
    static QString validate(const Staff::Member &member);

    bool afficher(QList<Staff::Member> *members, QString *error = nullptr) const;   // READ
    bool ajouter(const Staff::Member &member, QString *error = nullptr) const;      // CREATE
    bool modifier(const Staff::Member &member, QString *error = nullptr) const;     // UPDATE (also Activate/Deactivate)
    bool supprimer(const QString &id, QString *error = nullptr) const;             // DELETE
};

#endif // STAFFMODEL_H
