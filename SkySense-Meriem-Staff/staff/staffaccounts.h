#pragma once
#include <QStringList>

namespace StaffAccounts {
struct Account {
    qint64 id=0;
    int revision=0;
    QString email,role;
};
struct Identity {
    qint64 id=0;
    int revision=0;
    QString email,name,role;
};
QStringList managerRoles();
bool authenticate(const QString &email,const QString &password,Identity &identity,QString &error);
bool current(qint64 id,int revision,const QString &role);
bool forMember(const QString &staffId,Account &account,QString &error);
bool save(const QString &staffId,const Account &previous,const QString &email,const QString &role,
          const QString &password,QString &error);
bool remove(const QString &staffId,const Account &previous,QString &error);
}
