#include "stafforacle.h"
#include <QtWidgets>

namespace Staff {
bool connectOracleDialog(std::unique_ptr<OracleStore> &store,State &state,const State *localData,QWidget *parent) {
    QDialog dialog(parent);dialog.setObjectName("oracleConnectionDialog");dialog.setWindowTitle("SkySense - Oracle connection");dialog.setMinimumWidth(680);
    auto *layout=new QVBoxLayout(&dialog);layout->setContentsMargins(24,22,24,22);layout->setSpacing(12);
    auto *title=new QLabel("Connect Staff to Oracle",&dialog);title->setStyleSheet("font-size:23px;font-weight:700;color:#080d34;");layout->addWidget(title);
    auto *caption=new QLabel("Test your project account, then connect this Staff page to its Oracle tables. Your password is used for this session only.",&dialog);
    caption->setWordWrap(true);layout->addWidget(caption);
    QSettings settings("CoreVision","SkySense Staff");settings.beginGroup("Oracle");
    auto *form=new QFormLayout;
    auto *driver=new QComboBox(&dialog);driver->setObjectName("oracleDriver");driver->setEditable(true);driver->addItems(oracleOdbcDrivers());
    if(settings.contains("driver"))driver->setCurrentText(settings.value("driver").toString());
    auto *host=new QLineEdit(settings.value("host","localhost").toString(),&dialog);host->setObjectName("oracleHost");
    auto *port=new QSpinBox(&dialog);port->setObjectName("oraclePort");port->setRange(1,65535);port->setValue(settings.value("port",1521).toInt());
    auto *service=new QLineEdit(settings.value("service","xepdb1").toString(),&dialog);service->setObjectName("oracleService");
    auto *user=new QLineEdit(settings.value("user").toString(),&dialog);user->setObjectName("oracleUser");user->setPlaceholderText("Your project schema username");
    auto *password=new QLineEdit(&dialog);password->setObjectName("oraclePassword");password->setEchoMode(QLineEdit::Password);
    form->addRow("Oracle ODBC driver (64-bit)",driver);form->addRow("Host",host);form->addRow("Port",port);
    form->addRow("Service name",service);form->addRow("Username",user);form->addRow("Password",password);layout->addLayout(form);
    auto *setup=new QCheckBox("Create the SS_STAFF tables if missing",&dialog);setup->setObjectName("oracleCreateSchema");setup->setChecked(true);layout->addWidget(setup);
    auto *import=new QCheckBox(localData?QString("Import this device's %1 Staff records on first setup only").arg(localData->members.size()):"Local copy unavailable: import disabled",&dialog);
    import->setObjectName("oracleImportLocal");import->setEnabled(localData!=nullptr);import->setChecked(localData!=nullptr);layout->addWidget(import);
    auto *note=new QLabel("Existing Oracle Staff records are loaded without merging or replacing them. A local JSON copy is kept. Test connection never creates or changes tables.",&dialog);
    note->setWordWrap(true);note->setStyleSheet("color:#526faa;background:#edf5ff;padding:10px;border-radius:6px;");layout->addWidget(note);
    auto *status=new QLabel(&dialog);status->setObjectName("oracleConnectionStatus");status->setWordWrap(true);status->setTextFormat(Qt::PlainText);layout->addWidget(status);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Cancel,&dialog);
    auto *test=buttons->addButton("Test connection",QDialogButtonBox::ActionRole);test->setObjectName("testOracleConnection");
    auto *connectButton=buttons->addButton("Use Oracle",QDialogButtonBox::AcceptRole);connectButton->setObjectName("useOracle");connectButton->setProperty("primary",true);
    layout->addWidget(buttons);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    auto config=[&]{return OracleConfig{driver->currentText().trimmed(),host->text().trimmed(),service->text().trimmed(),user->text().trimmed(),password->text(),port->value()};};
    auto showError=[&](const QString &error){status->setStyleSheet("color:#c3263a;");status->setText(error);};
    auto run=[&](bool use){
        const OracleConfig parameters=config();const QString validation=validateOracleConfig(parameters);
        if(!validation.isEmpty()){showError(validation);return;}
        QApplication::setOverrideCursor(Qt::WaitCursor);
        auto candidate=std::make_unique<OracleStore>();QString error;
        const bool opened=candidate->open(parameters,error);
        QApplication::restoreOverrideCursor();
        if(!opened){showError(error);return;}
        if(!use){status->setStyleSheet("color:#00884b;");status->setText("Connection successful: "+candidate->description()+". SELECT on DUAL succeeded; no data changed.");return;}
        bool ready=false;
        if(!candidate->schemaReady(ready,error)){showError(error);return;}
        if(!ready){
            if(!setup->isChecked()){showError("Staff tables are missing. Enable table creation or run database/oracle_staff.sql in your project schema.");return;}
            if(!candidate->initialize(error)){showError("Staff setup failed: "+error+"\nAlready created objects are kept; setup can be retried.");return;}
        }
        State loaded;
        if(!candidate->load(loaded,error)){showError(error);return;}
        if(localData && import->isChecked() && loaded.revision==0 && loaded.members.isEmpty() && loaded.audit.isEmpty()){
            State imported=*localData;imported.revision=loaded.revision;
            imported.audit.prepend({QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),operatorName(),"STAFF-IMPORT","Staff data","Imported to Oracle",QString("Imported %1 members from this device.").arg(imported.members.size())});
            if(!candidate->save(imported,error)){showError("Import failed; the local copy is unchanged. "+error);return;}
            loaded=imported;
        }
        settings.setValue("driver",parameters.driver);settings.setValue("host",parameters.host);settings.setValue("port",parameters.port);
        settings.setValue("service",parameters.service);settings.setValue("user",parameters.user);
        settings.remove("password"); // Older/manual settings must never retain a password.
        state=loaded;store=std::move(candidate);password->clear();dialog.accept();
    };
    QObject::connect(test,&QPushButton::clicked,&dialog,[&]{run(false);});
    QObject::connect(connectButton,&QPushButton::clicked,&dialog,[&]{run(true);});
    return dialog.exec()==QDialog::Accepted;
}
}
