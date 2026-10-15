#include <QtWidgets>
#include <QtTest>
#include "staffui.h"
#include "staffdata.h"
#include "stafftraining.h"
#include "stafforacle.h"
#include <functional>

class StaffTests : public QObject {
    Q_OBJECT
    QWidget *root = nullptr;
    QWidget *canvas = nullptr;
    QTableWidget *table = nullptr;
    QLineEdit *search = nullptr;
    QTemporaryDir *storageDirectory = nullptr;

    template<class T> T *find(const char *name) const { return canvas->findChild<T *>(name); }
    QPushButton *tab(const QString &title) const {
        for (auto *button : canvas->findChildren<QPushButton *>())
            if (button->text() == title) return button;
        return nullptr;
    }
    void selectDaniel() { search->setText("Daniel Kim"); }
    void save(QDialog *dialog) { dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); }
    QString storagePath() const { return storageDirectory->filePath("staff.json"); }
    void openWindow(bool requireOracle=false) {
        root = new QWidget;
        root->setAttribute(Qt::WA_DontShowOnScreen);
        root->setProperty("staffStoragePath",storagePath());
        root->setProperty("staffOracleRequired",requireOracle);
        root->resize(1646,932);
        setupStaffUi(root);
        root->show();
        QApplication::processEvents();
        canvas = root->property("staffCanvas").value<QWidget *>();
        QVERIFY(canvas);
        table = find<QTableWidget>("staffTable");
        search = find<QLineEdit>("staffSearch");
        QVERIFY(table && search);
    }
    Staff::State savedState() const { Staff::State state;QString error;Staff::Store(storagePath()).load(state,error);return state; }
    void confirmDelete(QMessageBox::StandardButton answer) {
        QTimer::singleShot(0,this,[this,answer]{for(auto *message:root->findChildren<QMessageBox*>())if(message->isVisible())message->button(answer)->click();});
        find<QPushButton>("deleteStaff")->click();
    }
    void createMember(const QString &memberName,const QString &address) {
        QTimer::singleShot(0,this,[&,this]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;
            dialog->findChild<QLineEdit*>("memberName")->setText(memberName);dialog->findChild<QLineEdit*>("memberEmail")->setText(address);save(dialog);
        });
        QTimer::singleShot(3000,canvas,[this]{if(auto *dialog=find<QDialog>("staffEditor"))dialog->reject();});
        find<QPushButton>("addStaff")->click();
    }
    bool setAssessment(QDialog *dialog,const QString &skill,int level) {
        auto *grid=dialog->findChild<QTableWidget*>("trainingSkillsTable");
        for(int row=0;row<grid->rowCount();++row)if(grid->item(row,0)->text()==skill){
            auto *choice=qobject_cast<QComboBox*>(grid->cellWidget(row,2));
            choice->setCurrentIndex(choice->findData(level));return true;
        }
        return false;
    }
    void useTraining(const std::function<void(QDialog*)> &action) {
        bool opened=false;
        QTimer watchdog;watchdog.setSingleShot(true);
        connect(&watchdog,&QTimer::timeout,this,[this]{for(auto *dialog:canvas->findChildren<QDialog*>())if(dialog->isVisible())dialog->reject();});
        watchdog.start(5000);
        QTimer::singleShot(0,this,[&]{auto *dialog=find<QDialog>("staffTrainingDialog");if(dialog){opened=true;action(dialog);}});
        find<QPushButton>("staffTraining")->click();QVERIFY(opened);
    }
