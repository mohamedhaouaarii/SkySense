#ifndef PERSONNEL_H
#define PERSONNEL_H

#include <QDate>
#include <QString>
#include <QSqlQueryModel>

class QSqlQuery;

class Personnel
{
public:
    // Constructeurs
    Personnel() = default;

    Personnel(const QString &id, const QString &nomComplet,
              const QString &email, const QString &telephone,
              const QString &fonction, const QString &departement,
              const QString &statut, const QDate &dateEntree,
              const QString &note = QString());

    // Getters
    QString getId() const { return id; }
    QString getNomComplet() const { return nomComplet; }
    QString getEmail() const { return email; }
    QString getTelephone() const { return telephone; }
    QString getFonction() const { return fonction; }
    QString getDepartement() const { return departement; }
    QString getStatut() const { return statut; }
    QDate getDateEntree() const { return dateEntree; }
    QString getNote() const { return note; }

    QString derniereErreur() const { return erreur; }

    // Setters
    void setId(const QString &v) { id = v.trimmed(); }
    void setNomComplet(const QString &v) { nomComplet = v.trimmed(); }
    void setEmail(const QString &v) { email = v.trimmed(); }
    void setTelephone(const QString &v) { telephone = v.trimmed(); }
    void setFonction(const QString &v) { fonction = v.trimmed(); }
    void setDepartement(const QString &v) { departement = v.trimmed(); }
    void setStatut(const QString &v) { statut = v.trimmed(); }
    void setDateEntree(const QDate &v) { dateEntree = v; }
    void setNote(const QString &v) { note = v; }

    // Fonctionnalites CRUD
    bool ajouter();
    bool modifier();
    bool supprimer(const QString &identifiant);

    // Retourne nullptr en cas d'erreur.
    // Fournir un parent pour gerer la memoire du modele.
    QSqlQueryModel *afficher(QObject *parent = nullptr);

private:
    // Attributs
    QString id, nomComplet, email, telephone, fonction, departement;
    QString statut = "Active";
    QDate dateEntree = QDate::currentDate();
    QString note, erreur;

    // Methodes internes
    bool connexionOuverte();
    bool valider(bool ajout = false);
    bool genererIdentifiant();
    void lierValeurs(QSqlQuery &query) const;
    bool executer(QSqlQuery &query, bool verifierExistence = false);
};

#endif // PERSONNEL_H
