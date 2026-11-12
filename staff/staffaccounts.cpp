#include "staffaccounts.h"
#include "appsession.h"
#include "connection.h"
#include <QCoreApplication>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <algorithm>

namespace StaffAccounts {
namespace {
constexpr int rounds=600000;
const QString adminGuard=QStringLiteral("EXISTS (SELECT 1 FROM STAFF_ACCOUNT a JOIN STAFF_PERSONNEL p "
    "ON p.ID_PERSONNEL=a.ID_PERSONNEL WHERE a.ACCOUNT_ID=? AND a.REVISION=? AND a.ACCOUNT_ROLE='Administrator' AND p.STATUT='Active')");
QByteArray newSalt()
{
    quint32 words[4];
    QRandomGenerator::system()->generate(std::begin(words),std::end(words));
    return QByteArray(reinterpret_cast<const char*>(words),sizeof(words));
}
QByteArray hash(const QString &password,const QByteArray &salt,int iterations)
{
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,password.toUtf8(),salt,iterations,32);
}
bool sameDigest(const QByteArray &left,const QByteArray &right)
{
    if (left.size()!=32 || right.size()!=32) return false;
    unsigned difference=0;
    for (int i=0;i<32;++i) difference|=static_cast<unsigned char>(left[i]^right[i]);
    return difference==0;
}
bool adminOnly(QString &error)
{
    if (QCoreApplication::instance()->property("oraclePreview").toBool()) {error="Use the Oracle workspace to manage logins.";return false;}
    if (AppSession::isAdministrator() && current(AppSession::accountId(),AppSession::revision(),"Administrator")) return true;
    error="Only the signed-in administrator can manage accounts.";
    return false;
}
bool executeChange(QSqlQuery &q,QString &error)
{
    if (!q.exec()) {
        const QString dbError=q.lastError().text();
        if (dbError.contains("ORA-00001")) error="This email or staff member already has a login. Reload the account to continue.";
        else error="Oracle could not save the account. Check the connection and try again.";
        return false;
    }
    if (q.numRowsAffected()==1) return true;
    error="Access changed or this account was updated elsewhere. Reopen Access and try again.";
    return false;
}
}

QStringList managerRoles() { return {"Station","Equipment","Maintenance","Staff","Services"}; }

bool authenticate(const QString &email,const QString &password,Identity &identity,QString &error)
{
    identity={};
    const auto db=Connection::createInstance().database();
    if (!db.isOpen()) {error="Sign-in requires a connection to the SkySense Oracle database.";return false;}
    QSqlQuery q(db);
    q.prepare("SELECT a.ACCOUNT_ID,a.REVISION,a.LOGIN_EMAIL,a.ACCOUNT_ROLE,a.SALT_HEX,a.HASH_HEX,"
              "a.PBKDF2_ROUNDS,p.PRENOM,p.NOM,p.STATUT FROM STAFF_ACCOUNT a JOIN STAFF_PERSONNEL p "
              "ON p.ID_PERSONNEL=a.ID_PERSONNEL WHERE a.LOGIN_EMAIL=?");
    q.addBindValue(email.trimmed().toLower());
    if (!q.exec()) {
        error=q.lastError().text().contains("ORA-00942")?"Account setup is missing. Run database/09_staff_accounts.sql in SKYSENSE."
                                                      :"Could not verify your account with Oracle.";
        return false;
    }
    const bool found=q.next();
    const QByteArray salt=found?QByteArray::fromHex(q.value(4).toByteArray()):QByteArray(16,'\0');
    const int iterations=found?q.value(6).toInt():rounds;
    const QByteArray expected=found?QByteArray::fromHex(q.value(5).toByteArray()):QByteArray(32,'\0');
    const bool matched=salt.size()==16 && iterations>=rounds && iterations<=5000000 &&
                       sameDigest(hash(password,salt,iterations),expected);
    if (!found || !matched || q.value(9).toString()!="Active" ||
        !current(q.value(0).toLongLong(),q.value(1).toInt(),q.value(3).toString())) {
        error="Sign-in failed. Check your email and password or contact the administrator.";
        return false;
    }
    identity={q.value(0).toLongLong(),q.value(1).toInt(),q.value(2).toString(),
              (q.value(7).toString()+" "+q.value(8).toString()).trimmed(),q.value(3).toString()};
    return true;
}

