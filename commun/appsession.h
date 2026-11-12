#ifndef APPSESSION_H
#define APPSESSION_H
#include <QString>

namespace AppSession {
bool signIn(const QString &email,const QString &password,QString &error);
bool isAdministrator();
bool canOpen(const QString &module);
bool stillValid();
qint64 accountId();
int revision();
void clear();
bool active();
QString email();
QString displayName();
QString role();

}
#endif
