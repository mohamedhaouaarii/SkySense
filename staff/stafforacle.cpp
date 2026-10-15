#include "stafforacle.h"
#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <limits>

namespace Staff {
namespace {
QString sqlErrorText(const QSqlError &failure) {
    // Oracle ODBC may include bytes after the diagnostic's terminating NUL.
    const QString databaseText=failure.databaseText().section(QChar::Null,0,0).trimmed();
    const QString driverText=failure.driverText().section(QChar::Null,0,0).trimmed();
    return databaseText.isEmpty()?driverText:databaseText+(driverText.isEmpty()?QString():"\n"+driverText);
}
bool execute(QSqlQuery &query,const QString &sql,const QVariantList &values,QString &error) {
    if(!query.prepare(sql)){error=sqlErrorText(query.lastError());return false;}
    for(const auto &value:values)query.addBindValue(value);
    if(!query.exec()){error=sqlErrorText(query.lastError());return false;}
    return true;
}
bool command(QSqlDatabase database,const QString &sql,const QVariantList &values,QString &error) {
    QSqlQuery query(database);return execute(query,sql,values,error);
}
bool finishRows(QSqlQuery &query,QString &error) {
    if(query.lastError().isValid()){error=sqlErrorText(query.lastError());return false;}
    query.finish();return true;
}
QString quotedOdbc(QString value) { value.replace("}","}}");return "{"+value+"}"; }
const QStringList modules{"Stations","Equipment","Maintenance","Services"};

bool readState(QSqlDatabase database,State &state,QString &error) {
    State loaded;loaded.roleRequirements.clear();QSqlQuery query(database);
    if(!execute(query,"SELECT SCHEMA_VERSION,REVISION,NEXT_ID FROM SS_STAFF_META WHERE SINGLETON=1",{},error))return false;
    if(!query.next() || query.value(0).toInt()!=1){error="Staff schema is missing or incompatible.";return false;}
    loaded.revision=query.value(1).toInt();loaded.nextId=query.value(2).toInt();query.finish();
    if(!execute(query,"SELECT ROLE_NAME FROM SS_STAFF_ROLE WHERE HAS_REQUIREMENTS=1",{},error))return false;
    while(query.next())loaded.roleRequirements.insert(query.value(0).toString(),{});
    if(!finishRows(query,error))return false;
    if(!execute(query,"SELECT ROLE_NAME,SKILL_NAME,REQUIRED_LEVEL FROM SS_STAFF_REQUIREMENT",{},error))return false;
    while(query.next())loaded.roleRequirements[query.value(0).toString()].insert(query.value(1).toString(),query.value(2).toInt());
    if(!finishRows(query,error))return false;
    if(!execute(query,"SELECT STAFF_ID,FULL_NAME,EMAIL,ROLE_NAME,DEPT_NAME,STATUS,"
               "TO_CHAR(LAST_LOGIN,'YYYY-MM-DD HH24:MI'),PHONE,TO_CHAR(JOINED_ON,'YYYY-MM-DD'),NOTE "
               "FROM SS_STAFF_MEMBER ORDER BY STAFF_ID DESC",{},error))return false;
    QHash<QString,int> indexes;
    const QLocale english(QLocale::English);
    while(query.next()){
        const auto login=QDateTime::fromString(query.value(6).toString(),"yyyy-MM-dd HH:mm");
        const auto joined=QDate::fromString(query.value(8).toString(),Qt::ISODate);
        Member member{query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
                      query.value(3).toString(),query.value(4).toString(),query.value(5).toString(),
                      login.isValid()?english.toString(login,"MMM d, yyyy HH:mm"):"Never",
                      query.value(7).toString(),joined.isValid()?english.toString(joined,"MMM d, yyyy"):QString(),query.value(9).toString()};
        indexes.insert(member.id,loaded.members.size());loaded.members.append(member);
        loaded.permissions.insert(member.id,{false,false,false,false});
    }
    if(!finishRows(query,error))return false;
    if(!execute(query,"SELECT STAFF_ID,MODULE_CODE,IS_ALLOWED FROM SS_STAFF_ACCESS",{},error))return false;
    while(query.next()){
        const QString id=query.value(0).toString();const int index=modules.indexOf(query.value(1).toString());
        if(!indexes.contains(id) || index<0){error="Invalid Oracle Staff permission.";return false;}
        loaded.permissions[id][index]=query.value(2).toBool();
    }
    if(!finishRows(query,error))return false;
    if(!execute(query,"SELECT STAFF_ID,SKILL_NAME,ASSESSED_LEVEL FROM SS_STAFF_ASSESSMENT",{},error))return false;
    while(query.next()){
        const QString id=query.value(0).toString();
        if(!indexes.contains(id)){error="Invalid Oracle Staff assessment.";return false;}
        loaded.members[indexes.value(id)].skills.insert(query.value(1).toString(),query.value(2).toInt());
    }
    if(!finishRows(query,error))return false;
    if(!execute(query,"SELECT TO_CHAR(ACTION_AT_UTC,'YYYY-MM-DD\"T\"HH24:MI:SS.FF3'),OPERATOR_NAME,"
               "STAFF_ID,MEMBER_NAME,ACTION_NAME,DETAILS FROM SS_STAFF_AUDIT ORDER BY AUDIT_NO DESC",{},error))return false;
    while(query.next())loaded.audit.append({query.value(0).toString()+"Z",query.value(1).toString(),query.value(2).toString(),
                                           query.value(3).toString(),query.value(4).toString(),query.value(5).toString()});
    if(!finishRows(query,error))return false;
    if(!validateState(loaded,error))return false;
    state=loaded;return true;
}
}

QStringList oracleOdbcDrivers() {
    QStringList result;
#ifdef Q_OS_WIN
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers",QSettings::NativeFormat);
    for(const auto &key:registry.allKeys())if(key.contains("Oracle",Qt::CaseInsensitive))result.append(key);
#endif
    return result;
}
QString validateOracleConfig(const OracleConfig &config) {
    static const QRegularExpression host("^[A-Za-z0-9_.:-]+$"),service("^[A-Za-z0-9_.-]+$");
    if(config.driver.trimmed().isEmpty())return "Select the installed Oracle ODBC driver (64-bit).";
    if(!host.match(config.host).hasMatch() || config.port<1 || config.port>65535 || !service.match(config.service).hasMatch())
        return "Enter a valid host, port and Oracle service name.";
    if(config.user.trimmed().isEmpty() || config.password.isEmpty())return "Enter your Oracle username and password.";
    return {};
}
QString oracleConnectionString(const OracleConfig &config) {
    // Oracle 21c ODBC treats braces around DBQ as part of the Oracle identifier.
    // A descriptor avoids TNS alias resolution; its fields are validated before open().
    const QString target="(DESCRIPTION=(CONNECT_TIMEOUT=5)(ADDRESS=(PROTOCOL=TCP)(HOST="
        +config.host+")(PORT="+QString::number(config.port)+"))(CONNECT_DATA=(SERVICE_NAME="+config.service+")))";
    return "DRIVER="+quotedOdbc(config.driver)+";DBQ="+target+";";
}

OracleStore::OracleStore():connectionName("staff-oracle-"+QUuid::createUuid().toString(QUuid::WithoutBraces)) {}
OracleStore::~OracleStore() {
    if(database.isValid()){database.close();database=QSqlDatabase();QSqlDatabase::removeDatabase(connectionName);}
}
bool OracleStore::open(const OracleConfig &config,QString &error) {
    error=validateOracleConfig(config);if(!error.isEmpty())return false;
    if(!QSqlDatabase::isDriverAvailable("QODBC")){error="Qt QODBC plugin is unavailable. Use the Qt 6.7.3 MinGW 64-bit kit and its sqldrivers plugins.";return false;}
    if(!database.isValid())database=QSqlDatabase::addDatabase("QODBC",connectionName);
    database.setDatabaseName(oracleConnectionString(config));
    // SQL_ATTR_CONNECTION_TIMEOUT is unsupported by the installed Oracle 21c driver.
    database.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=5");
    // This overload does not retain the password in QSqlDatabase's settings.
    if(!database.open(config.user,config.password)){
        error=sqlErrorText(database.lastError());error.replace(config.password,"<redacted>");return false;
    }
    if(!database.driver()->hasFeature(QSqlDriver::Transactions)){
        error="The Oracle connection does not support transactions.";database.close();return false;
    }
    QString description;
    if(!test(description,error)){database.close();return false;}
    identity=description;return true;
}
bool OracleStore::test(QString &description,QString &error) const {
    error.clear();QSqlQuery query(database);
    if(!execute(query,"SELECT USER,SYS_CONTEXT('USERENV','SERVICE_NAME') FROM DUAL",{},error) || !query.next())return false;
    description=query.value(0).toString()+" / "+query.value(1).toString();return true;
}
bool OracleStore::schemaReady(bool &ready,QString &error) const {
    error.clear();ready=false;QSqlQuery query(database);
    if(!execute(query,"SELECT COUNT(*) FROM USER_TABLES WHERE TABLE_NAME IN "
               "('SS_STAFF_META','SS_STAFF_ROLE','SS_STAFF_DEPT','SS_STAFF_SKILL','SS_STAFF_MEMBER',"
               "'SS_STAFF_ACCESS','SS_STAFF_REQUIREMENT','SS_STAFF_ASSESSMENT','SS_STAFF_AUDIT')",{},error) || !query.next())return false;
    if(query.value(0).toInt()!=9)return true;
    query.finish();
    if(!execute(query,"SELECT SCHEMA_VERSION FROM SS_STAFF_META WHERE SINGLETON=1",{},error))return false;
    if(!query.next())return true;
    if(query.value(0).toInt()!=1){error="Unsupported Oracle Staff schema version. No changes were made.";return false;}
    query.finish();
    if(!execute(query,"SELECT COUNT(*) FROM USER_INDEXES WHERE INDEX_NAME='SS_STAFF_EMAIL_UQ'",{},error) || !query.next())return false;
    ready=query.value(0).toInt()==1;return true;
}
bool OracleStore::initialize(QString &error) {
    error.clear();QFile file(":/database/oracle_staff.sql");
    if(!file.open(QIODevice::ReadOnly)){error="Staff schema resource is missing.";return false;}
    QString script=QString::fromUtf8(file.readAll());
    script.remove(QRegularExpression("--[^\\r\\n]*"));
    for(const auto &part:script.split(';',Qt::SkipEmptyParts)){
        const QString sql=part.trimmed();if(sql.isEmpty() || sql=="COMMIT")continue;
        const auto match=QRegularExpression("^CREATE (TABLE|UNIQUE INDEX) (SS_STAFF_[A-Z_]+)\\b").match(sql);
        if(match.hasMatch()){
            QSqlQuery check(database);
            const QString objectSql=match.captured(1)=="TABLE"?"SELECT COUNT(*) FROM USER_TABLES WHERE TABLE_NAME=?":"SELECT COUNT(*) FROM USER_INDEXES WHERE INDEX_NAME=?";
            if(!execute(check,objectSql,{match.captured(2)},error) || !check.next())return false;
            if(check.value(0).toInt())continue;
        }else if(sql.startsWith("INSERT INTO SS_STAFF_META")){
            QSqlQuery check(database);
            if(!execute(check,"SELECT COUNT(*) FROM SS_STAFF_META WHERE SINGLETON=1",{},error) || !check.next())return false;
            if(check.value(0).toInt())continue;
        }else{error="Unrecognized statement in Staff schema resource.";return false;}
        if(!command(database,sql,{},error))return false;
    }
    bool ready=false;if(!schemaReady(ready,error))return false;
    if(!ready){error="Staff schema setup is incomplete.";return false;}
    return true;
}
bool OracleStore::load(State &state,QString &error) {
    error.clear();
    if(!database.transaction()){error=sqlErrorText(database.lastError());return false;}
    // Both load and save hold this row until commit, so a Staff save cannot
    // interleave with our multi-table read. READ COMMITTED also works immediately
    // after setup DDL, unlike the stale READ ONLY snapshot on Oracle XE (ORA-01466).
    QSqlQuery lock(database);
    if(!execute(lock,"SELECT REVISION FROM SS_STAFF_META WHERE SINGLETON=1 FOR UPDATE NOWAIT",{},error) || !lock.next()){
        if(error.isEmpty())error="Staff schema metadata is missing.";
        lock.finish();database.rollback();return false;
    }
    lock.finish();
    State loaded;
    if(!readState(database,loaded,error)){database.rollback();return false;}
    if(!database.commit()){error=sqlErrorText(database.lastError());database.rollback();return false;}
    state=loaded;return true;
}
bool OracleStore::save(State &state,QString &error) {
    error.clear();if(!validateState(state,error))return false;
    if(state.revision==std::numeric_limits<int>::max()){error="Staff revision limit reached.";return false;}
    if(!database.transaction()){error=sqlErrorText(database.lastError());return false;}
    auto fail=[&]{database.rollback();return false;};
    QSqlQuery lock(database);
    if(!execute(lock,"SELECT REVISION FROM SS_STAFF_META WHERE SINGLETON=1 FOR UPDATE NOWAIT",{},error) || !lock.next())return fail();
    if(lock.value(0).toInt()!=state.revision){error="Staff data changed in another instance. Reconnect before editing.";lock.finish();return fail();}
    lock.finish();
    State previous;
    if(!readState(database,previous,error))return fail();
    const int addedAudit=state.audit.size()-previous.audit.size();
    if(addedAudit<0){error="Existing Staff history cannot be removed.";return fail();}
    for(int i=0;i<previous.audit.size();++i){
        const auto &old=previous.audit[i],&incoming=state.audit[i+addedAudit];
        if(QDateTime::fromString(old.timestamp,Qt::ISODateWithMs)!=QDateTime::fromString(incoming.timestamp,Qt::ISODateWithMs)
            || old.actor!=incoming.actor || old.memberId!=incoming.memberId || old.memberName!=incoming.memberName
            || old.action!=incoming.action || old.details!=incoming.details){error="Existing Staff history cannot be changed.";return fail();}
    }
    auto ensureName=[&](const QString &table,const QString &column,const QString &name){
        QSqlQuery query(database);
        if(!execute(query,"SELECT COUNT(*) FROM "+table+" WHERE "+column+"=?",{name},error) || !query.next())return false;
        if(query.value(0).toInt())return true;
        query.finish();
        return command(database,"INSERT INTO "+table+" ("+column+") VALUES (?)",{name},error);
    };
    for(auto it=state.roleRequirements.cbegin();it!=state.roleRequirements.cend();++it){
        if(!ensureName("SS_STAFF_ROLE","ROLE_NAME",it.key()))return fail();
        for(auto skill=it.value().cbegin();skill!=it.value().cend();++skill)
            if(!ensureName("SS_STAFF_SKILL","SKILL_NAME",skill.key()))return fail();
    }
    QSet<QString> oldIds,newIds;for(const auto &member:previous.members)oldIds.insert(member.id);
    const QLocale english(QLocale::English);
    for(const auto &member:state.members){
        if(!ensureName("SS_STAFF_ROLE","ROLE_NAME",member.role) || !ensureName("SS_STAFF_DEPT","DEPT_NAME",member.department))return fail();
        const QDate joined=english.toDate(member.joined,"MMM d, yyyy");
        const QDateTime login=english.toDateTime(member.login,"MMM d, yyyy HH:mm");
        if((!member.joined.isEmpty() && !joined.isValid()) || (!member.login.isEmpty() && member.login!="Never" && !login.isValid())){
            error="Invalid date in Staff record "+member.id;return fail();
        }
        QVariantList values{member.name,member.email,member.phone,member.role,member.department,member.status,
                            joined.toString(Qt::ISODate),login.toString("yyyy-MM-dd HH:mm"),member.note,member.id};
        const QString sql=oldIds.contains(member.id)
            ?"UPDATE SS_STAFF_MEMBER SET FULL_NAME=?,EMAIL=?,PHONE=?,ROLE_NAME=?,DEPT_NAME=?,STATUS=?,JOINED_ON=TO_DATE(?,'YYYY-MM-DD'),LAST_LOGIN=TO_TIMESTAMP(?,'YYYY-MM-DD HH24:MI'),NOTE=? WHERE STAFF_ID=?"
            :"INSERT INTO SS_STAFF_MEMBER (FULL_NAME,EMAIL,PHONE,ROLE_NAME,DEPT_NAME,STATUS,JOINED_ON,LAST_LOGIN,NOTE,STAFF_ID) VALUES (?,?,?,?,?,?,TO_DATE(?,'YYYY-MM-DD'),TO_TIMESTAMP(?,'YYYY-MM-DD HH24:MI'),?,?)";
        if(!command(database,sql,values,error))return fail();
        newIds.insert(member.id);
        for(auto it=member.skills.cbegin();it!=member.skills.cend();++it)if(!ensureName("SS_STAFF_SKILL","SKILL_NAME",it.key()))return fail();
    }
    for(const auto &id:oldIds)if(!newIds.contains(id))if(!command(database,"DELETE FROM SS_STAFF_MEMBER WHERE STAFF_ID=?",{id},error))return fail();
    for(const auto &table:QStringList{"SS_STAFF_ACCESS","SS_STAFF_ASSESSMENT","SS_STAFF_REQUIREMENT"})
        if(!command(database,"DELETE FROM "+table,{},error))return fail();
    if(!command(database,"UPDATE SS_STAFF_ROLE SET HAS_REQUIREMENTS=0",{},error))return fail();
    for(auto role=state.roleRequirements.cbegin();role!=state.roleRequirements.cend();++role){
        if(!command(database,"UPDATE SS_STAFF_ROLE SET HAS_REQUIREMENTS=1 WHERE ROLE_NAME=?",{role.key()},error))return fail();
        for(auto skill=role.value().cbegin();skill!=role.value().cend();++skill)
            if(!command(database,"INSERT INTO SS_STAFF_REQUIREMENT (ROLE_NAME,SKILL_NAME,REQUIRED_LEVEL) VALUES (?,?,?)",{role.key(),skill.key(),skill.value()},error))return fail();
    }
    for(const auto &member:state.members){
        const auto rights=state.permissions.value(member.id,{false,false,false,false});
        for(int i=0;i<4;++i)if(!command(database,"INSERT INTO SS_STAFF_ACCESS (STAFF_ID,MODULE_CODE,IS_ALLOWED) VALUES (?,?,?)",{member.id,modules[i],rights[i]?1:0},error))return fail();
        for(auto skill=member.skills.cbegin();skill!=member.skills.cend();++skill)
            if(!command(database,"INSERT INTO SS_STAFF_ASSESSMENT (STAFF_ID,SKILL_NAME,ASSESSED_LEVEL) VALUES (?,?,?)",{member.id,skill.key(),skill.value()},error))return fail();
    }
    for(int i=addedAudit-1;i>=0;--i){
        const auto &entry=state.audit[i];
        const QString stamp=QDateTime::fromString(entry.timestamp,Qt::ISODateWithMs).toUTC().toString("yyyy-MM-dd'T'HH:mm:ss.zzz");
        const int number=previous.audit.size()+addedAudit-i;
        if(!command(database,"INSERT INTO SS_STAFF_AUDIT (AUDIT_NO,ACTION_AT_UTC,OPERATOR_NAME,STAFF_ID,MEMBER_NAME,ACTION_NAME,DETAILS) "
                    "VALUES (?,TO_TIMESTAMP(?,'YYYY-MM-DD\"T\"HH24:MI:SS.FF3'),?,?,?,?,?)",
                    {number,stamp,entry.actor,entry.memberId,entry.memberName,entry.action,entry.details},error))return fail();
    }
    if(!command(database,"UPDATE SS_STAFF_META SET REVISION=?,NEXT_ID=? WHERE SINGLETON=1",{state.revision+1,state.nextId},error))return fail();
    if(!database.commit()){error=sqlErrorText(database.lastError());return fail();}
    ++state.revision;return true;
}
}
