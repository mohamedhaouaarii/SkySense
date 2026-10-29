#include "personnel.h"

#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <utility>

// Constructeur avec parametres
Personnel::Personnel(const QString &id, const QString &nomComplet,
                     const QString &email, const QString &telephone,
                     const QString &fonction, const QString &departement,
                     const QString &statut, const QDate &dateEntree,
                     const QString &note)
    : id(id.trimmed()),
    nomComplet(nomComplet.trimmed()),
    email(email.trimmed()),
    telephone(telephone.trimmed()),
    fonction(fonction.trimmed()),
    departement(departement.trimmed()),
    statut(statut.trimmed()),
    dateEntree(dateEntree),
    note(note)
{
}

// Verifier la connexion deja ouverte par Connection.
bool Personnel::connexionOuverte()
{
    const QSqlDatabase db = QSqlDatabase::database(
        QSqlDatabase::defaultConnection, false);

    if (!db.isValid() || !db.isOpen()) {
        erreur = "La connexion a la base de donnees n'est pas ouverte.";
        return false;
    }

    return true;
}

// Controle de saisie en C++
bool Personnel::valider(bool ajout)
{
    if ((!ajout && id.isEmpty()) || nomComplet.isEmpty() || email.isEmpty()
        || fonction.isEmpty() || departement.isEmpty()) {
        erreur = "Nom, email, fonction et departement sont obligatoires. "
                 "Selectionnez un membre pour le modifier.";
        return false;
    }

    if (id.size() > 80 || nomComplet.size() > 100 || email.size() > 254
        || telephone.size() > 40 || fonction.size() > 80
        || departement.size() > 80 || note.size() > 1000) {
        erreur = "Un champ depasse la longueur autorisee dans la base.";
        return false;
    }

    static const QRegularExpression formatEmail(
        R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");

    if (!formatEmail.match(email).hasMatch()) {
        erreur = "L'adresse email n'est pas valide.";
        return false;
    }

    if (statut != "Active" && statut != "Inactive") {
        erreur = "Le statut doit etre Active ou Inactive.";
        return false;
    }

    // QDate() represente une date absente, autorisee par le schema actuel.
    return true;
}

// Associer les attributs aux parametres de la requete.
void Personnel::lierValeurs(QSqlQuery &query) const
{
    query.bindValue(":id", id);
    query.bindValue(":nom", nomComplet);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":fonction", fonction);
    query.bindValue(":departement", departement);
    query.bindValue(":statut", statut);

    query.bindValue(
        ":date_entree",
        dateEntree.isValid()
            ? QVariant(dateEntree)
            : QVariant(QMetaType::fromType<QDate>())
        );

    query.bindValue(":note", note);
}

// Executer la requete et conserver le message d'erreur.
bool Personnel::executer(QSqlQuery &query, bool verifierExistence)
{
    if (!query.exec()) {
        erreur = query.lastError().text();
        return false;
    }

    if (verifierExistence && query.numRowsAffected() == 0) {
        erreur = "Aucun membre ne correspond a cet identifiant.";
        return false;
    }

    return true;
}

// Oracle reserves a unique number even when several application instances add staff.
bool Personnel::genererIdentifiant()
{
    QSqlQuery query;
    if (!query.prepare("SELECT SS_STAFF_MEMBER_SEQ.NEXTVAL, "
                       "TO_CHAR(SYSDATE, 'YYYY') FROM DUAL")
        || !query.exec() || !query.next()) {
        erreur = "Impossible de generer le Staff ID. Verifiez le script "
                 "database/04_staff_auto_id.sql.\n" + query.lastError().text();
        return false;
    }
    bool ok = false;
    const qlonglong numero = query.value(0).toLongLong(&ok);
    if (!ok || numero < 1) {
        erreur = "La sequence Oracle a retourne un identifiant invalide.";
        return false;
    }
    id = QString("STF-%1-%2").arg(query.value(1).toString())
             .arg(numero, 3, 10, QLatin1Char('0'));
    return true;
}

// CREATE : an empty ID requests automatic allocation. Existing import IDs stay supported.
bool Personnel::ajouter()
{
    erreur.clear();

    if (!valider(true) || !connexionOuverte()) {
        return false;
    }

    QSqlQuery query;

    if (!query.prepare(
            "INSERT INTO SS_STAFF_MEMBER "
            "(STAFF_ID, FULL_NAME, EMAIL, PHONE, ROLE_NAME, DEPT_NAME, "
            "STATUS, JOINED_ON, NOTE) "
            "VALUES (:id, :nom, :email, :telephone, :fonction, "
            ":departement, :statut, :date_entree, :note)")) {
        erreur = query.lastError().text();
        return false;
    }

    const bool automatique = id.isEmpty();
    if (automatique && !genererIdentifiant())
        return false;

    lierValeurs(query);
    const bool ajoute = executer(query);
    if (!ajoute && automatique)
        id.clear();
    return ajoute;
}

// UPDATE : modifier le membre identifie par son STAFF_ID.
bool Personnel::modifier()
{
    erreur.clear();

    if (!valider() || !connexionOuverte()) {
        return false;
    }

    QSqlQuery query;

    if (!query.prepare(
            "UPDATE SS_STAFF_MEMBER SET FULL_NAME = :nom, EMAIL = :email, "
            "PHONE = :telephone, ROLE_NAME = :fonction, "
            "DEPT_NAME = :departement, STATUS = :statut, "
            "JOINED_ON = :date_entree, NOTE = :note "
            "WHERE STAFF_ID = :id")) {
        erreur = query.lastError().text();
        return false;
    }

    lierValeurs(query);

    return executer(query, true);
}

// DELETE : supprimer uniquement le membre selectionne.
bool Personnel::supprimer(const QString &identifiant)
{
    erreur.clear();

    const QString idSupprime = identifiant.trimmed();

    if (idSupprime.isEmpty() || idSupprime.size() > 80) {
        erreur = "Selectionnez un identifiant valide a supprimer.";
        return false;
    }

    if (!connexionOuverte()) {
        return false;
    }

    QSqlQuery query;

    if (!query.prepare(
            "DELETE FROM SS_STAFF_MEMBER WHERE STAFF_ID = :id")) {
        erreur = query.lastError().text();
        return false;
    }

    query.bindValue(":id", idSupprime);

    return executer(query, true);
}

// READ : recuperer la liste des membres.
QSqlQueryModel *Personnel::afficher(QObject *parent)
{
    erreur.clear();

    if (!connexionOuverte()) {
        return nullptr;
    }

    QSqlQuery query;

    if (!query.prepare(
            "SELECT STAFF_ID, FULL_NAME, EMAIL, PHONE, ROLE_NAME, "
            "DEPT_NAME, STATUS, JOINED_ON, LAST_LOGIN, NOTE "
            "FROM SS_STAFF_MEMBER ORDER BY STAFF_ID")) {
        erreur = query.lastError().text();
        return nullptr;
    }

    if (!executer(query)) {
        return nullptr;
    }

    auto *model = new QSqlQueryModel(parent);
    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        erreur = model->lastError().text();
        delete model;
        return nullptr;
    }

    const QStringList titres = {
        "Staff ID",
        "Nom complet",
        "Email",
        "Telephone",
        "Fonction",
        "Departement",
        "Statut",
        "Date d'entree",
        "Derniere connexion",
        "Note"
    };

    for (int i = 0; i < titres.size(); ++i) {
        model->setHeaderData(i, Qt::Horizontal, titres.at(i));
    }

    return model;
}
