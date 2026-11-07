#ifndef EQUIPEMENT_H
#define EQUIPEMENT_H

#include "equipmentdata.h"
#include <QSqlQueryModel>

// Modele de l'entite : attributs, constructeurs, accesseurs et CRUD.
// Les widgets appellent ces methodes ; les requetes restent dans ce fichier .cpp.
class Equipement
{
  private:
    QString id;
    QString type;
    QString station;
    QString unit;
    QString updated;
    QString status;
    QString model;
    QString serialNumber;
    QString photoBase64;
    QString location;
    int personnelId = 0;

  public:
    Equipement() = default;
    explicit Equipement(const EquipmentRecord &valeur);
    EquipmentRecord getDonnees() const;
    void setDonnees(const EquipmentRecord &valeur);
    QString getId() const { return id; }
    void setId(const QString &value) { id = value; }
    QString getType() const { return type; }
    void setType(const QString &value) { type = value; }
    QString getStation() const { return station; }
    void setStation(const QString &value) { station = value; }
    QString getUnit() const { return unit; }
    void setUnit(const QString &value) { unit = value; }
    QString getUpdated() const { return updated; }
    void setUpdated(const QString &value) { updated = value; }
    QString getStatus() const { return status; }
    void setStatus(const QString &value) { status = value; }
    QString getModel() const { return model; }
    void setModel(const QString &value) { model = value; }
    QString getSerialNumber() const { return serialNumber; }
    void setSerialNumber(const QString &value) { serialNumber = value; }
    QString getPhotoBase64() const { return photoBase64; }
    void setPhotoBase64(const QString &value) { photoBase64 = value; }
    QString getLocation() const { return location; }
    void setLocation(const QString &value) { location = value; }
    int getPersonnelId() const { return personnelId; }
    void setPersonnelId(const int &value) { personnelId = value; }

    bool ajouter(QString *error = nullptr) const;
    bool modifier(const QString &originalId, QString *error = nullptr) const;
    bool supprimer(const QString &cle, QString *error = nullptr) const;
    // Le demandeur possede le modele retourne (ou lui donne un parent Qt).
    QSqlQueryModel *afficher(QString *error = nullptr) const;
    // Lecture seule pour le tableau de bord.
    QSqlQueryModel *statistiques(QString *error = nullptr) const;
    bool charger(QVector<EquipmentRecord> *records, QString *error = nullptr) const;
    // Personnel pouvant suivre un equipement (association SUIVRE, 1,1 cote equipement).
    bool personnel(QVector<EquipmentPersonChoice> *people, QString *error = nullptr) const;
    bool enregistrerListe(const QVector<EquipmentRecord> &previous, const QVector<EquipmentRecord> &records,
                          QString *error = nullptr) const;
};
#endif
