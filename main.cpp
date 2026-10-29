#include "apptheme.h"
#include "skysensewindow.h"
#include "appsession.h"
#include "connection.h"
#include <QApplication>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QLineEdit>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QSqlError>
#include <QToolButton>
#include <QAction>
#include <QGraphicsEffect>

static void tryOracleConnection(QApplication &app)
{
#ifdef SKYSENSE_UI_TESTS
    Q_UNUSED(app);
    return;
#else
    if (app.arguments().contains("--skip-oracle"))
        return;

    // On recupere l'instance unique du singleton
    Connection &c = Connection::createInstance();

    // On tente d'etablir la connexion
    bool test = c.createConnection();

    //************************Test Connection BD**************************//
    if (test) {
        QMessageBox::information(
            nullptr,
            QObject::tr("Base de donnees"),
            QObject::tr("Connexion reussie."),
            QMessageBox::Ok
            );
    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de donnees"),
            QObject::tr("Echec de la connexion.\nL'application s'ouvre en mode apercu."),
            QMessageBox::Ok
            );
    }
#endif
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SkySense");
    app.setOrganizationName("CoreVision");
    app.setStyle("Fusion");
#ifdef SKYSENSE_UI_TESTS
    AppTheme::instance()->initialize(false);
    AppTheme::instance()->setDark(app.arguments().contains("--dark-theme"));
#else
    AppTheme::instance()->initialize();
#endif
    tryOracleConnection(app);
    SkySenseWindow window;
    const QRect available=app.primaryScreen()->availableGeometry();
    QSize targetSize(qMin(window.width(),available.width()-48),qMin(window.height(),available.height()-80));
    targetSize.setWidth(qMax(980,targetSize.width()));
    targetSize.setHeight(qMax(640,targetSize.height()));
    const QRect finalGeometry=QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,targetSize,available);
    QRect startGeometry=finalGeometry;
    startGeometry.translate(0,42);
    window.setGeometry(startGeometry);
    window.setWindowOpacity(0.0);
    window.show();
    auto *intro=new QParallelAnimationGroup(&window);
    auto *fade=new QPropertyAnimation(&window,"windowOpacity",intro);
    fade->setDuration(950); fade->setStartValue(0.0); fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    auto *slide=new QPropertyAnimation(&window,"geometry",intro);
    slide->setDuration(950); slide->setStartValue(startGeometry); slide->setEndValue(finalGeometry);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    intro->start(QAbstractAnimation::DeleteWhenStopped);
#ifdef SKYSENSE_UI_TESTS
    if (app.arguments().contains("--demo-login")) {
        QTimer::singleShot(1100, &window, [&window] {
            auto *email = window.findChild<QLineEdit *>("emailInput");
            auto *password = window.findChild<QLineEdit *>("passwordInput");
            auto *signIn = window.findChild<QPushButton *>("signInButton");
            if (email && password && signIn) {
                email->setText("validation@skysense.demo");
                password->setText("demo");
                signIn->click();
            }
        });
    }
    if(app.arguments().contains("--verify-theme")) {
        QTimer::singleShot(1200,&window,[&window,&app]{
            auto *a=window.findChild<QAbstractButton *>("loginThemeSwitch");
            auto *b=window.findChild<QAbstractButton *>("workspaceThemeSwitch");
            auto *form=window.findChild<QWidget *>("loginForm");
            if(!a || !b || !form){app.exit(90);return;}
            AppTheme::instance()->setDark(false);
            const QString original=form->styleSheet();
            a->click();
            if(!AppTheme::dark() || !a->isChecked() || !b->isChecked()){app.exit(91);return;}
            QDialog dialog(&window);QLabel label("New dialog",&dialog);
            dialog.setStyleSheet("QDialog{background:white;color:#111111;}");dialog.show();
            app.processEvents();
            if(!dialog.styleSheet().contains("#18283e")){app.exit(92);return;}
            b->click();app.processEvents();
            if(AppTheme::dark() || a->isChecked() || b->isChecked() || form->styleSheet()!=original || !dialog.styleSheet().contains("background:white")){app.exit(93);return;}
            dialog.close();app.exit(0);
        });
    }
    const bool captureLaptop = app.arguments().contains("--capture-laptop");
    const bool captureLogin = app.arguments().contains("--capture-login");
    const bool captureLoginCompact = app.arguments().contains("--capture-login-compact");
    const bool captureGlow = app.arguments().contains("--capture-glow");
    const bool captureEquipmentStatistics = app.arguments().contains("--capture-equipment-statistics");
    const bool verifyHeader = app.arguments().contains("--verify-header");
    const bool verifyAccess = app.arguments().contains("--verify-access");
    const bool verifyEmptyActions = app.arguments().contains("--verify-empty-actions");
