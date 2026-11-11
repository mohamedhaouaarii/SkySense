#include "staffprofile.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

namespace StaffProfile {
namespace {
int mask(const QList<bool> &rights)
{
    int value=0;
    for (int i=0;i<rights.size();++i)
        if (rights[i]) value|=1<<i;
    return value;
}
}

bool load(QHash<QString,QList<bool>> &permissions, QList<Staff::AuditEntry> &history, QString &error)
{
    QHash<QString,QList<bool>> loaded;
    QList<Staff::AuditEntry> events;
    QSqlQuery q;
    if (!q.exec("SELECT ID_PERSONNEL,DROITS_MODULES FROM STAFF_PERSONNEL WHERE DROITS_MODULES IS NOT NULL")) {
        error=q.lastError().text();
        if (error.contains("ORA-00904"))
            error+="\nRun database/08_staff_profile.sql in the SKYSENSE schema.";
        return false;
    }
    while (q.next()) {
        const int value=q.value(1).toInt();
        loaded.insert(q.value(0).toString(),{bool(value&1),bool(value&2),bool(value&4),bool(value&8)});
    }
    if (q.lastError().isValid()) { error=q.lastError().text();return false; }
    if (!q.exec("SELECT p.ID_PERSONNEL,p.PRENOM,p.NOM,e.moment,e.actor,e.action_name,e.details "
                "FROM STAFF_PERSONNEL p, JSON_TABLE(p.JOURNAL_ACTIVITE, '$[*]' COLUMNS ("
                "position_no FOR ORDINALITY, moment VARCHAR2(32) PATH '$.at',actor VARCHAR2(256) PATH '$.actor',"
                "action_name VARCHAR2(128) PATH '$.action',details VARCHAR2(1000) PATH '$.details')) e "
                "ORDER BY e.moment DESC,p.ID_PERSONNEL,e.position_no DESC")) {
        error=q.lastError().text();return false;
    }
    while (q.next()) {
        events.append({q.value(3).toString(), q.value(4).toString(), q.value(0).toString(),
                       q.value(1).toString()+" "+q.value(2).toString(),q.value(5).toString(),q.value(6).toString()});
    }
    if (q.lastError().isValid()) { error=q.lastError().text();return false; }
    permissions.swap(loaded);
    history.swap(events);
    return true;
}

bool saveGrants(const QString &id,const QList<bool> &previous,const QList<bool> &grants,QString &error)
{
    QSqlQuery q;
    if (grants.size()!=4) { error="Select permissions for the four modules.";return false; }
    if (!q.prepare("UPDATE STAFF_PERSONNEL SET DROITS_MODULES=? WHERE ID_PERSONNEL=? AND NVL(DROITS_MODULES,-1)=?")) {
        error=q.lastError().text();return false;
    }
    q.addBindValue(mask(grants));q.addBindValue(id);
    q.addBindValue(previous.isEmpty()?-1:mask(previous));
    if (!q.exec()) { error=q.lastError().text();return false; }
    if (q.numRowsAffected()!=1) { error="This profile changed. Review its current permissions and try again.";return false; }
    return true;
}
}
