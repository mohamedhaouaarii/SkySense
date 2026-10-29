#include "connection.h"
#include "personnel.h"
#include "equipment.h"
#include "servicemodel.h"
#include "maintenancemodel.h"
#include "skysensewindow.h"
#include "apptheme.h"
#include "appsession.h"
#include <QtTest>
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <iostream>

class IntegrationTests : public QObject {
    Q_OBJECT
public:
    QString password;
private:
    QString testId() const { return "QA-" + QUuid::createUuid().toString(QUuid::Id128); }
    bool begin() { return QSqlDatabase::database().transaction(); }
private slots:
    void initTestCase() {
        QVERIFY2(Connection::createInstance().createConnection(password), "ODBC login failed");
        password.clear();
        AppTheme::instance()->initialize(false);
        AppTheme::instance()->setDark(false);
    }
    void cleanup() {
        QSqlDatabase::database().rollback();
    }
    void singletonAndSchema() {
        QCOMPARE(&Connection::createInstance(), &Connection::createInstance());
        QCOMPARE(QSqlDatabase::connectionNames().size(), 1);
        QSqlQuery q;
        QVERIFY(q.exec("SELECT USER, SYS_CONTEXT('USERENV','CON_NAME') FROM dual"));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QString("SKYSENSE"));
        QCOMPARE(q.value(1).toString(), QString("XEPDB1"));
        QVERIFY(q.exec("SELECT COUNT(*) FROM user_tables WHERE table_name IN ('SS_STAFF_MEMBER','EQUIPEMENT','STATION','MAINTENANCE_INTERVENTION','SKYSENSE_SERVICES')"));
        QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(), 5);
    }
    void staffRoundTrip() {
        QVERIFY(begin());
        const QString id = testId();
        Personnel p(id,"Test integration","integration."+id+"@example.com","",
                    "Operations Manager","Operations","Active",QDate::currentDate(),"Note d'essai");
        QVERIFY2(p.ajouter(), qPrintable(p.derniereErreur()));
        p.setNomComplet("Personnel modifié"); p.setStatut("Inactive");
        QVERIFY2(p.modifier(), qPrintable(p.derniereErreur()));
        std::unique_ptr<QSqlQueryModel> rows(p.afficher());
        QVERIFY2(rows != nullptr, qPrintable(p.derniereErreur()));
        while(rows->canFetchMore()) rows->fetchMore();
        bool found=false;
        for(int i=0;i<rows->rowCount();++i) if(rows->record(i).value("STAFF_ID").toString()==id) {
            QCOMPARE(rows->record(i).value("FULL_NAME").toString(), QString("Personnel modifié"));
            QCOMPARE(rows->record(i).value("STATUS").toString(), QString("Inactive")); found=true;
        }
        QVERIFY(found); rows.reset();
        QVERIFY2(p.supprimer(id), qPrintable(p.derniereErreur()));
        QVERIFY(!p.supprimer(id));
    }
    void staffAutomaticIds() {
        QVERIFY(begin());
        const QString mail="auto."+testId()+"@example.com";
        Personnel first({},"Automatic ID test",mail,"","Operations Manager",
                        "Operations","Active",QDate::currentDate());
        QVERIFY2(first.ajouter(),qPrintable(first.derniereErreur()));
        const QRegularExpression format("^STF-[0-9]{4}-[0-9]{3,}$");
        QVERIFY(format.match(first.getId()).hasMatch());
        const QString firstId=first.getId();
        first.setNomComplet("Updated automatic ID");
        QVERIFY2(first.modifier(),qPrintable(first.derniereErreur()));
        QCOMPARE(first.getId(),firstId);
        Personnel second({},"Second ID test","second."+mail,"","Operations Manager",
                         "Operations","Active",QDate::currentDate());
        QVERIFY2(second.ajouter(),qPrintable(second.derniereErreur()));
        QVERIFY(format.match(second.getId()).hasMatch());
        QVERIFY(firstId!=second.getId());
        Personnel duplicate({},"Duplicate email",mail,"","Operations Manager",
                            "Operations","Active",QDate::currentDate());
        QVERIFY(!duplicate.ajouter());
        QVERIFY(duplicate.getId().isEmpty());
        Personnel invalid({},"Invalid email","invalid","","Operations Manager",
                          "Operations","Active",QDate::currentDate());
        QVERIFY(!invalid.ajouter());
        QVERIFY(invalid.getId().isEmpty());
        QVERIFY(!invalid.modifier());
        // A SQL insert without an ID also uses the Staff-only sequence via its trigger.
        QSqlQuery q;
        QVERIFY(q.prepare("INSERT INTO SS_STAFF_MEMBER (FULL_NAME,EMAIL,ROLE_NAME,DEPT_NAME,STATUS) "
                          "VALUES ('Trigger test',:email,'Operations Manager','Operations','Active')"));
        q.bindValue(":email","trigger."+mail);
        QVERIFY2(q.exec(),qPrintable(q.lastError().text()));
        QVERIFY(q.prepare("SELECT STAFF_ID FROM SS_STAFF_MEMBER WHERE EMAIL=:email"));
        q.bindValue(":email","trigger."+mail);QVERIFY(q.exec());QVERIFY(q.next());
        const QString triggerId=q.value(0).toString();
        QVERIFY(format.match(triggerId).hasMatch());
        QVERIFY(triggerId!=firstId && triggerId!=second.getId());
    }
    void staffInlineCrud() {
        QVERIFY(begin());
        QSqlQuery count;
        QVERIFY(count.exec("SELECT COUNT(*) FROM SS_STAFF_MEMBER"));QVERIFY(count.next());
        const int before=count.value(0).toInt();count.finish();
        SkySenseWindow window;window.resize(1490,950);window.show();
        window.findChild<QLineEdit *>("emailInput")->setText("admin");
        window.findChild<QLineEdit *>("passwordInput")->setText("integration-check");
        window.findChild<QPushButton *>("signInButton")->click();
        auto *modulePages=window.findChild<QStackedWidget *>("workspaceModulePages");
        QVERIFY(modulePages);modulePages->setCurrentIndex(4);
        auto *root=window.findChild<QWidget *>("staffRoot");QVERIFY(root);
        auto *canvas=root->property("staffCanvas").value<QWidget *>();QVERIFY(canvas);
        auto *pages=canvas->findChild<QStackedWidget *>("staffPages");
        QVERIFY(pages);QCOMPARE(pages->currentIndex(),0);
        auto *add=canvas->findChild<QPushButton *>("addStaff");QVERIFY(add);
        add->click();QTest::qWait(50);
        auto *editor=pages->currentWidget();QCOMPARE(editor->objectName(),QString("staffOracleEditor"));
        QVERIFY(!qobject_cast<QDialog *>(editor));QVERIFY(!QApplication::activeModalWidget());
        auto *id=editor->findChild<QLineEdit *>("memberId");QVERIFY(id && id->isReadOnly());
        QVERIFY(id->text().isEmpty());
        auto *buttons=editor->findChild<QDialogButtonBox *>("staffEditorButtons");QVERIFY(buttons);
        buttons->button(QDialogButtonBox::Save)->click();
        QCOMPARE(pages->currentWidget(),editor);
        QVERIFY(!editor->findChild<QLabel *>("staffValidationError")->text().isEmpty());
        buttons->button(QDialogButtonBox::Cancel)->click();QCOMPARE(pages->currentIndex(),0);
        QVERIFY(count.exec("SELECT COUNT(*) FROM SS_STAFF_MEMBER"));QVERIFY(count.next());
        QCOMPARE(count.value(0).toInt(),before);count.finish();
        add->click();editor=pages->currentWidget();
        const QString email="inline."+testId()+"@example.com";
        editor->findChild<QLineEdit *>("memberName")->setText("Inline Staff test");
        editor->findChild<QLineEdit *>("memberEmail")->setText(email);
        editor->findChild<QComboBox *>("memberRole")->setCurrentText("Operations Manager");
        editor->findChild<QComboBox *>("memberDepartment")->setCurrentText("Operations");
        const QString screenshots=qEnvironmentVariable("SKYSENSE_STAFF_SCREENSHOTS");
        QTest::qWait(80);
        if(!screenshots.isEmpty()) {QDir().mkpath(screenshots);QVERIFY(window.grab().save(screenshots+"/staff-add.png"));}
        editor->findChild<QDialogButtonBox *>("staffEditorButtons")->button(QDialogButtonBox::Save)->click();
        QCOMPARE(pages->currentIndex(),0);
        QSqlQuery q;QVERIFY(q.prepare("SELECT STAFF_ID,FULL_NAME FROM SS_STAFF_MEMBER WHERE EMAIL=:email"));
        q.bindValue(":email",email);QVERIFY(q.exec());QVERIFY(q.next());
        const QString savedId=q.value(0).toString();QCOMPARE(q.value(1).toString(),QString("Inline Staff test"));q.finish();
        auto *edit=canvas->findChild<QPushButton *>("editStaff");QVERIFY(edit);edit->click();
        editor=pages->currentWidget();QCOMPARE(editor->objectName(),QString("staffOracleEditor"));
        QCOMPARE(editor->findChild<QLineEdit *>("memberId")->text(),savedId);
        QVERIFY(editor->findChild<QLineEdit *>("memberId")->isReadOnly());
        editor->findChild<QLineEdit *>("memberName")->setText("Inline Staff updated");
        editor->findChild<QComboBox *>("memberStatus")->setCurrentText("Inactive");
        AppTheme::instance()->setDark(true);QTest::qWait(80);
        if(!screenshots.isEmpty()) QVERIFY(window.grab().save(screenshots+"/staff-edit-dark.png"));
        editor->findChild<QDialogButtonBox *>("staffEditorButtons")->button(QDialogButtonBox::Save)->click();
        QCOMPARE(pages->currentIndex(),0);
        QVERIFY(q.prepare("SELECT FULL_NAME,STATUS FROM SS_STAFF_MEMBER WHERE STAFF_ID=:id"));
        q.bindValue(":id",savedId);QVERIFY(q.exec());QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),QString("Inline Staff updated"));
        QCOMPARE(q.value(1).toString(),QString("Inactive"));q.finish();
        AppTheme::instance()->setDark(false);
        auto *remove=canvas->findChild<QPushButton *>("deleteStaff");QVERIFY(remove);remove->click();
        auto *confirmation=pages->currentWidget();
        QCOMPARE(confirmation->objectName(),QString("staffOracleDeletePage"));
        QVERIFY(!QApplication::activeModalWidget());
        confirmation->findChild<QDialogButtonBox *>("staffDeleteButtons")->button(QDialogButtonBox::No)->click();
        QCOMPARE(pages->currentIndex(),0);
        remove->click();confirmation=pages->currentWidget();
        QTest::qWait(50);
        if(!screenshots.isEmpty()) QVERIFY(window.grab().save(screenshots+"/staff-delete.png"));
        confirmation->findChild<QDialogButtonBox *>("staffDeleteButtons")->button(QDialogButtonBox::Yes)->click();
        QCOMPARE(pages->currentIndex(),0);
        QVERIFY(count.exec("SELECT COUNT(*) FROM SS_STAFF_MEMBER"));QVERIFY(count.next());
        QCOMPARE(count.value(0).toInt(),before);
        QTest::qWait(50);
        if(!screenshots.isEmpty()) QVERIFY(window.grab().save(screenshots+"/staff-list.png"));
        AppSession::clear();
    }
    void equipmentRoundTrip() {
        QVERIFY(begin());
        EquipmentRecord r;
        r.id="EQP-999999001"; r.serialNumber=testId(); r.type="Anemometer";
        r.model="Integration test"; r.unit="m/s"; r.status="Online";
        r.updated=QDateTime::currentDateTime().toString(Qt::ISODate);
        r.photoBase64=QString::fromLatin1(QByteArray("integration blob").toBase64());
        EquipmentModel model; QString error;
        QVERIFY2(model.ajouter(r,&error),qPrintable(error));
        r.model="Updated test";
        QVERIFY2(model.modifier(r,r.serialNumber,&error),qPrintable(error));
        QVector<EquipmentRecord> rows;
        QVERIFY2(model.afficher(&rows,&error),qPrintable(error));
        bool found=false;
        for(const auto &row:rows) if(row.serialNumber==r.serialNumber) {
            QCOMPARE(row.model,r.model); QCOMPARE(row.photoBase64,r.photoBase64); found=true;
        }
        QVERIFY(found);
        QVERIFY2(model.supprimer(r.serialNumber,&error),qPrintable(error));
    }
    void servicesRoundTrip() {
        QVERIFY(begin());
        ServiceRecord r;
        r.id=testId().left(30); r.title="Integration service";r.type="Inspection";
        r.stationCode="ST-QA";r.stationName="Station de test";r.assignee="Test integration";
        r.date=QLocale::c().toString(QDate::currentDate(),"MMM dd, yyyy");
        r.startTime="09:00";r.endTime="10:00";r.priority="Medium";r.status="Planned";
        r.description="Vérification d'intégration";r.checklist={"Check one","Check two"};
        r.checklistDone={false,true};
        ServiceModel model;QString error;
        QVERIFY2(model.insert(r,&error),qPrintable(error));
        r.title="Updated service";
        QVERIFY2(model.update(r,&error),qPrintable(error));
        QVector<ServiceRecord> rows;
        QVERIFY2(model.loadAll(rows,&error),qPrintable(error));
        bool found=false;
        for(const auto &row:rows) if(row.id==r.id) {
            QCOMPARE(row.title,r.title);QCOMPARE(row.description,r.description);
            QCOMPARE(row.checklist,r.checklist);QCOMPARE(row.checklistDone,r.checklistDone);found=true;
        }
        QVERIFY(found);QVERIFY2(model.remove(r.id,&error),qPrintable(error));
    }
    void maintenanceRoundTrip() {
        QVERIFY(begin());
        Intervention r;
        r.equipId="EQP-QA";r.title="Integration maintenance";r.tech="Test integration";
        r.priority="Medium";r.status="Planned";r.desc="Essai sans données réelles";
        r.plannedStart=QDateTime::currentDateTime();r.plannedEnd=r.plannedStart.addSecs(3600);
        MaintenanceModel model;QString error;
        QVERIFY2(model.insert(r,&error),qPrintable(error));
        r.title="Updated maintenance";
        QVERIFY2(model.update(r,&error),qPrintable(error));
        QVector<Intervention> rows;
        QVERIFY2(model.loadAll(rows,&error),qPrintable(error));
        bool found=false;for(const auto &row:rows) if(row.id==r.id) {QCOMPARE(row.title,r.title);found=true;}
        QVERIFY(found);QVERIFY2(model.remove(r.id,&error),qPrintable(error));
    }
    void integratedNavigationAndStaff() {
        SkySenseWindow window;window.resize(1490,950);window.show();
        QTest::qWait(150);
        const QString screenshots=qEnvironmentVariable("SKYSENSE_TEST_SCREENSHOTS");
        if(!screenshots.isEmpty()) { QDir().mkpath(screenshots); QVERIFY(window.grab().save(screenshots+"/login.png")); }
        auto *email=window.findChild<QLineEdit *>("emailInput");
        auto *passwordInput=window.findChild<QLineEdit *>("passwordInput");
        auto *signIn=window.findChild<QPushButton *>("signInButton");
        QVERIFY(email && passwordInput && signIn);
        // This is the supplied project's local prototype session, not an Oracle login.
        email->setText("admin"); passwordInput->setText("integration-check");signIn->click();
        auto *stack=window.findChild<QStackedWidget *>("workspaceModulePages");
        auto *sidebar=window.findChild<QWidget *>("servicesSidebar");
        QVERIFY(stack && sidebar);QCOMPARE(stack->count(),6);
        const QStringList modules{"Dashboard","Stations","Equipment","Maintenance","Staff","Services"};
        for(int i=0;i<modules.size();++i) {
            QPushButton *nav=nullptr;
            for(auto *b:sidebar->findChildren<QPushButton *>())
                if(b->property("nav").toBool() && b->text()==modules[i]) nav=b;
            QVERIFY2(nav,qPrintable(modules[i]));nav->click();QCOMPARE(stack->currentIndex(),i);
            auto *module=qobject_cast<QWidget *>(stack->currentWidget()->property("protectedModuleWidget").value<QObject *>());
            QVERIFY(module && module->isEnabled());
            QTest::qWait(80);
            if(!screenshots.isEmpty()) QVERIFY(window.grab().save(screenshots+"/"+modules[i].toLower()+".png"));
        }
        stack->setCurrentIndex(4);
        auto *root=window.findChild<QWidget *>("staffRoot");QVERIFY(root);
        auto *canvas=root->property("staffCanvas").value<QWidget *>();QVERIFY(canvas);
        auto *table=canvas->findChild<QTableWidget *>("staffTable");QVERIFY(table);
        QCOMPARE(table->columnCount(),9);QVERIFY(table->rowCount()>0);
        QVERIFY(canvas->findChild<QLabel *>("staffConnectionStatus")->text().contains("SKYSENSE"));
        QVERIFY(canvas->findChild<QPushButton *>("addStaff")->isEnabled());
        auto *search=window.findChild<QLineEdit *>("workspaceSearch");QVERIFY(search);
        search->setText("NO_MATCH_INTEGRATION_CHECK");QCOMPARE(table->rowCount(),0);
        search->clear();QVERIFY(table->rowCount()>0);
        AppTheme::instance()->setDark(true);QTest::qWait(100);
        QVERIFY(AppTheme::dark());
        if(!screenshots.isEmpty()) QVERIFY(window.grab().save(screenshots+"/staff-dark.png"));
        AppTheme::instance()->setDark(false);
        // Preserve the group's per-module access routing with the replacement Staff page.
        email->setText("staff.test@skysense.com");passwordInput->setText("integration-check");signIn->click();
        QCOMPARE(stack->currentIndex(),4);
        auto *equipment=qobject_cast<QWidget *>(stack->widget(2)->property("protectedModuleWidget").value<QObject *>());
        QVERIFY(!equipment->isEnabled());
        QVERIFY(canvas->isEnabled());
        AppSession::clear();
    }
};

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    app.setStyle("Fusion");
    app.setApplicationName("SkySense integration checks");
    std::string password;std::getline(std::cin,password);
    IntegrationTests test;test.password=QString::fromStdString(password);password.clear();
    return QTest::qExec(&test,argc,argv);
}
#include "tst_integration.moc"
