#include "mainwindow.h"
#include "connection.h"
#include <QtWidgets>
#include <QPrinter>
#include <QTextDocument>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QFile>
#include <algorithm>
#ifdef SKYSENSE_HAS_MULTIMEDIA
#include <QSoundEffect>
#endif
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

// =====================================================================
//  1. UI HELPERS (badges, avatars, logo, charts)
// =====================================================================
namespace {

QPair<QString, QString> prioColor(const QString &p) {
    if (p == "High")   return {"#fde2e2", "#d92d2d"};
    if (p == "Medium") return {"#fdebd0", "#e08a00"};
    return {"#dcf5e3", "#1f9d55"};
}
QPair<QString, QString> statColor(const QString &s) {
    if (s == "Overdue")     return {"#fde2e2", "#d92d2d"};
    if (s == "In Progress") return {"#fdebd0", "#e08a00"};
    if (s == "Completed")   return {"#dcf5e3", "#1f9d55"};
    return {"#dbe8ff", "#2f62d9"};
}
int prioW(const QString &p) { return p == "High" ? 3 : (p == "Medium" ? 2 : 1); }
int statRank(const QString &s) { return s == "Overdue" ? 0 : (s == "In Progress" ? 1 : (s == "Planned" ? 2 : 3)); }
QString healthText(int h) { return h < 0 ? "Unknown" : (h >= 70 ? "Good" : (h >= 40 ? "Degraded" : "Critical")); }

QLocale englishLocale() { return QLocale(QLocale::English, QLocale::UnitedStates); }

QByteArray clickWaveBuffer;

bool playNativeClickSound() {
#ifdef Q_OS_WIN
    if (!clickWaveBuffer.isEmpty()) {
        return PlaySoundW(reinterpret_cast<LPCWSTR>(clickWaveBuffer.constData()), nullptr,
                          SND_ASYNC | SND_MEMORY | SND_NODEFAULT);
    }
#endif
    return false;
}

void addCardShadow(QFrame *card) {
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(15, 42, 92, 20));
    card->setGraphicsEffect(shadow);
}

void prepareFadeIn(QWidget *widget) {
    auto *effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);
}

void showMessage(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text) {
    QMessageBox box(icon, title, text, QMessageBox::NoButton, parent);
    box.addButton("Close", QMessageBox::AcceptRole);
    box.exec();
}

bool askChoice(QWidget *parent, const QString &title, const QString &text, const QString &action) {
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    QPushButton *confirm = box.addButton(action, QMessageBox::AcceptRole);
    box.addButton("Cancel", QMessageBox::RejectRole);
    box.exec();
    return box.clickedButton() == confirm;
}

QLabel *badge(const QString &t, QPair<QString, QString> c) {
    auto *l = new QLabel(t);
    l->setAlignment(Qt::AlignCenter);
    l->setFixedSize(84, 24);
    l->setStyleSheet(QString("background:%1;color:%2;border-radius:12px;font-weight:600;font-size:11px;")
                         .arg(c.first, c.second));
    return l;
}
QWidget *center(QWidget *w) {
    auto *c = new QWidget; auto *l = new QHBoxLayout(c);
    l->setContentsMargins(6, 0, 6, 0); l->addWidget(w, 0, Qt::AlignCenter); return c;
}
QWidget *twoLines(const QString &a, const QString &b, bool bold) {
    auto *w = new QWidget; auto *l = new QVBoxLayout(w);
    l->setContentsMargins(6, 4, 6, 4); l->setSpacing(0);
    auto *x = new QLabel(a); auto *y = new QLabel(b);
    y->setStyleSheet("color:#68758b;font-size:12px;");
    if (bold) x->setStyleSheet("font-weight:600;");
    l->addWidget(x); l->addWidget(y); return w;
}
QPixmap avatar(const QString &name, int s) {
    QPixmap pm(s, s); pm.fill(Qt::transparent);
    QPainter p(&pm); p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor::fromHsv(int(qHash(name) % 360), 120, 200)); p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, s, s); p.setPen(Qt::white);
    QFont f = p.font(); f.setBold(true); f.setPixelSize(qMax(8, s / 3)); p.setFont(f);
    QString ini; for (const QString &w : name.split(' ', Qt::SkipEmptyParts)) ini += w.at(0).toUpper();
    p.drawText(pm.rect(), Qt::AlignCenter, ini.left(2));
    return pm;
}
// Subtle light background for the workspace.
class BgWidget : public QWidget {
public:
    explicit BgWidget(QWidget *p = nullptr) : QWidget(p) {}
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0, QColor("#f8faff"));
        gradient.setColorAt(1, QColor("#eef3fa"));
        p.fillRect(rect(), gradient);
    }
};

// Scenic sidebar inspired by the supplied SkySense navigation reference.
class SideWidget : public QFrame {
public:
    explicit SideWidget(QWidget *p = nullptr) : QFrame(p), mt(":/images/weather_landscape.png") {}
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::SmoothPixmapTransform);
        if (!mt.isNull()) {
            QPixmap scaled = mt.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            const QRect source((scaled.width() - width()) / 2, (scaled.height() - height()) / 2, width(), height());
            p.drawPixmap(rect(), scaled, source);
        } else {
            p.fillRect(rect(), QColor("#071a31"));
        }
        QLinearGradient shade(0, 0, 0, height());
        shade.setColorAt(0.0, QColor(5, 20, 39, 238));
        shade.setColorAt(0.42, QColor(5, 20, 39, 205));
        shade.setColorAt(0.72, QColor(5, 20, 39, 120));
        shade.setColorAt(1.0, QColor(5, 20, 39, 78));
        p.fillRect(rect(), shade);
    }
private:
    QPixmap mt;
};

// Custom-drawn bar chart (B4); no Qt Charts dependency required.
class ChartWidget : public QWidget {
public:
    ChartWidget(const QString &t, const QVector<QPair<QString, double>> &d, const QVector<QColor> &c,
                const QString &u, QWidget *p = nullptr) : QWidget(p), title(t), d(d), col(c), unit(u) {
        setMinimumSize(400, 280);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(), Qt::white);
        QFont f = font(); f.setBold(true); f.setPointSize(11); p.setFont(f);
        p.setPen(QColor("#0b1437")); p.drawText(14, 26, title);
        f.setBold(false); f.setPointSize(9); p.setFont(f);
        double mx = 1; for (const auto &x : d) mx = qMax(mx, x.second);
        int n = qMax(1, int(d.size())), top = 60, bottom = 34, left = 16;
        int w = width() - 2 * left, h = height() - top - bottom, slot = w / n;
        for (int i = 0; i < d.size(); ++i) {
            int bh = int(h * d[i].second / mx);
            QRect r(left + i * slot + slot / 6, top + h - bh, slot * 2 / 3, bh);
            p.setPen(Qt::NoPen); p.setBrush(col[i % col.size()]); p.drawRoundedRect(r, 6, 6);
            p.setPen(QColor("#374151"));
            p.drawText(QRect(left + i * slot, top + h - bh - 20, slot, 18), Qt::AlignCenter,
                       QString::number(d[i].second, 'f', unit.isEmpty() ? 0 : 1) + unit);
            p.drawText(QRect(left + i * slot, top + h + 6, slot, 20), Qt::AlignCenter, d[i].first);
        }
    }
private:
    QString title; QVector<QPair<QString, double>> d; QVector<QColor> col; QString unit;
};