private slots:
    void init() { storageDirectory=new QTemporaryDir;QVERIFY(storageDirectory->isValid());openWindow(); }
    void cleanup() { delete root;root=nullptr;delete storageDirectory;storageDirectory=nullptr; }

    void oracleConfigKeepsCredentialsOutOfConnectionString() {
        Staff::OracleConfig config{"Oracle test driver","localhost","xepdb1","project_user","Private;Password}",1521};
        QVERIFY(Staff::validateOracleConfig(config).isEmpty());
        const QString connection=Staff::oracleConnectionString(config);
        QVERIFY(!connection.contains(config.password));QVERIFY(!connection.contains(config.user));
        QVERIFY(connection.contains("DBQ=(DESCRIPTION="));
        QVERIFY(connection.contains("(HOST=localhost)(PORT=1521)"));
        QVERIFY(connection.contains("(SERVICE_NAME=xepdb1)"));
        QVERIFY(!connection.contains("DBQ={")); // Regression: ORA-12154 with Oracle 21c ODBC.
        config.host="localhost;UID=other";QVERIFY(!Staff::validateOracleConfig(config).isEmpty());
        config.host="localhost)(PORT=9999)";QVERIFY(!Staff::validateOracleConfig(config).isEmpty());
        config.host="localhost";config.service="xepdb1);UID=other";QVERIFY(!Staff::validateOracleConfig(config).isEmpty());
        config.service="xepdb1";
        config.host="localhost";config.password.clear();QVERIFY(!Staff::validateOracleConfig(config).isEmpty());
    }
    void oracleModeNeverSilentlyEditsTheLocalCopy() {
        const auto before=savedState();delete root;openWindow(true);
        QCOMPARE(table->rowCount(),0);
        QVERIFY(!find<QPushButton>("addStaff")->isEnabled());
        QVERIFY(!find<QPushButton>("editStaff")->isEnabled());
        QVERIFY(find<QPushButton>("staffDatabase")->isEnabled());
        QCOMPARE(savedState().revision,before.revision);
        QCOMPARE(savedState().members.size(),before.members.size());
    }
    void oracleDialogValidatesBeforeConnectingAndCancelsCleanly() {
        const int revision=savedState().revision;bool opened=false;
        QTimer::singleShot(0,this,[&]{
            auto *dialog=find<QDialog>("oracleConnectionDialog");if(!dialog)return;opened=true;
            auto *password=dialog->findChild<QLineEdit*>("oraclePassword");QCOMPARE(password->echoMode(),QLineEdit::Password);
            dialog->findChild<QLineEdit*>("oracleUser")->clear();password->clear();
            dialog->findChild<QPushButton*>("testOracleConnection")->click();
            QVERIFY(!dialog->findChild<QLabel*>("oracleConnectionStatus")->text().isEmpty());
            const QString capture=qEnvironmentVariable("STAFF_ORACLE_CAPTURE");
            if(!capture.isEmpty()){dialog->resize(740,650);QApplication::processEvents();QVERIFY(dialog->grab().save(capture));}
            dialog->reject();
        });
        QTimer watchdog;watchdog.setSingleShot(true);connect(&watchdog,&QTimer::timeout,this,[this]{for(auto *dialog:canvas->findChildren<QDialog*>())if(dialog->isVisible())dialog->reject();});watchdog.start(5000);
        find<QPushButton>("staffDatabase")->click();QVERIFY(opened);QCOMPARE(savedState().revision,revision);
    }

    void initialDesignAndResize() {
        QCOMPARE(table->rowCount(),8);
        QCOMPARE(table->item(table->currentRow(),1)->text(),QString("STF-2025-008"));
        QCOMPARE(table->item(4,0)->checkState(),Qt::Checked);
        auto *name=find<QLabel>("staffDetailName");
        auto *status=find<QLabel>("staffDetailStatus");
        QVERIFY(status->x()>name->x()+QFontMetrics(name->font()).horizontalAdvance(name->text()));
        const QImage image=canvas->grab().toImage();
        const qreal dpr=image.devicePixelRatio();
        for (int x : {24,46,68}) {
            const QColor sample=image.pixelColor(int(x*dpr),int(22*dpr));
            QVERIFY2(sample.red()<20 && sample.green()<45 && sample.blue()<65,"Decorative traffic-light dots must be absent");
        }
        auto *view=root->findChild<QGraphicsView*>("staffView");
        root->resize(1100,700);QApplication::processEvents();
        QCOMPARE(view->transform().m11(),view->transform().m22());
        QVERIFY(view->mapFromScene(QPointF(1672,941)).x()<=view->viewport()->width()+1);
        QVERIFY(view->mapFromScene(QPointF(1672,941)).y()<=view->viewport()->height()+1);
    }
    void filtersPaginationAndEmptyState() {
        find<QPushButton>("nextPage")->click();QCOMPARE(table->rowCount(),4);
        QVERIFY(!find<QPushButton>("nextPage")->isEnabled());
        find<QPushButton>("previousPage")->click();QCOMPARE(table->rowCount(),8);
        search->setText("  DaNiEl  ");QCOMPARE(table->rowCount(),1);
        QCOMPARE(table->item(0,2)->text(),QString("Daniel Kim"));
        search->setText("no-member-matches-this");QCOMPARE(table->rowCount(),0);
        QVERIFY(find<QLabel>("emptyStaffList")->isVisible());
        QVERIFY(!find<QPushButton>("editStaff")->isEnabled());
        QVERIFY(!tab("Role && Permissions")->isEnabled());
        search->clear();QVERIFY(find<QPushButton>("editStaff")->isEnabled());
        find<QComboBox>("statusFilter")->setCurrentText("Inactive");QCOMPARE(table->rowCount(),1);
        QCOMPARE(table->item(0,2)->text(),QString("Ryan Patel"));
        find<QComboBox>("statusFilter")->setCurrentIndex(0);
        find<QComboBox>("roleFilter")->setCurrentText("Data Analyst");QCOMPARE(table->rowCount(),2);
        QCOMPARE(find<QComboBox>("listRoleFilter")->currentText(),QString("Data Analyst"));
        find<QComboBox>("listRoleFilter")->setCurrentIndex(0);QCOMPARE(table->rowCount(),8);
    }
    void actualCheckboxClicksAndKeyboard() {
        const auto index=table->model()->index(0,0);
        const QRect cell=table->visualRect(index);
        QTest::mouseClick(table->viewport(),Qt::LeftButton,Qt::NoModifier,QPoint(cell.left()+21,cell.center().y()));
        QCOMPARE(table->item(0,0)->checkState(),Qt::Checked);
        QTest::mouseClick(table->viewport(),Qt::LeftButton,Qt::NoModifier,QPoint(cell.left()+21,cell.center().y()));
        QCOMPARE(table->item(0,0)->checkState(),Qt::Unchecked);
        table->setCurrentCell(0,0);QTest::keyClick(table,Qt::Key_Space);
        QCOMPARE(table->item(0,0)->checkState(),Qt::Checked);
        search->setText("Alex");QCOMPARE(table->item(0,0)->checkState(),Qt::Checked);
        search->clear();
        QTest::mouseClick(table->horizontalHeader()->viewport(),Qt::LeftButton,Qt::NoModifier,QPoint(22,18));
        for(int row=0;row<table->rowCount();++row)QCOMPARE(table->item(row,0)->checkState(),Qt::Checked);
    }
    void formValidationAndEdit() {
        bool exercised=false;
        QTimer::singleShot(0,this,[&]{
            auto *dialog=find<QDialog>("staffEditor");
            if(!dialog)return;
            auto *fullName=dialog->findChild<QLineEdit*>("memberName");
            auto *mail=dialog->findChild<QLineEdit*>("memberEmail");
            auto *error=dialog->findChild<QLabel*>("staffValidationError");
            save(dialog);QVERIFY(!error->text().isEmpty());QVERIFY(dialog->isVisible());
            fullName->setText("Donia Test");mail->setText("invalid@");save(dialog);
            QVERIFY(error->text().contains("valid email"));QVERIFY(dialog->isVisible());
            mail->setText(" ALEX.MARTIN@SKYSENSE.COM ");save(dialog);
            QVERIFY(error->text().contains("already belongs"));QVERIFY(dialog->isVisible());
            mail->setText("donia.test@example.com");
            dialog->findChild<QComboBox*>("memberRole")->setEditText("  ");save(dialog);
            QVERIFY(error->text().contains("Role and department"));
            dialog->findChild<QComboBox*>("memberRole")->setEditText(" Technical Specialist ");
            dialog->findChild<QLineEdit*>("memberName")->setText("  Donia Test  ");
            save(dialog);exercised=true;
        });
        QTimer::singleShot(3000,canvas,[this]{if(auto *dialog=find<QDialog>("staffEditor"))dialog->reject();});
        find<QPushButton>("addStaff")->click();
        QVERIFY(exercised);QCOMPARE(table->item(0,2)->text(),QString("Donia Test"));
        QCOMPARE(table->item(0,4)->text(),QString("Technical Specialist"));
        exercised=false;
        QTimer::singleShot(0,this,[&]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;
            dialog->findChild<QLineEdit*>("memberName")->setText("Donia Edited");
            save(dialog);exercised=true;
        });
        find<QPushButton>("editStaff")->click();
        QVERIFY(exercised);QCOMPARE(table->item(0,2)->text(),QString("Donia Edited"));
        QCOMPARE(savedState().audit.size(),2);
        delete root;openWindow();search->setText("Donia Edited");
        QCOMPARE(table->rowCount(),1);QCOMPARE(table->item(0,3)->text(),QString("donia.test@example.com"));
    }
    void cancelKeepsMemberUnchanged() {
        selectDaniel();
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;
            dialog->findChild<QLineEdit*>("memberName")->setText("Cancelled");dialog->reject();
        });
        find<QPushButton>("editStaff")->click();
        QCOMPARE(table->item(0,2)->text(),QString("Daniel Kim"));
    }
    void permissionsAndTabs() {
        selectDaniel();
        const auto boxes=find<QWidget>("overviewPanel")->findChildren<QCheckBox*>();
        boxes[0]->setChecked(false);search->setText("Alex");selectDaniel();
        QVERIFY(!boxes[0]->isChecked());
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffAccessEditor");if(!dialog)return;
            dialog->findChildren<QCheckBox*>()[1]->setChecked(false);save(dialog);
        });
        find<QPushButton>("editStaffAccess")->click();QVERIFY(!boxes[1]->isChecked());
        tab("Role && Permissions")->click();QVERIFY(find<QWidget>("overviewPanel")->isHidden());
        tab("Overview")->click();QVERIFY(!find<QWidget>("overviewPanel")->isHidden());
    }
    void longTextStaysInsideDetails() {
        selectDaniel();
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;
            dialog->findChild<QLineEdit*>("memberName")->setText(QString(100,'W'));save(dialog);
        });
        find<QPushButton>("editStaff")->click();
        auto *name=find<QLabel>("staffDetailName");
        QCOMPARE(name->toolTip(),QString(100,'W'));
        QVERIFY(QFontMetrics(name->font()).horizontalAdvance(name->text())<=name->width());
        QVERIFY(find<QLabel>("staffDetailStatus")->geometry().right()<find<QPushButton>("staffTraining")->x());
    }
    void pdfExportCreatesDocument() {
        QTemporaryDir directory;QVERIFY(directory.isValid());
        const QString path=directory.filePath("staff.pdf");
        selectDaniel();bool chosen=false,confirmed=false;
        QTimer driver;driver.setInterval(25);
        connect(&driver,&QTimer::timeout,this,[&]{
            if(auto *dialog=canvas->findChild<QFileDialog*>()){
                if(dialog->isVisible()){dialog->selectFile(path);QMetaObject::invokeMethod(dialog,"accept");chosen=true;}
            }
            for(auto *message:root->findChildren<QMessageBox*>())if(message->isVisible()){
                confirmed=message->text().contains("saved successfully");message->accept();
            }
        });
        driver.start();
        QTimer::singleShot(4000,canvas,[this]{for(auto *dialog:canvas->findChildren<QDialog*>())if(dialog->isVisible())dialog->reject();});
        find<QPushButton>("exportStaffPdf")->click();driver.stop();
        QVERIFY(chosen && confirmed);QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().startsWith("%PDF-"));QVERIFY(file.size()>1000);
    }
    void sortingAcrossAllPages() {
        auto *header=table->horizontalHeader();
        const QPoint nameHeader(header->sectionViewportPosition(2)+header->sectionSize(2)/2,16);
        QTest::mouseClick(header->viewport(),Qt::LeftButton,Qt::NoModifier,nameHeader);
        QCOMPARE(table->item(0,2)->text(),QString("Alex Martin"));
        find<QPushButton>("nextPage")->click();QCOMPARE(table->item(0,2)->text(),QString("Noah Brown"));
        QTest::mouseClick(header->viewport(),Qt::LeftButton,Qt::NoModifier,nameHeader);
        QCOMPARE(table->item(0,2)->text(),QString("Sophia Chen"));
        search->setText("Data Analyst");QCOMPARE(table->rowCount(),2);QCOMPARE(table->item(0,2)->text(),QString("Mia Johnson"));
    }
    void deletionConfirmationRendersAndResponds_data() {
        QTest::addColumn<QSize>("windowSize");
        QTest::addColumn<int>("response");
        QTest::newRow("compact-no") << QSize(1100,700) << 0;
        QTest::newRow("original-yes") << QSize(1646,932) << 1;
        QTest::newRow("large-escape") << QSize(1920,1080) << 2;
    }
    void deletionConfirmationRendersAndResponds() {
        QFETCH(QSize,windowSize);
        QFETCH(int,response);
        root->resize(windowSize);QApplication::processEvents();selectDaniel();
        auto *view=root->findChild<QGraphicsView*>("staffView");
        auto *remove=find<QPushButton>("deleteStaff");
        bool inspected=false,answered=false,timedOut=false;
        QTimer watchdog;watchdog.setSingleShot(true);
        connect(&watchdog,&QTimer::timeout,this,[&]{
            timedOut=true;
            for(auto *message:root->findChildren<QMessageBox*>())message->reject();
        });
        QTimer driver;driver.setInterval(30);
        connect(&driver,&QTimer::timeout,this,[&]{
            for(auto *message:root->findChildren<QMessageBox*>()){
                if(!message->isVisible() || message->windowTitle()!="Delete Staff")continue;
                driver.stop();
                QVERIFY(message->testOption(QMessageBox::Option::DontUseNativeDialog));
                QCOMPARE(message->textFormat(),Qt::PlainText);
                QVERIFY(message->text().contains("Daniel Kim (STF-2025-008)"));
                auto *yes=message->button(QMessageBox::Yes),*no=message->button(QMessageBox::No);
                QVERIFY(yes && no);QVERIFY(yes->isVisible());QVERIFY(no->isVisible());
                QCOMPARE(message->defaultButton(),qobject_cast<QPushButton*>(no));
                QCOMPARE(message->escapeButton(),no);
                auto *label=message->findChild<QLabel*>("qt_msgbox_label");
                QVERIFY(label && label->isVisible());QVERIFY(!label->text().isEmpty());
                QVERIFY(!message->graphicsProxyWidget());
                QCOMPARE(message->parentWidget(),root->window());
                for(auto *widget:QList<QWidget*>{label,yes,no}){
                    const QRect area(widget->mapTo(message,QPoint()),widget->size());
                    QVERIFY2(message->rect().contains(area),"Confirmation text and buttons must fit inside the dialog.");
                }
                QVERIFY(message->width()<=windowSize.width());
                QVERIFY(message->height()<=windowSize.height());
                const QString capture=qEnvironmentVariable("STAFF_DELETE_CAPTURE");
                if(response==1 && !capture.isEmpty()){
                    QVERIFY(message->grab().save(capture));
                }
                inspected=true;
                if(response==2)QTest::keyClick(message,Qt::Key_Escape);
                else {
                    auto *choice=response==1?yes:no;
                    QTest::mouseClick(choice,Qt::LeftButton);
                }
                answered=true;
                return;
            }
        });
        driver.start();watchdog.start(5000);
        const QPoint point=view->mapFromScene(canvas->graphicsProxyWidget()->mapToScene(remove->geometry().center()));
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,point);
        driver.stop();watchdog.stop();
        QVERIFY(!timedOut);QVERIFY(inspected && answered);
        const auto state=savedState();
        QCOMPARE(state.members.size(),response==1?11:12);
        QCOMPARE(state.audit.size(),response==1?1:0);
        if(response==1)QCOMPARE(state.audit.first().memberId,QString("STF-2025-008"));
    }
    void deletionCancellationPersistenceAndAudit() {
        selectDaniel();confirmDelete(QMessageBox::No);QCOMPARE(table->rowCount(),1);
        QCOMPARE(savedState().members.size(),12);QVERIFY(savedState().audit.isEmpty());
        confirmDelete(QMessageBox::Yes);QCOMPARE(table->rowCount(),0);
        auto state=savedState();QCOMPARE(state.members.size(),11);QVERIFY(!state.permissions.contains("STF-2025-008"));
        QCOMPARE(state.audit.size(),1);QCOMPARE(state.audit[0].action,QString("Staff deleted"));
        QCOMPARE(state.audit[0].memberName,QString("Daniel Kim"));QVERIFY(!state.audit[0].actor.isEmpty());
        delete root;openWindow();selectDaniel();QCOMPARE(table->rowCount(),0);
        int beforeSearch=-1,afterSearch=-1;QString deletedName;
        QTimer::singleShot(0,this,[&]{auto *dialog=find<QDialog>("auditDialog");if(!dialog)return;
            auto *grid=dialog->findChild<QTableWidget*>("auditTable");beforeSearch=grid->rowCount();if(beforeSearch)deletedName=grid->item(0,3)->text();
            dialog->findChild<QLineEdit*>("auditSearch")->setText("no-result");afterSearch=grid->rowCount();dialog->reject();
        });
        find<QPushButton>("staffAudit")->click();QCOMPARE(beforeSearch,1);QCOMPARE(afterSearch,0);QCOMPARE(deletedName,QString("Daniel Kim"));
    }
    void permissionProfilesAndInactiveAccess() {
        selectDaniel();
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffAccessEditor");if(!dialog)return;
            dialog->findChild<QComboBox*>("accessProfile")->setCurrentIndex(3);save(dialog);
        });
        find<QPushButton>("editStaffAccess")->click();
        auto state=savedState();QCOMPARE(state.permissions.value("STF-2025-008"),QList<bool>({false,true,true,false}));
        QVERIFY(Staff::effectiveAccess(state,"STF-2025-008",1));QVERIFY(!Staff::effectiveAccess(state,"STF-2025-008",0));
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;
            dialog->findChild<QComboBox*>("memberStatus")->setCurrentText("Inactive");save(dialog);
        });
        find<QPushButton>("editStaff")->click();
        delete root;openWindow();selectDaniel();state=savedState();
        QCOMPARE(state.permissions.value("STF-2025-008"),QList<bool>({false,true,true,false}));
        for(int module=0;module<4;++module)QVERIFY(!Staff::effectiveAccess(state,"STF-2025-008",module));
        QCOMPARE(state.audit.size(),2);QCOMPARE(state.audit.last().action,QString("Access changed"));
    }
    void statisticsReflectRecords() {
        selectDaniel();confirmDelete(QMessageBox::Yes);
        const auto s=Staff::statistics(savedState());QCOMPARE(s.total,11);QCOMPARE(s.active,10);QCOMPARE(s.inactive,1);
        QCOMPARE(s.departments.value("Operations"),4);QCOMPARE(s.departments.value("Engineering"),1);
        QString summary;int departments=-1,roles=-1,total=0;
        QTimer::singleShot(0,this,[&]{auto *dialog=find<QDialog>("statisticsDialog");if(!dialog)return;
            summary=dialog->findChild<QLabel*>("statisticsSummary")->text();auto *grid=dialog->findChild<QTableWidget*>("statisticsTable");departments=grid->rowCount();
            for(int row=0;row<departments;++row)total+=grid->item(row,1)->text().toInt();
            dialog->findChild<QComboBox*>("statisticsGrouping")->setCurrentIndex(1);roles=grid->rowCount();dialog->reject();
        });
        find<QPushButton>("staffStatistics")->click();QVERIFY(summary.contains("Total: 11"));QCOMPARE(total,11);QCOMPARE(departments,5);QCOMPARE(roles,8);
    }
    void emptyDataSurvivesRestart() {
        for(int i=0;i<12;++i)confirmDelete(QMessageBox::Yes);
        QCOMPARE(table->rowCount(),0);QVERIFY(!find<QPushButton>("deleteStaff")->isEnabled());
        QCOMPARE(savedState().audit.size(),12);
        delete root;openWindow();QCOMPARE(table->rowCount(),0);QVERIFY(savedState().members.isEmpty());
        const auto s=Staff::statistics(savedState());QCOMPARE(s.total,0);QCOMPARE(s.active,0);QCOMPARE(s.inactive,0);
    }
    void deletedIdentifiersAreNotReused() {
        createMember("New Member A","a@example.com");search->setText("a@example.com");QCOMPARE(table->rowCount(),1);const QString first=table->item(0,1)->text();
        confirmDelete(QMessageBox::Yes);delete root;openWindow();
        createMember("New Member B","b@example.com");search->setText("b@example.com");QCOMPARE(table->rowCount(),1);const QString second=table->item(0,1)->text();
        QVERIFY(first!=second);QCOMPARE(savedState().audit.size(),3);
    }
    void staleUiWriteRollsBack() {
        Staff::Store store(storagePath());auto external=savedState();external.members[0].name="External edit";QString error;QVERIFY(store.save(external,error));
        search->setText("Alex Martin");bool warned=false;
        QTimer driver;driver.setInterval(20);connect(&driver,&QTimer::timeout,this,[&]{for(auto *message:root->findChildren<QMessageBox*>())if(message->isVisible()){warned=message->text().contains("another instance");message->accept();}});driver.start();
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");if(!dialog)return;dialog->findChild<QLineEdit*>("memberName")->setText("Must not overwrite");save(dialog);});
        find<QPushButton>("editStaff")->click();driver.stop();QVERIFY(warned);
        search->setText("Alex Martin");QCOMPARE(table->rowCount(),1);QCOMPARE(savedState().members[0].name,QString("External edit"));QVERIFY(savedState().audit.isEmpty());
    }
    void storageRejectsCorruptionDuplicatesAndLocks() {
        QString error;Staff::Store store(storagePath());auto invalid=savedState();const int revision=invalid.revision;
        invalid.members[0].email=invalid.members[1].email;QVERIFY(!store.save(invalid,error));QCOMPARE(savedState().revision,revision);
        QLockFile lock(storagePath()+".lock");QVERIFY(lock.tryLock());auto state=savedState();QVERIFY(!store.save(state,error));lock.unlock();QCOMPARE(savedState().revision,revision);
        const QString corruptPath=storageDirectory->filePath("corrupt.json");QFile file(corruptPath);QVERIFY(file.open(QIODevice::WriteOnly));file.write("{invalid data");file.close();
        Staff::Store corrupt(corruptPath);QVERIFY(!corrupt.load(state,error));QVERIFY(!corrupt.save(state,error));
        QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.readAll(),QByteArray("{invalid data"));
    }
    void trainingDistinguishesUnknownFromGapsAndPrioritizes() {
        Staff::State state;Staff::Member member;member.role="Station Technician";
        auto report=Staff::trainingReport(state,member);
        QCOMPARE(report.required,3);QCOMPARE(report.assessed,0);QCOMPARE(report.unassessed.size(),3);QVERIFY(report.needs.isEmpty());
        member.skills={{"Sensor installation",3},{"Sensor calibration",0},{"Fault diagnosis",1}};
        report=Staff::trainingReport(state,member);
        QCOMPARE(report.assessed,3);QCOMPARE(report.met,1);QVERIFY(report.unassessed.isEmpty());QCOMPARE(report.needs.size(),2);
        QCOMPARE(report.needs[0].skill,QString("Sensor calibration"));QCOMPARE(report.needs[0].gap,2);
        QCOMPARE(report.needs[1].skill,QString("Fault diagnosis"));QCOMPARE(report.needs[1].gap,1);
        member.skills["Sensor calibration"]=2;member.skills["Fault diagnosis"]=3;
        report=Staff::trainingReport(state,member);QVERIFY(report.needs.isEmpty());QCOMPARE(report.met,3);
        member.role="  station TECHNICIAN ";member.skills={{"sensor   INSTALLATION",2}};
        report=Staff::trainingReport(state,member);QCOMPARE(report.met,1);QCOMPARE(report.unassessed.size(),2);
        member.role="New role without a profile";report=Staff::trainingReport(state,member);
        QCOMPARE(report.required,0);QVERIFY(report.needs.isEmpty());
    }
    void trainingDialogPersistsAssessmentAndAudit() {
        search->setText("Sophia Chen");
        useTraining([&](QDialog *dialog){
            auto *needs=dialog->findChild<QTableWidget*>("trainingNeedsTable");QCOMPARE(needs->rowCount(),0);
            QVERIFY(setAssessment(dialog,"Sensor installation",2));QVERIFY(setAssessment(dialog,"Sensor calibration",0));QVERIFY(setAssessment(dialog,"Fault diagnosis",1));
            QCOMPARE(needs->rowCount(),2);QCOMPARE(needs->item(0,0)->text(),QString("High"));QCOMPARE(needs->item(0,1)->text(),QString("Sensor calibration"));
            QCOMPARE(needs->item(1,0)->text(),QString("Normal"));
            QVERIFY(dialog->findChild<QLabel*>("trainingSummary")->text().contains("3 / 3"));
            dialog->resize(860,620);QApplication::processEvents();
            auto *skillGrid=dialog->findChild<QTableWidget*>("trainingSkillsTable");
            auto *heading=dialog->findChild<QLabel*>("trainingRecommendationsHeading");
            QVERIFY(heading->y()>skillGrid->geometry().bottom());
            auto *scroll=dialog->findChild<QScrollArea*>("trainingScrollArea");
            auto *saveButton=dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save);
            QVERIFY(saveButton->mapTo(dialog,QPoint()).y()>scroll->geometry().bottom());
            const QString capture=qEnvironmentVariable("STAFF_TRAINING_CAPTURE");
            if(!capture.isEmpty()){dialog->resize(1020,790);QApplication::processEvents();QVERIFY(dialog->grab().save(capture));}
            save(dialog);
        });
        auto state=savedState();QCOMPARE(state.audit.first().action,QString("Training assessment updated"));
        QVERIFY(state.audit.first().details.contains("Sensor calibration"));
        delete root;openWindow();search->setText("Sophia Chen");
        useTraining([&](QDialog *dialog){
            auto *needs=dialog->findChild<QTableWidget*>("trainingNeedsTable");QCOMPARE(needs->rowCount(),2);
            QVERIFY(setAssessment(dialog,"Sensor calibration",2));QCOMPARE(needs->rowCount(),1);save(dialog);
        });
        QCOMPARE(savedState().audit.size(),2);
        search->setText("no matching record");QVERIFY(!find<QPushButton>("staffTraining")->isEnabled());
    }
    void trainingCancelDiscardsAssessmentAndRoleEdits() {
        selectDaniel();const auto before=savedState();bool nested=false;
        useTraining([&](QDialog *dialog){
            QVERIFY(setAssessment(dialog,"Sensor calibration",0));
            QTimer::singleShot(0,this,[&]{
                auto *editor=dialog->findChild<QDialog*>("roleRequirementsEditor");if(!editor)return;
                auto *grid=editor->findChild<QTableWidget*>("roleRequirementsTable");
                qobject_cast<QComboBox*>(grid->cellWidget(0,1))->setCurrentIndex(0);save(editor);nested=true;
            });
            dialog->findChild<QPushButton*>("editRoleRequirements")->click();dialog->reject();
        });
        QVERIFY(nested);const auto after=savedState();QCOMPARE(after.revision,before.revision);
        QCOMPARE(after.roleRequirements,before.roleRequirements);QVERIFY(after.audit.isEmpty());
        for(const auto &member:after.members)QVERIFY(member.skills.isEmpty());
    }
    void trainingCustomRoleValidationAndSharedRequirements() {
        createMember("Custom role member","custom@example.com");search->setText("custom@example.com");
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");dialog->findChild<QComboBox*>("memberRole")->setEditText("Network Inspector");save(dialog);});
        find<QPushButton>("editStaff")->click();
        bool validated=false;
        useTraining([&](QDialog *dialog){
            QCOMPARE(dialog->findChild<QTableWidget*>("trainingSkillsTable")->rowCount(),0);
            QVERIFY(dialog->findChild<QLabel*>("trainingEmptyState")->text().contains("No requirements"));
            QTimer::singleShot(0,this,[&]{
                auto *editor=dialog->findChild<QDialog*>("roleRequirementsEditor");if(!editor)return;
                auto *add=editor->findChild<QPushButton*>("addRoleRequirement");add->click();save(editor);
                QVERIFY(editor->isVisible());QVERIFY(!editor->findChild<QLabel*>("roleRequirementsError")->text().isEmpty());
                auto *grid=editor->findChild<QTableWidget*>("roleRequirementsTable");
                qobject_cast<QLineEdit*>(grid->cellWidget(0,0))->setText("Network inspection");add->click();
                qobject_cast<QLineEdit*>(grid->cellWidget(1,0))->setText(" NETWORK INSPECTION ");save(editor);QVERIFY(editor->isVisible());
                grid->selectRow(1);editor->findChild<QPushButton*>("removeRoleRequirement")->click();save(editor);validated=true;
            });
            dialog->findChild<QPushButton*>("editRoleRequirements")->click();
            QCOMPARE(dialog->findChild<QTableWidget*>("trainingSkillsTable")->rowCount(),1);
            QVERIFY(setAssessment(dialog,"Network inspection",1));save(dialog);
        });
        QVERIFY(validated);auto state=savedState();QCOMPARE(state.roleRequirements.value("Network Inspector").value("Network inspection"),2);
        Staff::Member colleague;colleague.role="Network Inspector";colleague.skills={{"Network inspection",1}};
        QCOMPARE(Staff::trainingReport(state,colleague).needs.size(),1);
        delete root;openWindow();QCOMPARE(savedState().roleRequirements,state.roleRequirements);
    }
    void trainingRoleChangeRecalculatesWithoutErasingSkills() {
        selectDaniel();
        useTraining([&](QDialog *dialog){QVERIFY(setAssessment(dialog,"Sensor calibration",2));QVERIFY(setAssessment(dialog,"Fault diagnosis",2));QVERIFY(setAssessment(dialog,"Station configuration",2));save(dialog);});
        QTimer::singleShot(0,this,[this]{auto *dialog=find<QDialog>("staffEditor");dialog->findChild<QComboBox*>("memberRole")->setCurrentText("Station Technician");save(dialog);});
        find<QPushButton>("editStaff")->click();
        useTraining([&](QDialog *dialog){
            QCOMPARE(dialog->findChild<QTableWidget*>("trainingNeedsTable")->rowCount(),0);
            QCOMPARE(dialog->findChild<QTableWidget*>("trainingSkillsTable")->rowCount(),4);
            QVERIFY(dialog->findChild<QLabel*>("trainingSummary")->text().contains("1 to assess"));dialog->reject();
        });
        for(const auto &member:savedState().members)if(member.id=="STF-2025-008")QCOMPARE(member.skills.value("Station configuration"),2);
    }
    void trainingLegacyMigrationAndInvalidLevels() {
        QFile original(storagePath());QVERIFY(original.open(QIODevice::ReadOnly));
        auto legacy=QJsonDocument::fromJson(original.readAll()).object();original.close();
        legacy["schema"]=1;legacy.remove("roleRequirements");auto records=legacy["members"].toArray();
        for(int i=0;i<records.size();++i){auto row=records[i].toObject();row.remove("skills");records[i]=row;}legacy["members"]=records;
        const auto legacyBytes=QJsonDocument(legacy).toJson();const QString path=storageDirectory->filePath("legacy.json");
        QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));file.write(legacyBytes);file.close();
        Staff::Store store(path);Staff::State state;QString error;QVERIFY2(store.load(state,error),qPrintable(error));
        QCOMPARE(state.members.size(),12);QVERIFY(state.members.first().skills.isEmpty());QCOMPARE(state.roleRequirements.size(),8);
        QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.readAll(),legacyBytes);file.close();
        const auto rights=state.permissions;state.members[0].skills={{"Team coordination",1}};QVERIFY(store.save(state,error));
        Staff::State reloaded;QVERIFY(store.load(reloaded,error));QCOMPARE(reloaded.permissions,rights);QCOMPARE(reloaded.members[0].skills,state.members[0].skills);
        QVERIFY(file.open(QIODevice::ReadOnly));const auto validBytes=file.readAll();file.close();QCOMPARE(QJsonDocument::fromJson(validBytes).object()["schema"].toInt(),2);
        auto invalid=reloaded;invalid.members[0].skills["Bad level"]=4;QVERIFY(!store.save(invalid,error));
        invalid=reloaded;invalid.members[0].skills["TEAM COORDINATION"]=2;QVERIFY(!store.save(invalid,error));
        invalid=reloaded;invalid.roleRequirements["New role"]={{"Calibration",0}};QVERIFY(!store.save(invalid,error));
        QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.readAll(),validBytes);file.close();
        auto malformed=QJsonDocument::fromJson(validBytes).object();auto malformedMembers=malformed["members"].toArray();
        auto row=malformedMembers[0].toObject();row["skills"]=QJsonObject{{"Calibration",1.5}};malformedMembers[0]=row;malformed["members"]=malformedMembers;
        QVERIFY(file.open(QIODevice::WriteOnly));file.write(QJsonDocument(malformed).toJson());file.close();QVERIFY(!store.load(reloaded,error));
    }
    void trainingConcurrentWriteRollsBackAssessment() {
        selectDaniel();auto external=savedState();external.members[0].name="External edit";QString error;
        QVERIFY(Staff::Store(storagePath()).save(external,error));bool warned=false;
        QTimer driver;driver.setInterval(20);connect(&driver,&QTimer::timeout,this,[&]{for(auto *message:root->findChildren<QMessageBox*>())if(message->isVisible()){warned=message->text().contains("another instance");message->accept();}});driver.start();
        useTraining([&](QDialog *dialog){QVERIFY(setAssessment(dialog,"Sensor calibration",0));save(dialog);});driver.stop();QVERIFY(warned);
        useTraining([&](QDialog *dialog){QCOMPARE(dialog->findChild<QTableWidget*>("trainingNeedsTable")->rowCount(),0);dialog->reject();});
        const auto saved=savedState();QCOMPARE(saved.members[0].name,QString("External edit"));QVERIFY(saved.audit.isEmpty());
        for(const auto &member:saved.members)QVERIFY(member.skills.isEmpty());
    }
};

int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc,argv);app.setStyle("Fusion");
    StaffTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "tst_staff.moc"
