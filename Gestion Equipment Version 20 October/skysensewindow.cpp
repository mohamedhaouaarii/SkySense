#include "apptheme.h"
#include "skysensewindow.h"
#include "loginatmosphere.h"
#include "ui_skysensewindow.h"
#include "equipmentui.h"
#include "serviceui.h"
#include "stationui.h"
#include "staffui.h"
#include "maintenanceui.h"
#include "mainwindow.h"
#include "appsession.h"
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QLocale>
#include <QEvent>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QFrame>
#include <QRegularExpression>
#include <QLineEdit>
#include <QMouseEvent>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSoundEffect>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QToolButton>
#include <QMenu>
#include <QWidgetAction>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>
#include <QTextCharFormat>
#include <QTimer>
#include <QtMath>
#include <cmath>
#include <QDialog>
#include <QFormLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <algorithm>

namespace {
QString responsibleModule(const QString &role)
{
    const QString normalized = role.trimmed().toCaseFolded();
    if (normalized == QStringLiteral("responsable equipment")) return QStringLiteral("Equipment");
    if (normalized == QStringLiteral("responsable service")) return QStringLiteral("Services");
    if (normalized == QStringLiteral("responsable maintenance")) return QStringLiteral("Maintenance");
    if (normalized == QStringLiteral("responsable station")) return QStringLiteral("Stations");
    if (normalized == QStringLiteral("responsable staff")) return QStringLiteral("Staff");
    return {};
}

QString demoRoleForEmail(const QString &email)
{
    const QString user = email.trimmed().section(QLatin1Char('@'), 0, 0).toCaseFolded();
    if (user == QStringLiteral("equipment.test")) return QStringLiteral("Responsable Equipment");
    if (user == QStringLiteral("service.test")) return QStringLiteral("Responsable Service");
    if (user == QStringLiteral("maintenance.test")) return QStringLiteral("Responsable Maintenance");
    if (user == QStringLiteral("station.test")) return QStringLiteral("Responsable Station");
    if (user == QStringLiteral("staff.test")) return QStringLiteral("Responsable Staff");
    return QStringLiteral("Responsable");
}

bool roleCanOpenModule(const QString &role, const QString &module)
{
    // The unscoped Responsable account is the workspace administrator.
    if (role.trimmed().compare(QStringLiteral("Responsable"), Qt::CaseInsensitive) == 0)
        return true;
    return responsibleModule(role) == module;
}

class EyeSlashMark final : public QWidget
{
public:
    explicit EyeSlashMark(QWidget *parent = nullptr) : QWidget(parent)
    { setFixedSize(66, 66); setAccessibleName(QStringLiteral("Invisible, access restricted")); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        p.setPen(AppTheme::pen(p, QPen(QColor("#315784"), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
        p.setBrush(AppTheme::brush(p, Qt::NoBrush));
        QPainterPath eye; eye.moveTo(8, 33); eye.cubicTo(20, 15, 46, 15, 58, 33);
        eye.cubicTo(48, 49, 20, 50, 8, 33); p.drawPath(eye);
        p.drawEllipse(QPointF(33, 33), 7, 7);
        p.setPen(AppTheme::pen(p, QPen(QColor("#d64b5b"), 4.0, Qt::SolidLine, Qt::RoundCap)));
        p.drawLine(QPointF(13, 54), QPointF(54, 12));
    }
};

class AccessRestrictedOverlay final : public QWidget
{
public:
    explicit AccessRestrictedOverlay(const QString &module, QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(QStringLiteral("accessRestrictedOverlay_%1").arg(module));
        setAccessibleName(QStringLiteral("Restricted access to %1").arg(module));
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(QStringLiteral("background:rgba(239,245,253,224);"));
        auto *outer = new QVBoxLayout(this); outer->setContentsMargins(22, 22, 22, 22);
        outer->addStretch();
        auto *card = new QFrame(this); card->setObjectName(QStringLiteral("accessRestrictedCard"));
        card->setMaximumWidth(490); card->setMinimumWidth(310);
        card->setStyleSheet("QFrame#accessRestrictedCard{background:rgba(255,255,255,246);border:1px solid #dce7f5;border-radius:22px;}");
        auto *shadow = new QGraphicsDropShadowEffect(card); shadow->setBlurRadius(34); shadow->setOffset(0, 12); shadow->setColor(QColor(29, 65, 115, 42)); card->setGraphicsEffect(shadow);
        auto *content = new QVBoxLayout(card); content->setContentsMargins(32, 28, 32, 30); content->setSpacing(12);
        auto *iconCircle = new QFrame(card); iconCircle->setObjectName(QStringLiteral("restrictedEyeCircle"));
        iconCircle->setFixedSize(94, 94); iconCircle->setStyleSheet("QFrame#restrictedEyeCircle{background:#eef4fc;border:1px solid #e0eaf7;border-radius:47px;}");
        auto *iconLayout = new QVBoxLayout(iconCircle); iconLayout->setContentsMargins(0,0,0,0); iconLayout->addWidget(new EyeSlashMark(iconCircle), 0, Qt::AlignCenter);
        content->addWidget(iconCircle, 0, Qt::AlignHCenter);
        auto *eyebrow = new QLabel(QStringLiteral("ACCÈS À %1").arg(module.toUpper()), card); eyebrow->setAlignment(Qt::AlignCenter);
        eyebrow->setStyleSheet("color:#6680a8;font:700 10px 'Segoe UI';letter-spacing:1.6px;background:transparent;border:0;"); content->addWidget(eyebrow);
        auto *title = new QLabel(QStringLiteral("Interface masquée"), card); title->setAlignment(Qt::AlignCenter);
        title->setObjectName(QStringLiteral("restrictedModuleTitle")); title->setStyleSheet("color:#102448;font:700 24px 'Segoe UI';background:transparent;border:0;"); content->addWidget(title);
        auto *message = new QLabel(QStringLiteral("Votre rôle ne donne pas accès à cette gestion. Elle est réservée à son responsable."), card);
        message->setAlignment(Qt::AlignCenter); message->setWordWrap(true); message->setMinimumHeight(56); message->setStyleSheet("color:#647895;font:14px 'Segoe UI';background:transparent;border:0;"); content->addWidget(message);
        auto *roleNote = new QLabel(QStringLiteral("Connecté comme : %1").arg(AppSession::role()), card); roleNote->setObjectName(QStringLiteral("restrictedRoleNote"));
        roleNote->setAlignment(Qt::AlignCenter); roleNote->setWordWrap(true); roleNote->setMinimumHeight(38); roleNote->setStyleSheet("color:#1769c2;font:600 12px 'Segoe UI';background:#f3f8ff;border:1px solid #e4effc;border-radius:10px;padding:8px 10px;"); content->addWidget(roleNote);
        outer->addWidget(card, 0, Qt::AlignHCenter); outer->addStretch();
    }
};

QIcon workspaceHeaderIcon(const QString &name, const QColor &color = QColor("#17366f"))
{
    QPixmap pixels(48, 48);
    pixels.fill(Qt::transparent);
    QPainter painter(&pixels);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(2, 2);
    painter.setPen(AppTheme::pen(painter, QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
    painter.setBrush(AppTheme::brush(painter, Qt::NoBrush));
    if (name == "search") {
        painter.drawEllipse(QRectF(3, 3, 13, 13));
        painter.drawLine(15, 15, 22, 22);
    } else if (name == "calendar") {
        painter.drawRoundedRect(QRectF(3, 5, 18, 17), 1, 1);
        painter.drawLine(3, 10, 21, 10);
        painter.drawLine(7, 2, 7, 7); painter.drawLine(17, 2, 17, 7);
        painter.drawLine(9, 15, 14, 15); painter.drawLine(14, 15, 14, 19);
    } else if (name == "down") {
        painter.drawLine(7, 10, 12, 15); painter.drawLine(12, 15, 17, 10);
    } else if (name == "user") {
        painter.drawEllipse(QRectF(8, 2, 8, 8));
        QPainterPath path; path.moveTo(3, 22); path.cubicTo(3, 9, 21, 9, 21, 22);
        painter.drawPath(path);
    } else if (name == "logout") {
        painter.drawLine(13, 4, 21, 4); painter.drawLine(21, 4, 21, 20); painter.drawLine(21, 20, 13, 20);
        painter.drawLine(3, 12, 15, 12); painter.drawLine(10, 7, 15, 12); painter.drawLine(10, 17, 15, 12);
    }
    painter.end();
    pixels.setDevicePixelRatio(2);
    return QIcon(pixels);
}

enum class Symbol { Chart, Target, Shield, Mail, Lock, Eye, EyeSlash, Enter };
void drawSymbol(QPainter &p, Symbol symbol, const QColor &color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(AppTheme::pen(p, QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
    p.setBrush(AppTheme::brush(p, Qt::NoBrush));
    QPainterPath path;
    switch (symbol) {
    case Symbol::Chart:
        path.moveTo(3, 19); path.lineTo(9, 9); path.lineTo(14, 15); path.lineTo(21, 4);
        p.drawPath(path); p.drawLine(16, 4, 21, 4); p.drawLine(21, 4, 21, 9); break;
    case Symbol::Target:
        p.drawArc(QRectF(2, 2, 20, 20), -60 * 16, 300 * 16);
        p.drawArc(QRectF(6, 6, 12, 12), -60 * 16, 300 * 16);
        p.drawEllipse(QPointF(12, 12), 2, 2); break;
    case Symbol::Shield:
        path.moveTo(12, 2); path.cubicTo(8, 5, 5, 5, 3, 6); path.lineTo(4, 13);
        path.cubicTo(5, 18, 8, 20, 12, 22); path.cubicTo(16, 20, 19, 18, 20, 13);
        path.lineTo(21, 6); path.cubicTo(19, 5, 16, 5, 12, 2); p.drawPath(path); break;
    case Symbol::Mail:
        p.drawRoundedRect(QRectF(3, 5, 18, 14), 1.5, 1.5);
        p.drawLine(4, 6, 12, 12); p.drawLine(12, 12, 20, 6); break;
    case Symbol::Lock:
        p.drawRoundedRect(QRectF(5, 10, 14, 11), 2, 2);
        p.drawArc(QRectF(8, 2, 8, 13), 0, 180 * 16); p.drawLine(12, 14, 12, 17); break;
    case Symbol::Eye:
        path.moveTo(2, 12); path.cubicTo(7, 3, 17, 3, 22, 12);
        path.cubicTo(17, 21, 7, 21, 2, 12); p.drawPath(path);
        p.drawEllipse(QPointF(12, 12), 3, 3); break;
    case Symbol::EyeSlash:
        path.moveTo(2, 12); path.cubicTo(7, 3, 17, 3, 22, 12);
        path.cubicTo(17, 21, 7, 21, 2, 12); p.drawPath(path);
        p.drawEllipse(QPointF(12, 12), 3, 3); p.drawLine(3, 21, 21, 3); break;
    case Symbol::Enter:
        p.drawLine(14, 3, 21, 3); p.drawLine(21, 3, 21, 21); p.drawLine(21, 21, 14, 21);
        p.drawLine(2, 12, 16, 12); p.drawLine(11, 7, 16, 12); p.drawLine(11, 17, 16, 12); break;
    }
}
QIcon icon(Symbol symbol, const QColor &color)
{
    QPixmap pixmap(48, 48); pixmap.fill(Qt::transparent);
    QPainter p(&pixmap); p.scale(2, 2); drawSymbol(p, symbol, color); p.end();
    pixmap.setDevicePixelRatio(2);
    return QIcon(pixmap);
}
class Brand final : public QWidget
{
    QPixmap image{":/assets/logo/skysense_logo_login_hd.png"};
public:
    explicit Brand(QWidget *parent = nullptr) : QWidget(parent)
    { setFixedHeight(130); setMaximumWidth(440); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this); p.setRenderHint(QPainter::SmoothPixmapTransform);
        QRectF available = rect();
        QSizeF size = image.size(); size.scale(available.size(), Qt::KeepAspectRatio);
        p.drawPixmap(QRectF(available.x(), (height() - size.height()) / 2, size.width(), size.height()), image, image.rect());
    }
};
class RememberBox final : public QCheckBox
{
public:
    explicit RememberBox(QWidget *parent = nullptr) : QCheckBox("Remember me", parent) {}
protected:
    void paintEvent(QPaintEvent *event) override
    {
        QCheckBox::paintEvent(event);
        if (!isChecked()) return;
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        p.setPen(AppTheme::pen(p, QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
        const qreal y = height() / 2.0;
        QPainterPath tick; tick.moveTo(6, y); tick.lineTo(10, y + 4); tick.lineTo(17, y - 4); p.drawPath(tick);
    }
};
QLabel *label(const QString &text, const char *name)
{
    auto *widget = new QLabel(text); widget->setObjectName(name);
    widget->setWordWrap(true); widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); return widget;
}
QLineEdit *input(const char *name, const QString &placeholder, Symbol symbol)
{
    auto *edit = new QLineEdit; edit->setObjectName(name); edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(54); edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    edit->addAction(icon(symbol, QColor("#486594")), QLineEdit::LeadingPosition);
    QPalette palette = edit->palette(); palette.setColor(QPalette::PlaceholderText, QColor("#94a3b8"));
    edit->setPalette(palette); return edit;
}

class UiFeedbackFilter final : public QObject
{
public:
    UiFeedbackFilter(QWidget *scope, QObject *parent)
        : QObject(parent), scopeRoot(scope), clickSound(new QSoundEffect(this))
    {
        clickSound->setObjectName("uiClickSound");
        clickSound->setSource(QUrl("qrc:/sounds/button_click.wav"));
        clickSound->setLoopCount(1);
        clickSound->setVolume(0.42f);
        QObject::connect(clickSound, &QSoundEffect::statusChanged, this, [this] {
            if (pendingClick && clickSound->status() == QSoundEffect::Ready) {
                pendingClick = false;
                clickSound->play();
            }
        });
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        auto *control = qobject_cast<QWidget *>(watched);
        if (!control || (control != scopeRoot && !scopeRoot->isAncestorOf(control)))
            return QObject::eventFilter(watched, event);

        auto *button = qobject_cast<QAbstractButton *>(control);
        auto *combo = qobject_cast<QComboBox *>(control);
        if ((!button && !combo) || !control->isEnabled())
            return QObject::eventFilter(watched, event);

        if (event->type() == QEvent::Enter) {
            control->setCursor(Qt::PointingHandCursor);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton && control->underMouse())
                playClick();
        } else if (button && event->type() == QEvent::KeyPress) {
            const auto *key = static_cast<QKeyEvent *>(event);
            if (!key->isAutoRepeat() &&
                (key->key() == Qt::Key_Space || key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter))
                playClick();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void playClick()
    {
        clickSound->stop();
        if (clickSound->status() == QSoundEffect::Ready)
            clickSound->play();
        else if (clickSound->status() == QSoundEffect::Loading)
            pendingClick = true;
        else
            QApplication::beep();
    }

    QWidget *scopeRoot;
    QSoundEffect *clickSound;
    bool pendingClick = false;
};
}
SkySenseWindow::SkySenseWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this); setMinimumSize(1100, 700);
    qApp->installEventFilter(new UiFeedbackFilter(this, this));
    setWindowIcon(QIcon(":/assets/logo/skysense_logo_login_hd.png"));
    setupLoginUi(); setupStyles(loginPage); updateResponsiveLayout();
}
SkySenseWindow::~SkySenseWindow() { delete ui; }
void SkySenseWindow::setupLoginUi()
{
    auto *pages = new QStackedWidget(ui->centralWidget);
    pages->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    pages->setMinimumSize(0, 0);
    auto *hostLayout = new QVBoxLayout(ui->centralWidget); hostLayout->setContentsMargins(0, 0, 0, 0); hostLayout->addWidget(pages);
    loginPage = new LoginScene; auto *root = new QHBoxLayout(loginPage); root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);
    auto *leftPanel = new QWidget; leftPanel->setObjectName("leftPanel");
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *left = new QVBoxLayout(leftPanel); leftLayout = left; left->setContentsMargins(70, 48, 42, 40); left->setSpacing(0);
    left->addWidget(label("SKYSENSE   /   WEATHER OPERATIONS", "heroEyebrow"));
    addResponsiveGap(left, 30);
    left->addWidget(label("Take control of your", "headline"));
    left->addWidget(label("weather operations.", "headlineCyan"));
    addResponsiveGap(left, 15);
    left->addWidget(label("Plan work, track progress and coordinate your team.", "supporting"));
    addResponsiveGap(left, 26);
    auto *values = new QHBoxLayout; values->setSpacing(18);
    for (const QString &value : {QString("Clarity."), QString("Precision."), QString("Confidence.")}) {
        auto *item = new QFrame; item->setObjectName("valueTile");
        auto *row = new QHBoxLayout(item); row->setContentsMargins(15,12,15,12); row->setSpacing(10);
        auto *gem = new QLabel; gem->setObjectName("valueGem"); gem->setFixedSize(28,28);
        const Symbol glyph = value == "Clarity." ? Symbol::Eye : value == "Precision." ? Symbol::Target : Symbol::Shield;
        gem->setPixmap(icon(glyph,QColor("#9aeaff")).pixmap(28,28));
        row->addWidget(gem); row->addWidget(label(value,"valueText")); values->addWidget(item);
    }
    auto *sceneSpace = new QWidget(leftPanel); sceneSpace->setObjectName("sceneSpace");
    sceneSpace->setMinimumHeight(245); left->addWidget(sceneSpace, 3);
    addResponsiveGap(left, 14);
    left->addLayout(values);
    auto *rightPanel = new QWidget; rightPanel->setObjectName("rightPanel");
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *right = new QHBoxLayout(rightPanel); rightLayout = right; right->setContentsMargins(36, 40, 36, 38);
    auto *form = new QWidget; form->setObjectName("loginForm"); form->setMaximumWidth(460); form->setMaximumHeight(610); form->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto *formShadow=new QGraphicsDropShadowEffect(form); formShadow->setBlurRadius(65); formShadow->setOffset(0,24); formShadow->setColor(QColor(0,0,0,135)); form->setGraphicsEffect(formShadow);
    auto *column = new QVBoxLayout(form); column->setContentsMargins(34, 26, 34, 24); column->setSpacing(0);
    auto *accent = new QFrame(form); accent->setObjectName("loginAccent"); accent->setFixedSize(56, 4); auto *themeRow = new QHBoxLayout;
    themeRow->addWidget(accent); themeRow->addStretch();
    auto *loginTheme = new ThemeSwitch(form); loginTheme->setObjectName("loginThemeSwitch");
    themeRow->addWidget(loginTheme); column->addLayout(themeRow); addResponsiveGap(column, 12);
    column->addStretch(1); auto *formBrand = new Brand; formBrand->setObjectName("loginBrand"); formBrand->setMaximumWidth(360); column->addWidget(formBrand); addResponsiveGap(column, 8);
    auto *workspaceBadge=new QFrame; workspaceBadge->setObjectName("workspaceBadge"); workspaceBadge->setAccessibleName("SkySense operations workspace");
    auto *workspaceRow=new QHBoxLayout(workspaceBadge); workspaceRow->setContentsMargins(10,5,12,5); workspaceRow->setSpacing(7);
    auto *workspaceDot=new QLabel; workspaceDot->setObjectName("workspaceBadgeDot"); workspaceDot->setFixedSize(8,8); workspaceRow->addWidget(workspaceDot);
    auto *workspaceText=new QLabel("YOUR WORKSPACE"); workspaceText->setObjectName("workspaceBadgeText"); workspaceRow->addWidget(workspaceText);
    column->addWidget(workspaceBadge,0,Qt::AlignLeft); addResponsiveGap(column, 12);
    column->addWidget(label("Welcome back.", "welcomeTitle")); addResponsiveGap(column, 5);
    column->addWidget(label("Your SkySense workspace awaits.", "loginSubtitle")); addResponsiveGap(column, 22);
    auto *emailLabel = label("Email address", "fieldLabel"); column->addWidget(emailLabel); addResponsiveGap(column, 8);
    auto *emailInput = input("emailInput", "name@company.com", Symbol::Mail); emailLabel->setBuddy(emailInput);
    emailInput->setAccessibleName("Email address");
    emailInput->setInputMethodHints(Qt::ImhEmailCharactersOnly | Qt::ImhNoAutoUppercase);
    emailInput->setClearButtonEnabled(true);
    emailInput->setToolTip("Use the email associated with your SkySense profile.");
    column->addWidget(emailInput); addResponsiveGap(column, 18);
    auto *passwordRow = new QHBoxLayout; auto *passwordLabel = label("Password", "fieldLabel");
    passwordLabel->setText("Password");
    passwordRow->addWidget(passwordLabel); passwordRow->addStretch(); column->addLayout(passwordRow); addResponsiveGap(column, 8);
    auto *passwordInput = input("passwordInput", "Enter your password", Symbol::Lock);
    passwordInput->setEchoMode(QLineEdit::Password); passwordInput->setAccessibleName("Password"); passwordLabel->setBuddy(passwordInput);
    QAction *eye = passwordInput->addAction(icon(Symbol::Eye, QColor("#486594")), QLineEdit::TrailingPosition);
    eye->setText("Show password"); eye->setToolTip(eye->text());
    connect(eye, &QAction::triggered, this, [passwordInput, eye] {
        const bool reveal = passwordInput->echoMode() == QLineEdit::Password;
        passwordInput->setEchoMode(reveal ? QLineEdit::Normal : QLineEdit::Password);
        eye->setText(reveal ? "Hide password" : "Show password");
        eye->setToolTip(eye->text());
        eye->setIcon(icon(reveal ? Symbol::EyeSlash : Symbol::Eye, QColor("#486594")));
    });
    column->addWidget(passwordInput); addResponsiveGap(column, 12);
    auto *signInButton = new LoginAction("  Enter SkySense"); signInButton->setObjectName("signInButton");
    signInButton->setIcon(icon(Symbol::Enter, Qt::white)); signInButton->setIconSize(QSize(24, 24));
    signInButton->setFixedHeight(60); signInButton->setCursor(Qt::PointingHandCursor);
    signInButton->setDefault(true); signInButton->setAutoDefault(true);
    auto *shadow = new QGraphicsDropShadowEffect(signInButton); shadow->setBlurRadius(24);
    shadow->setOffset(0, 6); shadow->setColor(QColor(0, 130, 230, 45)); signInButton->setGraphicsEffect(shadow);
    column->addWidget(signInButton); column->addStretch(2); addResponsiveGap(column, 18);
    column->addStretch(1);
    right->addStretch(); right->addWidget(form, 1); right->addStretch(); root->addWidget(leftPanel, 60); root->addWidget(rightPanel, 40);
    QWidget::setTabOrder(emailInput, passwordInput);
    QWidget::setTabOrder(passwordInput, signInButton);
    auto *workspacePage = new QWidget;
    workspacePage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    auto *workspaceLayout = new QHBoxLayout(workspacePage);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(0);
    auto *modulePages = new QStackedWidget(workspacePage);
    modulePages->setObjectName(QStringLiteral("workspaceModulePages"));
    modulePages->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    modulePages->setMinimumSize(0, 0);
    QVector<QWidget *> restrictedOverlays;
    QVector<QGraphicsBlurEffect *> moduleBlurEffects;
    auto *equipmentPage = new QWidget;
    auto *stationsPage = new QWidget;
    auto *servicesPage = new QWidget;
    auto *maintenancePage = new QWidget;
    auto *staffPage = new QWidget;
    setupEquipmentUi(equipmentPage);
    setupStationUi(stationsPage);
    setupServiceContent(servicesPage);
    setupMaintenanceContent(maintenancePage);
    setupStaffUi(staffPage);
    const QStringList moduleNames{"Dashboard", "Stations", "Equipment", "Maintenance", "Staff", "Services"};
    QList<QLabel *> dashboardCountLabels;
    for (const QString &name : moduleNames) {
        QWidget *module = nullptr;
        if (name == "Stations") module = stationsPage;
        else if (name == "Equipment") module = equipmentPage;
        else if (name == "Maintenance") module = maintenancePage;
        else if (name == "Staff") module = staffPage;
        else if (name == "Services") module = servicesPage;
        else {
            auto *dashboard = new QWidget;
            dashboard->setObjectName("dashboardPage");
            dashboard->setStyleSheet("background:#f4f8ff;color:#14264d;font-family:'Segoe UI';");
            auto *page = new QVBoxLayout(dashboard); page->setContentsMargins(34,28,34,30); page->setSpacing(18);
            auto *heading = new QLabel("Operations overview"); heading->setStyleSheet("font-size:30px;font-weight:700;color:#111b3d;");
            auto *subheading = new QLabel("A preview of the SkySense workspace."); subheading->setStyleSheet("font-size:15px;color:#60749c;");
            page->addWidget(heading); page->addWidget(subheading);
            auto *cards = new QHBoxLayout; cards->setSpacing(16);
            const QStringList titles{"Stations","Equipment","Maintenance","Services"};
            const QStringList demoCounts{"6","8","5","5"};
            const QStringList values=demoCounts;
            const QStringList notes{"Stations","Equipment","Interventions","Services"};
            const QStringList tones{"#0a82ff","#00a66a","#8055ed","#ef9a18"};
            for (int i=0;i<titles.size();++i) {
                auto *card = new QFrame; card->setStyleSheet(QString("QFrame{background:white;border:1px solid #dbe7f8;border-radius:16px;border-top:4px solid %1;}").arg(tones[i]));
                auto *shadow = new QGraphicsDropShadowEffect(card); shadow->setBlurRadius(20); shadow->setOffset(0,5); shadow->setColor(QColor(24,63,120,30)); card->setGraphicsEffect(shadow);
                auto *inner = new QVBoxLayout(card); inner->setContentsMargins(18,18,18,18); inner->setSpacing(8);
                auto *title = new QLabel(titles[i]); title->setStyleSheet("font-size:15px;font-weight:600;color:#64779c;border:0;");
                auto *value = new QLabel(values[i]); value->setStyleSheet(QString("font-size:33px;font-weight:700;color:%1;border:0;").arg(tones[i]));
                dashboardCountLabels.append(value);
                auto *note = new QLabel(notes[i]); note->setStyleSheet("font-size:12px;color:#7889a7;border:0;");
                inner->addWidget(title); inner->addWidget(value); inner->addWidget(note); cards->addWidget(card,1);
            }
            page->addLayout(cards);
            auto *welcome = new QLabel("Welcome to SkySense"); welcome->setStyleSheet("font-size:22px;font-weight:700;color:#14264d;"); page->addWidget(welcome);
            auto *quick = new QLabel("Use the menu to preview stations, equipment, maintenance, staff, and services.");
            quick->setWordWrap(true); quick->setStyleSheet("font-size:14px;color:#64779c;background:white;border:1px solid #dbe7f8;border-radius:14px;padding:18px;"); page->addWidget(quick);
            auto *sampleNote = new QLabel("Demo data is available in each module. Changes remain available until the app closes."); sampleNote->setStyleSheet("font-size:12px;color:#8291aa;"); page->addWidget(sampleNote); page->addStretch();
            module = dashboard;
        }
        auto *moduleHost = new QWidget(modulePages);
        moduleHost->setObjectName(QStringLiteral("moduleHost_%1").arg(name));
        moduleHost->setProperty("protectedModuleWidget", QVariant::fromValue(static_cast<QObject *>(module)));
        auto *layer = new QStackedLayout(moduleHost);
        layer->setContentsMargins(0, 0, 0, 0);
        layer->setStackingMode(QStackedLayout::StackAll);
        // QWidget takes ownership of graphics effects; keep the effect parentless to avoid a QObject cycle.
        module->setGraphicsEffect(new QGraphicsBlurEffect);
        auto *blur = qobject_cast<QGraphicsBlurEffect *>(module->graphicsEffect());
        blur->setObjectName(QStringLiteral("moduleBlurEffect_%1").arg(name));
        blur->setBlurRadius(12.0);
        module->setProperty("roleAccessBlurEnabled", true);
        moduleBlurEffects.append(blur);
        layer->addWidget(module);
        auto *restricted = new AccessRestrictedOverlay(name, moduleHost);
        restrictedOverlays.append(restricted);
        layer->addWidget(restricted);
        restricted->raise();
        modulePages->addWidget(moduleHost);
    }
    modulePages->setCurrentIndex(moduleNames.indexOf("Dashboard"));
    auto *workspaceBody = new QWidget(workspacePage);
    auto *workspaceBodyLayout = new QVBoxLayout(workspaceBody);
    workspaceBodyLayout->setContentsMargins(0, 0, 0, 0);
    workspaceBodyLayout->setSpacing(0);
    auto *sharedHeader = new QFrame(workspaceBody);
    sharedHeader->setObjectName("workspaceHeader");
    sharedHeader->setFixedHeight(80);
    sharedHeader->setStyleSheet(
        "QFrame#workspaceHeader{background:#fcfdff;border:0;border-bottom:1px solid #e0ebf8;}"
        "QLineEdit#workspaceSearch,QDateEdit#workspaceReportDate{background:white;border:1px solid #d7e5f7;border-radius:8px;color:#17366f;font:14px 'Segoe UI';padding:0 12px;}"
        "QLineEdit#workspaceSearch:focus,QDateEdit#workspaceReportDate:focus{border-color:#0785ff;}"
        "QDateEdit#workspaceReportDate::drop-down{width:0;border:0;}"
        "QDateEdit#workspaceReportDate::down-arrow{image:none;}"
        "QToolButton#workspaceDateArrow{background:white;border:1px solid #d7e5f7;border-radius:8px;padding:0;}"
        "QToolButton#workspaceDateArrow:hover{background:#eef6ff;border-color:#9bcaff;}"
        "QFrame#workspaceDateCalendarPopup{background:white;border:1px solid #c9def7;border-radius:12px;}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar{background:#f5f9ff;border:0;}"
        "QCalendarWidget QToolButton{color:#17366f;font:600 13px 'Segoe UI';background:transparent;border:0;border-radius:6px;padding:5px;}"
        "QCalendarWidget QToolButton:hover{background:#eaf4ff;}"
        "QCalendarWidget QAbstractItemView:enabled{color:#17366f;background:white;selection-background-color:#0785ff;selection-color:white;}"
        "QCalendarWidget QAbstractItemView:disabled{color:#aab7ca;}"
        "QLabel#workspaceProfileName{color:#091b40;font:600 16px 'Segoe UI';background:transparent;}"
        "QLabel#workspaceProfileRole{color:#5d77ad;font:13px 'Segoe UI';background:transparent;}"
        "QToolButton#workspaceProfileMenuButton{background:transparent;border:0;border-radius:7px;padding:4px;}"
        "QToolButton#workspaceProfileMenuButton:hover{background:#edf4ff;}"
        "QLabel#workspaceAvatar{background:#edf2fb;border-radius:22px;}"
        "QFrame#workspaceDivider{background:#d7e2f1;border:0;}"
    );
    auto *headerLayout = new QHBoxLayout(sharedHeader);
    headerLayout->setContentsMargins(34, 12, 24, 12);
    headerLayout->setSpacing(16);
    auto *sharedSearch = new QLineEdit(sharedHeader);
    sharedSearch->setObjectName("workspaceSearch");
    sharedSearch->setFixedHeight(50);
    sharedSearch->setMinimumWidth(180);
    sharedSearch->setClearButtonEnabled(true);
    sharedSearch->addAction(workspaceHeaderIcon("search", QColor("#5d77ad")), QLineEdit::LeadingPosition);
    headerLayout->addWidget(sharedSearch, 1);

    auto *calendarLabel = new QLabel(sharedHeader);
    calendarLabel->setPixmap(workspaceHeaderIcon("calendar").pixmap(24, 24));
    calendarLabel->setFixedSize(24, 24);
    headerLayout->addWidget(calendarLabel);
    auto *sharedDate = new QDateEdit(QDate::currentDate(), sharedHeader);
    sharedDate->setObjectName("workspaceReportDate");
    sharedDate->setDisplayFormat("ddd, dd MMM yyyy");
    sharedDate->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    sharedDate->setCalendarPopup(false);
    sharedDate->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sharedDate->setReadOnly(true);
    sharedDate->setFixedSize(235, 38);
    sharedDate->setToolTip("Choose a date for the current workspace.");
    headerLayout->addWidget(sharedDate);
    auto *dateArrow = new QToolButton(sharedHeader);
    dateArrow->setObjectName("workspaceDateArrow");
    dateArrow->setIcon(workspaceHeaderIcon("down", QColor("#5d77ad")));
    dateArrow->setFixedSize(40, 38);
    dateArrow->setCursor(Qt::PointingHandCursor);
    dateArrow->setToolTip("Open calendar");
    dateArrow->setAccessibleName("Open date picker");
    headerLayout->addWidget(dateArrow);
    auto *datePopup = new QFrame(sharedHeader, Qt::Popup);
    datePopup->setObjectName("workspaceDateCalendarPopup");
    datePopup->setFixedSize(340, 330);
    auto *datePopupLayout = new QVBoxLayout(datePopup);
    datePopupLayout->setContentsMargins(12, 12, 12, 10);
    datePopupLayout->setSpacing(8);
    auto *dateCalendar = new QCalendarWidget(datePopup);
    dateCalendar->setObjectName("workspaceDateCalendar");
    dateCalendar->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    dateCalendar->setFirstDayOfWeek(Qt::Monday);
    dateCalendar->setDateRange(QDate(1900, 1, 1), QDate(2100, 12, 31));
    dateCalendar->setGridVisible(false);
    datePopupLayout->addWidget(dateCalendar, 1);
    auto *datePopupFooter = new QHBoxLayout;
    auto *datePopupToday = new QLabel("Choose a date", datePopup);
    datePopupToday->setStyleSheet("color:#7183a2;font:12px 'Segoe UI';background:transparent;border:0;");
    auto *todayButton = new QPushButton("Today", datePopup);
    todayButton->setObjectName("workspaceDateTodayButton");
    todayButton->setCursor(Qt::PointingHandCursor);
    todayButton->setStyleSheet("QPushButton{color:white;background:#087cff;border:0;border-radius:7px;padding:7px 14px;font:600 12px 'Segoe UI';} QPushButton:hover{background:#006be0;}");
    datePopupFooter->addWidget(datePopupToday);
    datePopupFooter->addStretch();
    datePopupFooter->addWidget(todayButton);
    datePopupLayout->addLayout(datePopupFooter);
    connect(dateCalendar, &QCalendarWidget::clicked, datePopup, [sharedDate, datePopup](const QDate &date) {
        sharedDate->setDate(date);
        datePopup->hide();
    });
    connect(todayButton, &QPushButton::clicked, datePopup, [sharedDate, datePopup] {
        sharedDate->setDate(QDate::currentDate());
        datePopup->hide();
    });
    auto *divider = new QFrame(sharedHeader);
    divider->setObjectName("workspaceDivider"); divider->setFixedSize(1, 35);
    headerLayout->addWidget(divider);
    auto *avatar = new QLabel(sharedHeader);
    avatar->setObjectName("workspaceAvatar");
    avatar->setFixedSize(44, 44);
    avatar->setPixmap(workspaceHeaderIcon("user", QColor("#5d77ad")).pixmap(28, 28));
    avatar->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(avatar);
    auto *profileLayout = new QVBoxLayout;
    profileLayout->setSpacing(1);
    auto *profileName = new QLabel("Alex Martin", sharedHeader);
    profileName->setObjectName("workspaceProfileName");
    auto *profileRole = new QLabel("Operations Manager", sharedHeader);
    profileRole->setObjectName("workspaceProfileRole");
    profileLayout->addWidget(profileName);
    profileLayout->addWidget(profileRole);
    headerLayout->addLayout(profileLayout);
    auto *profileMenuButton = new QToolButton(sharedHeader);
    profileMenuButton->setObjectName("workspaceProfileMenuButton");
    profileMenuButton->setIcon(workspaceHeaderIcon("down", QColor("#091b40")));
    profileMenuButton->setFixedSize(30, 34);
    profileMenuButton->setCursor(Qt::PointingHandCursor);
    profileMenuButton->setAccessibleName("Open account menu");
    profileMenuButton->setToolTip("Account options");
    headerLayout->addWidget(profileMenuButton);

    auto *profileMenu = new QMenu(profileMenuButton);
    profileMenu->setObjectName("workspaceProfileMenu");
    profileMenu->setStyleSheet(
        "QMenu{background:#ffffff;border:1px solid #dbe6f5;border-radius:12px;padding:8px;}"
        "QMenu::item{color:#b42332;padding:10px 16px;border-radius:7px;font:600 13px 'Segoe UI';}"
        "QMenu::item:selected{background:#fff1f2;color:#9f1d2b;}"
        "QMenu::separator{height:1px;background:#e7edf6;margin:5px 8px;}"
    );
    auto *profileCard = new QFrame(profileMenu);
    profileCard->setObjectName("profileMenuCard");
    profileCard->setStyleSheet("QFrame#profileMenuCard{background:#f6f9fe;border:1px solid #e6edf7;border-radius:8px;}");
    auto *profileCardLayout = new QVBoxLayout(profileCard);
    profileCardLayout->setContentsMargins(12, 10, 12, 10);
    profileCardLayout->setSpacing(3);
    auto *menuProfileName = new QLabel(profileCard);
    menuProfileName->setObjectName("workspaceProfileMenuName");
    menuProfileName->setStyleSheet("color:#102448;font:600 14px 'Segoe UI';background:transparent;border:0;");
    auto *menuProfileEmail = new QLabel(profileCard);
    menuProfileEmail->setObjectName("workspaceProfileMenuEmail");
    menuProfileEmail->setStyleSheet("color:#667895;font:12px 'Segoe UI';background:transparent;border:0;");
    menuProfileEmail->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto *menuProfileRole = new QLabel(profileCard);
    menuProfileRole->setObjectName("workspaceProfileMenuRole");
    menuProfileRole->setStyleSheet("color:#1769c2;font:600 11px 'Segoe UI';background:transparent;border:0;");
    profileCardLayout->addWidget(menuProfileName);
    profileCardLayout->addWidget(menuProfileEmail);
    profileCardLayout->addWidget(menuProfileRole);
    auto *profileCardAction = new QWidgetAction(profileMenu);
    profileCardAction->setObjectName("workspaceProfileCardAction");
    profileCardAction->setDefaultWidget(profileCard);
    profileMenu->addAction(profileCardAction);
    profileMenu->addSeparator();
    auto *headerSignOut = profileMenu->addAction("Sign out");
    headerSignOut->setObjectName("workspaceHeaderSignOutAction");
    headerSignOut->setIcon(workspaceHeaderIcon("logout", QColor("#b42332")));
    profileMenu->insertSeparator(headerSignOut);
    connect(profileMenu, &QMenu::aboutToShow, this, [menuProfileName, menuProfileEmail, menuProfileRole, profileName, profileRole] {
        menuProfileName->setText(profileName->text());
        menuProfileEmail->setText(AppSession::email());
        menuProfileRole->setText(profileRole->text());
    });
    profileMenuButton->setMenu(profileMenu);
    profileMenuButton->setPopupMode(QToolButton::InstantPopup);
    workspaceBodyLayout->addWidget(sharedHeader);
    workspaceBodyLayout->addWidget(modulePages, 1);
    const QVector<QLineEdit *> moduleSearches{
        nullptr,
        stationsPage->findChild<QLineEdit *>("stationGlobalSearch"),
        equipmentPage->findChild<QLineEdit *>("globalEquipmentSearch"),
        maintenancePage->findChild<QLineEdit *>("maintenanceGlobalSearch"),
        staffPage->property("staffCanvas").value<QWidget *>()->findChild<QLineEdit *>("staffSearch"),
        servicesPage->findChild<QLineEdit *>("serviceGlobalSearch")
    };
    const QStringList searchPrompts{
        "Search stations, equipment, maintenance, staff or services...",
        "Search stations by ID, name, city or region...",
        "Search equipment by ID, type, model or serial number...",
        "Search maintenance by equipment, station or technician...",
        "Search staff by name, function, status or email...",
        "Search services by ID, title, type, station or assignee..."
    };
    auto syncHeader = [modulePages, moduleSearches, searchPrompts, sharedSearch] {
        const int index = modulePages->currentIndex();
        QSignalBlocker blocked(sharedSearch);
        sharedSearch->setPlaceholderText(searchPrompts.value(index));
        sharedSearch->setText(index >= 0 && index < moduleSearches.size() && moduleSearches[index]
                              ? moduleSearches[index]->text() : QString());
    };
    connect(modulePages, &QStackedWidget::currentChanged, this, [syncHeader](int) { syncHeader(); });
    connect(sharedSearch, &QLineEdit::textChanged, this, [modulePages, moduleSearches](const QString &value) {
        const int index = modulePages->currentIndex();
        if (index >= 0 && index < moduleSearches.size() && moduleSearches[index])
            moduleSearches[index]->setText(value);
    });
    connect(sharedSearch, &QLineEdit::returnPressed, this, [modulePages, moduleNames, sharedSearch] {
        if (modulePages->currentIndex() != 0) return;
        const QString query = sharedSearch->text().trimmed();
        for (int index = 1; index < moduleNames.size(); ++index)
            if (moduleNames[index].contains(query, Qt::CaseInsensitive) && !query.isEmpty()) {
                modulePages->setCurrentIndex(index);
                return;
            }
    });
    for (auto *moduleSearch : moduleSearches)
        if (moduleSearch) connect(moduleSearch, &QLineEdit::textChanged, this, [syncHeader](const QString &) { syncHeader(); });
    const QList<QWidget *> dateAwarePages{stationsPage,equipmentPage,maintenancePage,staffPage,servicesPage};
    for (QWidget *page : dateAwarePages) page->setProperty("workspaceReportDate", sharedDate->date());
    connect(sharedDate, &QDateEdit::dateChanged, this, [equipmentPage, servicesPage, dateAwarePages](const QDate &date) {
        for (QWidget *page : dateAwarePages) page->setProperty("workspaceReportDate", date);
        if (auto *equipmentDate = equipmentPage->findChild<QDateEdit *>("reportDatePicker")) equipmentDate->setDate(date);
        if (auto *servicesDate = servicesPage->findChild<QDateEdit *>("serviceReportDatePicker")) servicesDate->setDate(date);
    });
    connect(dateArrow, &QToolButton::clicked, this, [sharedDate, dateArrow, datePopup, dateCalendar] {
        const QDate selected = sharedDate->date();
        dateCalendar->setSelectedDate(selected);
        dateCalendar->setCurrentPage(selected.year(), selected.month());
        const QRect available = dateArrow->screen()->availableGeometry();
        QPoint position = dateArrow->mapToGlobal(QPoint(0, dateArrow->height() + 6));
        if (position.y() + datePopup->height() > available.bottom())
            position.setY(dateArrow->mapToGlobal(QPoint(0, -datePopup->height() - 6)).y());
        position.setX(qBound(available.left(), position.x(), qMax(available.left(), available.right() - datePopup->width())));
        datePopup->move(position);
        datePopup->show();
        dateCalendar->setFocus();
    });
    syncHeader();
    auto applyRoleAccess = [modulePages, moduleNames, restrictedOverlays, moduleBlurEffects,
                            sharedSearch, sharedDate, dateArrow, syncHeader](const QString &role) {
        int landingIndex = moduleNames.indexOf(responsibleModule(role));
        const bool workspaceAdmin = role.trimmed().compare(QStringLiteral("Responsable"), Qt::CaseInsensitive) == 0;
        if (workspaceAdmin) landingIndex = moduleNames.indexOf(QStringLiteral("Dashboard"));
        for (int index = 0; index < moduleNames.size(); ++index) {
            const bool allowed = roleCanOpenModule(role, moduleNames.at(index));
            restrictedOverlays.value(index)->setVisible(!allowed);
            moduleBlurEffects.value(index)->setEnabled(!allowed);
            if (auto *module = qobject_cast<QWidget *>(modulePages->widget(index)->property("protectedModuleWidget").value<QObject *>()))
            {
                module->setProperty("roleAccessBlurEnabled", !allowed);
                module->setEnabled(allowed);
            }
            if (auto *roleLabel = restrictedOverlays.value(index)->findChild<QLabel *>("restrictedRoleNote"))
                roleLabel->setText(QStringLiteral("Connecté comme : %1").arg(role));
        }
        if (landingIndex < 0) landingIndex = moduleNames.indexOf(QStringLiteral("Dashboard"));
        modulePages->setCurrentIndex(landingIndex);
        const bool currentPageAllowed = roleCanOpenModule(role, moduleNames.at(landingIndex));
        sharedSearch->setEnabled(currentPageAllowed);
        if (!currentPageAllowed) sharedSearch->clear();
        sharedDate->setEnabled(currentPageAllowed);
        dateArrow->setEnabled(currentPageAllowed);
        syncHeader();
    };
    connect(modulePages, &QStackedWidget::currentChanged, this,
            [moduleNames, modulePages, sharedSearch, sharedDate, dateArrow, dashboardCountLabels](int index) {
        if(index==0) {
            
                auto *stations=modulePages->widget(moduleNames.indexOf("Stations"));
                auto *equipment=modulePages->widget(moduleNames.indexOf("Equipment"));
                auto *maintenance=modulePages->widget(moduleNames.indexOf("Maintenance"));
                auto *services=modulePages->widget(moduleNames.indexOf("Services"));
                if(auto *label=stations->findChild<QLabel *>("totalValue")) dashboardCountLabels[0]->setText(label->text());
                if(auto *label=equipment->findChild<QLabel *>("metricValue")) dashboardCountLabels[1]->setText(label->text());
                if(auto *label=maintenance->findChild<QLabel *>("maintenanceResultCount")) dashboardCountLabels[2]->setText(label->text().section(' ',2,2));
                for(auto *label:services->findChildren<QLabel *>()) if(label->property("role").toString()=="metricCount") {dashboardCountLabels[3]->setText(label->text());break;}
            
        }
        if (!AppSession::active() || index < 0 || index >= moduleNames.size()) return;
        const QString role = AppSession::role();
        const bool allowed = roleCanOpenModule(role, moduleNames.at(index));
        sharedSearch->setEnabled(allowed);
        if (!allowed) sharedSearch->clear();
        sharedDate->setEnabled(allowed);
        dateArrow->setEnabled(allowed);
    });
    auto *workspaceSidebar = createSkySenseSidebar([modulePages, moduleNames, maintenancePage](const QString &name) {
        const int index = moduleNames.indexOf(name);
        if (index >= 0) modulePages->setCurrentIndex(index);
        if (name == "Maintenance") {
            if (auto *maintenance = maintenancePage->findChild<MainWindow *>())
                maintenance->activateEmbeddedPage();
        }
    }, "Dashboard");
    workspaceSidebar->setObjectName("servicesSidebar");
    workspaceLayout->addWidget(workspaceSidebar);
    auto syncSidebarSelection = [workspaceSidebar, modulePages, moduleNames] {
        const QString activeSection = moduleNames.value(modulePages->currentIndex());
        for (auto *button : workspaceSidebar->findChildren<QPushButton *>()) {
            if (!button->property("nav").toBool()) continue;
            const bool active = button->text() == activeSection;
            if (button->property("active").toBool() == active) continue;
            button->setProperty("active", active);
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
        }
    };
    connect(modulePages, &QStackedWidget::currentChanged, this,
            [syncSidebarSelection](int) { syncSidebarSelection(); });
    syncSidebarSelection();
    workspaceLayout->addWidget(workspaceBody, 1);
    pages->addWidget(loginPage);
    pages->addWidget(workspacePage);
    pages->setCurrentWidget(loginPage);
    auto performSignOut = [this, pages, emailInput, passwordInput]() {
        AppSession::clear(); passwordInput->clear(); emailInput->clear();
        pages->setCurrentWidget(loginPage);
    };
    connect(headerSignOut, &QAction::triggered, this, [this, profileMenu, performSignOut]() {
        profileMenu->close();
#ifdef SKYSENSE_UI_TESTS
        const auto arguments = QApplication::arguments();
        if (arguments.contains("--verify-header") || arguments.contains("--verify-access") ||
            arguments.contains("--capture-pages") || arguments.contains("--capture-laptop") ||
            arguments.contains("--capture-glow")) {
            performSignOut();
            return;
        }
#endif
        QMessageBox confirmation(QMessageBox::Question, "Sign out",
                                 "Sign out of the current SkySense session?",
                                 QMessageBox::Yes | QMessageBox::No, this);
        confirmation.setObjectName("workspaceSignOutConfirmation");
        confirmation.setInformativeText("You will return to the sign-in screen.");
        confirmation.setDefaultButton(QMessageBox::No);
        confirmation.setEscapeButton(QMessageBox::No);
        if (confirmation.exec() == QMessageBox::Yes) performSignOut();
    });
    connect(signInButton, &QPushButton::clicked, this, [this, pages, workspacePage, emailInput, passwordInput, applyRoleAccess]() {
        static const QRegularExpression emailPattern(QStringLiteral(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)"));
        const QString loginId = emailInput->text().trimmed();
        const bool initialAdmin = loginId.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0;
        if((!emailPattern.match(loginId).hasMatch() && !initialAdmin) || passwordInput->text().isEmpty()) {
            QMessageBox::warning(this, "Check your sign-in details", "Enter a valid work email and password to continue.");
            return;
        }
        const QString normalizedEmail = emailInput->text().trimmed();
        const QString role = demoRoleForEmail(normalizedEmail);
        passwordInput->clear();
        AppSession::start(normalizedEmail, AppSession::nameFromEmail(normalizedEmail), role);
        if (auto *profileName = workspacePage->findChild<QLabel *>("workspaceProfileName"))
            profileName->setText(AppSession::displayName());
        if (auto *profileRole = workspacePage->findChild<QLabel *>("workspaceProfileRole"))
            profileRole->setText(AppSession::role());
        applyRoleAccess(AppSession::role());
        for (auto *nameLabel : workspacePage->findChildren<QLabel *>()) {
            if (!nameLabel->property("sessionNameTemplate").isValid() && nameLabel->text().contains("Alex Martin"))
                nameLabel->setProperty("sessionNameTemplate", nameLabel->text());
            const QString nameTemplate = nameLabel->property("sessionNameTemplate").toString();
            if (!nameTemplate.isEmpty()) {
                QString value = nameTemplate;
                value.replace("Alex Martin", AppSession::displayName());
                nameLabel->setText(value);
            }
        }
        if (auto *staffRoot = workspacePage->findChild<QWidget *>("staffRoot")) {
            if (auto *canvas = staffRoot->property("staffCanvas").value<QWidget *>()) {
                if (auto *staffName = canvas->findChild<QLabel *>("staffOperatorName"))
                    staffName->setText(AppSession::displayName());
            }
        }
        pages->setCurrentWidget(workspacePage);
    });
}
void SkySenseWindow::setupStyles(QWidget *target)
{
    QFile file(":/styles/skysense.qss");
    if (target && file.open(QIODevice::ReadOnly)) target->setStyleSheet(QString::fromUtf8(file.readAll()));
}

void SkySenseWindow::addResponsiveGap(QVBoxLayout *layout, int height)
{
    auto *gap = new QSpacerItem(0, height, QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addSpacerItem(gap);
    responsiveGaps.append(qMakePair(gap, height));
}
void SkySenseWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (leftLayout) updateResponsiveLayout();
}
void SkySenseWindow::updateResponsiveLayout()
{
    const qreal room = qBound(0.0, qMin((width() - 1100) / 546.0, (height() - 700) / 232.0), 1.0);
    const auto adapt = [room](int small, int large) { return qRound(small + (large - small) * room); };
    leftLayout->setContentsMargins(adapt(32, 70), adapt(22, 48), adapt(28, 42), adapt(20, 40));
    rightLayout->setContentsMargins(adapt(20, 36), adapt(18, 40), adapt(20, 36), adapt(18, 38));
    for (const auto &gap : responsiveGaps)
        gap.first->changeSize(0, qRound(gap.second * (.5 + .5 * room)), QSizePolicy::Minimum, QSizePolicy::Fixed);
    for (auto *widget : findChildren<QWidget *>()) {
        if (auto *brand = dynamic_cast<Brand *>(widget))
            brand->setFixedHeight(brand->objectName() == "loginBrand" ? adapt(58, 76) : adapt(86, 130));
    }
    for (const char *name : {"headline", "headlineCyan"})
        findChild<QLabel *>(name)->setStyleSheet(QString("font-size: %1px;").arg(adapt(34, 49)));
    if (auto *scene = findChild<QWidget *>("sceneSpace")) scene->setMinimumHeight(adapt(245, 295));
    // Recompute height-for-width labels before Qt assigns the reduced space.
    for (auto *layout : findChildren<QLayout *>()) layout->invalidate();
}
