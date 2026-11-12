#include "skysensewindow.h"
#include "connection.h"
#include "appsession.h"
#include "apptheme.h"
#include "staffaccounts.h"
#include "personnel.h"
#include "planningpersonnel.h"
#include "stationdata.h"
#include "dashboarddata.h"
#include "equipement.h"
#include "servicemodel.h"
#include "maintenancemodel.h"
#include <QtWidgets>
#include <QtTest>
#include <QScopeGuard>
#include <QSqlQuery>
#include <iostream>
#include <memory>

class Integration : public QObject {
    Q_OBJECT
public:
    QString databasePassword;
private:
    QString adminEmail() const {return qEnvironmentVariable("SKYSENSE_TEST_ADMIN_EMAIL","admin@skysense.com");}
    QString adminPassword() const {return qEnvironmentVariable("SKYSENSE_TEST_ADMIN_PASSWORD");}
private slots:
    void initTestCase() {
        Connection &connection = Connection::createInstance();
        QCOMPARE(&connection, &Connection::createInstance());
        QVERIFY2(connection.createConnection(databasePassword), qPrintable(connection.lastConnectionError()));
        const QStringList sessions = QSqlDatabase::connectionNames();
        QCOMPARE(sessions.size(), 1);
        QCOMPARE(connection.database().connectionName(), QSqlDatabase::database().connectionName());
        QVERIFY(connection.createConnection());
        QCOMPARE(QSqlDatabase::connectionNames(), sessions);
        databasePassword.clear();
        QSqlQuery q;QVERIFY(q.exec("SELECT SYS_CONTEXT('USERENV','CURRENT_SCHEMA') FROM dual"));QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),QString("SKYSENSE_MERIEM_STAFF"));
        AppTheme::instance()->initialize(false);
    }
    void cleanup() {QSqlDatabase::database().rollback();AppSession::clear();}
    void modulesAndAssignments() {
        QString error;DashboardSnapshot dashboard;
        QVERIFY2(loadDashboardSnapshot(QSqlDatabase::database(),&dashboard,&error),qPrintable(error));
        QVERIFY(dashboard.available && dashboard.equipment>=8);
        QVector<StationRecord> stations;
        QVERIFY2(StationModel().afficher(&stations,&error),qPrintable(error));QVERIFY(stations.size()>=6);
        std::unique_ptr<QSqlQueryModel> equipment(Equipement().afficher(&error));
        QVERIFY2(equipment && !equipment->lastError().isValid(),qPrintable(error));QVERIFY(equipment->rowCount()>=8);
        QVector<ServiceRecord> services;
        QVERIFY2(ServiceModel().loadAll(services,&error),qPrintable(error));QVERIFY(services.size()>=6);
        QVector<Intervention> jobs;
        QVERIFY2(MaintenanceModel().loadAll(jobs,&error),qPrintable(error));QVERIFY(jobs.size()>=5);
        Personnel personnel;std::unique_ptr<QSqlQueryModel> rows(personnel.afficher());
        QVERIFY2(rows && rows->rowCount()>=13,qPrintable(personnel.derniereErreur()));
        // The same individual is referenced by a numeric department ID and by the original Staff ID.
        QSqlQuery q;QVERIFY(q.exec("SELECT p.STAFF_KEY FROM PERSONNEL p JOIN EQUIPEMENT e ON p.ID_PERSONNEL=e.ID_PERSONNEL WHERE e.IDENTIFIANT_EQUIPEMENT='EQP-001'"));
        QVERIFY(q.next());QCOMPARE(q.value(0).toString(),QString("STF-2025-012"));
        PlanningPersonnel::Bilan result;QDate serviceDay(2026,10,21);
        QVERIFY2(PlanningPersonnel::analyser("MER-002",serviceDay,serviceDay,result,error),qPrintable(error));
        QVERIFY(result.affectations.join(" ").contains("Sensor Calibration"));
        QDate maintenanceDay(2026,10,29);
        QVERIFY2(PlanningPersonnel::analyser("MER-002",maintenanceDay,maintenanceDay,result,error),qPrintable(error));
        QVERIFY(result.affectations.join(" ").contains("Anemometer inspection"));
    }
    void departmentWrites() {
        QString error;
        const QString token="QA-"+QUuid::createUuid().toString(QUuid::Id128).left(12).toUpper();
        StationModel stations;StationRecord station;station.id=token;station.name=token;station.status="Online";
        QVERIFY2(stations.ajouter(station,&error),qPrintable(error));
        auto stationCleanup=qScopeGuard([&]{stations.supprimer(station.id);});
        station.name+=" updated";QVERIFY2(stations.modifier(station,token,&error),qPrintable(error));
        QVERIFY2(stations.supprimer(token,&error),qPrintable(error));stationCleanup.dismiss();
        Equipement equipment;equipment.setId(token);equipment.setSerialNumber(token);
        equipment.setType("Anemometer");equipment.setModel("QA sensor");equipment.setStatus("Online");
        equipment.setUpdated(QDateTime::currentDateTime().toString(Qt::ISODate));equipment.setPersonnelId(1);
        QVERIFY2(equipment.ajouter(&error),qPrintable(error));
        auto equipmentCleanup=qScopeGuard([&]{equipment.supprimer(token);});
        equipment.setModel("Integration test");QVERIFY2(equipment.modifier(token,&error),qPrintable(error));
        QVERIFY2(equipment.supprimer(token,&error),qPrintable(error));equipmentCleanup.dismiss();
        ServiceModel services;QVector<ServiceRecord> serviceRows;
        QVERIFY2(services.loadAll(serviceRows,&error) && !serviceRows.isEmpty(),qPrintable(error));
        ServiceRecord service=serviceRows.first();service.id.clear();service.title=token;service.personnelIds={1};
        QVERIFY2(services.insert(service,&error),qPrintable(error));
        auto serviceCleanup=qScopeGuard([&]{services.remove(service.id);});
        service.title+=" updated";QVERIFY2(services.update(service,&error),qPrintable(error));
        QVERIFY2(services.remove(service.id,&error),qPrintable(error));serviceCleanup.dismiss();
        MaintenanceModel maintenance;QVector<Intervention> jobs;
        QVERIFY2(maintenance.loadAll(jobs,&error) && !jobs.isEmpty(),qPrintable(error));
        Intervention job=jobs.first();job.id.clear();job.databaseId=0;job.title=token;job.technicianId=1;job.technicianIds={1};
        QVERIFY2(maintenance.insert(job,&error),qPrintable(error));
        auto maintenanceCleanup=qScopeGuard([&]{maintenance.remove(job);});
        job.title+=" updated";QVERIFY2(maintenance.update(job,&error),qPrintable(error));
        QVERIFY2(maintenance.remove(job,&error),qPrintable(error));maintenanceCleanup.dismiss();
    }
    void staffAndLogin() {
        QVERIFY2(!adminPassword().isEmpty(), "Set SKYSENSE_TEST_ADMIN_PASSWORD before running the login test.");
        QVERIFY(QSqlDatabase::database().transaction());
        QString error;QVERIFY2(AppSession::signIn(adminEmail(),adminPassword(),error),qPrintable(error));
        const QString token=QUuid::createUuid().toString(QUuid::Id128);
        Personnel member({},{},"Integration","Test",token+"@example.com",{},"Meteorologist","Active",QDate::currentDate());
        QVERIFY2(member.ajouter(),qPrintable(member.derniereErreur()));
        bool numeric=false;member.getId().toLongLong(&numeric);QVERIFY(numeric);
        member.setTelephone("12345678");QVERIFY2(member.modifier(),qPrintable(member.derniereErreur()));
        QVERIFY2(StaffAccounts::save(member.getId(),{},member.getEmail(),"Staff",token,error),qPrintable(error));
        StaffAccounts::Account saved;QVERIFY(StaffAccounts::forMember(member.getId(),saved,error));
        QVERIFY(StaffAccounts::save(member.getId(),saved,member.getEmail(),"Equipment",{},error));
        QVERIFY2(AppSession::signIn(member.getEmail(),token,error),qPrintable(error));
        QVERIFY(AppSession::canOpen("Dashboard") && AppSession::canOpen("Equipment"));QVERIFY(!AppSession::canOpen("Staff"));
        QVERIFY(!StaffAccounts::remove(member.getId(),saved,error));
        member.setStatut("Inactive");QVERIFY(member.modifier());QVERIFY(!AppSession::signIn(member.getEmail(),token,error));
        QVERIFY(AppSession::signIn(adminEmail(),adminPassword(),error));
        QVERIFY(StaffAccounts::forMember(member.getId(),saved,error));QVERIFY(StaffAccounts::remove(member.getId(),saved,error));
        QVERIFY2(member.supprimer(member.getId()),qPrintable(member.derniereErreur()));
    }
    void windowNavigation() {
        QVERIFY2(!adminPassword().isEmpty(), "Set SKYSENSE_TEST_ADMIN_PASSWORD before running the navigation test.");
        SkySenseWindow window;window.resize(1440,900);window.show();QTest::qWait(80);
        window.findChild<QLineEdit*>("emailInput")->setText(adminEmail());
        window.findChild<QLineEdit*>("passwordInput")->setText(adminPassword());
        window.findChild<QPushButton*>("signInButton")->click();
        QVERIFY(AppSession::isAdministrator());
        auto *pages=window.findChild<QStackedWidget*>("workspaceModulePages");
        auto *sidebar=window.findChild<QWidget*>("servicesSidebar");
        QVERIFY(pages && sidebar);QCOMPARE(pages->count(),6);
        const QStringList names{"Dashboard","Stations","Equipment","Maintenance","Staff","Services"};
        for (int i=0;i<names.size();++i) {
            QPushButton *nav=nullptr;
            for(auto *b:sidebar->findChildren<QPushButton*>()) if(b->property("nav").toBool() && b->text()==names[i])nav=b;
            QVERIFY2(nav && nav->isEnabled(),qPrintable(names[i]));nav->click();QCOMPARE(pages->currentIndex(),i);
            auto *module=qobject_cast<QWidget*>(pages->currentWidget()->property("protectedModuleWidget").value<QObject*>());
            QVERIFY(module && module->isEnabled());QTest::qWait(40);
        }
        window.findChild<QAction*>("manageManagerAccounts")->trigger();QCOMPARE(pages->currentIndex(),4);
        auto *root=window.findChild<QWidget*>("staffRoot");auto *canvas=root->property("staffCanvas").value<QWidget*>();
        QVERIFY(canvas->findChild<QWidget*>("staffAccountPanel")->isVisible());
        auto *search=window.findChild<QLineEdit*>("workspaceSearch");
        search->setText("STF-2025-012");QCOMPARE(canvas->findChild<QLabel*>("staffDetailName")->text(),QString("Alex Martin"));
        for(auto *b:canvas->findChildren<QPushButton*>())if(b->property("tab").toBool() && b->text()=="Overview"){b->click();break;}
        AppTheme::instance()->setDark(true);QTest::qWait(50);AppTheme::instance()->setDark(false);
        const auto out=qEnvironmentVariable("SKYSENSE_INTEGRATION_PREVIEW");
        if(!out.isEmpty()) QVERIFY(window.grab().save(out));
    }
};
int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");app.setApplicationName("SkySense integration check");
    std::string password;std::getline(std::cin,password);
    Integration checks;checks.databasePassword=QString::fromStdString(password);password.clear();
    return QTest::qExec(&checks,argc,argv);
}
#include "integration.moc"

