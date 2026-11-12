#include "staffaccountpanel.h"
#include "appsession.h"
#include "staffdesign.h"
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

StaffAccountPanel::StaffAccountPanel(QWidget *parent):QWidget(parent)
{
    setObjectName("staffAccountPanel");
    auto *layout=new QVBoxLayout(this);layout->setContentsMargins(7,12,7,8);layout->setSpacing(10);
    heading=StaffDesign::label(this,"Manager login","subheading");layout->addWidget(heading);
    state=StaffDesign::label(this,{},"muted");state->setWordWrap(true);layout->addWidget(state);
    auto *form=new QFormLayout;form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    email=new QLineEdit(this);email->setObjectName("accountEmail");email->setMaxLength(254);
    role=new QComboBox(this);role->setObjectName("accountRole");role->addItems(StaffAccounts::managerRoles());
    password=new QLineEdit(this);password->setObjectName("accountPassword");password->setEchoMode(QLineEdit::Password);
    form->addRow("Login email",email);form->addRow("Department role",role);form->addRow("Password",password);
    layout->addLayout(form);
    auto *actions=new QHBoxLayout;
    submit=StaffDesign::action(this,"Create login","saveManagerAccount");submit->setProperty("primary",true);
    removeButton=StaffDesign::action(this,"Remove login","removeManagerAccount");removeButton->setProperty("danger",true);
    actions->addWidget(submit);actions->addWidget(removeButton);actions->addStretch();layout->addLayout(actions);
    message=StaffDesign::label(this,{},"value");message->setObjectName("accountFeedback");message->setWordWrap(true);
    layout->addWidget(message);layout->addStretch();
    connect(submit,&QPushButton::clicked,this,[this]{save();});
    connect(removeButton,&QPushButton::clicked,this,[this]{removeLogin();});
    reset();
}
void StaffAccountPanel::reset()
{
    account={};staffId.clear();memberName.clear();contact.clear();password->clear();email->clear();
    role->setCurrentIndex(-1);message->clear();state->clear();setEnabled(false);
}
void StaffAccountPanel::selectMember(const QString &id,const QString &name,const QString &staffEmail,bool active)
{
    if (!AppSession::isAdministrator() || id.isEmpty()) {reset();return;}
    if (id==staffId && name==memberName && contact==staffEmail && active==memberActive) return;
    staffId=id;memberName=name;contact=staffEmail;memberActive=active;reload();
}
void StaffAccountPanel::reload()
{
    account={};password->clear();message->clear();
    QString error;
    if (!StaffAccounts::forMember(staffId,account,error)) {message->setText(error);setEnabled(false);return;}
    setEnabled(true);
    const bool admin=account.role=="Administrator";
    heading->setText((admin?"Administrator login · ":"Manager login · ")+memberName);
    state->setText(!memberActive?"Inactive staff cannot sign in.":admin?"This is the workspace administrator.":
                   account.id?"This account can open Dashboard and its assigned module.":"Create a login for this staff member.");
    email->setText(account.id?account.email:contact);email->setReadOnly(admin);
    role->clear();
    if (admin) role->addItem("Administrator");else role->addItems(StaffAccounts::managerRoles());
    role->setPlaceholderText("Choose a role");role->setCurrentIndex(role->findText(account.role));
    role->setEnabled(!admin);password->setEnabled(!admin);
    password->setPlaceholderText(admin?"Administrator credential":account.id?"Leave blank to keep current password":"Enter the manager’s password");
    submit->setVisible(!admin);submit->setText(account.id?"Save changes":"Create login");
    removeButton->setVisible(account.id && !admin);
}
void StaffAccountPanel::save()
{
    QString error;
    if (!StaffAccounts::save(staffId,account,email->text(),role->currentText(),password->text(),error)) {
        message->setText(error);return;
    }
    reload();message->setText("Login saved.");
}
void StaffAccountPanel::removeLogin()
{
    if (!AppSession::isAdministrator() || !account.id || account.role=="Administrator") return;
    if (QMessageBox::question(StaffDesign::dialogOwner(this),"Remove manager login","Remove sign-in access for "+memberName+"? Their staff record will be kept.",
                              QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    QString error;
    if (!StaffAccounts::remove(staffId,account,error)) {message->setText(error);return;}
    reload();message->setText("Login removed.");
}

