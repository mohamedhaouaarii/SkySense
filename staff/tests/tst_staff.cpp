#include <QtWidgets>
#include <QtTest>
#include "staffui.h"

class StaffTests : public QObject {
    Q_OBJECT
    QWidget *root = nullptr;
    QWidget *canvas = nullptr;
    QTableWidget *table = nullptr;
    QLineEdit *search = nullptr;

    template<class T> T *find(const char *name) const { return canvas->findChild<T *>(name); }
    QPushButton *tab(const QString &title) const {
        for (auto *button : canvas->findChildren<QPushButton *>())
            if (button->text() == title) return button;
        return nullptr;
    }
    void selectDaniel() { search->setText("Daniel Kim"); }
    void save(QDialog *dialog) { dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click(); }

private slots:
    void init() {
        root = new QWidget;
        root->setAttribute(Qt::WA_DontShowOnScreen);
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
    void cleanup() { delete root; root = nullptr; }

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
        QTimer::singleShot(3000,this,[this]{if(auto *dialog=find<QDialog>("staffEditor"))dialog->reject();});
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
            for(auto *message:canvas->findChildren<QMessageBox*>())if(message->isVisible()){
                confirmed=message->text().contains("saved successfully");message->accept();
            }
        });
        driver.start();
        QTimer::singleShot(4000,this,[this]{for(auto *dialog:canvas->findChildren<QDialog*>())if(dialog->isVisible())dialog->reject();});
        find<QPushButton>("exportStaffPdf")->click();driver.stop();
        QVERIFY(chosen && confirmed);QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().startsWith("%PDF-"));QVERIFY(file.size()>1000);
    }
};

int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc,argv);app.setStyle("Fusion");
    StaffTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "tst_staff.moc"
