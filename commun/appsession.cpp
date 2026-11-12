#include "appsession.h"
#include "staffaccounts.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace {
StaffAccounts::Identity session;
void identifyDatabaseSession(const QString &name)
{
    if (!QSqlDatabase::contains()) return;
    const auto db=QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen() || db.driverName()!="QODBC") return;
    QSqlQuery query(db);
    if (!query.prepare("BEGIN DBMS_SESSION.SET_IDENTIFIER(?); END;")) {
        qWarning()<<"Could not identify the Oracle operator:"<<query.lastError().text();
        return;
    }
    query.addBindValue(name.left(48));
    if (!query.exec()) qWarning()<<"Could not identify the Oracle operator:"<<query.lastError().text();
}
}

namespace AppSession {

bool signIn(const QString &email,const QString &password,QString &error)
{
    clear();
    if (!StaffAccounts::authenticate(email,password,session,error)) return false;
    identifyDatabaseSession(session.name);
    return true;
}
void clear() { session={};identifyDatabaseSession({}); }
bool active() { return session.id!=0; }
bool isAdministrator() { return active() && session.role=="Administrator"; }
bool canOpen(const QString &module)
{
    if (!active()) return false;
    if (isAdministrator() || module=="Dashboard") return true;
    return module==(session.role=="Station" ? QStringLiteral("Stations") : session.role);
}
bool stillValid() { return active() && StaffAccounts::current(session.id,session.revision,session.role); }
qint64 accountId() { return session.id; }
int revision() { return session.revision; }
QString email() { return session.email; }
QString displayName() { return session.name; }
QString role() { return session.role; }
}
