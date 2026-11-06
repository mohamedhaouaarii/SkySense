#include "compte.h"
#include "connection.h"
#include <QCryptographicHash>
#include <QDate>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace
{
constexpr int iterationsHash = 20000;

bool echec(QString *error, const QString &message)
{
    if (error)
        *error = message;
    return false;
}

bool reussite(QString *error)
{
    if (error)
        error->clear();
    return true;
}
} // namespace

QStringList Compte::roles()
{
    return {QStringLiteral("ADMIN"), QStringLiteral("STATION"), QStringLiteral("EQUIPEMENT"),
            QStringLiteral("MAINTENANCE"), QStringLiteral("PERSONNEL"), QStringLiteral("SERVICE")};
}

QString Compte::libelleRole(const QString &role)
{
    if (role == QStringLiteral("ADMIN")) return QStringLiteral("Administrator (all modules)");
    if (role == QStringLiteral("STATION")) return QStringLiteral("Station manager");
    if (role == QStringLiteral("EQUIPEMENT")) return QStringLiteral("Equipment manager");
    if (role == QStringLiteral("MAINTENANCE")) return QStringLiteral("Maintenance manager");
    if (role == QStringLiteral("PERSONNEL")) return QStringLiteral("Staff manager");
    if (role == QStringLiteral("SERVICE")) return QStringLiteral("Services manager");
    return QStringLiteral("No access");
}

QString Compte::roleInterface(const QString &role)
{
    // Ces libelles sont ceux reconnus par le controle d'acces de SkySenseWindow.
    if (role == QStringLiteral("ADMIN")) return QStringLiteral("Administrator");
    if (role == QStringLiteral("STATION")) return QStringLiteral("Responsable Station");
    if (role == QStringLiteral("EQUIPEMENT")) return QStringLiteral("Responsable Equipment");
    if (role == QStringLiteral("MAINTENANCE")) return QStringLiteral("Responsable Maintenance");
    if (role == QStringLiteral("PERSONNEL")) return QStringLiteral("Responsable Staff");
    if (role == QStringLiteral("SERVICE")) return QStringLiteral("Responsable Service");
    return {};
}

bool Compte::estAdmin(const QString &roleInterface)
{
    const QString role = roleInterface.trimmed();
    // "Responsable" sans gestion est l'administrateur du mode apercu (hors Oracle).
    return role.compare(QStringLiteral("Administrator"), Qt::CaseInsensitive) == 0 ||
           role.compare(QStringLiteral("Responsable"), Qt::CaseInsensitive) == 0;
}

QString Compte::nouveauSel()
{
    QByteArray octets(16, Qt::Uninitialized);
    for (char &octet : octets)
        octet = static_cast<char>(QRandomGenerator::system()->bounded(256));
    return QString::fromLatin1(octets.toHex());
}

QString Compte::hacher(const QString &motDePasse, const QString &sel)
{
    const QByteArray secret = motDePasse.toUtf8();
    QByteArray empreinte = sel.toLatin1() + secret;
    for (int i = 0; i < iterationsHash; ++i)
        empreinte = QCryptographicHash::hash(empreinte + sel.toLatin1() + secret, QCryptographicHash::Sha256);
    return QString::fromLatin1(empreinte.toHex());
}

QString Compte::validerMotDePasse(const QString &motDePasse)
{
    if (motDePasse.size() < 6)
        return QStringLiteral("The password must contain at least 6 characters.");
    if (motDePasse.size() > 64)
        return QStringLiteral("The password must not exceed 64 characters.");
    return {};
}

bool Compte::adminExiste(bool *existe, QString *error) const
{
    QSqlQuery query(Connection::createInstance().database());
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM PERSONNEL WHERE ROLE = 'ADMIN'")) || !query.next())
        return echec(error, query.lastError().text());
    if (existe)
        *existe = query.value(0).toInt() > 0;
    return reussite(error);
}

bool Compte::authentifier(const QString &email, const QString &motDePasse, Utilisateur *utilisateur,
                          QString *error) const
{
    const QString refus = QStringLiteral("Incorrect email or password.");
    QSqlQuery query(Connection::createInstance().database());
    if (!query.prepare(QStringLiteral(
            "SELECT ID_PERSONNEL, EMAIL, PRENOM, NOM, MOT_DE_PASSE_HASH, SEL, ROLE, STATUT FROM PERSONNEL "
            "WHERE LOWER(EMAIL) = LOWER(:email) AND ROLE IS NOT NULL")))
        return echec(error, query.lastError().text());
    query.bindValue(QStringLiteral(":email"), email.trimmed());
    if (!query.exec())
        return echec(error, query.lastError().text());
    if (!query.next())
        return echec(error, refus);
    if (hacher(motDePasse, query.value(5).toString()) != query.value(4).toString())
        return echec(error, refus);
    if (query.value(7).toString().compare(QStringLiteral("Inactive"), Qt::CaseInsensitive) == 0)
        return echec(error, QStringLiteral("This account is disabled (status Inactive). Contact the administrator."));
    if (utilisateur)
    {
        utilisateur->personnelId = query.value(0).toInt();
        utilisateur->email = query.value(1).toString();
        utilisateur->nomComplet = (query.value(2).toString() + QLatin1Char(' ') + query.value(3).toString()).trimmed();
        utilisateur->role = query.value(6).toString();
    }
    return reussite(error);
}

