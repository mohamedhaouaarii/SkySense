#ifndef COMPTE_H
#define COMPTE_H

#include <QString>
#include <QStringList>

// Comptes de connexion portes par PERSONNEL (colonnes MOT_DE_PASSE_HASH, SEL, ROLE).
// Le mot de passe n'est jamais stocke en clair : SHA-256 itere avec un sel propre a chaque compte.
// Les widgets appellent ces methodes ; les requetes restent dans ce fichier .cpp.
class Compte
{
  public:
    struct Utilisateur
    {
        int personnelId = 0;
        QString email, nomComplet, role; // role = valeur de la colonne ROLE (ADMIN, EQUIPEMENT, ...)
    };

    // Valeurs autorisees pour la colonne ROLE, et libelle affiche pour chacune.
    static QStringList roles();
    static QString libelleRole(const QString &role);
    // Role utilise par l'interface ("Administrator", "Responsable Equipment", ...).
    static QString roleInterface(const QString &role);
    static bool estAdmin(const QString &roleInterface);

    static QString nouveauSel();
    static QString hacher(const QString &motDePasse, const QString &sel);
    static QString validerMotDePasse(const QString &motDePasse);

    bool adminExiste(bool *existe, QString *error = nullptr) const;
    bool authentifier(const QString &email, const QString &motDePasse, Utilisateur *utilisateur,
                      QString *error = nullptr) const;
    bool creerAdmin(const QString &cin, const QString &prenom, const QString &nom, const QString &email,
                    const QString &motDePasse, QString *error = nullptr) const;
    bool chargerAcces(int personnelId, QString *role, QString *error = nullptr) const;
    // role vide = aucun acces (le mot de passe est efface) ; motDePasse vide = mot de passe inchange.
    bool enregistrerAcces(int personnelId, const QString &role, const QString &motDePasse,
                          QString *error = nullptr) const;
};

#endif // COMPTE_H
