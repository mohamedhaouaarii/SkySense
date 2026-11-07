#include "compteui.h"
#include "compte.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
class AdminSetupDialog final : public QDialog
{
    // Objet du modele utilise par la vue (atelier CRUD : "Etudiant Etmp;").
    Compte Ctmp;

  public:
    explicit AdminSetupDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(QStringLiteral("Create the administrator account — SkySense"));
        setMinimumWidth(460);
        setStyleSheet(QStringLiteral(
            "QDialog{background:#f5f8fd;}"
            "QLabel#setupTitle{color:#102448;font:700 20px 'Segoe UI';}"
            "QLabel#setupText{color:#5b6f8f;font:13px 'Segoe UI';}"
            "QLabel{color:#253b5e;font:600 12px 'Segoe UI';}"
            "QLineEdit{background:#ffffff;border:1px solid #cfdcee;border-radius:8px;padding:8px 10px;"
            "font:13px 'Segoe UI';color:#102448;}"
            "QLineEdit:focus{border:1px solid #1769c2;}"
            "QPushButton{border-radius:8px;padding:8px 18px;font:600 13px 'Segoe UI';}"
            "QPushButton#setupCreate{background:#1769c2;color:white;border:0;}"
            "QPushButton#setupCreate:hover{background:#0f58a8;}"
            "QPushButton#setupLater{background:white;color:#253b5e;border:1px solid #cfdcee;}"));

        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(26, 24, 26, 20);
        root->setSpacing(14);
        auto *title = new QLabel(QStringLiteral("Create the administrator account"));
        title->setObjectName(QStringLiteral("setupTitle"));
        auto *intro = new QLabel(QStringLiteral(
            "No administrator exists in Oracle yet. This account signs in with its email and password, "
            "and can then give each module manager their own access from the Staff page."));
        intro->setObjectName(QStringLiteral("setupText"));
        intro->setWordWrap(true);
        root->addWidget(title);
        root->addWidget(intro);

        auto *form = new QFormLayout;
        form->setSpacing(10);
        form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        prenom = champ(QStringLiteral("e.g. Sara"));
        nom = champ(QStringLiteral("e.g. Ben Ali"));
        cin = champ(QStringLiteral("e.g. 01234567"));
        cin->setMaxLength(20);
        email = champ(QStringLiteral("e.g. admin@company.com"));
        motDePasse = champ(QStringLiteral("At least 6 characters"));
        motDePasse->setEchoMode(QLineEdit::Password);
        confirmation = champ(QStringLiteral("Type the password again"));
        confirmation->setEchoMode(QLineEdit::Password);
        form->addRow(QStringLiteral("First name *"), prenom);
        form->addRow(QStringLiteral("Last name *"), nom);
        form->addRow(QStringLiteral("CIN *"), cin);
        form->addRow(QStringLiteral("Email *"), email);
        form->addRow(QStringLiteral("Password *"), motDePasse);
        form->addRow(QStringLiteral("Confirm password *"), confirmation);
        root->addLayout(form);

        auto *buttons = new QDialogButtonBox;
        auto *plusTard = buttons->addButton(QStringLiteral("Later"), QDialogButtonBox::RejectRole);
        plusTard->setObjectName(QStringLiteral("setupLater"));
        auto *creer = buttons->addButton(QStringLiteral("Create administrator"), QDialogButtonBox::AcceptRole);
        creer->setObjectName(QStringLiteral("setupCreate"));
        creer->setDefault(true);
        root->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(creer, &QPushButton::clicked, this, [this] { creerAdmin(); });
    }

  private:
    QLineEdit *prenom = nullptr, *nom = nullptr, *cin = nullptr, *email = nullptr;
    QLineEdit *motDePasse = nullptr, *confirmation = nullptr;

    static QLineEdit *champ(const QString &exemple)
    {
        auto *edit = new QLineEdit;
        edit->setPlaceholderText(exemple);
        return edit;
    }

    void creerAdmin()
    {
        if (motDePasse->text() != confirmation->text())
        {
            QMessageBox::warning(this, QStringLiteral("Check the password"),
                                 QStringLiteral("The two passwords do not match."));
            return;
        }
        QString error;
        if (!Ctmp.creerAdmin(cin->text(), prenom->text(), nom->text(), email->text(), motDePasse->text(), &error))
        {
            QMessageBox::warning(this, QStringLiteral("Administrator not created"), error);
            return;
        }
        motDePasse->clear();
        confirmation->clear();
        accept();
    }
};
} // namespace

bool assurerCompteAdmin(QWidget *parent)
{
    bool existe = false;
    QString error;
    if (!Compte().adminExiste(&existe, &error))
    {
        QMessageBox::warning(parent, QStringLiteral("Accounts unavailable"),
                             QStringLiteral("Could not read the accounts stored in PERSONNEL. "
                                            "Run database/ajout_comptes_personnel.sql first.\n\n") + error);
        return false;
    }
    if (existe)
        return true;
    AdminSetupDialog dialog(parent);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    QMessageBox::information(parent, QStringLiteral("Administrator created"),
                             QStringLiteral("The administrator account is saved in Oracle. "
                                            "Sign in with its email and password."));
    return true;
}