bool Compte::creerAdmin(const QString &cin, const QString &prenom, const QString &nom, const QString &email,
                        const QString &motDePasse, QString *error) const
{
    static const QRegularExpression emailValide(QStringLiteral(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)"));
    if (cin.trimmed().isEmpty() || cin.trimmed().size() > 20)
        return echec(error, QStringLiteral("The CIN is required (20 characters maximum)."));
    if (prenom.trimmed().isEmpty() || nom.trimmed().isEmpty())
        return echec(error, QStringLiteral("First name and last name are required."));
    if (!emailValide.match(email.trimmed()).hasMatch())
        return echec(error, QStringLiteral("Enter a valid email address."));
    const QString motDePasseInvalide = validerMotDePasse(motDePasse);
    if (!motDePasseInvalide.isEmpty())
        return echec(error, motDePasseInvalide);

    QSqlQuery query(Connection::createInstance().database());
    if (!query.prepare(QStringLiteral(
            "INSERT INTO PERSONNEL (ID_PERSONNEL, CIN, NOM, PRENOM, EMAIL, FONCTION, STATUT, DATE_ENTREE, "
            "MOT_DE_PASSE_HASH, SEL, ROLE) SELECT NVL(MAX(ID_PERSONNEL), 0) + 1, :cin, :nom, :prenom, :email, "
            "'Administrator', 'Active', :entree, :hash, :sel, 'ADMIN' FROM PERSONNEL")))
        return echec(error, query.lastError().text());
    const QString sel = nouveauSel();
    query.bindValue(QStringLiteral(":cin"), cin.trimmed());
    query.bindValue(QStringLiteral(":nom"), nom.trimmed());
    query.bindValue(QStringLiteral(":prenom"), prenom.trimmed());
    query.bindValue(QStringLiteral(":email"), email.trimmed());
    query.bindValue(QStringLiteral(":entree"), QDate::currentDate());
    query.bindValue(QStringLiteral(":hash"), hacher(motDePasse, sel));
    query.bindValue(QStringLiteral(":sel"), sel);
    if (!query.exec())
    {
        const QString detail = query.lastError().text();
        if (detail.contains(QStringLiteral("UQ_PERSONNEL_EMAIL"), Qt::CaseInsensitive))
            return echec(error, QStringLiteral("This email is already used by a staff member."));
        if (detail.contains(QStringLiteral("UQ_PERSONNEL_CIN"), Qt::CaseInsensitive))
            return echec(error, QStringLiteral("This CIN is already used by a staff member."));
        return echec(error, detail);
    }
    return reussite(error);
}

bool Compte::chargerAcces(int personnelId, QString *role, QString *error) const
{
    QSqlQuery query(Connection::createInstance().database());
    if (!query.prepare(QStringLiteral("SELECT ROLE FROM PERSONNEL WHERE ID_PERSONNEL = :id")))
        return echec(error, query.lastError().text());
    query.bindValue(QStringLiteral(":id"), personnelId);
    if (!query.exec())
        return echec(error, query.lastError().text());
    if (role)
        *role = query.next() ? query.value(0).toString() : QString();
    return reussite(error);
}

bool Compte::enregistrerAcces(int personnelId, const QString &role, const QString &motDePasse,
                              QString *error) const
{
    QSqlQuery query(Connection::createInstance().database());
    if (role.isEmpty())
    {
        if (!query.prepare(QStringLiteral(
                "UPDATE PERSONNEL SET ROLE = NULL, MOT_DE_PASSE_HASH = NULL, SEL = NULL WHERE ID_PERSONNEL = :id")))
            return echec(error, query.lastError().text());
    }
    else if (!roles().contains(role))
    {
        return echec(error, QStringLiteral("Unknown module."));
    }
    else if (motDePasse.isEmpty())
    {
        // Changement de gestion seulement : le compte doit deja avoir un mot de passe.
        if (!query.prepare(QStringLiteral(
                "UPDATE PERSONNEL SET ROLE = :role WHERE ID_PERSONNEL = :id AND MOT_DE_PASSE_HASH IS NOT NULL")))
            return echec(error, query.lastError().text());
        query.bindValue(QStringLiteral(":role"), role);
    }
    else
    {
        const QString motDePasseInvalide = validerMotDePasse(motDePasse);
        if (!motDePasseInvalide.isEmpty())
            return echec(error, motDePasseInvalide);
        if (!query.prepare(QStringLiteral(
                "UPDATE PERSONNEL SET ROLE = :role, MOT_DE_PASSE_HASH = :hash, SEL = :sel WHERE ID_PERSONNEL = :id")))
            return echec(error, query.lastError().text());
        const QString sel = nouveauSel();
        query.bindValue(QStringLiteral(":role"), role);
        query.bindValue(QStringLiteral(":hash"), hacher(motDePasse, sel));
        query.bindValue(QStringLiteral(":sel"), sel);
    }
    query.bindValue(QStringLiteral(":id"), personnelId);
    if (!query.exec())
    {
        const QString detail = query.lastError().text();
        if (detail.contains(QStringLiteral("CK_PERSONNEL_COMPTE"), Qt::CaseInsensitive))
            return echec(error, QStringLiteral("An account needs an email and a password."));
        return echec(error, detail);
    }
    if (!role.isEmpty() && motDePasse.isEmpty() && query.numRowsAffected() == 0)
        return echec(error, QStringLiteral("Enter a password to give this member access."));
    return reussite(error);
}
