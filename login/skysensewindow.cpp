#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include <QAction>
#include <QCheckBox>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace {
enum class Symbol { Chart, Target, Shield, Mail, Lock, Eye, Enter };
void drawSymbol(QPainter &p, Symbol symbol, const QColor &color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
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
class BackgroundPanel final : public QWidget
{
    QPixmap image{":/assets/backgrounds/login_background.jpg"};
public:
    explicit BackgroundPanel(QWidget *parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this); p.setRenderHint(QPainter::SmoothPixmapTransform);
        QSizeF size = image.size(); size.scale(this->size(), Qt::KeepAspectRatioByExpanding);
        // Center the cover crop to retain the radar, mountains and lake.
        p.drawPixmap(QRectF((width() - size.width()) / 2, (height() - size.height()) / 2,
                           size.width(), size.height()), image, image.rect());
        QLinearGradient shade(0, 0, width(), height() * .7);
        shade.setColorAt(0, QColor(0, 15, 36, 130));
        shade.setColorAt(.65, QColor(0, 20, 45, 55));
        shade.setColorAt(1, QColor(0, 20, 45, 8));
        p.fillRect(rect(), shade);
    }
};
class Brand final : public QWidget
{
    QPixmap image{":/assets/logo/skysense_logo.png"};
public:
    explicit Brand(QWidget *parent = nullptr) : QWidget(parent)
    { setFixedHeight(130); setMaximumWidth(440); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this); p.setRenderHint(QPainter::SmoothPixmapTransform);
        QSizeF size = image.size(); size.scale(this->size(), Qt::KeepAspectRatio);
        p.drawPixmap(QRectF(0, (height() - size.height()) / 2, size.width(), size.height()), image, image.rect());
    }
};
class FeatureIcon final : public QWidget
{
    Symbol symbol;
public:
    explicit FeatureIcon(Symbol value) : symbol(value) { setFixedSize(72, 72); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor("#009fee"), 1.3)); p.setBrush(QColor(0, 44, 77, 100));
        p.drawRoundedRect(QRectF(1, 1, 70, 70), 12, 12);
        p.translate(16, 16); p.scale(40.0 / 24, 40.0 / 24);
        drawSymbol(p, symbol, QColor("#00bfff"));
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
        p.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const qreal y = height() / 2.0;
        QPainterPath tick; tick.moveTo(6, y); tick.lineTo(10, y + 4); tick.lineTo(17, y - 4); p.drawPath(tick);
    }
};
QLabel *label(const QString &text, const char *name)
{
    auto *widget = new QLabel(text); widget->setObjectName(name);
    widget->setWordWrap(true); widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); return widget;
}
QPushButton *link(const QString &text, const char *name)
{
    auto *button = new QPushButton(text); button->setObjectName(name);
    button->setProperty("link", true); button->setCursor(Qt::PointingHandCursor); return button;
}
void addFeature(QVBoxLayout *layout, Symbol symbol, const QString &title, const QString &subtitle)
{
    auto *row = new QHBoxLayout; row->setSpacing(26); row->addWidget(new FeatureIcon(symbol));
    auto *copy = new QVBoxLayout; copy->setSpacing(2);
    copy->addStretch(); copy->addWidget(label(title, "featureTitle"));
    copy->addWidget(label(subtitle, "featureSubtitle")); copy->addStretch();
    row->addLayout(copy, 1); layout->addLayout(row);
}
QLineEdit *input(const char *name, const QString &placeholder, Symbol symbol)
{
    auto *edit = new QLineEdit; edit->setObjectName(name); edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(54); edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    edit->addAction(icon(symbol, QColor("#486594")), QLineEdit::LeadingPosition);
    QPalette palette = edit->palette(); palette.setColor(QPalette::PlaceholderText, QColor("#94a3b8"));
    edit->setPalette(palette); return edit;
}
}
SkySenseWindow::SkySenseWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this); setMinimumSize(1100, 700);
    setWindowIcon(QIcon(":/assets/logo/skysense_logo.png"));
    setupLoginUi(); setupStyles(); updateResponsiveLayout();
}
SkySenseWindow::~SkySenseWindow() { delete ui; }
void SkySenseWindow::setupLoginUi()
{
    auto *root = new QHBoxLayout(ui->centralWidget); root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);
    auto *leftPanel = new BackgroundPanel; leftPanel->setObjectName("leftPanel");
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *left = new QVBoxLayout(leftPanel); leftLayout = left; left->setContentsMargins(70, 48, 42, 40); left->setSpacing(0);
    left->addWidget(new Brand); addResponsiveGap(left, 28);
    left->addWidget(label("Smarter Weather.", "headline"));
    left->addWidget(label("Safer Tomorrow.", "headlineCyan")); addResponsiveGap(left, 10);
    left->addWidget(label("Real-time data. Smarter insights.\nA more resilient world.", "supporting"));
    addResponsiveGap(left, 26);
    auto *features = new QVBoxLayout; features->setSpacing(16);
    addFeature(features, Symbol::Chart, "Real-Time Monitoring", "Live weather data from global networks");
    addFeature(features, Symbol::Target, "Data-Driven Decisions", "Turn insights into smarter actions.");
    addFeature(features, Symbol::Shield, "Safer Communities", "Build a more resilient tomorrow");
    left->addLayout(features); left->addStretch(1);
    auto *rightPanel = new QWidget; rightPanel->setObjectName("rightPanel");
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *right = new QHBoxLayout(rightPanel); rightLayout = right; right->setContentsMargins(36, 40, 36, 38);
    auto *form = new QWidget; form->setObjectName("loginForm"); form->setMaximumWidth(460); form->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *column = new QVBoxLayout(form); column->setContentsMargins(0, 0, 0, 0); column->setSpacing(0);
    column->addStretch(1); column->addWidget(new Brand); addResponsiveGap(column, 28);
    column->addWidget(label("Welcome Back", "welcomeTitle")); addResponsiveGap(column, 4);
    column->addWidget(label("Sign in to access your SkySense workspace.", "loginSubtitle")); addResponsiveGap(column, 36);
    auto *emailLabel = label("Email Address", "fieldLabel"); column->addWidget(emailLabel); addResponsiveGap(column, 8);
    auto *emailInput = input("emailInput", "name@company.com", Symbol::Mail); emailLabel->setBuddy(emailInput);
    emailInput->setAccessibleName("Email Address"); column->addWidget(emailInput); addResponsiveGap(column, 22);
    auto *passwordRow = new QHBoxLayout; auto *passwordLabel = label("Password", "fieldLabel");
    passwordRow->addWidget(passwordLabel); passwordRow->addStretch();
    passwordRow->addWidget(link("Forgot password?", "forgotPasswordLabel")); column->addLayout(passwordRow); addResponsiveGap(column, 8);
    auto *passwordInput = input("passwordInput", "Enter your password", Symbol::Lock);
    passwordInput->setEchoMode(QLineEdit::Password); passwordInput->setAccessibleName("Password"); passwordLabel->setBuddy(passwordInput);
    QAction *eye = passwordInput->addAction(icon(Symbol::Eye, QColor("#486594")), QLineEdit::TrailingPosition);
    eye->setText("Show password"); eye->setToolTip("Show password");
    connect(eye, &QAction::triggered, this, [passwordInput, eye] {
        const bool reveal = passwordInput->echoMode() == QLineEdit::Password;
        passwordInput->setEchoMode(reveal ? QLineEdit::Normal : QLineEdit::Password);
        eye->setText(reveal ? "Hide password" : "Show password"); eye->setToolTip(eye->text());
    });
    column->addWidget(passwordInput); addResponsiveGap(column, 30);
    auto *rememberCheckBox = new RememberBox; rememberCheckBox->setObjectName("rememberCheckBox");
    rememberCheckBox->setChecked(true); rememberCheckBox->setCursor(Qt::PointingHandCursor); column->addWidget(rememberCheckBox);
    auto *rememberNote = label("Keep me signed in on this device", "rememberNote");
    rememberNote->setContentsMargins(38, 0, 0, 0); column->addWidget(rememberNote); addResponsiveGap(column, 26);
    auto *signInButton = new QPushButton("  Sign In"); signInButton->setObjectName("signInButton");
    signInButton->setIcon(icon(Symbol::Enter, Qt::white)); signInButton->setIconSize(QSize(24, 24));
    signInButton->setFixedHeight(66); signInButton->setCursor(Qt::PointingHandCursor);
    auto *shadow = new QGraphicsDropShadowEffect(signInButton); shadow->setBlurRadius(24);
    shadow->setOffset(0, 6); shadow->setColor(QColor(0, 130, 230, 45)); signInButton->setGraphicsEffect(shadow);
    column->addWidget(signInButton); column->addStretch(3); addResponsiveGap(column, 24);
    auto *contact = label("New to SkySense? <span style='color:#007bff'>Contact your administrator.</span>", "contactLabel");
    contact->setAlignment(Qt::AlignCenter); column->addWidget(contact); addResponsiveGap(column, 28);
    auto *footer = label("SkySense v1.0.0   |   Smart Weather Management", "footer");
    footer->setAlignment(Qt::AlignCenter); column->addWidget(footer); column->addStretch(1);
    right->addStretch(); right->addWidget(form, 1); right->addStretch(); root->addWidget(leftPanel, 62); root->addWidget(rightPanel, 38);
    QWidget::setTabOrder(emailInput, passwordInput); QWidget::setTabOrder(passwordInput, rememberCheckBox);
    QWidget::setTabOrder(rememberCheckBox, signInButton);
}
void SkySenseWindow::setupStyles()
{
    QFile file(":/styles/skysense.qss");
    if (file.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(file.readAll()));
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
        if (auto *brand = dynamic_cast<Brand *>(widget)) brand->setFixedHeight(adapt(86, 130));
    }
    for (const char *name : {"headline", "headlineCyan"})
        findChild<QLabel *>(name)->setStyleSheet(QString("font-size: %1px;").arg(adapt(40, 52)));
    // Recompute height-for-width labels before Qt assigns the reduced space.
    for (auto *layout : findChildren<QLayout *>()) layout->invalidate();
}
