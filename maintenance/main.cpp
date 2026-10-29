#include "mainwindow.h"
#include "connection.h"

#include <QApplication>
#include <QInputDialog>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>

static void tryOracleConnection(QApplication &app)
{
    if (app.arguments().contains(QStringLiteral("--skip-oracle")))
        return;

    QSettings settings(QStringLiteral("CoreVision"), QStringLiteral("SkySenseMaintenance"));
    const QString dataSource = qEnvironmentVariable("SKYSENSE_DB_DSN",
        settings.value(QStringLiteral("oracle/dsn"), Connection::defaultDataSource()).toString()).trimmed();
    const QString userName = qEnvironmentVariable("SKYSENSE_DB_USER",
        settings.value(QStringLiteral("oracle/user"), Connection::defaultUserName()).toString()).trimmed();
    QString password = qEnvironmentVariable("SKYSENSE_DB_PASSWORD");

    if (password.isEmpty()) {
        bool accepted = false;
        password = QInputDialog::getText(nullptr, QStringLiteral("Oracle sign in"),
                                         QStringLiteral("Password for %1").arg(userName),
                                         QLineEdit::Password, QString(), &accepted);
        if (!accepted) {
            QMessageBox::information(nullptr, QStringLiteral("Maintenance preview"),
                                     QStringLiteral("Oracle sign in was skipped. The sample interventions are temporary and will not be saved."));
            return;
        }
    }

    QString error;
    if (Connection::createInstance().createConnection(dataSource, userName, password, &error)) {
        settings.setValue(QStringLiteral("oracle/dsn"), dataSource);
        settings.setValue(QStringLiteral("oracle/user"), userName);
        return;
    }

    QMessageBox::warning(nullptr, QStringLiteral("Oracle connection failed"),
                         QStringLiteral("Could not connect through ODBC data source '%1'.\n\n%2\n\n"
                                        "Check the 64-bit SKYSENSE data source and the Oracle XEPDB1 service. "
                                        "The application will open with temporary preview data.")
                             .arg(dataSource, error));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("SkySense");
    a.setOrganizationName("CoreVision");
    QLocale::setDefault(QLocale(QLocale::French, QLocale::France));
    tryOracleConnection(a);
    MainWindow w;
    w.show();
    return a.exec();
}
