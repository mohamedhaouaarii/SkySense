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

    // Les onze attributs de la table PERSONNEL du modele conceptuel.
    Personnel(const QString &id, const QString &cin, const QString &nom,
              const QString &prenom, const QString &email, const QString &telephone,
              const QString &fonction, const QString &statut, const QDate &dateEntree,
              const QDate &debutIndisponibilite = QDate(),
              const QDate &finIndisponibilite = QDate());

    // Getters
    QString getId() const { return id; }
    QString getCin() const { return cin; }
    QString getNom() const { return nom; }
    QString getPrenom() const { return prenom; }
    QString getNomComplet() const { return (prenom + " " + nom).trimmed(); }
    QString getEmail() const { return email; }
    QString getTelephone() const { return telephone; }
    QString getFonction() const { return fonction; }
    QString getStatut() const { return statut; }
    QDate getDateEntree() const { return dateEntree; }
    QDate getDateDebutIndisponibilite() const { return debutIndisponibilite; }
    QDate getDateFinIndisponibilite() const { return finIndisponibilite; }

    QString derniereErreur() const { return erreur; }

    // Setters
    void setId(const QString &v) { id = v.trimmed(); }
    void setCin(const QString &v) { cin = v.trimmed(); }
    void setNom(const QString &v) { nom = v.trimmed(); }
    void setPrenom(const QString &v) { prenom = v.trimmed(); }
    void setEmail(const QString &v) { email = v.trimmed(); }
    void setTelephone(const QString &v) { telephone = v.trimmed(); }
    void setFonction(const QString &v) { fonction = v.trimmed(); }
    void setStatut(const QString &v) { statut = v.trimmed(); }
    void setDateEntree(const QDate &v) { dateEntree = v; }
    void setDateDebutIndisponibilite(const QDate &v) { debutIndisponibilite = v; }
    void setDateFinIndisponibilite(const QDate &v) { finIndisponibilite = v; }

    // Fonctionnalites CRUD
    bool ajouter();
    bool modifier();
    bool supprimer(const QString &identifiant);

    // Retourne nullptr en cas d'erreur.
    // Fournir un parent pour gerer la memoire du modele.
    QSqlQueryModel *afficher(QObject *parent = nullptr);

private:
    // Attributs
    QString id, cin, nom, prenom, email, telephone, fonction;
    QString statut = "Active";
    QDate dateEntree = QDate::currentDate();
    QDate debutIndisponibilite, finIndisponibilite;
    QString erreur;

    // Methodes internes
    bool connexionOuverte();
    bool valider(bool ajout = false);
    bool genererIdentifiant();
    void lierValeurs(QSqlQuery &query) const;
    bool executer(QSqlQuery &query, bool verifierExistence = false);
};

#endif // PERSONNEL_H
