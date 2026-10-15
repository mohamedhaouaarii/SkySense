#include "stafforacle.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QLocale>
#include <QSettings>
#include <QTextStream>
#include <QUuid>

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);const auto args=app.arguments();QTextStream out(stdout);
    const int configIndex=args.indexOf("--config");
    if(configIndex<0 || configIndex+1>=args.size() || !QFileInfo::exists(args[configIndex+1])){
        out<<"Usage: oracle_check --config <private.ini> [--setup] [--import <staff.json>] [--exercise]\n";return 2;
    }
    QSettings file(args[configIndex+1],QSettings::IniFormat);file.beginGroup("Oracle");
    Staff::OracleConfig config{file.value("driver").toString(),file.value("host").toString(),file.value("service").toString(),
                               file.value("user").toString(),file.value("password").toString(),file.value("port",1521).toInt()};
    auto failure=[&](QString message){if(!config.password.isEmpty())message.replace(config.password,"<redacted>");out<<"FAIL: "<<message<<"\n";out.flush();return 1;};
    Staff::OracleStore store;QString error,description;
    if(!store.open(config,error) || !store.test(description,error))return failure(error);
    out<<"PASS: Qt QODBC connected to "<<description<<"; SELECT on DUAL succeeded.\n";out.flush();
    bool ready=false;if(!store.schemaReady(ready,error))return failure(error);
    if(!ready && args.contains("--setup")){if(!store.initialize(error))return failure(error);ready=true;out<<"PASS: Staff tables initialized.\n";}
    if(!ready){out<<"Connection verified. Staff schema is not initialized; no tables changed.\n";return 0;}
    Staff::State state;if(!store.load(state,error))return failure(error);
    const int importIndex=args.indexOf("--import");
    if(importIndex>=0){
        if(importIndex+1>=args.size())return failure("Missing local import path.");
        if(state.revision!=0 || !state.members.isEmpty() || !state.audit.isEmpty())return failure("Import refused: Staff database is already initialized. Existing records were not changed.");
        Staff::State imported;
        if(!Staff::Store(args[importIndex+1]).load(imported,error))return failure(error);
        imported.revision=state.revision;
        imported.audit.prepend({QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),Staff::operatorName(),"STAFF-IMPORT","Staff data","Imported to Oracle",QString("Imported %1 local records.").arg(imported.members.size())});
        if(!store.save(imported,error))return failure(error);
        state=imported;out<<"PASS: Local Staff records imported.\n";
    }
    out<<"PASS: Oracle contains "<<state.members.size()<<" Staff records and "<<state.audit.size()<<" audit events.\n";out.flush();
    if(!args.contains("--exercise"))return 0;
    {
        auto invalid=config;invalid.service="SS_STAFF_MISSING_SERVICE";
        Staff::OracleStore unavailable;QString diagnostic;
        if(unavailable.open(invalid,diagnostic) || !diagnostic.contains("ORA-12514") || diagnostic.contains(QChar::Null))
            return failure("Missing-service diagnostic was not returned cleanly.");
        out<<"PASS: Invalid service reports ORA-12514 without malformed diagnostic text.\n";
    }
    const int initialCount=state.members.size();
    const QString id="CHECK-"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(12);
    auto record=[&](const QString &action){state.audit.prepend({QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),Staff::operatorName(),id,"Oracle verification",action,"Temporary test member; existing Staff records preserved."});};
    Staff::Member member{id,QString::fromUtf8("Oracle vérification"),id.toLower()+"@example.invalid","Technical Specialist","Engineering","Active","Never","",QLocale(QLocale::English).toString(QDate::currentDate(),"MMM d, yyyy")};
    member.skills={{"Sensor calibration",1}};
    state.members.append(member);state.permissions[id]={true,true,false,false};record("Oracle test: create");
    if(!store.save(state,error))return failure(error);
    auto cleanup=[&]{
        Staff::State current;QString cleanupError;
        if(store.load(current,cleanupError)){
            state=current;
            for(int i=state.members.size()-1;i>=0;--i)if(state.members[i].id==id || state.members[i].id==id+"-D")state.members.removeAt(i);
            state.permissions.remove(id);state.permissions.remove(id+"-D");record("Oracle test: delete");
            if(store.save(state,cleanupError))return true;
        }
        out<<"Cleanup pending for temporary record "<<id<<": "<<cleanupError<<"\n";return false;
    };
    if(!store.load(state,error)){cleanup();return failure(error);}
    bool found=false;
    for(const auto &m:state.members)if(m.id==id)found=m.name==member.name && m.skills.value("Sensor calibration")==1;
    if(!found){cleanup();return failure("Create/read round trip or Unicode failed.");}
    Staff::State stale=state;
    for(auto &m:state.members)if(m.id==id){m.status="Inactive";m.skills["Sensor calibration"]=3;}
    record("Oracle test: update");if(!store.save(state,error)){cleanup();return failure(error);}
    Staff::OracleStore second;
    if(!second.open(config,error)){cleanup();return failure(error);}
    if(second.save(stale,error) || !error.contains("another instance")){cleanup();return failure("Concurrent stale write was not refused.");}
    if(!store.load(state,error)){cleanup();return failure(error);}
    if(Staff::effectiveAccess(state,id,0)){cleanup();return failure("Inactive member access was not denied.");}
    Staff::State duplicate=state;member.id+="-D";duplicate.members.append(member);
    if(store.save(duplicate,error)){cleanup();return failure("Duplicate email was not rejected.");}
    if(!cleanup())return 1;
    if(!store.load(state,error))return failure(error);
    if(state.members.size()!=initialCount)return failure("Unexpected final Staff record count.");
    out<<"PASS: Oracle create/read/update/delete, Unicode, skills, permissions and concurrency checks.\n";
    out<<"PASS: Temporary member removed; verification events remain in the audit trail.\n";
    return 0;
}
