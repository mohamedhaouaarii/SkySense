#include "personnel.h"

#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <utility>

// Constructeur avec parametres
Personnel::Personnel(const QString &id, const QString &cin, const QString &nom,
                     const QString &prenom, const QString &email, const QString &telephone,
                     const QString &fonction, const QString &statut, const QDate &dateEntree,
                     const QDate &debutIndisponibilite, const QDate &finIndisponibilite)
    : id(id.trimmed()), cin(cin.trimmed()), nom(nom.trimmed()), prenom(prenom.trimmed()),
      email(email.trimmed()), telephone(telephone.trimmed()), fonction(fonction.trimmed()),
      statut(statut.trimmed()), dateEntree(dateEntree),
      debutIndisponibilite(debutIndisponibilite), finIndisponibilite(finIndisponibilite)
{
}

// Verifier la connexion deja ouverte par Connection.
bool Personnel::connexionOuverte()
{
    const QSqlDatabase db = QSqlDatabase::database(
        QSqlDatabase::defaultConnection, false);

    if (!db.isValid() || !db.isOpen()) {
        erreur = "The database connection is closed.";
        return false;
    }

    return true;
}

// Controle de saisie en C++
bool Personnel::valider(bool ajout)
{
    if (!ajout && id.isEmpty()) {
        erreur = "Select a staff member to edit.";
        return false;
    }
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() || fonction.isEmpty()) {
        erreur = "Last name, first name, email and function are required.";
        return false;
    }
    if (id.size() > 80 || cin.size() > 20 || nom.size() > 100 || prenom.size() > 100
        || email.size() > 254 || telephone.size() > 40 || fonction.size() > 80) {
        erreur = "One of the fields exceeds its maximum length.";
        return false;
    }
    static const QRegularExpression formatEmail(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
    if (!formatEmail.match(email).hasMatch()) {
        erreur = "Enter a valid email address.";
        return false;
    }
    if (statut != "Active" && statut != "Inactive") {
        erreur = "Choose Active or Inactive.";
        return false;
    }
    if (debutIndisponibilite.isValid() != finIndisponibilite.isValid()
        || (debutIndisponibilite.isValid() && finIndisponibilite < debutIndisponibilite)) {
        erreur = "Enter both leave dates. The end date must be on or after the start date.";
        return false;
    }
    return true;
}

// Associer les attributs aux parametres de la requete.
void Personnel::lierValeurs(QSqlQuery &query) const
{
    query.bindValue(":id", id);
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":fonction", fonction);
    query.bindValue(":statut", statut);
    const auto dateOuNull = [](const QDate &date) {
        return date.isValid() ? QVariant(date) : QVariant(QMetaType::fromType<QDate>());
    };
    query.bindValue(":date_entree", dateOuNull(dateEntree));
    query.bindValue(":debut", dateOuNull(debutIndisponibilite));
    query.bindValue(":fin", dateOuNull(finIndisponibilite));
}

// Executer la requete et conserver le message d'erreur.
bool Personnel::executer(QSqlQuery &query, bool verifierExistence)
{
    if (!query.exec()) {
        erreur = query.lastError().text().section(QChar::Null,0,0).trimmed();
        if (erreur.contains("UQ_PERSONNEL_CIN",Qt::CaseInsensitive))
            erreur = "This CIN already belongs to another staff member.";
        return false;
    }

    if (verifierExistence && query.numRowsAffected() == 0) {
        erreur = "No staff member matches this ID.";
        return false;
    }

    return true;
}

// Oracle reserves a unique number even when several application instances add staff.
bool Personnel::genererIdentifiant()
{
    QSqlQuery query;
    if (!query.prepare("SELECT PERSONNEL_SEQ.NEXTVAL FROM DUAL")
        || !query.exec() || !query.next()) {
        erreur = "Could not generate the staff ID. Check "
                 "database/05_personnel.sql.\n" + query.lastError().text();
        return false;
    }
    bool ok = false;
    const qlonglong numero = query.value(0).toLongLong(&ok);
    if (!ok || numero < 1) {
        erreur = "Oracle returned an invalid staff ID.";
        return false;
    }
    id = QString::number(numero);
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
            "INSERT INTO STAFF_PERSONNEL (ID_PERSONNEL, CIN, NOM, PRENOM, EMAIL, TELEPHONE, "
            "FONCTION, STATUT, DATE_ENTREE, DATE_DEBUT_INDISPONIBILITE, DATE_FIN_INDISPONIBILITE) "
            "VALUES (:id, :cin, :nom, :prenom, :email, :telephone, :fonction, :statut, "
            ":date_entree, :debut, :fin)")) {
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

// UPDATE : modifier le membre identifie par son ID_PERSONNEL.
bool Personnel::modifier()
{
    erreur.clear();

    if (!valider() || !connexionOuverte()) {
        return false;
    }

    QSqlQuery query;

    if (!query.prepare(
            "UPDATE STAFF_PERSONNEL SET CIN = :cin, NOM = :nom, PRENOM = :prenom, EMAIL = :email, "
            "TELEPHONE = :telephone, FONCTION = :fonction, STATUT = :statut, "
            "DATE_ENTREE = :date_entree, DATE_DEBUT_INDISPONIBILITE = :debut, "
            "DATE_FIN_INDISPONIBILITE = :fin WHERE ID_PERSONNEL = :id")) {
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
        erreur = "Choose a valid staff ID to delete.";
        return false;
    }

    if (!connexionOuverte()) {
        return false;
    }

    QSqlQuery query;

    if (!query.prepare(
            "DELETE FROM STAFF_PERSONNEL WHERE ID_PERSONNEL = :id")) {
        erreur = query.lastError().text();
        return false;
    }

    query.bindValue(":id", idSupprime);
    if (executer(query,true)) return true;
    if (erreur.contains("ORA-02292")) {
        QSqlQuery linked;
        linked.prepare("SELECT ACCOUNT_ROLE FROM STAFF_ACCOUNT WHERE ID_PERSONNEL=?");
        linked.addBindValue(idSupprime);
        if (linked.exec() && linked.next())
            erreur=linked.value(0).toString()=="Administrator"?"The administrator’s staff record is protected.":
                "This member has a manager login. The administrator must remove it in Access before deleting the staff record.";
    }
    return false;
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
            "SELECT ID_PERSONNEL, CIN, NOM, PRENOM, EMAIL, TELEPHONE, FONCTION, STATUT, "
            "DATE_ENTREE, DATE_DEBUT_INDISPONIBILITE, DATE_FIN_INDISPONIBILITE "
            "FROM STAFF_PERSONNEL ORDER BY ID_PERSONNEL")) {
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
        "Staff ID", "CIN", "Last name", "First name", "Email", "Phone", "Function",
        "Status", "Join date", "Leave starts", "Leave ends"
    };

    for (int i = 0; i < titres.size(); ++i) {
        model->setHeaderData(i, Qt::Horizontal, titres.at(i));
    }

    return model;
}