bool current(qint64 id,int revision,const QString &role)
{
    QSqlQuery q(Connection::createInstance().database());
    q.prepare("SELECT 1 FROM STAFF_ACCOUNT a JOIN STAFF_PERSONNEL p ON p.ID_PERSONNEL=a.ID_PERSONNEL "
              "WHERE a.ACCOUNT_ID=? AND a.REVISION=? AND a.ACCOUNT_ROLE=? AND p.STATUT='Active'");
    q.addBindValue(QString::number(id));q.addBindValue(revision);q.addBindValue(role);
    return q.exec() && q.next();
}

bool forMember(const QString &staffId,Account &account,QString &error)
{
    account={};
    if (!adminOnly(error)) return false;
    QSqlQuery q;
    q.prepare("SELECT ACCOUNT_ID,REVISION,LOGIN_EMAIL,ACCOUNT_ROLE FROM STAFF_ACCOUNT WHERE ID_PERSONNEL=?");
    q.addBindValue(staffId);
    if (!q.exec()) {error="Could not load this login from Oracle.";return false;}
    if (q.next()) account={q.value(0).toLongLong(),q.value(1).toInt(),q.value(2).toString(),q.value(3).toString()};
    else if (q.lastError().isValid()) {error="Could not read this login from Oracle.";return false;}
    return true;
}

bool save(const QString &staffId,const Account &previous,const QString &email,const QString &role,
          const QString &password,QString &error)
{
    if (!adminOnly(error)) return false;
    const QString normalized=email.trimmed().toLower();
    static const QRegularExpression format(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)");
    if (!format.match(normalized).hasMatch() || normalized.size()>254) {error="Enter a valid login email.";return false;}
    if (!managerRoles().contains(role) || previous.role=="Administrator") {error="Choose a department manager role.";return false;}
    if (!previous.id && password.isEmpty()) {error="Enter a password to create a manager login.";return false;}
    QSqlQuery q;
    const bool replacingPassword=!password.isEmpty();
    QString sql;
    if (!previous.id)
        sql="INSERT INTO STAFF_ACCOUNT (LOGIN_EMAIL,ACCOUNT_ROLE,SALT_HEX,HASH_HEX,PBKDF2_ROUNDS,ID_PERSONNEL) "
            "SELECT ?,?,?,?,?,p.ID_PERSONNEL FROM STAFF_PERSONNEL p WHERE p.ID_PERSONNEL=? AND "+adminGuard;
    else {
        sql="UPDATE STAFF_ACCOUNT SET LOGIN_EMAIL=?,ACCOUNT_ROLE=?,REVISION=REVISION+1";
        if (replacingPassword) sql+=",SALT_HEX=?,HASH_HEX=?,PBKDF2_ROUNDS=?";
        sql+=" WHERE ID_PERSONNEL=? AND ACCOUNT_ID=? AND REVISION=? AND ACCOUNT_ROLE<>'Administrator' AND "+adminGuard;
    }
    if (!q.prepare(sql)) {error="Could not prepare the account change.";return false;}
    q.addBindValue(normalized);q.addBindValue(role);
    if (replacingPassword) {
        const auto salt=newSalt();
        q.addBindValue(QString::fromLatin1(salt.toHex()));
        q.addBindValue(QString::fromLatin1(hash(password,salt,rounds).toHex()));
        q.addBindValue(rounds);
    }
    q.addBindValue(staffId);
    if (previous.id) {q.addBindValue(QString::number(previous.id));q.addBindValue(previous.revision);}
    q.addBindValue(QString::number(AppSession::accountId()));q.addBindValue(AppSession::revision());
    return executeChange(q,error);
}
bool remove(const QString &staffId,const Account &previous,QString &error)
{
    if (!adminOnly(error)) return false;
    QSqlQuery q;
    q.prepare("DELETE FROM STAFF_ACCOUNT WHERE ID_PERSONNEL=? AND ACCOUNT_ID=? AND REVISION=? "
              "AND ACCOUNT_ROLE<>'Administrator' AND "+adminGuard);
    q.addBindValue(staffId);q.addBindValue(QString::number(previous.id));q.addBindValue(previous.revision);
    q.addBindValue(QString::number(AppSession::accountId()));q.addBindValue(AppSession::revision());
    return executeChange(q,error);
}
}