#else
    const bool captureLaptop = false;
    const bool captureLogin = false;
    const bool captureLoginCompact = false;
    const bool captureGlow = false;
    const bool captureEquipmentStatistics = false;
    const bool verifyHeader = false;
    const bool verifyAccess = false;
    const bool verifyEmptyActions = false;
#endif
    if (captureLogin || captureLoginCompact) {
        intro->stop();
        window.resize(captureLoginCompact ? QSize(1100, 700) : QSize(1490, 850));
        window.setWindowOpacity(1.0);
        QTimer::singleShot(1100, &window, [&window, &app, captureLoginCompact] {
            const QString fileName = captureLoginCompact ? "/login_compact.png" : "/login_premium.png";
            if (!window.grab().save(QDir::currentPath() + fileName)) { app.exit(83); return; }
            app.quit();
        });
    }
    if (app.arguments().contains("--capture-pages") || captureLaptop || captureGlow || captureEquipmentStatistics || verifyHeader || verifyAccess || verifyEmptyActions) {
        window.resize(captureLaptop ? QSize(1440, 900) : QSize(1920, 1080));
        window.setWindowOpacity(1.0);
        QTimer::singleShot(1100, &window, [&window, &app, captureLaptop, captureGlow, captureEquipmentStatistics, verifyHeader, verifyAccess, verifyEmptyActions] {
            auto *email = window.findChild<QLineEdit *>("emailInput");
            auto *password = window.findChild<QLineEdit *>("passwordInput");
            auto *signIn = window.findChild<QPushButton *>("signInButton");
            if (!email || !password || !signIn) { app.exit(2); return; }
            email->setText(verifyEmptyActions ? "staff.test@skysense.com" : "mohamed.haouari@skysense.com");
            password->setText("demo");
            signIn->click();
            if (captureLaptop) window.resize(1440, 900);
            auto *sidebar = window.findChild<QWidget *>("servicesSidebar");
            if (!sidebar) { app.exit(3); return; }
            if (captureEquipmentStatistics) {
                for (auto *button : sidebar->findChildren<QPushButton *>())
                    if (button->property("nav").toBool() && button->text() == "Equipment") { button->click(); break; }
                auto *equipmentHost = window.findChild<QWidget *>("moduleHost_Equipment");
                auto *equipment = equipmentHost ? qobject_cast<QWidget *>(equipmentHost->property("protectedModuleWidget").value<QObject *>()) : nullptr;
                auto *statisticsButton = equipment ? equipment->findChild<QPushButton *>("equipmentStatisticsButton") : nullptr;
                if (!statisticsButton) { app.exit(80); return; }
                QTimer::singleShot(700, &window, [&app] {
                    auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    if (!dialog || !dialog->grab().save(QDir::currentPath() + "/equipment_statistics.png")) { app.exit(81); return; }
                    dialog->accept();
                });
                statisticsButton->click();
                if (!QFileInfo::exists(QDir::currentPath() + "/equipment_statistics.png")) { app.exit(82); return; }
                app.quit();
                return;
            }
            if (verifyEmptyActions) {
                auto *moduleStack = window.findChild<QStackedWidget *>("workspaceModulePages");
                auto selectModule = [sidebar](const QString &name) {
                    for (auto *button : sidebar->findChildren<QPushButton *>())
                        if (button->property("nav").toBool() && button->text() == name) { button->click(); return true; }
                    return false;
                };
                if (!moduleStack || !selectModule("Maintenance")) { app.exit(60); return; }
                auto *maintenanceHost = window.findChild<QWidget *>("moduleHost_Maintenance");
                auto *maintenance = maintenanceHost ? qobject_cast<QWidget *>(maintenanceHost->property("protectedModuleWidget").value<QObject *>()) : nullptr;
                auto *complete = maintenance ? maintenance->findChild<QPushButton *>(QString(), Qt::FindChildrenRecursively) : nullptr;
                if (!maintenance || !complete) { app.exit(61); return; }
                for (auto *button : maintenance->findChildren<QPushButton *>())
                    if (button->property("maintenanceCompleteAction").toBool()) complete = button;
                if (complete->isEnabled()) { app.exit(62); return; }
                for (auto *action : maintenance->findChildren<QCheckBox *>())
                    if (action->isEnabled()) { app.exit(63); return; }
                auto *notes = maintenance->findChild<QPlainTextEdit *>();
                if (!notes || notes->isEnabled()) { app.exit(64); return; }
                app.quit();
                return;
            }
            if (verifyAccess) {
                qInfo("ROLE_ACCESS_TEST: harness entered");
                const QStringList emails{"equipment.test@skysense.com", "service.test@skysense.com",
                                         "maintenance.test@skysense.com", "station.test@skysense.com",
                                         "staff.test@skysense.com"};
                const QStringList roles{"Responsable Equipment", "Responsable Service",
                                        "Responsable Maintenance", "Responsable Station", "Responsable Staff"};
                const QStringList modules{"Equipment", "Services", "Maintenance", "Stations", "Staff"};
                const QStringList allModules{"Dashboard", "Stations", "Equipment", "Maintenance", "Staff", "Services"};
                auto *moduleStack = window.findChild<QStackedWidget *>("workspaceModulePages");
                auto *headerSignOut = window.findChild<QAction *>("workspaceHeaderSignOutAction");
                auto *workspaceSearch = window.findChild<QLineEdit *>("workspaceSearch");
                if (!moduleStack || !headerSignOut || !workspaceSearch || sidebar->findChild<QPushButton *>("workspaceLogoutButton")) { app.exit(31); return; }
                for (int user = 0; user < modules.size(); ++user) {
                    email->setText(emails.at(user)); password->setText("test-only"); signIn->click();
                    qInfo().noquote() << "ROLE_ACCESS_TEST: signed in test role" << roles.at(user);
                    if (!AppSession::active() || AppSession::role() != roles.at(user)) { app.exit(32 + user); return; }
                    if (moduleStack->currentWidget()->objectName() != QStringLiteral("moduleHost_%1").arg(modules.at(user))) { app.exit(36 + user); return; }
                    bool activeNavigationMatches = false;
                    for (auto *button : sidebar->findChildren<QPushButton *>()) {
                        if (button->property("nav").toBool() && button->property("active").toBool()
                            && button->text() == modules.at(user)) activeNavigationMatches = true;
                    }
                    if (!activeNavigationMatches) { app.exit(55 + user); return; }
                    for (const QString &module : allModules) {
                        auto *overlay = window.findChild<QWidget *>(QStringLiteral("accessRestrictedOverlay_%1").arg(module));
                        auto *host = window.findChild<QWidget *>(QStringLiteral("moduleHost_%1").arg(module));
                        auto *content = host ? qobject_cast<QWidget *>(host->property("protectedModuleWidget").value<QObject *>()) : nullptr;
                        auto *blur = content ? content->graphicsEffect() : nullptr;
                        if (!overlay || !blur) { app.exit(40); return; }
                        const bool expectedAllowed = module == modules.at(user);
                        if (overlay->isHidden() != expectedAllowed || blur->isEnabled() == expectedAllowed
                            || content->isEnabled() != expectedAllowed) { app.exit(41 + user); return; }
                    }
                    const QString deniedModule = modules.at(user) == QStringLiteral("Services") ? QStringLiteral("Staff") : QStringLiteral("Services");
                    for (auto *button : sidebar->findChildren<QPushButton *>())
                        if (button->text() == deniedModule) { button->click(); break; }
                    auto *deniedOverlay = window.findChild<QWidget *>(QStringLiteral("accessRestrictedOverlay_%1").arg(deniedModule));
                    if (!deniedOverlay || !deniedOverlay->isVisible() || workspaceSearch->isEnabled()) { app.exit(45 + user); return; }
                    if (user == 0) window.grab().save(QDir::currentPath() + "/role_access_equipment_denied.png");
                    headerSignOut->trigger();
                    if (AppSession::active() || !email->isVisible()) { app.exit(49 + user); return; }
                }
                email->setText("administrator.test@skysense.com"); password->setText("test-only"); signIn->click();
                if(!AppSession::active()||AppSession::role()!=QStringLiteral("Responsable")
                    ||moduleStack->currentWidget()->objectName()!=QStringLiteral("moduleHost_Dashboard")){app.exit(51);return;}
                bool adminDashboardActive = false;
                for (auto *button : sidebar->findChildren<QPushButton *>()) {
                    if (button->property("nav").toBool() && button->property("active").toBool()
                        && button->text() == QStringLiteral("Dashboard")) adminDashboardActive = true;
                }
                if (!adminDashboardActive) { app.exit(57); return; }
                for(const QString &module:allModules){auto *overlay=window.findChild<QWidget *>(QStringLiteral("accessRestrictedOverlay_%1").arg(module));if(!overlay||!overlay->isHidden()){app.exit(52);return;}}
                app.quit();
                return;
            }
            if (verifyHeader) {
                auto *search = window.findChild<QLineEdit *>("workspaceSearch");
                auto *date = window.findChild<QDateEdit *>("workspaceReportDate");
                if (!search || !date || !search->isVisible()) { app.exit(6); return; }
                const QStringList moduleNames{"Stations", "Equipment", "Maintenance", "Staff", "Services"};
                const QStringList searchNames{"stationGlobalSearch", "globalEquipmentSearch", "maintenanceGlobalSearch", "staffSearch", "serviceGlobalSearch"};
                for (int index = 0; index < moduleNames.size(); ++index) {
                    for (auto *button : sidebar->findChildren<QPushButton *>())
                        if (button->text() == moduleNames[index]) { button->click(); break; }
                    const QString query = QString("header-test-%1").arg(index);
                    search->setText(query);
                    QLineEdit *target = window.findChild<QLineEdit *>(searchNames[index]);
                    if (moduleNames[index] == "Staff") {
                        auto *staff = window.findChild<QWidget *>("staffRoot");
                        auto *canvas = staff ? staff->property("staffCanvas").value<QWidget *>() : nullptr;
                        target = canvas ? canvas->findChild<QLineEdit *>("staffSearch") : nullptr;
                    }
                    if (!target || target->text() != query) { app.exit(7 + index); return; }
                    if (!date->isEnabled()) { app.exit(16 + index); return; }
                }
                const QDate chosen(2026, 10, 20);
                auto *dateArrow = window.findChild<QToolButton *>("workspaceDateArrow");
                auto *datePopup = window.findChild<QWidget *>("workspaceDateCalendarPopup");
                auto *dateCalendar = window.findChild<QCalendarWidget *>("workspaceDateCalendar");
                if (!dateArrow || !datePopup || !dateCalendar) { app.exit(30); return; }
                dateArrow->click();
                if (!datePopup->isVisible()) { app.exit(33); return; }
                if (!QMetaObject::invokeMethod(dateCalendar, "clicked", Qt::DirectConnection, Q_ARG(QDate, chosen))) { app.exit(34); return; }
                if (date->date() != chosen || datePopup->isVisible()) { app.exit(35); return; }
                for (const QString &name : {QStringLiteral("reportDatePicker"), QStringLiteral("serviceReportDatePicker")}) {
                    auto *target = window.findChild<QDateEdit *>(name);
                    if (!target || target->date() != chosen) { app.exit(20); return; }
                }
                const QStringList dateModuleNames{"Stations","Equipment","Maintenance","Staff","Services"};
                for(const QString &moduleName:dateModuleNames) {
                    auto *host=window.findChild<QWidget *>(QStringLiteral("moduleHost_%1").arg(moduleName));
                    auto *module=host?qobject_cast<QWidget *>(host->property("protectedModuleWidget").value<QObject *>()):nullptr;
                    if(!module || module->property("workspaceReportDate").toDate()!=chosen) {
                        qCritical().noquote() << "HEADER_DATE_TEST: date mismatch in" << moduleName
                                             << "expected" << chosen.toString(Qt::ISODate)
                                             << "actual" << (module ? module->property("workspaceReportDate").toDate().toString(Qt::ISODate) : QStringLiteral("missing module"));
                        app.exit(29 + dateModuleNames.indexOf(moduleName));return;
                    }
                }
                auto *profile = window.findChild<QLabel *>("workspaceProfileName");
                if (!profile || profile->text() != "Mohamed Haouari") { app.exit(21); return; }
                auto *profileButton = window.findChild<QToolButton *>("workspaceProfileMenuButton");
                auto *menu = window.findChild<QMenu *>("workspaceProfileMenu");
                auto *menuName = window.findChild<QLabel *>("workspaceProfileMenuName");
                auto *menuEmail = window.findChild<QLabel *>("workspaceProfileMenuEmail");
                auto *menuRole = window.findChild<QLabel *>("workspaceProfileMenuRole");
                auto *headerSignOut = window.findChild<QAction *>("workspaceHeaderSignOutAction");
                if (!profileButton || !menu || !menuName || !menuEmail || !menuRole || !headerSignOut) { app.exit(22); return; }
                if (profileButton->menu() != menu || profileButton->popupMode() != QToolButton::InstantPopup) { app.exit(23); return; }
                QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
                if (menuName->text() != "Mohamed Haouari") { app.exit(25); return; }
                if (menuEmail->text() != "mohamed.haouari@skysense.com") { app.exit(26); return; }
                if (menuRole->text() != "Responsable") { app.exit(27); return; }
                for (auto *button : sidebar->findChildren<QPushButton *>())
                    if (button->text() == "Staff") { button->click(); break; }
                auto *staffRoot = window.findChild<QWidget *>("staffRoot");
                auto *staffCanvas = staffRoot ? staffRoot->property("staffCanvas").value<QWidget *>() : nullptr;
                auto *staffSearch = staffCanvas ? staffCanvas->findChild<QLineEdit *>("staffSearch") : nullptr;
                auto *staffOperator = staffCanvas ? staffCanvas->findChild<QLabel *>("staffOperatorName") : nullptr;
                auto *staffTitle = staffCanvas ? staffCanvas->findChild<QLabel *>("staffPageTitle") : nullptr;
                if (!staffSearch || staffSearch->isVisible() || !staffOperator || staffOperator->isVisible()
                    || !staffTitle || !staffTitle->isVisible()) { app.exit(28); return; }
                headerSignOut->trigger();
                if (!email->isVisible() || !email->text().isEmpty() || !password->text().isEmpty() || AppSession::active()) {
                    app.exit(24); return;
                }
                app.quit();
                return;
            }
            if (captureGlow) {
                const QStringList animatedPages{"Stations", "Equipment", "Services"};
                int offset=200;
                for (const QString &name:animatedPages) {
                    QTimer::singleShot(offset,&window,[&window,sidebar,name] {
                        for (auto *button:sidebar->findChildren<QPushButton *>())
                            if (button->text()==name) { button->click(); break; }
                    });
                    QTimer::singleShot(offset+300,&window,[&window,name] {
                        window.grab().save(QDir::currentPath()+"/glow_"+name.toLower()+"_a.png");
                    });
                    QTimer::singleShot(offset+900,&window,[&window,name] {
                        window.grab().save(QDir::currentPath()+"/glow_"+name.toLower()+"_b.png");
                    });
                    offset+=1300;
                }
                QTimer::singleShot(offset+200,&window,&QApplication::quit);
                return;
            }
            const QStringList names{"Dashboard", "Stations", "Equipment", "Maintenance", "Staff", "Services"};
            int delay = 200;
            for (const QString &name : names) {
                QTimer::singleShot(delay, &window, [&window, sidebar, name, captureLaptop] {
                    for (auto *button : sidebar->findChildren<QPushButton *>()) {
                        if (button->text() == name) { button->click(); break; }
                    }
                    QTimer::singleShot(200, &window, [&window, name, captureLaptop] {
                        const QString path = QDir::currentPath() + (captureLaptop ? "/laptop_" : "/capture_") + name.toLower() + ".png";
                        window.grab().save(path);
                    });
                });
                delay += 550;
            }
            QTimer::singleShot(delay + 300, &app, &QApplication::quit);
        });
    }
    return app.exec();
}

