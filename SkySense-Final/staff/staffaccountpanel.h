#pragma once
#include "staffaccounts.h"
#include <QWidget>
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class StaffAccountPanel final : public QWidget {
public:
    explicit StaffAccountPanel(QWidget *parent);
    void reset();
    void selectMember(const QString &id,const QString &name,const QString &staffEmail,bool active);
private:
    void reload();
    void save();
    void removeLogin();
    StaffAccounts::Account account;
    QString staffId,memberName,contact;
    bool memberActive=false;
    QLabel *heading,*state,*message;
    QLineEdit *email,*password;
    QComboBox *role;
    QPushButton *submit,*removeButton;
};