// =====================================================================
//  2. ADD / EDIT FORM (includes business rules)
// =====================================================================
class InterventionDialog : public QDialog {
public:
    InterventionDialog(const QVector<Equipment> &eq, const QVector<Technician> &techs,
                       const Intervention *it, QWidget *parent) : QDialog(parent), eqs(eq) {
        setObjectName("interventionDialog");
        setAttribute(Qt::WA_StyledBackground, true);
        setWindowTitle(it ? "Edit intervention" : "Add intervention");
        setMinimumWidth(560);
        auto *f = new QFormLayout(this);
        f->setHorizontalSpacing(16);
        f->setVerticalSpacing(10);
        f->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        cEq = new QComboBox; for (const auto &e : eq) cEq->addItem(e.name + " — " + e.station, e.id);
        lSt = new QLabel;
        cTech = new QComboBox; for (const auto &t : techs) if (t.active && t.certified) cTech->addItem(t.name);
        cTech->setEditable(true);
        cTech->setInsertPolicy(QComboBox::NoInsert);
        if (cTech->lineEdit()) cTech->lineEdit()->setMaxLength(120);
        cPr = new QComboBox; cPr->addItems({"High", "Medium", "Low"});
        cSt = new QComboBox; cSt->addItems({"Planned", "In Progress", "Completed"});
        const QDateTime now = QDateTime::currentDateTime();
        dPS = new QDateTimeEdit(now.addDays(1)); dPE = new QDateTimeEdit(now.addDays(1).addSecs(7200));
        dRS = new QDateTimeEdit(now); dRE = new QDateTimeEdit(now.addSecs(7200));
        for (auto *d : {dPS, dPE, dRS, dRE}) { d->setCalendarPopup(true); d->setDisplayFormat("dd/MM/yyyy HH:mm"); }
        eTitle = new QLineEdit; eTitle->setObjectName("maintenanceInterventionTitle");
        eTitle->setPlaceholderText("Enter intervention title"); eTitle->setMaxLength(150);
        eDesc = new QLineEdit; eDesc->setMaxLength(500); eRep = new QPlainTextEdit; eRep->setFixedHeight(70);
        f->addRow("Intervention title", eTitle);
        f->addRow("Equipment", cEq); f->addRow("Associated station", lSt); f->addRow("Technician", cTech);
        f->addRow("Priority", cPr); f->addRow("Status", cSt); f->addRow("Planned start", dPS);
        f->addRow("Planned end", dPE); f->addRow("Actual start", dRS); f->addRow("Actual end", dRE);
        f->addRow("Description", eDesc); f->addRow("Work report", eRep);
        auto sync = [this] {
            int i = cEq->currentIndex();
            lSt->setText(i >= 0 && i < eqs.size() ? eqs[i].station : QStringLiteral("Station not available"));
            QString s = cSt->currentText();
            dRS->setEnabled(s != "Planned"); dRE->setEnabled(s == "Completed"); eRep->setEnabled(s == "Completed");
        };
        connect(cEq, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sync);
        connect(cSt, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sync);
        if (it) {
            if (cEq->findData(it->equipId) < 0)
                cEq->addItem(QStringLiteral("Unknown equipment (%1)").arg(it->equipId), it->equipId);
            cEq->setCurrentIndex(cEq->findData(it->equipId)); cTech->setCurrentText(it->tech);
            cPr->setCurrentText(it->priority); cSt->setCurrentText(it->status);
            dPS->setDateTime(it->plannedStart); dPE->setDateTime(it->plannedEnd);
            if (it->realStart.isValid()) dRS->setDateTime(it->realStart);
            if (it->realEnd.isValid()) dRE->setDateTime(it->realEnd);
            eTitle->setText(it->title); eDesc->setText(it->desc); eRep->setPlainText(it->report);
        }
        sync();
        auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        bb->button(QDialogButtonBox::Ok)->setText("Save");
        bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
        f->addRow(bb);
        connect(bb, &QDialogButtonBox::accepted, this, [this] { if (validate()) accept(); });
        connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    Intervention result() const {
        Intervention r; QString s = cSt->currentText();
        r.equipId = cEq->currentData().toString(); r.title = eTitle->text().trimmed(); r.tech = cTech->currentText().trimmed();
        r.priority = cPr->currentText(); r.status = s; r.desc = eDesc->text().trimmed();
        r.plannedStart = dPS->dateTime(); r.plannedEnd = dPE->dateTime(); r.report = eRep->toPlainText().trimmed();
        if (s != "Planned") r.realStart = dRS->dateTime();
        if (s == "Completed") r.realEnd = dRE->dateTime();
        return r;
    }
private:
    bool validate() {
        QString s = cSt->currentText(), err;
        if (eTitle->text().trimmed().isEmpty())       err = "Enter a title for this intervention.";
        else if (cTech->currentText().trimmed().isEmpty()) err = "Select or enter a technician.";
        else if (dPE->dateTime() < dPS->dateTime())   err = "The planned end must be at or after the planned start.";
        else if (s == "Completed" && dRE->dateTime() < dRS->dateTime()) err = "The actual end must be at or after the actual start.";
        else if (s == "Completed" && eRep->toPlainText().trimmed().isEmpty()) err = "A work report is required to complete an intervention.";
        if (!err.isEmpty()) { showMessage(this, QMessageBox::Warning, "Review required", err); return false; }
        return true;
    }
    QVector<Equipment> eqs;
    QComboBox *cEq, *cTech, *cPr, *cSt; QLabel *lSt;
    QDateTimeEdit *dPS, *dPE, *dRS, *dRE; QLineEdit *eTitle, *eDesc; QPlainTextEdit *eRep;
};

const char *QSS = R"(
QWidget{font-family:'Segoe UI','Inter',Arial;font-size:14px;color:#1f2937;}
QDialog{background:#f7f9fd;color:#17243a;}
QDialog QLabel{color:#34445c;background:transparent;}
QDialog#interventionDialog QLabel{color:#26364f;font-weight:600;}
#page{background:transparent;}
#side QLabel{background:transparent;}
#nav{background:transparent;color:#e3edf9;border:none;text-align:left;padding-left:16px;font-size:14px;border-radius:10px;}
#nav:hover{background:rgba(33,111,209,150);color:white;border:1px solid rgba(125,187,255,180);} #nav:checked{background:#0787f5;color:white;font-weight:700;}
#systemStatusTitle{color:#b4c7df;font-size:10px;font-weight:700;background:transparent;}
#systemStatusText{color:#f1f7ff;font-size:11px;background:transparent;}
#systemStatusTime{color:#b8c9de;font-size:10px;background:transparent;}
#windowControlBar{background:#071a31;border:none;}
#windowControl{background:transparent;color:#dce7f4;border:none;border-radius:0;padding:0;font-size:16px;}
#windowControl:hover{background:rgba(255,255,255,35);color:white;border:none;}
#windowClose:hover{background:#e81123;color:white;border:none;}
#windowControl:pressed{background:rgba(255,255,255,55);color:white;}
#windowClose:pressed{background:#b50f1b;color:white;}
#bar{background:rgba(255,255,255,235);border-bottom:1px solid #e6eaf2;} #bar QLabel{background:transparent;}
#card{background:white;border:1px solid #e6eaf2;border-radius:14px;} #card QLabel,#card QCheckBox{background:transparent;}
QLabel#maintenanceTitle{font-size:28px;font-weight:800;color:#0b1437;background:transparent;}
QLabel#maintenanceSubtitle{font-size:14px;color:#5b6880;background:transparent;}
QLineEdit,QComboBox,QDateTimeEdit,QPlainTextEdit{background:#ffffff;color:#17243a;border:1px solid #cbd5e1;border-radius:9px;padding:7px 10px;font-size:14px;selection-background-color:#1f6fe5;selection-color:white;}
QComboBox QAbstractItemView{background:white;color:#17243a;selection-background-color:#e3edff;selection-color:#153f88;outline:0;}
QLineEdit:disabled,QComboBox:disabled,QDateTimeEdit:disabled,QPlainTextEdit:disabled{background:#f0f3f7;color:#56647a;border-color:#d7dee9;}
QCheckBox{color:#24334c;spacing:7px;}
QPushButton,QToolButton{background:white;border:1px solid #cbd5e1;border-radius:10px;padding:7px 14px;font-size:14px;color:#1f2937;}
QPushButton:hover,QToolButton:hover{background:#e3edff;border:1px solid #6b9cff;color:#153f88;}
QPushButton:pressed,QToolButton:pressed{background:#c5d9ff;border:1px solid #1f6fe5;color:#103a83;}
QPushButton[clickFlash="true"],QToolButton[clickFlash="true"]{background:#c5d9ff;border:1px solid #1f6fe5;color:#103a83;}
QPushButton#primary{background:#1f6fe5;color:white;border:none;font-weight:700;} QPushButton#primary:hover{background:#155ac5;border:1px solid #7eaeff;color:white;} QPushButton#primary[clickFlash="true"]{background:#124ca8;border:1px solid #9bc0ff;color:white;}
QPushButton#danger{color:#b42318;border-color:#f3c2c2;} QPushButton#danger:hover{background:#fde2e2;border-color:#e98d8d;color:#981b13;} QPushButton#danger[clickFlash="true"]{background:#f6caca;border-color:#d92d2d;color:#8e1710;}
QPushButton#nav[clickFlash="true"]{background:#326fe0;color:white;}
QToolButton::menu-indicator{image:none;}
QTableWidget{background:white;border:none;color:#17243a;selection-background-color:#dfeaff;selection-color:#111827;font-size:13px;}
QTableWidget::item{border-bottom:1px solid #eef1f6;}
QHeaderView::section{background:#fbfcfe;border:none;border-bottom:1px solid #e6eaf2;padding:9px 8px;font-weight:700;color:#42526e;}
QTableWidget QWidget{background:transparent;}
QMenu{background:white;color:#17243a;border:1px solid #dde3ee;padding:6px;} QMenu::item{padding:8px 22px;border-radius:6px;} QMenu::item:selected{background:#eef3ff;color:#153f88;}
)";
} // namespace

// =====================================================================
//  3. BUILD THE INTERFACE
// =====================================================================
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("SkySense — Weather Station Management");
    setWindowIcon(QIcon(":/images/logo.png"));
    setWindowFlags(Qt::FramelessWindowHint);
    resize(1360, 860);
    seedData();
    auto *central = new BgWidget; auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0); h->setSpacing(0);
    h->addWidget(buildSidebar());
    auto *right = new QVBoxLayout; right->setSpacing(0);
    right->addWidget(buildWindowControlBar());
    right->addWidget(buildTopBar());
    stack = new QStackedWidget;
    for (const QString &n : {QString("Dashboard"), QString("Stations"), QString("Equipment")}) {
        auto *l = new QLabel(QString("The %1 module will be available here.").arg(n));
        l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color:#24334c;font-size:16px;"); stack->addWidget(l);
    }
    maintenancePage = buildMaintenancePage();
    stack->addWidget(maintenancePage);   // Index 3.
    for (const QString &n : {QString("Staff"), QString("Services")}) {
        auto *l = new QLabel(QString("The %1 module will be available here.").arg(n));
        l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color:#24334c;font-size:16px;"); stack->addWidget(l);
    }
    stack->setCurrentIndex(3);
    right->addWidget(stack, 1); h->addLayout(right, 1);
    setCentralWidget(central);
    setStyleSheet(QSS);
    qApp->installEventFilter(this);
    QFile clickFile(":/sounds/click.wav");
    if (clickFile.open(QIODevice::ReadOnly)) clickWaveBuffer = clickFile.readAll();
#ifdef SKYSENSE_HAS_MULTIMEDIA
    clickSound = new QSoundEffect(this);
    clickSound->setSource(QUrl("qrc:/sounds/click.wav"));
    clickSound->setLoopCount(1);
    clickSound->setVolume(0.60);
#endif
    connect(stack, &QStackedWidget::currentChanged, this, [this](int index) {
        if (index == 3) animateMaintenancePage();
    });
    QTimer::singleShot(0, this, [this] { animateMaintenancePage(); });

    if (Connection::createInstance().isConnected()) {
        QVector<Intervention> storedInterventions;
        QString error;
        if (maintenanceModel.loadAll(storedInterventions, &error)) {
            data = storedInterventions;
            databaseBacked = true;
            selId = data.isEmpty() ? QString() : data.first().id;
        } else {
            data.clear();
            selId.clear();
            databaseSetupRequired = true;
            QTimer::singleShot(0, this, [this, error] {
                showMessage(this, QMessageBox::Warning, "Maintenance table not ready",
                    "Oracle is connected, but the Maintenance table could not be loaded.\n\n"
                    "Connect as SKYSENSE to XEPDB1 in SQL Developer and run maintenance_schema.sql, then restart SkySense.\n\n"
                    "Oracle message: " + error);
            });
        }
    }

    if (auto *status = findChild<QLabel *>("systemStatusText")) {
        if (databaseBacked)
            status->setText("<span style='color:#19d48b'>●</span>&nbsp; Oracle connected");
        else if (databaseSetupRequired)
            status->setText("<span style='color:#f59e0b'>●</span>&nbsp; Maintenance schema required");
        else
            status->setText("<span style='color:#f59e0b'>●</span>&nbsp; Preview data · not saved");
    }
    refresh();
}

QWidget *MainWindow::buildSidebar()
{
    auto *sb = new SideWidget; sb->setObjectName("side"); sb->setFixedWidth(238);
    auto *l = new QVBoxLayout(sb); l->setContentsMargins(14, 20, 14, 18);
    auto *dots = new QHBoxLayout; dots->setSpacing(7);
    const QStringList dotColors = {"#ff5f57", "#febc2e", "#28c840"};
    for (const QString &color : dotColors) {
        auto *d = new QLabel; d->setFixedSize(8, 8);
        d->setStyleSheet(QString("background:%1;border-radius:4px;").arg(color));
        dots->addWidget(d);
    }
    dots->addStretch(); l->addLayout(dots); l->addSpacing(10);
    auto *lg = new QLabel; lg->setPixmap(QPixmap(":/images/logo_light.png").scaledToWidth(178, Qt::SmoothTransformation));
    lg->setAlignment(Qt::AlignCenter);
    l->addWidget(lg); l->addSpacing(24);
    const QStringList items = {"⌂  Dashboard", "◎  Stations", "▤  Equipment", "⚒  Maintenance", "♙  Staff", "⊞  Services"};
    for (int i = 0; i < items.size(); ++i) {
        auto *b = new QPushButton(items[i]); b->setObjectName("nav"); b->setCheckable(true);
        b->setAutoExclusive(true); b->setFixedHeight(44); b->setChecked(i == 3);
        connect(b, &QPushButton::clicked, this, [this, i] { stack->setCurrentIndex(i); });
        l->addWidget(b);
    }
    l->addStretch();
    auto *status = new QWidget; auto *statusLayout = new QVBoxLayout(status);
    statusLayout->setContentsMargins(5, 8, 5, 0); statusLayout->setSpacing(5);
    auto *statusTitle = new QLabel("SYSTEM STATUS"); statusTitle->setObjectName("systemStatusTitle");
    auto *statusText = new QLabel("<span style='color:#19d48b'>●</span>&nbsp; All systems operational");
    statusText->setObjectName("systemStatusText"); statusText->setTextFormat(Qt::RichText);
    auto *statusTime = new QLabel("Updated " + englishLocale().toString(QDateTime::currentDateTime(), "MMM d, yyyy HH:mm"));
    statusTime->setObjectName("systemStatusTime");
    statusLayout->addWidget(statusTitle); statusLayout->addWidget(statusText); statusLayout->addWidget(statusTime);
    l->addWidget(status);
    return sb;
}

QWidget *MainWindow::buildWindowControlBar()
{
    auto *bar = new QFrame; bar->setObjectName("windowControlBar"); bar->setFixedHeight(32);
    windowControlBar = bar;
    auto *layout = new QHBoxLayout(bar); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    layout->addStretch();

    auto makeControl = [bar](const QString &text, const QString &name, const QString &tip) {
        auto *button = new QPushButton(text, bar); button->setObjectName(name);
        button->setFixedSize(46, 32);
        button->setToolTip(tip); button->setAccessibleName(tip); button->setCursor(Qt::PointingHandCursor);
        return button;
    };
    auto *minimize = makeControl("—", "windowControl", "Minimize window");
    maximizeWindowButton = makeControl("□", "windowControl", "Maximize window");
    auto *closeButton = makeControl("×", "windowClose", "Close application");
    layout->addWidget(minimize); layout->addWidget(maximizeWindowButton); layout->addWidget(closeButton);
    connect(minimize, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeWindowButton, &QPushButton::clicked, this, &MainWindow::toggleWindowMaximize);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    return bar;
}

void MainWindow::toggleWindowMaximize()
{
    if (isMaximized()) showNormal(); else showMaximized();
    if (maximizeWindowButton) {
        const bool maximized = isMaximized();
        maximizeWindowButton->setText(maximized ? "❐" : "□");
        maximizeWindowButton->setToolTip(maximized ? "Restore window" : "Maximize window");
        maximizeWindowButton->setAccessibleName(maximizeWindowButton->toolTip());
    }
}

QWidget *MainWindow::buildTopBar()
{
    auto *bar = new QFrame; bar->setObjectName("bar"); bar->setFixedHeight(64);
    auto *l = new QHBoxLayout(bar);
    auto *gs = new QLineEdit; gs->setPlaceholderText("🔍 Search by equipment, station, or technician…");
    gs->setFixedWidth(430);
    l->addWidget(gs); l->addStretch();
    l->addWidget(new QLabel("📅  " + englishLocale().toString(QDate::currentDate(), "dddd, MMMM d, yyyy")));
    auto *av = new QLabel; av->setPixmap(avatar("Alex Martin", 38)); l->addWidget(av);
    l->addWidget(new QLabel("<b>Alex Martin</b><br><small style='color:#65728a'>Operations Manager</small>"));
    connect(gs, &QLineEdit::textChanged, this, [this](const QString &t) { search->setText(t); });
    return bar;
}

QWidget *MainWindow::buildMaintenancePage()
{
    auto *page = new QWidget; page->setObjectName("page");
    auto *root = new QVBoxLayout(page); root->setContentsMargins(24, 12, 24, 16); root->setSpacing(12);

    // ---- Header and primary actions
    auto *headSection = new QWidget(page); headSection->setObjectName("maintenanceHeader"); prepareFadeIn(headSection);
    auto *head = new QHBoxLayout(headSection); head->setContentsMargins(0, 0, 0, 0);
    auto *tb = new QVBoxLayout; tb->setSpacing(0);
    auto *t = new QLabel("Maintenance"); t->setObjectName("maintenanceTitle");
    auto *s = new QLabel("Plan maintenance work and track service across your weather network.");
    s->setObjectName("maintenanceSubtitle");
    prepareFadeIn(t); prepareFadeIn(s);
    tb->addWidget(t); tb->addWidget(s);
    head->addLayout(tb); head->addStretch();

    auto *bStats = new QPushButton("📊 Statistics");
    connect(bStats, &QPushButton::clicked, this, &MainWindow::showStats);
    auto *bPdf = new QToolButton; bPdf->setText("📄 Export to PDF ▾"); bPdf->setPopupMode(QToolButton::InstantPopup);
    auto *mp = new QMenu(bPdf);
    mp->addAction("Filtered list", this, &MainWindow::exportList);
    mp->addAction("Selected intervention report", this, &MainWindow::exportSheet);
    bPdf->setMenu(mp);
    topDeleteButton = new QPushButton("🗑 Delete"); topDeleteButton->setObjectName("danger");
    topEditButton = new QPushButton("✏ Edit");
    connect(topDeleteButton, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(topEditButton, &QPushButton::clicked, this, &MainWindow::onEdit);
    auto *bAdd = new QPushButton("＋ Add intervention"); bAdd->setObjectName("primary");
    connect(bAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    for (QWidget *w : {(QWidget *)bStats, (QWidget *)bPdf, (QWidget *)topDeleteButton,
                       (QWidget *)topEditButton, (QWidget *)bAdd}) {
        w->setFixedHeight(38); head->addWidget(w);
    }
    root->addWidget(headSection);

    // ---- Search, filters, and sorting (B1 + B2)
    auto *filterSection = new QWidget(page); filterSection->setObjectName("maintenanceFilters"); prepareFadeIn(filterSection);
    auto *fl = new QHBoxLayout(filterSection); fl->setContentsMargins(0, 0, 0, 0);
    search = new QLineEdit; search->setPlaceholderText("🔍  Equipment, station, technician, or status…");
    fStatus = new QComboBox; fStatus->addItems({"All statuses", "Planned", "In Progress", "Overdue", "Completed"});
    fPeriod = new QComboBox; fPeriod->addItems({"All periods", "Today", "Next 7 days", "Last 30 days"});
    fSort = new QComboBox; fSort->addItems({"Sort: due date", "Sort: priority", "Sort: status", "Sort: delay", "Sort: proposed order"});
    fl->addWidget(search, 1); fl->addWidget(fStatus); fl->addWidget(fPeriod); fl->addWidget(fSort);
    root->addWidget(filterSection);
    connect(search, &QLineEdit::textChanged, this, [this] { refresh(); });
    for (auto *c : {fStatus, fPeriod, fSort})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { refresh(); });

    // ---- KPI cards
    auto *kl = new QHBoxLayout; kl->setSpacing(14);
    struct K { const char *ic, *title, *col, *bg; };
    const K ks[4] = {{"🔧", "Planned", "#2f62d9", "#eef3ff"}, {"❗", "Overdue", "#d92d2d", "#fff0f0"},
                     {"✔", "Completed", "#1f9d55", "#eefaf1"}, {"⏱", "Average duration", "#7a4fd6", "#f3efff"}};
    for (int i = 0; i < 4; ++i) {
        auto *f = new QFrame; f->setObjectName("kpi");
        f->setProperty("maintenanceKpiIndex", i); prepareFadeIn(f);
        f->setStyleSheet(QString("#kpi{background:%1;border-radius:14px;border:1px solid #e6eaf2;}").arg(ks[i].bg));
        auto *ic = new QLabel(ks[i].ic); ic->setFixedSize(46, 46); ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet(QString("background:white;color:%1;border-radius:23px;font-size:20px;").arg(ks[i].col));
        auto *tt = new QLabel(ks[i].title); tt->setStyleSheet("font-weight:600;background:transparent;");
        kVal[i] = new QLabel; kVal[i]->setStyleSheet("background:transparent;");
        kSub[i] = new QLabel; kSub[i]->setStyleSheet(QString("color:%1;font-size:11px;font-weight:600;background:transparent;").arg(ks[i].col));
        auto *bx = new QVBoxLayout; bx->setSpacing(0); bx->addWidget(tt); bx->addWidget(kVal[i]); bx->addWidget(kSub[i]);
        auto *hl = new QHBoxLayout(f); hl->addWidget(ic); hl->addLayout(bx); hl->addStretch();
        kl->addWidget(f);
    }
    root->addLayout(kl);

    // ---- Interventions table
    auto *tableSection = new QWidget(page); tableSection->setObjectName("maintenanceTable");
    auto *tableSectionLayout = new QVBoxLayout(tableSection); tableSectionLayout->setContentsMargins(0, 0, 0, 0);
    auto *card = new QFrame; card->setObjectName("card"); addCardShadow(card); auto *cl = new QVBoxLayout(card);
    auto *top = new QHBoxLayout; auto *ttl = new QLabel("☰  Maintenance interventions");
    ttl->setStyleSheet("font-weight:700;font-size:15px;"); count = new QLabel;
    top->addWidget(ttl); top->addStretch(); top->addWidget(count); cl->addLayout(top);
    table = new QTableWidget(0, 8);
    table->setHorizontalHeaderLabels({"ID", "Equipment", "Scheduled date", "Priority", "Technician", "Status", "Delay", "Actions"});
    table->verticalHeader()->hide(); table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->verticalHeader()->setDefaultSectionSize(50);
    auto *hh = table->horizontalHeader(); hh->setStretchLastSection(true); hh->setSectionsClickable(true);
    const int w[8] = {110, 230, 150, 100, 170, 110, 90, 60};
    for (int i = 0; i < 8; ++i) table->setColumnWidth(i, w[i]);
    hh->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(hh, &QHeaderView::sectionClicked, this, [this](int c) {      // B1: sort when a column header is clicked.
        static const QMap<int, int> m = {{2, 0}, {3, 1}, {5, 2}, {6, 3}};
        if (m.contains(c)) fSort->setCurrentIndex(m[c]);
    });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this] {
        int r = table->currentRow();
        if (r >= 0 && table->item(r, 0)) { selId = table->item(r, 0)->data(Qt::UserRole).toString(); showDetail(); }
    });
    cl->addWidget(table); tableSectionLayout->addWidget(card); root->addWidget(tableSection, 1);
    root->addWidget(buildDetailCard());
    return page;
}

QWidget *MainWindow::buildDetailCard()
{
    auto *section = new QWidget; section->setObjectName("maintenanceDetail");
    auto *sectionLayout = new QVBoxLayout(section); sectionLayout->setContentsMargins(0, 0, 0, 0);
    auto *d = new QFrame; d->setObjectName("card"); addCardShadow(d); auto *dl = new QHBoxLayout(d); dl->setSpacing(18);
    auto *img = new QLabel; img->setPixmap(QPixmap(":/images/station.jpg").scaled(150, 110, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation).copy(0, 0, 150, 110)); img->setFixedSize(150, 110);
    dId = new QLabel; dId->setStyleSheet("font-size:17px;font-weight:800;");
    dSt = new QLabel; dName = new QLabel; dName->setStyleSheet("font-weight:700;font-size:14px;");
    dWhen = new QLabel; dInfo = new QLabel; dInfo->setWordWrap(true); dInfo->setStyleSheet("color:#4b5563;");
    auto *r = new QHBoxLayout; r->addWidget(dId); r->addWidget(dSt); r->addStretch(); r->addWidget(dWhen);
    auto *info = new QVBoxLayout; info->addLayout(r); info->addWidget(dName); info->addWidget(dInfo); info->addStretch();
    dl->addWidget(img); dl->addLayout(info, 3);

    auto *rc = new QVBoxLayout; auto *rt = new QLabel("Recommended actions"); rt->setStyleSheet("font-weight:700;");
    rc->addWidget(rt); auto *g = new QGridLayout;
    const QStringList at = {"Clean and inspect the solar panel", "Check and calibrate the sensors",
                            "Check the communication signal", "Verify data after maintenance"};
    for (int i = 0; i < 4; ++i) {
        acts[i] = new QCheckBox(at[i]); g->addWidget(acts[i], i % 2, i / 2);
        connect(acts[i], &QCheckBox::toggled, this, [this, i](bool on) {
            int k = idxOf(selId); if (k < 0) return;
            if (on) data[k].actionsMask |= (1 << i); else data[k].actionsMask &= ~(1 << i);
            markDetailsDirty();
        });
    }
    rc->addLayout(g);
    notes = new QPlainTextEdit; notes->setPlaceholderText("Work report / notes for this intervention...");
    notes->setFixedHeight(56);
    connect(notes, &QPlainTextEdit::textChanged, this, [this] {
        int k = idxOf(selId); if (k >= 0) data[k].report = notes->toPlainText();
        if (k >= 0) markDetailsDirty();
    });
    auto *bl = new QVBoxLayout;
    saveDetailsButton = new QPushButton("Save notes & actions");
    saveDetailsButton->setEnabled(false);
    connect(saveDetailsButton, &QPushButton::clicked, this, &MainWindow::saveSelectedDetails);
    completeInterventionButton = new QPushButton("Mark as completed");
    completeInterventionButton->setObjectName("primary");
    connect(completeInterventionButton, &QPushButton::clicked, this, &MainWindow::onComplete);
    bl->addWidget(saveDetailsButton);
    bl->addWidget(completeInterventionButton);
    auto *nl = new QHBoxLayout; nl->addWidget(notes, 1); nl->addLayout(bl);
    rc->addLayout(nl); dl->addLayout(rc, 4);
    sectionLayout->addWidget(d);
    return section;
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{   // Drag the frameless window from its top bar.
    dragging = (e->button() == Qt::LeftButton && e->pos().y() < 64 && !isMaximized());
    if (dragging) dragOff = e->globalPosition().toPoint() - frameGeometry().topLeft();
}
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (dragging && (e->buttons() & Qt::LeftButton)) move(e->globalPosition().toPoint() - dragOff);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == windowControlBar) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) { toggleWindowMaximize(); return true; }
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            dragging = mouse->button() == Qt::LeftButton && !isMaximized();
            if (dragging) dragOff = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
            return true;
        } else if (event->type() == QEvent::MouseMove) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (dragging && (mouse->buttons() & Qt::LeftButton)) {
                move(mouse->globalPosition().toPoint() - dragOff);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            dragging = false;
            return true;
        }
    }
    auto *button = qobject_cast<QAbstractButton *>(watched);
    if (button && event->type() == QEvent::Enter) {
        button->setCursor(Qt::PointingHandCursor);
        playButtonSound(true);
    } else if (button && event->type() == QEvent::Leave) {
        button->setProperty("clickFlash", false);
    } else if (button && event->type() == QEvent::MouseButtonPress) {
        button->setProperty("clickFlash", true);
        button->style()->unpolish(button);
        button->style()->polish(button);
    } else if (button && event->type() == QEvent::MouseButtonRelease) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton && button->underMouse()) {
            playButtonSound(true);
            button->setProperty("clickFlash", true);
            button->style()->unpolish(button);
            button->style()->polish(button);
            QTimer::singleShot(220, button, [button] {
                button->setProperty("clickFlash", false);
                button->style()->unpolish(button);
                button->style()->polish(button);
            });
        }
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::playButtonSound(bool beepFallback)
{
    bool soundPlayed = playNativeClickSound();
#ifdef SKYSENSE_HAS_MULTIMEDIA
    if (!soundPlayed && clickSound && clickSound->status() == QSoundEffect::Ready) {
        clickSound->play();
        soundPlayed = true;
    }
#endif
    if (!soundPlayed && beepFallback) QApplication::beep();
}

void MainWindow::animateMaintenancePage()
{
    if (!maintenancePage) return;
    auto *title = maintenancePage->findChild<QLabel *>("maintenanceTitle");
    auto *subtitle = maintenancePage->findChild<QLabel *>("maintenanceSubtitle");
    if (!title || !subtitle) return;

    auto *titleEffect = qobject_cast<QGraphicsOpacityEffect *>(title->graphicsEffect());
    auto *subtitleEffect = qobject_cast<QGraphicsOpacityEffect *>(subtitle->graphicsEffect());
    if (!titleEffect) { titleEffect = new QGraphicsOpacityEffect(title); title->setGraphicsEffect(titleEffect); }
    if (!subtitleEffect) { subtitleEffect = new QGraphicsOpacityEffect(subtitle); subtitle->setGraphicsEffect(subtitleEffect); }
    if (maintenanceAnimation) {
        maintenanceAnimation->stop();
        delete maintenanceAnimation;
    }

    maintenanceAnimation = new QSequentialAnimationGroup(this);
    auto appendFade = [this](QGraphicsOpacityEffect *effect, int duration) {
        effect->setOpacity(0.0);
        auto *animation = new QPropertyAnimation(effect, "opacity", maintenanceAnimation);
        animation->setDuration(duration);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        maintenanceAnimation->addAnimation(animation);
    };
    auto appendWidgetFade = [&appendFade](QWidget *widget, int duration) {
        auto *effect = qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect());
        if (effect) appendFade(effect, duration);
    };

    appendWidgetFade(maintenancePage->findChild<QWidget *>("maintenanceHeader"), 180);
    appendFade(titleEffect, 180);
    maintenanceAnimation->addAnimation(new QPauseAnimation(35, maintenanceAnimation));
    appendFade(subtitleEffect, 200);
    appendWidgetFade(maintenancePage->findChild<QWidget *>("maintenanceFilters"), 180);

    const auto kpis = maintenancePage->findChildren<QFrame *>();
    for (QFrame *kpi : kpis) {
        if (!kpi->property("maintenanceKpiIndex").isValid()) continue;
        appendWidgetFade(kpi, 120);
        maintenanceAnimation->addAnimation(new QPauseAnimation(25, maintenanceAnimation));
    }

    // Keep the table's native rendering; reveal its rows one at a time.
    revealingRows = table && table->rowCount() > 0;
    const quint64 revealRun = ++maintenanceRevealGeneration;
    if (revealingRows) {
        const int rowCount = table->rowCount();
        for (int row = 0; row < rowCount; ++row) table->setRowHidden(row, true);
        for (int row = 0; row < rowCount; ++row) {
            QTimer::singleShot(35 + row * 45, this, [this, revealRun, row, rowCount] {
                if (revealRun != maintenanceRevealGeneration || !table) return;
                if (row < table->rowCount()) table->setRowHidden(row, false);
                if (row == rowCount - 1) revealingRows = false;
            });
        }
    }
    maintenanceAnimation->start();
}

// =====================================================================
//  4. PREVIEW DATA (used when Oracle is skipped or unavailable)
// =====================================================================
void MainWindow::seedData()
{
    eq = {{"EQ-01", "Weather station", "COA-01", "WS-3000", 55, 90},
          {"EQ-02", "Anemometer", "FOR-12", "WD-210", 80, 120},
          {"EQ-03", "Rain gauge", "VAL-07", "RG-50", 35, 60},
          {"EQ-04", "Solar panel", "ALP-03", "SP-400", 45, 90},
          {"EQ-05", "Communication module", "VAL-07", "CM-7", 75, 180},
          {"EQ-06", "Temperature sensor", "FOR-12", "TS-12", 85, 120},
          {"EQ-07", "Battery", "COA-01", "BP-12V", 60, 365},
          {"EQ-08", "Data logger", "ALP-03", "DL-9", 90, 180}};
    techs = {{"Sarah Chen", true, true}, {"Mike Rodriguez", true, true}, {"Emma Wilson", true, true},
             {"David Park", true, true}, {"Paul Martin", false, true}};
    QDateTime now = QDateTime::currentDateTime(); now.setTime(QTime(now.time().hour(), 0));
    auto add = [&](int n, const char *e, const char *t, const char *p, const char *s, int off, int dur,
                   const char *desc, double resH = 0) {
        Intervention it; it.id = QString("MT-%1-%2").arg(now.date().year()).arg(n, 3, 10, QChar('0'));
        it.equipId = e; it.title = desc; it.tech = t; it.priority = p; it.status = s; it.desc = desc;
        it.plannedStart = now.addSecs(off * 3600); it.plannedEnd = it.plannedStart.addSecs(dur * 3600);
        if (it.status != "Planned") it.realStart = it.plannedStart;
        if (it.status == "Completed") { it.realEnd = it.realStart.addSecs(qint64(resH * 3600)); it.report = "Maintenance completed; equipment inspected and verified."; }
        data.append(it);
    };
    add(14, "EQ-01", "Sarah Chen", "High", "Planned", 48, 2, "Routine maintenance and calibration");
    add(13, "EQ-02", "Mike Rodriguez", "Medium", "Planned", 72, 2, "Anemometer inspection");
    add(12, "EQ-03", "Emma Wilson", "High", "Planned", -30, 2, "Rain gauge cleaning and calibration");
    add(11, "EQ-04", "David Park", "High", "Planned", -50, 3, "Solar panel inspection");
    add(10, "EQ-05", "Sarah Chen", "Low", "In Progress", -1, 3, "Communication signal test");
    add(9, "EQ-06", "Mike Rodriguez", "Medium", "Planned", 96, 2, "Temperature probe replacement");
    add(8, "EQ-07", "Emma Wilson", "High", "Completed", -120, 2, "Battery replacement", 2.5);
    add(7, "EQ-08", "David Park", "Low", "Completed", -200, 2, "Data logger inspection", 1.5);
    add(6, "EQ-01", "Sarah Chen", "Medium", "Completed", -2200, 3, "Semiannual maintenance", 4.0);
    add(5, "EQ-03", "Mike Rodriguez", "Low", "Completed", -1500, 3, "Rain gauge inspection", 3.0);
    selId = data.first().id;
}

// =====================================================================
//  5. BUSINESS LOGIC
// =====================================================================
const Equipment *MainWindow::equip(const QString &id) const
{
    for (const auto &e : eq) if (e.id == id) return &e;
    static const Equipment unknown{"", "Unknown equipment", "Unknown station", "Unknown model", -1, 0};
    return &unknown;
}
int MainWindow::idxOf(const QString &id) const
{
    for (int i = 0; i < data.size(); ++i) if (data[i].id == id) return i;
    return -1;
}
qint64 MainWindow::delayHours(const Intervention &it) const
{   // Delay is calculated from the due date (planned end).
    QDateTime ref = (it.status == "Completed" && it.realEnd.isValid()) ? it.realEnd : QDateTime::currentDateTime();
    qint64 s = it.plannedEnd.secsTo(ref);
    return s > 0 ? s / 3600 : 0;
}
QString MainWindow::effStatus(const Intervention &it) const
{
    return (it.status == "Planned" && delayHours(it) > 0) ? "Overdue" : it.status;
}

QVector<int> MainWindow::filtered() const
{
    QVector<int> r; QDateTime now = QDateTime::currentDateTime();
    const QString q = search->text().trimmed().toLower();
    for (int i = 0; i < data.size(); ++i) {
        const auto &it = data[i]; const Equipment *e = equip(it.equipId); QString st = effStatus(it);
        QString hay = (it.id + " " + it.title + " " + e->name + " " + e->station + " " + it.tech + " " + st + " " + it.priority).toLower();
        if (!q.isEmpty() && !hay.contains(q)) continue;
        if (fStatus->currentIndex() > 0 && st != fStatus->currentText()) continue;
        int p = fPeriod->currentIndex();
        if (p == 1 && it.plannedEnd.date() != now.date()) continue;
        if (p == 2 && !(it.plannedEnd >= now && it.plannedEnd <= now.addDays(7))) continue;
        if (p == 3 && !(it.plannedEnd <= now && it.plannedEnd >= now.addDays(-30))) continue;
        r << i;
    }
    int s = fSort->currentIndex();
    std::stable_sort(r.begin(), r.end(), [&](int a, int b) {
        const auto &x = data[a]; const auto &y = data[b];
        switch (s) {
        case 1:  return prioW(x.priority) > prioW(y.priority);
        case 2:  return statRank(effStatus(x)) < statRank(effStatus(y));
        case 3:  return delayHours(x) > delayHours(y);
        case 4: { int pa = proposed.indexOf(x.id), pb = proposed.indexOf(y.id);
                  return (pa < 0 ? 9999 : pa) < (pb < 0 ? 9999 : pb); }
        default: return x.plannedEnd < y.plannedEnd;
        }
    });
    return r;
}

// I1: explainable scheduling = priority + delay + equipment condition.
QVector<Rank> MainWindow::ranking() const
{
    QVector<Rank> r;
    for (int i = 0; i < data.size(); ++i) {
        const auto &it = data[i]; if (it.status == "Completed") continue;
        const Equipment *e = equip(it.equipId);
        int pp = prioW(it.priority) * 10, dh = int(delayHours(it));
        int dp = qMin(40, int(dh / 24.0 * 5 + (dh > 0 ? 5 : 0)));
        int hp = e->health < 0 ? 0 : (e->health < 40 ? 20 : (e->health < 70 ? 10 : 0));
        r.append({i, pp + dp + hp, QString("Priority %1 (+%2) • Delay %3 h (+%4) • Equipment condition %5 (+%6)")
                      .arg(it.priority).arg(pp).arg(dh).arg(dp).arg(healthText(e->health)).arg(hp)});
    }
    std::stable_sort(r.begin(), r.end(), [](const Rank &a, const Rank &b) { return a.score > b.score; });
    return r;
}

// =====================================================================
//  6. RENDERING
// =====================================================================
void MainWindow::refresh()
{
    if (revealingRows) {
        revealingRows = false;
        ++maintenanceRevealGeneration;
    }
    int np = 0, no = 0, nc = 0; double sum = 0;
    for (const auto &it : data) {
        QString s = effStatus(it);
        if (s == "Planned") ++np; else if (s == "Overdue") ++no;
        else if (s == "Completed") { ++nc; sum += it.realStart.secsTo(it.realEnd) / 3600.0; }
    }
    QString tot = QString("/ %1 total").arg(data.size());
    auto big = [](const QString &a, const QString &b) {
        return QString("<span style='font-size:26px;font-weight:700;color:#111'>%1</span> <span style='color:#888'>%2</span>").arg(a, b); };
    kVal[0]->setText(big(QString::number(np), tot)); kSub[0]->setText("Upcoming interventions");
    kVal[1]->setText(big(QString::number(no), tot)); kSub[1]->setText("Needs priority attention");
    kVal[2]->setText(big(QString::number(nc), tot)); kSub[2]->setText("Recorded history");
    kVal[3]->setText(big(englishLocale().toString(nc ? sum / nc : 0, 'f', 1) + " h", "")); kSub[3]->setText("For completed interventions");

    QVector<int> rows = filtered();
    if (rows.isEmpty()) selId.clear();
    count->setText(QString("%1 of %2").arg(rows.size()).arg(data.size()));
    table->blockSignals(true); table->clearContents(); table->setRowCount(rows.size());
    int selRow = -1;
    for (int r = 0; r < rows.size(); ++r) {
        table->setRowHidden(r, false);
        const Intervention &it = data[rows[r]]; const Equipment *e = equip(it.equipId); QString st = effStatus(it);
        auto *idi = new QTableWidgetItem(it.id); idi->setData(Qt::UserRole, it.id); table->setItem(r, 0, idi);
        for (int c = 1; c < 8; ++c) table->setItem(r, c, new QTableWidgetItem);
        const QString shownTitle = it.title.trimmed().isEmpty() ? QStringLiteral("Title not recorded") : it.title.trimmed();
        table->setCellWidget(r, 1, twoLines(shownTitle, "Equipment " + e->name + " · Station " + e->station, true));
        table->setCellWidget(r, 2, twoLines(englishLocale().toString(it.plannedStart.date(), "MMM d, yyyy"),
                             it.plannedStart.toString("HH:mm") + " – " + it.plannedEnd.toString("HH:mm"), false));
        table->setCellWidget(r, 3, center(badge(it.priority, prioColor(it.priority))));
        auto *tc = new QWidget; auto *tl = new QHBoxLayout(tc); tl->setContentsMargins(6, 0, 6, 0);
        auto *av = new QLabel; av->setPixmap(avatar(it.tech, 30)); tl->addWidget(av); tl->addWidget(new QLabel(it.tech)); tl->addStretch();
        table->setCellWidget(r, 4, tc);
        table->setCellWidget(r, 5, center(badge(st, statColor(st))));
        qint64 dh = delayHours(it); auto *dl = new QLabel(dh ? QString("+%1 d %2 h").arg(dh / 24).arg(dh % 24) : "—");
        if (dh) dl->setStyleSheet("color:#d92d2d;font-weight:600;");
        table->setCellWidget(r, 6, center(dl));
        auto *ab = new QToolButton; ab->setText("•••"); ab->setToolTip("Intervention actions"); ab->setPopupMode(QToolButton::InstantPopup);
        auto *m = new QMenu(ab); QString id = it.id;
        m->addAction("✏ Edit", this, [this, id] { QTimer::singleShot(0, this, [this, id] { selId = id; onEdit(); }); });
        m->addAction("✔ Mark as completed", this, [this, id] { QTimer::singleShot(0, this, [this, id] { selId = id; onComplete(); }); });
        m->addAction("📄 PDF report", this, [this, id] { QTimer::singleShot(0, this, [this, id] { selId = id; exportSheet(); }); });
        m->addSeparator();
        m->addAction("🗑 Delete", this, [this, id] { QTimer::singleShot(0, this, [this, id] { selId = id; onDelete(); }); });
        ab->setMenu(m); table->setCellWidget(r, 7, center(ab));
        if (it.id == selId) selRow = r;
    }
    if (selRow < 0 && !rows.isEmpty()) { selRow = 0; selId = data[rows[0]].id; }
    if (selRow >= 0) table->selectRow(selRow);
    table->blockSignals(false);
    showDetail();
}

void MainWindow::showDetail()
{
    int k = idxOf(selId);
    if (k < 0) {
        dId->setText("No intervention selected"); dSt->clear(); dName->clear(); dWhen->clear(); dInfo->clear();
        if (topEditButton) topEditButton->setEnabled(false);
        if (topDeleteButton) topDeleteButton->setEnabled(false);
        if (completeInterventionButton) completeInterventionButton->setEnabled(false);
        if (saveDetailsButton) saveDetailsButton->setEnabled(false);
        for (QCheckBox *action : acts) {
            if (!action) continue;
            QSignalBlocker blocker(action);
            action->setChecked(false);
            action->setEnabled(false);
        }
        if (notes) { QSignalBlocker blocker(notes); notes->clear(); notes->setEnabled(false); }
        return;
    }
    if (topEditButton) topEditButton->setEnabled(true);
    if (topDeleteButton) topDeleteButton->setEnabled(true);
    const Intervention &it = data[k]; const Equipment *e = equip(it.equipId); QString st = effStatus(it);
    if (completeInterventionButton) completeInterventionButton->setEnabled(st != "Completed" && !databaseSetupRequired);
    for (QCheckBox *action : acts) if (action) action->setEnabled(!databaseSetupRequired);
    if (notes) notes->setEnabled(!databaseSetupRequired);
    if (saveDetailsButton) {
        const bool dirty = dirtyDetailIds.contains(it.id);
        saveDetailsButton->setEnabled(dirty && !databaseSetupRequired);
        saveDetailsButton->setText(databaseBacked ? (dirty ? "Save notes & actions *" : "Save notes & actions")
                                                   : (dirty ? "Save notes & actions (temporary)" : "Save notes & actions"));
    }
    auto c = statColor(st);
    dId->setText(it.id); dSt->setText(st);
    dSt->setStyleSheet(QString("background:%1;color:%2;border-radius:10px;padding:2px 10px;font-weight:600;").arg(c.first, c.second));
    dName->setText(it.title.trimmed().isEmpty()
                       ? QStringLiteral("Intervention title: not recorded")
                       : QStringLiteral("Intervention title: %1").arg(it.title));
    dWhen->setText(QString("📅 %1\n%2 – %3").arg(englishLocale().toString(it.plannedStart.date(), "dddd, MMMM d, yyyy"),
                   it.plannedStart.toString("HH:mm"), it.plannedEnd.toString("HH:mm")));
    const QString equipmentCondition = e->health < 0
        ? QStringLiteral("not recorded")
        : QStringLiteral("%1 (%2%)").arg(healthText(e->health)).arg(e->health);
    dInfo->setText(QString("📍 Station %1   🛠 %2 — %3\n📝 %4\n👤 %5   •   Equipment condition: %6")
                   .arg(e->station, e->name, e->model, it.desc, it.tech, equipmentCondition));
    for (int i = 0; i < 4; ++i) { QSignalBlocker b(acts[i]); acts[i]->setChecked(it.actionsMask & (1 << i)); }
    QSignalBlocker b(notes); notes->setPlainText(it.report);
}

// =====================================================================
//  7. BUTTON ACTIONS (CRUD)
// =====================================================================
void MainWindow::onAdd()
{
    InterventionDialog d(eq, techs, nullptr, this);
    if (d.exec() != QDialog::Accepted) return;
    // Add remains available before schema installation; block only the write.
    if (!allowMaintenanceWrite()) return;
    Intervention it = d.result();
    QString error;
    if (databaseBacked) {
        if (!maintenanceModel.insert(it, &error)) {
            showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
            return;
        }
    } else {
        int n = 0;
        for (const auto &x : data) n = qMax(n, x.id.section('-', -1).toInt());
        it.id = QString("MT-%1-%2").arg(QDate::currentDate().year()).arg(n + 1, 3, 10, QChar('0'));
    }
    data.prepend(it); selId = it.id; refresh();
}
void MainWindow::onEdit()
{
    if (!allowMaintenanceWrite()) return;
    int k = idxOf(selId); if (k < 0) { showMessage(this, QMessageBox::Information, "Edit intervention", "Select an intervention first."); return; }
    InterventionDialog d(eq, techs, &data[k], this);
    if (d.exec() != QDialog::Accepted) return;
    Intervention r = d.result(); r.id = data[k].id; r.actionsMask = data[k].actionsMask;
    QString error;
    if (databaseBacked && !maintenanceModel.update(r, &error)) {
        showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
        return;
    }
    dirtyDetailIds.remove(r.id);
    data[k] = r; refresh();
}
void MainWindow::onDelete()
{
    if (!allowMaintenanceWrite()) return;
    int k = idxOf(selId); if (k < 0) { showMessage(this, QMessageBox::Information, "Delete intervention", "Select an intervention first."); return; }
    if (!askChoice(this, "Confirm deletion", QString("Delete intervention %1?").arg(data[k].id), "Delete")) return;
    const QString id = data[k].id;
    QString error;
    if (databaseBacked && !maintenanceModel.remove(id, &error)) {
        showMessage(this, QMessageBox::Critical, "Oracle delete failed", error);
        return;
    }
    dirtyDetailIds.remove(id);
    data.removeAt(k); selId = data.isEmpty() ? QString() : data.first().id; refresh();
}
void MainWindow::onComplete()
{
    if (!allowMaintenanceWrite()) return;
    int k = idxOf(selId); if (k < 0) return;
    Intervention it = data[k];
    if (it.status == "Completed") { showMessage(this, QMessageBox::Information, "Complete intervention", "This intervention is already completed."); return; }
    if (it.report.trimmed().isEmpty()) {   // Completing an intervention requires a work report.
        showMessage(this, QMessageBox::Warning, "Work report required", "Enter a work report in the notes field before completing this intervention.");
        notes->setFocus(); return;
    }
    QDateTime now = QDateTime::currentDateTime();
    if (!it.realStart.isValid()) it.realStart = qMin(it.plannedStart, now.addSecs(-3600));
    it.realEnd = now; it.status = "Completed";
    QString error;
    if (databaseBacked && !maintenanceModel.update(it, &error)) {
        showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
        return;
    }
    dirtyDetailIds.remove(it.id);
    data[k] = it;
    refresh();
}

bool MainWindow::allowMaintenanceWrite()
{
    if (!databaseSetupRequired)
        return true;
    showMessage(this, QMessageBox::Warning, "Maintenance database not ready",
                "Run maintenance_schema.sql in SQL Developer while connected as SKYSENSE to XEPDB1, then restart SkySense.");
    return false;
}

void MainWindow::markDetailsDirty()
{
    if (databaseSetupRequired || selId.isEmpty())
        return;
    dirtyDetailIds.insert(selId);
    if (saveDetailsButton) {
        saveDetailsButton->setEnabled(true);
        saveDetailsButton->setText(databaseBacked ? "Save notes & actions *" : "Save notes & actions (temporary)");
    }
}

void MainWindow::saveSelectedDetails()
{
    if (!allowMaintenanceWrite()) return;
    const int k = idxOf(selId);
    if (k < 0 || !dirtyDetailIds.contains(selId))
        return;

    if (databaseBacked) {
        QString error;
        if (!maintenanceModel.update(data[k], &error)) {
            showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
            return;
        }
    }

    dirtyDetailIds.remove(selId);
    if (saveDetailsButton) {
        saveDetailsButton->setEnabled(false);
        saveDetailsButton->setText("Save notes & actions");
    }
    showMessage(this, QMessageBox::Information, "Saved",
                databaseBacked ? "The notes and actions were saved in Oracle."
                               : "The notes and actions are in preview memory only; Oracle did not save them.");
}

// =====================================================================
//  8. PDF EXPORT (B3)
// =====================================================================
static void writePdf(QWidget *parent, const QString &name, const QString &html)
{
    QString path = QFileDialog::getSaveFileName(parent, "Save PDF report", name, "PDF file (*.pdf)");
    if (path.isEmpty()) return;
    QPrinter pr(QPrinter::HighResolution); pr.setOutputFormat(QPrinter::PdfFormat); pr.setOutputFileName(path);
    QTextDocument doc; doc.setHtml(html); doc.print(&pr);
    showMessage(parent, QMessageBox::Information, "Export complete", "The PDF report was created successfully:\n" + path);
}
void MainWindow::exportList()
{
    QString rows;
    for (int i : filtered()) {
        const auto &it = data[i]; const Equipment *e = equip(it.equipId);
        rows += QString("<tr><td>%1</td><td>%2<br><small>Station %3</small></td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                    .arg(it.id, e->name, e->station, englishLocale().toString(it.plannedEnd, "MMM d, yyyy HH:mm"), it.priority, it.tech, effStatus(it));
    }
    writePdf(this, "skysense_maintenance_report.pdf",
        "<h1 style='color:#0b1437'>SkySense — Maintenance Report</h1><p>Generated on " +
        englishLocale().toString(QDateTime::currentDateTime(), "MMM d, yyyy HH:mm") + "</p>"
        "<table border='1' cellspacing='0' cellpadding='6' width='100%'><tr style='background:#e6eefc'>"
        "<th>ID</th><th>Equipment</th><th>Due date</th><th>Priority</th><th>Technician</th><th>Status</th></tr>" + rows + "</table>");
}
void MainWindow::exportSheet()
{
    int k = idxOf(selId); if (k < 0) { showMessage(this, QMessageBox::Information, "PDF report", "Select an intervention first."); return; }
    const auto &it = data[k]; const Equipment *e = equip(it.equipId);
    auto dt = [](const QDateTime &d) { return d.isValid() ? englishLocale().toString(d, "MMM d, yyyy HH:mm") : QString("—"); };
    writePdf(this, it.id + ".pdf", QString(
        "<h1 style='color:#0b1437'>Intervention Report %1</h1><table border='1' cellspacing='0' cellpadding='7' width='100%'>"
        "<tr><td><b>Equipment</b></td><td>%2 (%3)</td></tr><tr><td><b>Station</b></td><td>%4</td></tr>"
        "<tr><td><b>Technician</b></td><td>%5</td></tr><tr><td><b>Priority / Status</b></td><td>%6 / %7</td></tr>"
        "<tr><td><b>Planned schedule</b></td><td>%8 → %9</td></tr><tr><td><b>Actual schedule</b></td><td>%10 → %11</td></tr>"
        "<tr><td><b>Delay</b></td><td>%12 h</td></tr><tr><td><b>Description</b></td><td>%13</td></tr>"
        "<tr><td><b>Work report</b></td><td>%14</td></tr></table>")
        .arg(it.id, e->name, e->model, e->station, it.tech, it.priority, effStatus(it), dt(it.plannedStart))
        .arg(dt(it.plannedEnd), dt(it.realStart), dt(it.realEnd)).arg(delayHours(it))
        .arg(it.desc, it.report.toHtmlEscaped()));
}

// =====================================================================
//  9. STATISTICS (B4) AND SMART FEATURES (I1, I2)
// =====================================================================
void MainWindow::showStats()
{
    QMap<QString, int> cnt; QMap<QString, QPair<double, int>> res;
    for (const auto &it : data) {
        cnt[effStatus(it)]++;
        if (it.status == "Completed") { auto &p = res[it.priority]; p.first += it.realStart.secsTo(it.realEnd) / 3600.0; p.second++; }
    }
    QVector<QPair<QString, double>> a, b;
    const QStringList statuses = {QStringLiteral("Planned"), QStringLiteral("In Progress"),
                                  QStringLiteral("Overdue"), QStringLiteral("Completed")};
    const QStringList priorities = {QStringLiteral("High"), QStringLiteral("Medium"), QStringLiteral("Low")};
    for (const QString &s : statuses) a.append({s, double(cnt[s])});
    for (const QString &p : priorities) b.append({p, res[p].second ? res[p].first / res[p].second : 0});
    QDialog d(this); d.setWindowTitle("Intervention statistics"); auto *l = new QHBoxLayout(&d);
    l->addWidget(new ChartWidget("Interventions by status", a, {QColor("#2f62d9"), QColor("#e08a00"), QColor("#d92d2d"), QColor("#1f9d55")}, "", &d));
    l->addWidget(new ChartWidget("Average resolution time by priority", b, {QColor("#d92d2d"), QColor("#e08a00"), QColor("#1f9d55")}, " h", &d));
    d.exec();
}

void MainWindow::showRanking()
{
    QVector<Rank> rk = ranking(); QDialog d(this); d.setWindowTitle("Explainable scheduling"); d.resize(960, 420);
    auto *l = new QVBoxLayout(&d);
    l->addWidget(new QLabel("The ranking combines priority, delay, and equipment condition. The maintenance manager can approve the proposed order."));
    auto *t = new QTableWidget(rk.size(), 5); t->setHorizontalHeaderLabels({"Rank", "ID", "Equipment", "Score", "Reason"});
    t->verticalHeader()->hide(); t->setEditTriggers(QAbstractItemView::NoEditTriggers); t->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < rk.size(); ++i) {
        const auto &it = data[rk[i].idx];
        const QStringList c = {QString::number(i + 1), it.id, equip(it.equipId)->name, QString::number(rk[i].score), rk[i].why};
        for (int j = 0; j < 5; ++j) t->setItem(i, j, new QTableWidgetItem(c[j]));
    }
    t->setColumnWidth(2, 190); t->setColumnWidth(4, 520); l->addWidget(t);
    auto *bb = new QDialogButtonBox; bb->addButton("Approve order", QDialogButtonBox::AcceptRole);
    bb->addButton("Close", QDialogButtonBox::RejectRole);
    connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept); connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    l->addWidget(bb);
    if (d.exec() != QDialog::Accepted) return;
    proposed.clear(); for (const auto &r : rk) proposed << data[r.idx].id;
    fSort->setCurrentIndex(4); refresh();
}

void MainWindow::showPreventive()
{   // I2: next service date = last completed intervention + maintenance interval.
    int k = idxOf(selId); if (k < 0) return;
    const QString equipmentId = data[k].equipId;
    const Equipment *e = equip(equipmentId); QDateTime last;
    if (e->intervalDays <= 0) {
        showMessage(this, QMessageBox::Information, "Preventive maintenance due date",
                    "A maintenance interval is not recorded for this equipment.");
        return;
    }
    for (const auto &it : data)
        if (it.equipId == equipmentId && it.status == "Completed" && it.realEnd.isValid() && (!last.isValid() || it.realEnd > last)) last = it.realEnd;
    if (!last.isValid()) { showMessage(this, QMessageBox::Information, "Preventive maintenance due date", "No maintenance history is available for " + e->name + "."); return; }
    QDateTime next = last.addDays(e->intervalDays); qint64 left = QDateTime::currentDateTime().daysTo(next);
    QString txt = QString("%1 — last service: %2\nMaintenance interval: %3 days\nProposed next service: %4 (%5)\n\nWould you like to schedule this intervention?")
                      .arg(e->name, englishLocale().toString(last.date(), "MMM d, yyyy"), QString::number(e->intervalDays),
                           englishLocale().toString(next.date(), "MMM d, yyyy"),
                           left >= 0 ? QString("in %1 days").arg(left) : QString("%1 days overdue").arg(-left));
    if (!askChoice(this, "Preventive maintenance due date", txt, "Schedule")) return;
    if (!allowMaintenanceWrite()) return;
    Intervention it;
    it.equipId = equipmentId; it.title = "Preventive maintenance — " + e->name;
    it.priority = "Medium"; it.status = "Planned"; it.desc = "Proposed preventive maintenance";
    for (const auto &t : techs) if (t.active && t.certified) { it.tech = t.name; break; }
    it.plannedStart = next > QDateTime::currentDateTime() ? next : QDateTime::currentDateTime().addDays(1);
    it.plannedEnd = it.plannedStart.addSecs(7200);
    QString error;
    if (databaseBacked) {
        if (!maintenanceModel.insert(it, &error)) {
            showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
            return;
        }
    } else {
        int n = 0; for (const auto &x : data) n = qMax(n, x.id.section('-', -1).toInt());
        it.id = QString("MT-%1-%2").arg(QDate::currentDate().year()).arg(n + 1, 3, 10, QChar('0'));
    }
    data.prepend(it); selId = it.id; refresh();
}
