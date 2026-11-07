#include "apptheme.h"
#include "mainwindow.h"
#include "connection.h"
#include <QtWidgets>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPrinter>
#include <QTextDocument>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QInputDialog>
#include <QFile>
#include <algorithm>

// =====================================================================
//  1. UI HELPERS (badges, avatars, logo, charts)
// =====================================================================
namespace {

void drawMaintenanceIcon(QPainter &p, const QString &name, const QColor &color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    if (name == "export") {
        p.drawLine(12, 2, 12, 15); p.drawLine(7, 10, 12, 15); p.drawLine(12, 15, 17, 10);
        p.drawLine(3, 16, 3, 21); p.drawLine(3, 21, 21, 21); p.drawLine(21, 21, 21, 16);
    } else if (name == "chart") {
        p.drawLine(3, 21, 21, 21); p.drawLine(4, 21, 4, 4);
        p.drawRoundedRect(QRectF(7,13,3,6),1,1); p.drawRoundedRect(QRectF(12,9,3,10),1,1); p.drawRoundedRect(QRectF(17,5,3,14),1,1);
    } else if (name == "edit") {
        path.moveTo(4,17); path.lineTo(15,6); path.lineTo(19,10); path.lineTo(8,21); path.lineTo(3,22); path.closeSubpath();
        p.drawPath(path); p.drawLine(13,8,17,12); p.drawLine(4,17,8,21); p.drawLine(3,22,8,21);
    } else if (name == "delete") {
        p.drawLine(4,7,20,7); p.drawLine(9,4,15,4); p.drawLine(6,8,7,21); p.drawLine(18,8,17,21);
        p.drawLine(7,21,17,21); p.drawLine(10,10,10,18); p.drawLine(14,10,14,18);
    } else if (name == "plus") {
        p.drawLine(3,12,21,12); p.drawLine(12,3,12,21);
    } else if (name == "tool") {
        path.moveTo(15,3); path.cubicTo(6,0,8,10,10,11); path.lineTo(2,19); path.lineTo(5,22); path.lineTo(14,13);
        path.cubicTo(23,15,25,6,21,4); path.lineTo(17,8); path.lineTo(14,6); path.closeSubpath(); p.drawPath(path);
    } else if (name == "calendar") {
        p.drawRoundedRect(QRectF(3,5,18,17),1,1); p.drawLine(3,10,21,10); p.drawLine(7,2,7,7); p.drawLine(17,2,17,7);
        p.drawLine(8,14,10,14); p.drawLine(14,14,16,14); p.drawLine(8,18,10,18);
    } else if (name == "warning") {
        path.moveTo(12,2); path.lineTo(23,21); path.lineTo(1,21); path.closeSubpath(); p.drawPath(path);
        p.drawLine(12,8,12,14); p.drawPoint(12,17);
    } else if (name == "check") {
        p.drawLine(4,12,10,18); p.drawLine(10,18,21,6);
    } else if (name == "clock") {
        p.drawEllipse(QRectF(2,2,20,20)); p.drawLine(12,5,12,12); p.drawLine(12,12,17,15);
    } else if (name == "search") {
        p.drawEllipse(QRectF(3,3,13,13)); p.drawLine(15,15,22,22);
    } else if (name == "gear") {
        p.drawEllipse(QRectF(5,5,14,14)); p.drawEllipse(QRectF(9,9,6,6));
        for (int i=0;i<8;++i) { const qreal a=i*M_PI/4; p.drawLine(QPointF(12+8*qCos(a),12+8*qSin(a)),QPointF(12+11*qCos(a),12+11*qSin(a))); }
    }
}

QIcon maintenanceIcon(const QString &name, const QColor &color = QColor("#091b40"))
{
    QPixmap pixmap(48,48); pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap); painter.scale(2,2); drawMaintenanceIcon(painter,name,color); painter.end();
    pixmap.setDevicePixelRatio(2); return QIcon(pixmap);
}

class MaintenanceGlyph final : public QWidget {
public:
    MaintenanceGlyph(QString symbol, QColor accent, QWidget *parent = nullptr)
        : QWidget(parent), symbolName(std::move(symbol)), accentColor(std::move(accent)) { setFixedSize(48,48); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const bool dark=AppTheme::dark();
        QColor fill=accentColor; fill.setAlpha(dark?56:30);
        QColor edge=accentColor; edge.setAlpha(dark?170:95);
        const QColor ink=dark?accentColor.lighter(125):accentColor.darker(118);
        p.setPen(QPen(edge,1.4)); p.setBrush(fill); p.drawEllipse(rect().adjusted(1,1,-1,-1));
        p.save(); p.translate(12,12); drawMaintenanceIcon(p,symbolName,ink); p.restore();
    }
private:
    QString symbolName;
    QColor accentColor;
};

class MaintenanceRing final : public QWidget {
public:
    explicit MaintenanceRing(QColor accent, QWidget *parent = nullptr) : QWidget(parent), color(std::move(accent)) { setFixedSize(56,56); }
    void setProgress(qreal value) { progress=qBound(0.0,value,1.0); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF ring=QRectF(rect()).adjusted(5,5,-5,-5);
        QColor track=color; track.setAlpha(AppTheme::dark()?55:38);
        p.setBrush(Qt::NoBrush); p.setPen(QPen(track,5,Qt::SolidLine,Qt::RoundCap)); p.drawEllipse(ring);
        if (progress>0) {
            QConicalGradient sweep(ring.center(),90); sweep.setColorAt(0,color.lighter(135)); sweep.setColorAt(1,color);
            p.setPen(QPen(QBrush(sweep),5,Qt::SolidLine,Qt::RoundCap)); p.drawArc(ring,90*16,-qRound(360*16*progress));
        }
        QFont f("Segoe UI"); f.setPixelSize(11); f.setWeight(QFont::DemiBold); p.setFont(f);
        p.setPen(AppTheme::dark()?color.lighter(150):color.darker(115));
        p.drawText(rect(),Qt::AlignCenter,QString("%1%").arg(qRound(progress*100)));
    }
private:
    QColor color;
    qreal progress=0;
};

void setRingProgress(QWidget *widget, qreal value)
{
    if (auto *ring=dynamic_cast<MaintenanceRing *>(widget)) ring->setProgress(value);
}

class MaintenanceStatusBadge final : public QWidget {
public:
    explicit MaintenanceStatusBadge(const QString &value, QWidget *parent = nullptr) : QWidget(parent), status(value) {
        setFixedSize(140,34); setAccessibleName(value);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const bool overdue=status=="Overdue", active=status=="In Progress", done=status=="Completed";
        QColor dot=overdue?QColor("#ef4444"):active?QColor("#f59e0b"):done?QColor("#22c55e"):QColor("#4c9dff");
        QColor fill,ink,edge;
        if (AppTheme::dark()) {
            fill=overdue?QColor("#452332"):active?QColor("#40351e"):done?QColor("#143e35"):QColor("#193752");
            ink=overdue?QColor("#ffb7c1"):active?QColor("#ffe09a"):done?QColor("#91f5c4"):QColor("#acd8ff");
            edge=ink; edge.setAlpha(115);
        } else {
            fill=overdue?QColor("#fee2e2"):active?QColor("#fef3c7"):done?QColor("#dcfce7"):QColor("#dbeafe");
            ink=overdue?QColor("#b91c1c"):active?QColor("#92400e"):done?QColor("#15803d"):QColor("#1d4ed8");
            edge=overdue?QColor("#fecaca"):active?QColor("#f3d58b"):done?QColor("#b9efd2"):QColor("#bfdbfe");
        }
        const QRectF pill=QRectF(rect()).adjusted(.5,.5,-.5,-.5);
        p.setPen(QPen(edge,1)); p.setBrush(fill); p.drawRoundedRect(pill,17,17);
        p.setPen(Qt::NoPen); p.setBrush(dot); p.drawEllipse(QPointF(15,pill.center().y()),4,4);
        QFont f("Segoe UI",10,QFont::DemiBold); p.setFont(f); p.setPen(ink);
        p.drawText(pill.adjusted(24,0,-8,0),Qt::AlignCenter,status);
    }
private:
    QString status;
};

class MaintenanceTable final : public QTableWidget {
public:
    MaintenanceTable() : QTableWidget(0,8) {
        setObjectName("maintenanceTableWidget");
        verticalHeader()->hide();
        verticalHeader()->setDefaultSectionSize(52);
        horizontalHeader()->setFixedHeight(40);
        horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        horizontalHeader()->setSectionsClickable(true);
        setShowGrid(false);
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    void updateTableGeometry() {
        const int base[]{120,260,170,105,175,150,94,84};
        const bool narrow=viewport()->width()<1050;
        setColumnHidden(6,narrow);
        int total=0;
        for(int c=0;c<8;++c) if(!isColumnHidden(c)) total+=base[c];
        const int extra=qMax(0,viewport()->width()-total);
        const int flex[]{1,2,4,5,6};
        const int share[]{30,18,22,22,8};
        int weightTotal=0; for(int i=0;i<5;++i) if(!isColumnHidden(flex[i])) weightTotal+=share[i];
        int allocated=0,lastVisible=-1;
        for(int i=0;i<5;++i) if(!isColumnHidden(flex[i])) lastVisible=i;
        for(int i=0;i<5;++i) {
            if(isColumnHidden(flex[i])) continue;
            const int add=(i==lastVisible)?extra-allocated:(extra*share[i])/qMax(1,weightTotal);
            setColumnWidth(flex[i],base[flex[i]]+add); allocated+=add;
        }
        for(int c : {0,3,7}) setColumnWidth(c,base[c]);
        setFixedHeight(40+qMax(2,qMin(rowCount(),15))*52+18);
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QTableWidget::resizeEvent(event);
        updateTableGeometry();
    }
};

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
QString healthText(int h) { return h < 0 ? "Not recorded" : h >= 70 ? "Good" : (h >= 40 ? "Degraded" : "Critical"); }

QLocale englishLocale() { return QLocale(QLocale::English, QLocale::UnitedStates); }

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
    p.setBrush(AppTheme::brush(p, QColor::fromHsv(int(qHash(name) % 360), 120, 200))); p.setPen(AppTheme::pen(p, Qt::NoPen));
    p.drawEllipse(0, 0, s, s); p.setPen(AppTheme::pen(p, Qt::white));
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
        p.fillRect(rect(),AppTheme::brush(p,  gradient));
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
            p.fillRect(rect(),AppTheme::brush(p,  QColor("#071a31")));
        }
        QLinearGradient shade(0, 0, 0, height());
        shade.setColorAt(0.0, QColor(5, 20, 39, 238));
        shade.setColorAt(0.42, QColor(5, 20, 39, 205));
        shade.setColorAt(0.72, QColor(5, 20, 39, 120));
        shade.setColorAt(1.0, QColor(5, 20, 39, 78));
        p.fillRect(rect(),AppTheme::brush(p,  shade));
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
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(),AppTheme::brush(p,  Qt::white));
        QFont f = font(); f.setBold(true); f.setPointSize(11); p.setFont(f);
        p.setPen(AppTheme::pen(p, QColor("#0b1437"))); p.drawText(14, 26, title);
        f.setBold(false); f.setPointSize(9); p.setFont(f);
        double mx = 1; for (const auto &x : d) mx = qMax(mx, x.second);
        int n = qMax(1, int(d.size())), top = 60, bottom = 34, left = 16;
        int w = width() - 2 * left, h = height() - top - bottom, slot = w / n;
        for (int i = 0; i < d.size(); ++i) {
            int bh = int(h * d[i].second / mx);
            QRect r(left + i * slot + slot / 6, top + h - bh, slot * 2 / 3, bh);
            p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, col[i % col.size()])); p.drawRoundedRect(r, 6, 6);
            p.setPen(AppTheme::pen(p, QColor("#374151")));
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
                       const Intervention *it, bool oracleMode, QWidget *parent)
        : QDialog(parent), eqs(eq), oracleMode(oracleMode) {
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
        cTech = new QComboBox;
        for (const auto &t : techs)
            if (t.active && (t.certified || oracleMode)) cTech->addItem(t.name, t.id);
        cTech->setEditable(!oracleMode);
        cTech->setInsertPolicy(QComboBox::NoInsert);
        if (cTech->lineEdit()) {
            cTech->lineEdit()->setMaxLength(120);
            cTech->lineEdit()->setPlaceholderText("Choose or type a technician name");
        }
        cTech->setCurrentIndex(-1);
        cPr = new QComboBox; cPr->addItems({"High", "Medium", "Low"});
        cSt = new QComboBox; cSt->addItems({"Planned", "In Progress", "Completed"});
        const QDateTime actualNow = QDateTime::currentDateTime();
        const QDateTime planned = actualNow.addDays(1);
        dPS = new QDateTimeEdit(planned); dPE = new QDateTimeEdit(planned.addSecs(7200));
        dRS = new QDateTimeEdit(actualNow); dRE = new QDateTimeEdit(actualNow);
        for (auto *d : {dPS, dPE, dRS, dRE}) { d->setCalendarPopup(true); d->setDisplayFormat("dd/MM/yyyy HH:mm"); }
        eTitle = new QLineEdit; eTitle->setObjectName("maintenanceInterventionTitle");
        eTitle->setPlaceholderText("Enter intervention title"); eTitle->setMaxLength(150);
        eDesc = new QLineEdit; eDesc->setMaxLength(500); eRep = new QPlainTextEdit; eRep->setFixedHeight(70);
        f->addRow("Intervention title", eTitle);
        f->addRow("Equipment", cEq); f->addRow("Associated station", lSt); f->addRow("Technician", cTech);
        f->addRow("Priority", cPr); f->addRow("Status", cSt); f->addRow("Planned start", dPS);
        f->addRow("Planned end", dPE); f->addRow("Actual start", dRS); f->addRow("Actual end", dRE);
        f->addRow("Description", eDesc); f->addRow("Work report", eRep);
        if (oracleMode) {
            cEq->setEnabled(false);
            cEq->setToolTip("The approved Oracle schema has no maintenance-to-equipment relationship.");
            lSt->setText("Not linked in the approved Oracle schema");
            dPE->setEnabled(false);
            dRS->setEnabled(false);
            if (QLabel *label = qobject_cast<QLabel *>(f->labelForField(cEq))) label->setText("Equipment (not stored)");
            if (QLabel *label = qobject_cast<QLabel *>(f->labelForField(dPE))) label->setText("Planned end (not stored)");
            if (QLabel *label = qobject_cast<QLabel *>(f->labelForField(dRS))) label->setText("Actual start (not stored)");
            dPE->setToolTip("The approved Oracle schema stores DATE_PREVUE only.");
            dRS->setToolTip("The approved Oracle schema stores DATE_REALISEE only.");
            cTech->setToolTip("Saved through the MAINTENANCE–REALISER–PERSONNEL relationship.");
        }
        auto sync = [this] {
            int i = cEq->currentIndex(); if (i >= 0) lSt->setText(eqs[i].station);
            QString s = cSt->currentText();
            dRS->setEnabled(!this->oracleMode && s != "Planned");
            dRE->setEnabled(s == "Completed");
            eRep->setEnabled(s == "Completed");
        };
        connect(cEq, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sync);
        connect(cSt, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sync);
        if (it) {
            cEq->setCurrentIndex(qMax(0, cEq->findData(it->equipId))); cTech->setCurrentText(it->tech);
            if (!cTech->isEditable()) {
                int technicianIndex = cTech->findData(it->technicianId);
                if (technicianIndex < 0 && it->technicianId > 0) {
                    cTech->addItem(it->tech, it->technicianId);
                    technicianIndex = cTech->count() - 1;
                }
                cTech->setCurrentIndex(technicianIndex);
            }
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
        r.equipId = oracleMode ? QString() : cEq->currentData().toString();
        r.title = eTitle->text().trimmed(); r.tech = cTech->currentText().trimmed();
        r.technicianId = oracleMode ? cTech->currentData().toInt() : 0;
        if (r.technicianId > 0) r.technicianIds.append(r.technicianId);
        r.priority = cPr->currentText(); r.status = s; r.desc = eDesc->text().trimmed();
        r.plannedStart = dPS->dateTime();
        r.plannedEnd = oracleMode ? r.plannedStart : dPE->dateTime();
        r.report = eRep->toPlainText().trimmed();
        if (!oracleMode && s != "Planned") r.realStart = dRS->dateTime();
        if (s == "Completed") r.realEnd = dRE->dateTime();
        return r;
    }
private:
    bool validate() {
        QString s = cSt->currentText(), err;
        if (eTitle->text().trimmed().isEmpty())        err = "Enter a title for this intervention.";
        else if (cTech->currentText().trimmed().isEmpty()
                 || (oracleMode && cTech->currentData().toInt() <= 0)) err = "Choose an active staff member for this intervention.";
        else if (!oracleMode && dPE->dateTime() < dPS->dateTime()) err = "The planned end must be at or after the planned start.";
        else if (!oracleMode && s == "Completed" && dRE->dateTime() < dRS->dateTime()) err = "The actual end must be at or after the actual start.";
        else if (s == "Completed" && eRep->toPlainText().trimmed().isEmpty()) err = "A work report is required to complete an intervention.";
        if (!err.isEmpty()) { showMessage(this, QMessageBox::Warning, "Review required", err); return false; }
        return true;
    }
    QVector<Equipment> eqs;
    QComboBox *cEq, *cTech, *cPr, *cSt; QLabel *lSt;
    QDateTimeEdit *dPS, *dPE, *dRS, *dRE; QLineEdit *eTitle, *eDesc; QPlainTextEdit *eRep;
    bool oracleMode = false;
};

const char *QSS = R"(
*{font-family:'Segoe UI';font-size:13px;color:#091b40;}
QDialog{background:#f7f9fd;color:#17243a;}
QDialog QLabel{color:#34445c;background:transparent;}
QDialog#interventionDialog QLabel{color:#26364f;font-weight:600;}
#page{background:#f5f9ff;}
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
#card{background:white;border:1px solid #e0ebf8;border-radius:12px;} #card QLabel,#card QCheckBox{background:transparent;}
QLabel#maintenanceTitle{font-size:34px;font-weight:700;color:#080d2d;background:transparent;}
QLabel#maintenanceSubtitle{font-size:16px;color:#5d77ad;background:transparent;}
QLineEdit,QComboBox,QDateTimeEdit,QPlainTextEdit{background:#ffffff;color:#17366f;border:1px solid #d7e5f7;border-radius:7px;padding:7px 12px;font-size:13px;selection-background-color:#d6ecff;selection-color:white;}
QComboBox QAbstractItemView{background:white;color:#17243a;selection-background-color:#e3edff;selection-color:#153f88;outline:0;}
QLineEdit:disabled,QComboBox:disabled,QDateTimeEdit:disabled,QPlainTextEdit:disabled{background:#f0f3f7;color:#56647a;border-color:#d7dee9;}
QCheckBox{color:#24334c;spacing:7px;}
QPushButton,QToolButton{background:white;border:1px solid #d6e5f7;border-radius:8px;padding:7px 14px;font-size:13px;color:#091b40;}
QPushButton:hover,QToolButton:hover{background:#e3edff;border:1px solid #6b9cff;color:#153f88;}
QPushButton:pressed,QToolButton:pressed{background:#c5d9ff;border:1px solid #1f6fe5;color:#103a83;}
QPushButton[clickFlash="true"],QToolButton[clickFlash="true"]{background:#c5d9ff;border:1px solid #1f6fe5;color:#103a83;}
QPushButton#primary{background:#0785ff;color:white;border:1px solid #087aff;font-weight:600;} QPushButton#primary:hover{background:#006fe0;border:1px solid #7eaeff;color:white;} QPushButton#primary[clickFlash="true"]{background:#005bc6;border:1px solid #9bc0ff;color:white;}
QPushButton#danger{color:#b42318;border-color:#f3c2c2;} QPushButton#danger:hover{background:#fde2e2;border-color:#e98d8d;color:#981b13;} QPushButton#danger[clickFlash="true"]{background:#f6caca;border-color:#d92d2d;color:#8e1710;}
QPushButton#nav[clickFlash="true"]{background:#326fe0;color:white;}
QToolButton::menu-indicator{image:none;}
QTableWidget{background:white;border:none;color:#17243a;selection-background-color:#dfeaff;selection-color:#111827;font-size:13px;}
QTableWidget::item{border-bottom:1px solid #eef1f6;}
QHeaderView::section{background:#ffffff;border:none;border-bottom:1px solid #e0ebf8;padding:9px 8px;font-weight:600;color:#17366f;font-size:12px;}
QTableWidget QWidget{background:transparent;}
QMenu{background:white;color:#17243a;border:1px solid #dde3ee;padding:6px;} QMenu::item{padding:8px 22px;border-radius:6px;} QMenu::item:selected{background:#eef3ff;color:#153f88;}
/* Shared management-page rhythm: clear title row, accent metrics, filters, and a clean data surface. */
#page{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #f7faff,stop:0.52 #f1f7ff,stop:1 #f8fbff);}
#card{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ffffff,stop:1 #f9fbff);border:1px solid #e0ebf8;border-radius:14px;}
QPushButton,QToolButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ffffff,stop:0.45 #fbfdff,stop:1 #edf4fc);border:1px solid #d2e1f2;border-radius:9px;padding:0 14px;}
QPushButton:hover,QToolButton:hover{background:#e5f2ff;border:1px solid #91c5f8;}
QPushButton#primary{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #4eb3ff,stop:0.18 #168cff,stop:1 #0061dd);border:1px solid #087aff;border-top-color:#84ceff;border-bottom-color:#0054c1;border-radius:9px;color:white;font-weight:600;}
QPushButton#primary:hover{background:#168fff;border-color:#61b5ff;color:white;}
QToolButton#editMaintenanceRowButton,QToolButton#deleteMaintenanceRowButton{background:transparent;border:1px solid transparent;border-radius:6px;padding:0;}
QToolButton#editMaintenanceRowButton:hover{background:#eef6ff;border-color:#c9e2ff;}
QToolButton#deleteMaintenanceRowButton:hover{background:#fff0f0;border-color:#ffd2d2;}
QToolButton#editMaintenanceRowButton:pressed{background:#dceeff;}
QToolButton#deleteMaintenanceRowButton:pressed{background:#ffe0e0;}
QLabel#maintenanceResultCount{color:#5d77ad;font-size:12px;font-weight:600;}
QFrame#kpi{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #ffffff,stop:1 #f7faff);border:1px solid #e0ebf8;border-top:3px solid #56aaff;border-radius:14px;}
QFrame#kpi[tone="blue"]{border-top-color:#56aaff;} QFrame#kpi[tone="red"]{border-top-color:#ff7486;} QFrame#kpi[tone="green"]{border-top-color:#41c895;} QFrame#kpi[tone="purple"]{border-top-color:#9b7bff;}
QLabel#maintenanceKpiIcon{background:#eaf5ff;color:#0878df;border:1px solid #cce6ff;border-radius:24px;font-size:20px;font-weight:700;}
QLabel#maintenanceKpiIcon[tone="red"]{background:#fff0f1;color:#d92d2d;border-color:#ffd0d4;} QLabel#maintenanceKpiIcon[tone="green"]{background:#e8fbf1;color:#087f4f;border-color:#b9efd2;} QLabel#maintenanceKpiIcon[tone="purple"]{background:#f4f0ff;color:#7948df;border-color:#ddd2ff;}
QLabel#maintenanceKpiTitle{font-size:13px;font-weight:600;color:#173052;}
QLabel#maintenanceKpiSub{font-size:12px;font-weight:600;color:#476388;}
QLabel#maintenanceKpiSub[tone="red"]{color:#d92d2d;} QLabel#maintenanceKpiSub[tone="green"]{color:#087f4f;} QLabel#maintenanceKpiSub[tone="purple"]{color:#7948df;}
QTableWidget{background:#ffffff;border:1px solid #dce8f6;border-radius:10px;color:#17243a;selection-background-color:#e3f1ff;selection-color:#0b3971;font-size:13px;}
QTableWidget::item{border-bottom:1px solid #edf2f9;padding:4px 8px;}
QTableWidget::item:hover{background:#f1f8ff;}
QHeaderView::section{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #fbfdff,stop:1 #f0f6fd);border:none;border-bottom:1px solid #dce8f6;padding:10px 8px;font-weight:700;color:#17366f;font-size:12px;}
)";
} // namespace

// =====================================================================
//  3. BUILD THE INTERFACE
// =====================================================================
MainWindow::MainWindow(QWidget *parent, bool embedded) : QWidget(parent), embeddedMode(embedded)
{
    seedData();
    setObjectName("maintenanceWidget");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    maintenancePage = buildMaintenancePage();
    if (embeddedMode) { auto *topBar = buildTopBar(); topBar->hide(); layout->addWidget(topBar); }
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(maintenancePage);
    layout->addWidget(scroll, 1);
    QFile managementTheme(":/styles/equipment.qss");
    QString pageStyle=QString::fromUtf8(QSS);
    if (managementTheme.open(QIODevice::ReadOnly))
        pageStyle += "\n" + QString::fromUtf8(managementTheme.readAll());
    setStyleSheet(pageStyle);
    qApp->installEventFilter(this);
    QTimer::singleShot(0, this, [this] { animateMaintenancePage(); });

    if (Connection::createInstance().isConnected()) {
        QVector<Intervention> storedInterventions;
        QVector<Technician> storedTechnicians;
        QString error;
        if (maintenanceModel.loadAll(storedInterventions, &error)
            && maintenanceModel.loadTechnicians(storedTechnicians, &error)) {
            data = storedInterventions;
            techs = storedTechnicians;
            eq.clear();
            databaseBacked = true;
            ensureOracleLookupData();
            selId = data.isEmpty() ? QString() : data.first().id;
            if (auto *actionsTitle = findChild<QLabel *>("maintenanceActionsTitle"))
                actionsTitle->setText("Checklist (not stored in Oracle)");
            for (QCheckBox *action : findChildren<QCheckBox *>())
                action->setEnabled(false);
        } else {
            // Do not show editable demo rows when Oracle is connected but its
            // Maintenance schema has not been installed or cannot be read.
            data.clear();
            selId.clear();
            databaseSetupRequired = true;
            if (saveDetailsButton) saveDetailsButton->setEnabled(false);
            if (completeInterventionButton) completeInterventionButton->setEnabled(false);
            QTimer::singleShot(0, this, [this, error] {
                showMessage(this, QMessageBox::Warning, "Maintenance tables not ready",
                    "Oracle is connected, but the approved MAINTENANCE, REALISER, or PERSONNEL tables could not be read. "
                    "The integrated module uses the schema in database/skysense_schema_profa.sql.\n\n"
                    "Do not create a separate MAINTENANCE_INTERVENTION table. Check the Oracle error and the existing "
                    "table columns in SQL Developer.\n\n"
                    "Oracle message: " + error);
            });
        }
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
    auto *bar = new QFrame; bar->setObjectName("bar"); bar->setFixedHeight(80);
    auto *l = new QHBoxLayout(bar);
    auto *gs = new QLineEdit; gs->setObjectName("maintenanceGlobalSearch"); gs->setPlaceholderText("🔍 Search by intervention title, equipment, station, or technician…");
    gs->setMinimumWidth(300); gs->setMaximumWidth(680); gs->setMinimumHeight(48);
    l->addWidget(gs); l->addStretch();
    l->addWidget(new QLabel("📅  " + englishLocale().toString(QDate::currentDate(), "dddd, MMMM d, yyyy")));
    auto *av = new QLabel; av->setPixmap(avatar("Alex Martin", 38)); l->addWidget(av);
    auto *user = new QLabel("<b>Alex Martin</b><br><small style='color:#65728a'>Operations Manager</small>");
    user->setObjectName("maintenanceOperatorName");
    l->addWidget(user);
    connect(gs, &QLineEdit::textChanged, this, [this](const QString &t) { search->setText(t); });
    return bar;
}

QWidget *MainWindow::buildMaintenancePage()
{
    auto *page = new QWidget; page->setObjectName("page");
    auto *root = new QVBoxLayout(page); root->setContentsMargins(24, 14, 24, 22); root->setSpacing(12);

    // ---- Header and primary actions
    auto *headSection = new QWidget(page); headSection->setObjectName("maintenanceHeader"); prepareFadeIn(headSection);
    auto *head = new QHBoxLayout(headSection); head->setContentsMargins(0, 0, 0, 0); head->setSpacing(18);
    auto *tb = new QVBoxLayout; tb->setSpacing(0);
    auto *t = new QLabel("Maintenance"); t->setObjectName("maintenanceTitle");
    auto *s = new QLabel("Plan maintenance work and track service across your weather network.");
    s->setObjectName("maintenanceSubtitle");
    prepareFadeIn(t); prepareFadeIn(s);
    tb->addWidget(t); tb->addWidget(s);
    head->addLayout(tb); head->addStretch();
    auto *actionRow = new QHBoxLayout; actionRow->setContentsMargins(0,0,0,0); actionRow->setSpacing(8);

    auto *bStats = new QPushButton("Statistics");
    bStats->setIcon(maintenanceIcon("chart")); bStats->setIconSize(QSize(20,20));
    connect(bStats, &QPushButton::clicked, this, &MainWindow::showStats);
    auto *bPdf = new QToolButton; bPdf->setText("Export PDF"); bPdf->setIcon(maintenanceIcon("export"));
    bPdf->setIconSize(QSize(20,20)); bPdf->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bPdf->setPopupMode(QToolButton::InstantPopup);
    auto *mp = new QMenu(bPdf);
    mp->addAction("Filtered list", this, &MainWindow::exportList);
    mp->addAction("Selected intervention report", this, &MainWindow::exportSheet);
    mp->addAction("Field maintenance checklist", this, &MainWindow::exportChecklist);
    bPdf->setMenu(mp);
    topDeleteButton = new QPushButton("Delete"); topDeleteButton->setObjectName("deleteSelectedMaintenanceButton");
    topDeleteButton->setIcon(maintenanceIcon("delete", QColor("#6b7280"))); topDeleteButton->setIconSize(QSize(20,20));
    topEditButton = new QPushButton("Edit"); topEditButton->setIcon(maintenanceIcon("edit")); topEditButton->setIconSize(QSize(20,20));
    connect(topDeleteButton, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(topEditButton, &QPushButton::clicked, this, &MainWindow::onEdit);
    addInterventionButton = new QPushButton("Add intervention"); addInterventionButton->setObjectName("addMaintenanceInterventionButton");
    addInterventionButton->setProperty("primary", true);
    addInterventionButton->setIcon(maintenanceIcon("plus", Qt::white)); addInterventionButton->setIconSize(QSize(20,20));
    connect(addInterventionButton, &QPushButton::clicked, this, &MainWindow::onAdd);
    for (QWidget *w : {(QWidget *)bPdf, (QWidget *)bStats, (QWidget *)topEditButton,
                       (QWidget *)topDeleteButton, (QWidget *)addInterventionButton}) {
        w->setFixedHeight(52); actionRow->addWidget(w);
    }
    bPdf->setMinimumWidth(148); bStats->setMinimumWidth(132); topEditButton->setMinimumWidth(100);
    topDeleteButton->setMinimumWidth(112); addInterventionButton->setMinimumWidth(172);
    head->addLayout(actionRow);
    root->addWidget(headSection);

    // ---- Search, filters, and sorting (B1 + B2)
    auto *filterSection = new QWidget(page); filterSection->setObjectName("maintenanceFilters"); prepareFadeIn(filterSection);
    auto *fl = new QHBoxLayout(filterSection); fl->setContentsMargins(0, 0, 0, 0);
    search = new QLineEdit; search->setPlaceholderText("Search intervention ID, title, equipment or technician...");
    search->setClearButtonEnabled(true); search->addAction(maintenanceIcon("search", QColor("#5d77ad")), QLineEdit::LeadingPosition);
    fStatus = new QComboBox; fStatus->addItems({"All statuses", "Planned", "In Progress", "Overdue", "Completed"});
    fPeriod = new QComboBox; fPeriod->addItems({"All periods", "Today", "Next 7 days", "Last 30 days"});
    fSort = new QComboBox; fSort->addItems({"Sort: due date", "Sort: priority", "Sort: status", "Sort: delay"});
    auto *clearFilters = new QPushButton("Clear filters"); clearFilters->setObjectName("clearMaintenanceFilters");
    clearFilters->setProperty("link", true);
    for (auto *control : {static_cast<QWidget *>(search), static_cast<QWidget *>(fStatus),
                          static_cast<QWidget *>(fPeriod), static_cast<QWidget *>(fSort), static_cast<QWidget *>(clearFilters)})
        control->setMinimumHeight(48);
    fl->setSpacing(10);
    fl->addWidget(search, 1); fl->addWidget(fStatus); fl->addWidget(fPeriod); fl->addWidget(fSort); fl->addWidget(clearFilters);
    root->addWidget(filterSection);
    connect(search, &QLineEdit::textChanged, this, [this] { refresh(); });
    for (auto *c : {fStatus, fPeriod, fSort})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { refresh(); });
    connect(clearFilters, &QPushButton::clicked, this, [this] {
        search->clear(); fStatus->setCurrentIndex(0); fPeriod->setCurrentIndex(0); fSort->setCurrentIndex(0);
    });

    // ---- KPI cards
    auto *kl = new QHBoxLayout; kl->setSpacing(10);
    struct K { const char *ic, *title, *tone, *symbol; QColor accent; };
    const K ks[4] = {{"tool", "Registered Interventions", "blue", "tool", QColor("#56aaff")},
                     {"calendar", "Planned", "green", "calendar", QColor("#41c895")},
                     {"warning", "Overdue", "red", "warning", QColor("#ff7486")},
                     {"check", "Completed", "purple", "check", QColor("#9b7bff")}};
    for (int i = 0; i < 4; ++i) {
        auto *f = new QFrame;
        f->setProperty("role", "metric");
        f->setProperty("tone", ks[i].tone);
        f->setProperty("maintenanceKpiIndex", i); prepareFadeIn(f);
        f->setMinimumHeight(148);
        auto *hl = new QHBoxLayout(f); hl->setContentsMargins(14,14,12,12); hl->setSpacing(12);
        hl->addWidget(new MaintenanceGlyph(ks[i].symbol,ks[i].accent),0,Qt::AlignTop);
        auto *bx = new QVBoxLayout; bx->setSpacing(4);
        auto *tt = new QLabel(ks[i].title); tt->setProperty("role","metricTitle"); bx->addWidget(tt);
        auto *numbers = new QHBoxLayout; numbers->setSpacing(6);
        kVal[i] = new QLabel("0"); kVal[i]->setObjectName("maintenanceKpiValue"); kVal[i]->setProperty("role","number");
        kTotal[i] = new QLabel; kTotal[i]->setObjectName("maintenanceKpiTotal"); kTotal[i]->setProperty("role","muted");
        numbers->addWidget(kVal[i]); numbers->addWidget(kTotal[i],1,Qt::AlignBottom); bx->addLayout(numbers);
        kSub[i] = new QLabel; kSub[i]->setObjectName("maintenanceKpiSub"); kSub[i]->setProperty("role","trend");
        kSub[i]->setProperty("tone",ks[i].tone); bx->addWidget(kSub[i]); hl->addLayout(bx,1);
        if(i>0) { kMeter[i]=new MaintenanceRing(ks[i].accent); kMeter[i]->setObjectName("maintenanceMetricRing"); hl->addWidget(kMeter[i],0,Qt::AlignVCenter); }
        kl->addWidget(f);
    }
    root->addLayout(kl);

    // ---- Interventions table
    count = new QLabel; count->setObjectName("maintenanceResultCount");
    root->addWidget(count);
    table = new MaintenanceTable;
    table->setHorizontalHeaderLabels({"ID", "Intervention / equipment", "Scheduled date", "Priority", "Technician", "Status", "Delay", "Actions"});
    auto *hh = table->horizontalHeader(); hh->setSortIndicatorShown(true); hh->setSortIndicator(2,Qt::AscendingOrder);
    connect(hh, &QHeaderView::sectionClicked, this, [this](int c) {      // B1: sort when a column header is clicked.
        static const QMap<int, int> m = {{2, 0}, {3, 1}, {5, 2}, {6, 3}};
        if (m.contains(c)) fSort->setCurrentIndex(m[c]);
    });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this] {
        int r = table->currentRow();
        if (r >= 0 && table->item(r, 0)) { selId = table->item(r, 0)->data(Qt::UserRole).toString(); showDetail(); }
    });
    root->addWidget(table);
    root->addWidget(buildDetailCard());
    return page;
}

QWidget *MainWindow::buildDetailCard()
{
    auto *section = new QWidget; section->setObjectName("maintenanceDetail");
    auto *sectionLayout = new QVBoxLayout(section); sectionLayout->setContentsMargins(0, 0, 0, 0);
    auto *d = new QFrame; d->setProperty("role","card"); addCardShadow(d); auto *dl = new QHBoxLayout(d); dl->setSpacing(18);
    auto *img = new QLabel; img->setPixmap(QPixmap(":/images/station.jpg").scaled(150, 110, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation).copy(0, 0, 150, 110)); img->setFixedSize(150, 110);
    dId = new QLabel; dId->setStyleSheet("font-size:17px;font-weight:800;");
    dSt = new QLabel; dName = new QLabel; dName->setStyleSheet("font-weight:700;font-size:14px;");
    dWhen = new QLabel; dInfo = new QLabel; dInfo->setWordWrap(true); dInfo->setStyleSheet("color:#4b5563;");
    auto *r = new QHBoxLayout; r->addWidget(dId); r->addWidget(dSt); r->addStretch(); r->addWidget(dWhen);
    auto *info = new QVBoxLayout; info->addLayout(r); info->addWidget(dName); info->addWidget(dInfo); info->addStretch();
    dl->addWidget(img); dl->addLayout(info, 3);

    auto *rc = new QVBoxLayout; auto *rt = new QLabel("Recommended actions");
    rt->setObjectName("maintenanceActionsTitle"); rt->setStyleSheet("font-weight:700;");
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
    completeInterventionButton = new QPushButton("Mark as completed"); completeInterventionButton->setObjectName("primary"); completeInterventionButton->setProperty("maintenanceCompleteAction", true);
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
    dragging = (!embeddedMode && e->button() == Qt::LeftButton && e->pos().y() < 64 && !isMaximized());
    if (dragging) dragOff = e->globalPosition().toPoint() - frameGeometry().topLeft();
}
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (!embeddedMode && dragging && (e->buttons() & Qt::LeftButton)) move(e->globalPosition().toPoint() - dragOff);
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
    if (button && !isAncestorOf(button))
        return QObject::eventFilter(watched, event);
    if (button && event->type() == QEvent::Enter) {
        button->setCursor(Qt::PointingHandCursor);
    } else if (button && event->type() == QEvent::Leave) {
        button->setProperty("clickFlash", false);
    } else if (button && event->type() == QEvent::MouseButtonPress) {
        button->setProperty("clickFlash", true);
        button->style()->unpolish(button);
        button->style()->polish(button);
    } else if (button && event->type() == QEvent::MouseButtonRelease) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton && button->underMouse()) {
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

void MainWindow::animateMaintenancePage()
{
    if (!maintenancePage) return;
    if (embeddedMode) {
        // In the integrated workspace the module is a page, so reveal it at
        // once. The standalone staggered animation made navigation feel slow
        // and left the table partially blank after switching tabs.
        ++maintenanceRevealGeneration;
        revealingRows = false;
        for (QWidget *widget : maintenancePage->findChildren<QWidget *>()) {
            if (auto *effect = qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect()))
                effect->setOpacity(1.0);
        }
        if (table)
            for (int row = 0; row < table->rowCount(); ++row)
                table->setRowHidden(row, false);
        return;
    }
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

void MainWindow::activateEmbeddedPage()
{
    if (embeddedMode)
        animateMaintenancePage();
}

// =====================================================================
//  4. MAINTENANCE DATA OPERATIONS
// =====================================================================
void MainWindow::seedData()
{
    eq={{"EQP-001","Weather Station","ALP-03 — Alpine Ridge","Vaisala WXT536",90,180},
        {"EQP-002","Anemometer","COA-01 — Coastal Bay","WindMaster Pro",55,90},
        {"EQP-003","Rain Gauge","FOR-12 — Pine Forest","RG-200",35,60},
        {"EQP-004","Pyranometer","VAL-07 — Green Valley","SMP10",85,180},
        {"EQP-005","Barometer","ALP-03 — Alpine Ridge","PTB110",65,90}};
    techs={{"Alex Martin",true,true},{"Sophie Chen",true,true},{"James Wilson",true,true},{"Emma Davis",true,true},{"Ryan Kim",true,true}};
    data.clear(); selId.clear();
    const QDateTime now=QDateTime::currentDateTime();
    auto addDemo=[&](int number,const QString &equipment,const QString &technician,const QString &priority,const QString &status,int offsetHours,const QString &title) {
        Intervention item; item.id=QString("MT-%1-%2").arg(now.date().year()).arg(number,3,10,QChar('0'));
        item.equipId=equipment; item.title=title; item.tech=technician; item.priority=priority; item.status=status; item.desc=title; item.plannedStart=now.addSecs(offsetHours*3600); item.plannedEnd=item.plannedStart.addSecs(7200);
        if(status!="Planned") item.realStart=item.plannedStart;
        if(status=="Completed") {item.realEnd=item.plannedEnd;item.report="Equipment inspected and verified.";}
        data.append(item);
    };
    addDemo(1,"EQP-001","James Wilson","High","Planned",48,"Weather station calibration");
    addDemo(2,"EQP-002","Sophie Chen","Medium","In Progress",-1,"Anemometer inspection");
    addDemo(3,"EQP-003","Emma Davis","High","Planned",-30,"Rain gauge cleaning");
    addDemo(4,"EQP-004","Ryan Kim","Low","Completed",-120,"Solar sensor check");
    addDemo(5,"EQP-005","Alex Martin","Medium","Planned",72,"Barometer service");
    selId=data.first().id;
}

void MainWindow::ensureOracleLookupData()
{
    // MAINTENANCE has no equipment foreign key in the approved schema. Keep a
    // clear display placeholder instead of implying a stored equipment link.
    for (const Intervention &item : data) {
        if (!equip(item.equipId)) {
            const bool linked = !item.equipId.trimmed().isEmpty();
            const QString label = linked ? item.equipId : QStringLiteral("No equipment link");
            eq.append({item.equipId, label, QStringLiteral("Not stored in Oracle"),
                       QStringLiteral("Approved schema"), -1, 0});
        }

        bool technicianFound = false;
        for (const Technician &technician : techs) {
            if (technician.name.compare(item.tech, Qt::CaseInsensitive) == 0) {
                technicianFound = true;
                break;
            }
        }
        if (!technicianFound && !item.tech.trimmed().isEmpty())
            techs.append({item.tech, false, false, item.technicianId});
    }
}

// =====================================================================
//  5. BUSINESS LOGIC
// =====================================================================
const Equipment *MainWindow::equip(const QString &id) const
{
    for (const auto &e : eq) if (e.id == id) return &e;
    return nullptr;
}
int MainWindow::idxOf(const QString &id) const
{
    for (int i = 0; i < data.size(); ++i) if (data[i].id == id) return i;
    return -1;
}
qint64 MainWindow::delayHours(const Intervention &it) const
{   // The approved Oracle schema stores only DATE_PREVUE, so that is the due timestamp.
    if (!it.plannedStart.isValid()) return 0;
    QDateTime ref = (it.status == "Completed" && it.realEnd.isValid()) ? it.realEnd : QDateTime::currentDateTime();
    qint64 s = it.plannedStart.secsTo(ref);
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
        QString hay = it.id + " " + it.title + " " + it.tech + " " + st + " " + it.priority;
        if (e) hay += " " + e->name + " " + e->station;
        hay = hay.toLower();
        if (!q.isEmpty() && !hay.contains(q)) continue;
        if (fStatus->currentIndex() > 0 && st != fStatus->currentText()) continue;
        int p = fPeriod->currentIndex();
        if (p == 1 && it.plannedStart.date() != now.date()) continue;
        if (p == 2 && !(it.plannedStart >= now && it.plannedStart <= now.addDays(7))) continue;
        if (p == 3 && !(it.plannedStart <= now && it.plannedStart >= now.addDays(-30))) continue;
        r << i;
    }
    int s = fSort->currentIndex();
    std::stable_sort(r.begin(), r.end(), [&](int a, int b) {
        const auto &x = data[a]; const auto &y = data[b];
        switch (s) {
        case 1:  return prioW(x.priority) > prioW(y.priority);
        case 2:  return statRank(effStatus(x)) < statRank(effStatus(y));
        case 3:  return delayHours(x) > delayHours(y);
        default: return x.plannedStart < y.plannedStart;
        }
    });
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
    int np = 0, no = 0, nc = 0;
    for (const auto &it : data) {
        QString s = effStatus(it);
        if (s == "Planned") ++np; else if (s == "Overdue") ++no;
        else if (s == "Completed") ++nc;
    }
    const int total=data.size();
    kVal[0]->setText(QString::number(total)); kTotal[0]->setText("in schedule"); kSub[0]->setText("All maintenance records");
    kVal[1]->setText(QString::number(np)); kTotal[1]->setText(QString("/ %1 total").arg(total)); kSub[1]->setText("Upcoming interventions");
    kVal[2]->setText(QString::number(no)); kTotal[2]->setText(QString("/ %1 total").arg(total)); kSub[2]->setText("Needs priority attention");
    kVal[3]->setText(QString::number(nc)); kTotal[3]->setText(QString("/ %1 total").arg(total)); kSub[3]->setText("Work orders completed");
    setRingProgress(kMeter[1], total ? qreal(np)/total : 0);
    setRingProgress(kMeter[2], total ? qreal(no)/total : 0);
    setRingProgress(kMeter[3], total ? qreal(nc)/total : 0);

    QVector<int> rows = filtered();
    if (rows.isEmpty()) selId.clear();
    count->setText(QString("Showing %1 of %2 interventions  •  Click a column heading to sort").arg(rows.size()).arg(data.size()));
    table->blockSignals(true); table->clearContents(); table->setRowCount(rows.size());
    int selRow = -1;
    for (int r = 0; r < rows.size(); ++r) {
        table->setRowHidden(r, false);
        const Intervention &it = data[rows[r]]; const Equipment *e = equip(it.equipId); QString st = effStatus(it);
        auto *idi = new QTableWidgetItem(it.id); idi->setData(Qt::UserRole, it.id);
        QFont idFont=table->font(); idFont.setWeight(QFont::DemiBold); idi->setFont(idFont); table->setItem(r, 0, idi);
        for (int c = 1; c < 8; ++c) table->setItem(r, c, new QTableWidgetItem);
        const QString shownTitle=it.title.trimmed().isEmpty()?QStringLiteral("Title not recorded"):it.title.trimmed();
        const QString location = databaseBacked
                                     ? QStringLiteral("Equipment and station not linked in Oracle")
                                     : e ? "Equipment " + e->name + " · Station " + e->station
                                         : QStringLiteral("Equipment not linked");
        table->setCellWidget(r, 1, twoLines(shownTitle, location, true));
        const QString scheduledDate = it.plannedStart.isValid()
                                          ? englishLocale().toString(it.plannedStart.date(), "MMM d, yyyy")
                                          : QStringLiteral("Date not set");
        const QString scheduledTime = !it.plannedStart.isValid() ? QStringLiteral("—")
                                         : databaseBacked ? it.plannedStart.toString("HH:mm")
                                                          : it.plannedStart.toString("HH:mm") + " – " + it.plannedEnd.toString("HH:mm");
        table->setCellWidget(r, 2, twoLines(scheduledDate, scheduledTime, false));
        table->setCellWidget(r, 3, center(badge(it.priority, prioColor(it.priority))));
        auto *tc = new QWidget; auto *tl = new QHBoxLayout(tc); tl->setContentsMargins(6, 0, 6, 0);
        auto *av = new QLabel; av->setPixmap(avatar(it.tech, 30)); tl->addWidget(av); tl->addWidget(new QLabel(it.tech)); tl->addStretch();
        table->setCellWidget(r, 4, tc);
        table->setCellWidget(r, 5, center(new MaintenanceStatusBadge(st)));
        qint64 dh = delayHours(it); auto *dl = new QLabel(dh ? QString("+%1 d %2 h").arg(dh / 24).arg(dh % 24) : "—");
        if (dh) dl->setStyleSheet("color:#d92d2d;font-weight:600;");
        table->setCellWidget(r, 6, center(dl));
        auto *rowActions=new QWidget; auto *actionsLayout=new QHBoxLayout(rowActions);
        actionsLayout->setContentsMargins(2,0,2,0); actionsLayout->setSpacing(4);
        auto *editAction=new QToolButton; editAction->setObjectName("editMaintenanceRowButton");
        editAction->setIcon(maintenanceIcon("edit",QColor("#0785ff"))); editAction->setIconSize(QSize(18,18));
        editAction->setFixedSize(30,30); editAction->setToolTip("Edit intervention"); editAction->setAccessibleName("Edit intervention");
        auto *deleteAction=new QToolButton; deleteAction->setObjectName("deleteMaintenanceRowButton");
        deleteAction->setIcon(maintenanceIcon("delete",QColor("#e5484d"))); deleteAction->setIconSize(QSize(18,18));
        deleteAction->setFixedSize(30,30); deleteAction->setToolTip("Delete intervention"); deleteAction->setAccessibleName("Delete intervention");
        const QString id=it.id;
        connect(editAction,&QToolButton::clicked,this,[this,id]{selId=id;onEdit();});
        connect(deleteAction,&QToolButton::clicked,this,[this,id]{selId=id;onDelete();});
        actionsLayout->addWidget(editAction); actionsLayout->addWidget(deleteAction); actionsLayout->addStretch();
        table->setCellWidget(r,7,rowActions);
        if (it.id == selId) selRow = r;
    }
    if (selRow < 0 && !rows.isEmpty()) { selRow = 0; selId = data[rows[0]].id; }
    if (selRow >= 0) table->selectRow(selRow);
    table->blockSignals(false);
    if (auto *maintenanceTable=dynamic_cast<MaintenanceTable *>(table))
        maintenanceTable->updateTableGeometry();
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
        for (int i = 0; i < 4; ++i) { QSignalBlocker blocker(acts[i]); acts[i]->setChecked(false); acts[i]->setEnabled(false); }
        if (notes) { QSignalBlocker blocker(notes); notes->clear(); notes->setEnabled(false); }
        return;
    }
    if (topEditButton) topEditButton->setEnabled(true);
    if (topDeleteButton) topDeleteButton->setEnabled(true);
    const Intervention &it = data[k]; const Equipment *e = equip(it.equipId); QString st = effStatus(it);
    if (completeInterventionButton) completeInterventionButton->setEnabled(st != "Completed" && !databaseSetupRequired);
    for (QCheckBox *action : acts) if (action) action->setEnabled(!databaseSetupRequired && !databaseBacked);
    if (notes) notes->setEnabled(!databaseSetupRequired);
    if (saveDetailsButton) {
        const bool dirty = dirtyDetailIds.contains(it.id);
        saveDetailsButton->setEnabled(databaseBacked && dirty);
        const QString label = databaseBacked ? QStringLiteral("Save work report") : QStringLiteral("Save notes & actions");
        saveDetailsButton->setText(dirty ? label + " *" : label);
    }
    auto c = statColor(st);
    dId->setText(it.id); dSt->setText(st);
    dSt->setStyleSheet(QString("background:%1;color:%2;border-radius:10px;padding:2px 10px;font-weight:600;").arg(c.first, c.second));
    dName->setText(it.title.trimmed().isEmpty()
                       ? QStringLiteral("Intervention title: not recorded")
                       : QStringLiteral("Intervention title: %1").arg(it.title));
    const QString plannedText = it.plannedStart.isValid()
                                    ? englishLocale().toString(it.plannedStart, "dddd, MMMM d, yyyy HH:mm")
                                    : QStringLiteral("Not scheduled");
    if (databaseBacked) {
        dWhen->setText(QStringLiteral("📅 Planned: %1\n✅ Completed: %2")
                           .arg(plannedText, it.realEnd.isValid()
                                                ? englishLocale().toString(it.realEnd, "dddd, MMMM d, yyyy HH:mm")
                                                : QStringLiteral("—")));
        dInfo->setText(QStringLiteral("Equipment and station are not linked in the approved Oracle schema.\n"
                                      "📝 %1\n👤 Assigned technician: %2")
                           .arg(it.desc.isEmpty() ? QStringLiteral("No description") : it.desc,
                                it.tech.isEmpty() ? QStringLiteral("Not assigned") : it.tech));
    } else {
        dWhen->setText(QString("📅 %1\n%2 – %3").arg(englishLocale().toString(it.plannedStart.date(), "dddd, MMMM d, yyyy"),
                       it.plannedStart.toString("HH:mm"), it.plannedEnd.toString("HH:mm")));
        dInfo->setText(QString("📍 Station %1   🛠 %2 — %3\n📝 %4\n👤 %5   •   Equipment condition score: %6")
                       .arg(e->station, e->name, e->model, it.desc, it.tech,
                            e->health < 0 ? QStringLiteral("not stored")
                                          : QStringLiteral("%1 (%2%)").arg(healthText(e->health)).arg(e->health)));
    }
    for (int i = 0; i < 4; ++i) { QSignalBlocker b(acts[i]); acts[i]->setChecked(it.actionsMask & (1 << i)); }
    QSignalBlocker b(notes); notes->setPlainText(it.report);
}

// =====================================================================
//  7. BUTTON ACTIONS (CRUD)
// =====================================================================
void MainWindow::onAdd()
{
    InterventionDialog d(eq, techs, nullptr, databaseBacked, this);
    if (d.exec() != QDialog::Accepted) return;
    // Keep Add available even before the schema is installed so the form can
    // be opened. Block the database write only after the user submits it.
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
    InterventionDialog d(eq, techs, &data[k], databaseBacked, this);
    if (d.exec() != QDialog::Accepted) return;
    Intervention r = d.result();
    r.id = data[k].id;
    r.databaseId = data[k].databaseId;
    r.actionsMask = data[k].actionsMask;
    r.technicianIds = data[k].technicianIds;
    if (databaseBacked && r.technicianId > 0) {
        if (data[k].technicianId > 0 && r.technicianId != data[k].technicianId)
            r.technicianIds.removeAll(data[k].technicianId);
        if (!r.technicianIds.contains(r.technicianId))
            r.technicianIds.append(r.technicianId);
    }
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
    QString error;
    const QString id = data[k].id;
    if (databaseBacked && !maintenanceModel.remove(data[k], &error)) {
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
    if (!databaseBacked && !it.realStart.isValid()) it.realStart = qMin(it.plannedStart, now.addSecs(-3600));
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
                "The approved MAINTENANCE, REALISER, or PERSONNEL tables could not be read. Check the Oracle error shown at startup. "
                "Do not create a separate MAINTENANCE_INTERVENTION table.");
    return false;
}

void MainWindow::markDetailsDirty()
{
    if (!databaseBacked || selId.isEmpty())
        return;
    dirtyDetailIds.insert(selId);
    if (saveDetailsButton) {
        saveDetailsButton->setEnabled(true);
        saveDetailsButton->setText(databaseBacked ? "Save work report *" : "Save notes & actions *");
    }
}

void MainWindow::saveSelectedDetails()
{
    if (!allowMaintenanceWrite()) return;
    const int k = idxOf(selId);
    if (k < 0 || !databaseBacked || !dirtyDetailIds.contains(selId))
        return;

    QString error;
    if (!maintenanceModel.update(data[k], &error)) {
        showMessage(this, QMessageBox::Critical, "Oracle save failed", error);
        return;
    }
    dirtyDetailIds.remove(selId);
    if (saveDetailsButton) {
        saveDetailsButton->setEnabled(false);
        saveDetailsButton->setText(databaseBacked ? "Save work report" : "Save notes & actions");
    }
    showMessage(this, QMessageBox::Information, "Saved", "The notes and actions were saved in Oracle.");
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
        const QString equipmentName = databaseBacked ? QStringLiteral("Not linked in Oracle") : e->name;
        const QString stationName = databaseBacked ? QStringLiteral("Not linked in Oracle") : e->station;
        const QString date = it.plannedStart.isValid()
                                 ? englishLocale().toString(it.plannedStart, "MMM d, yyyy HH:mm")
                                 : QStringLiteral("—");
        rows += QString("<tr><td>%1</td><td>%2<br><small>Station %3</small></td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                    .arg(it.id, equipmentName, stationName, date, it.priority, it.tech, effStatus(it));
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
    const QString equipmentName = databaseBacked ? QStringLiteral("Not linked in Oracle") : e->name;
    const QString equipmentModel = databaseBacked ? QStringLiteral("Not stored") : e->model;
    const QString stationName = databaseBacked ? QStringLiteral("Not linked in Oracle") : e->station;
    writePdf(this, it.id + ".pdf", QString(
        "<h1 style='color:#0b1437'>Intervention Report %1</h1><table border='1' cellspacing='0' cellpadding='7' width='100%'>"
        "<tr><td><b>Equipment</b></td><td>%2 (%3)</td></tr><tr><td><b>Station</b></td><td>%4</td></tr>"
        "<tr><td><b>Technician</b></td><td>%5</td></tr><tr><td><b>Priority / Status</b></td><td>%6 / %7</td></tr>"
        "<tr><td><b>Planned date</b></td><td>%8</td></tr><tr><td><b>Completed date</b></td><td>%9</td></tr>"
        "<tr><td><b>Delay</b></td><td>%10 h</td></tr><tr><td><b>Description</b></td><td>%11</td></tr>"
        "<tr><td><b>Work report</b></td><td>%12</td></tr></table>")
        .arg(it.id, equipmentName, equipmentModel, stationName, it.tech, it.priority, effStatus(it), dt(it.plannedStart))
        .arg(dt(it.realEnd), QString::number(delayHours(it)), it.desc.toHtmlEscaped(), it.report.toHtmlEscaped()));
}

// I1: a reusable blank checklist guides the technician during the visit.
// It is intentionally independent from the CRUD and Oracle data.
void MainWindow::exportChecklist()
{
    QStringList technicianOptions;
    for (const Technician &technician : techs) {
        const QString name = technician.name.trimmed();
        if (technician.active && technician.certified && !name.isEmpty()
            && !technicianOptions.contains(name, Qt::CaseInsensitive)) {
            technicianOptions.append(name);
        }
    }

    bool accepted = false;
    const QString technicianName = QInputDialog::getItem(
        this, "Maintenance checklist", "Choose a technician or type another name:",
        technicianOptions, 0, true, &accepted).trimmed();
    if (!accepted)
        return;
    if (technicianName.isEmpty()) {
        showMessage(this, QMessageBox::Warning, "Maintenance checklist",
                    "Enter or select a technician name.");
        return;
    }

    const QStringList checks = {
        QStringLiteral("Confirm the station and equipment identification."),
        QStringLiteral("Inspect the equipment for visible damage, dust, or obstruction."),
        QStringLiteral("Check the power supply, cables, and physical connections."),
        QStringLiteral("Clean the sensor and check its calibration when required."),
        QStringLiteral("Check communication and confirm that data is being received."),
        QStringLiteral("Verify that the equipment works after the maintenance check."),
        QStringLiteral("Note any anomaly or follow-up action for the station team.")
    };

    QString rows;
    for (const QString &check : checks) {
        rows += QString("<tr><td style='width:36px;text-align:center;font-size:16px'>[ ]</td>"
                        "<td style='padding:9px'>%1</td></tr>").arg(check.toHtmlEscaped());
    }

    const QString html = QString(
        "<h1 style='color:#0b1437'>SkySense — Field Maintenance Checklist</h1>"
        "<p>Print this reusable checklist and mark each item during the station visit.</p>"
        "<table border='1' cellspacing='0' cellpadding='7' width='100%' "
        "style='border-color:#cbd8ea'>"
        "<tr><td><b>Station</b></td><td>________________________________________</td></tr>"
        "<tr><td><b>Equipment / ID</b></td><td>________________________________________</td></tr>"
        "<tr><td><b>Technician</b></td><td>%1</td></tr>"
        "<tr><td><b>Date</b></td><td>____________________</td></tr>"
        "</table><h2 style='color:#0b1437'>Checks to complete</h2>"
        "<table border='1' cellspacing='0' cellpadding='5' width='100%' "
        "style='border-color:#cbd8ea'>%2</table>"
        "<h2 style='color:#0b1437'>Observations</h2>"
        "<p>__________________________________________________________________________</p>"
        "<p>__________________________________________________________________________</p>"
        "<p>Final status: [ ] Operational   [ ] Follow-up required</p>"
        "<p>Technician signature: ____________________ &nbsp;&nbsp; Date: ______________</p>"
        "<p>Station representative: __________________ &nbsp;&nbsp; Date: ______________</p>")
            .arg(technicianName.toHtmlEscaped(), rows);

    writePdf(this, "SkySense_field_maintenance_checklist.pdf", html);
}

// =====================================================================
//  9. STATISTICS (B4) AND PREVENTIVE FEATURE (I2)
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
    l->addWidget(new ChartWidget(databaseBacked ? "Resolution time (not stored in Oracle)" : "Average resolution time by priority",
                                 b, {QColor("#d92d2d"), QColor("#e08a00"), QColor("#1f9d55")}, " h", &d));
    d.exec();
}

void MainWindow::showPreventive()
{   // I2: next service date = last completed intervention + maintenance interval.
    if (databaseBacked) {
        showMessage(this, QMessageBox::Information, "Preventive maintenance",
                    "The approved Oracle schema does not link maintenance interventions to equipment or store equipment service intervals.");
        return;
    }
    int k = idxOf(selId); if (k < 0) return;
    const Equipment *e = equip(data[k].equipId); QDateTime last;
    for (const auto &it : data)
        if (it.equipId == e->id && it.status == "Completed" && it.realEnd.isValid() && (!last.isValid() || it.realEnd > last)) last = it.realEnd;
    if (!last.isValid()) { showMessage(this, QMessageBox::Information, "Preventive maintenance due date", "No completed maintenance history is available for " + e->name + "."); return; }
    if (e->intervalDays <= 0) { showMessage(this, QMessageBox::Information, "Preventive maintenance due date", "No maintenance interval is stored for " + e->name + ". Add an interval to a maintenance record first."); return; }
    QDateTime next = last.addDays(e->intervalDays); qint64 left = QDateTime::currentDateTime().daysTo(next);
    QString txt = QString("%1 — last service: %2\nMaintenance interval: %3 days\nProposed next service: %4 (%5)\n\nWould you like to schedule this intervention?")
                      .arg(e->name, englishLocale().toString(last.date(), "MMM d, yyyy"), QString::number(e->intervalDays),
                           englishLocale().toString(next.date(), "MMM d, yyyy"),
                           left >= 0 ? QString("in %1 days").arg(left) : QString("%1 days overdue").arg(-left));
    if (!askChoice(this, "Preventive maintenance due date", txt, "Schedule")) return;
    Intervention it; int n = 0; for (const auto &x : data) n = qMax(n, x.id.section('-', -1).toInt());
    it.id = QString("MT-%1-%2").arg(QDate::currentDate().year()).arg(n + 1, 3, 10, QChar('0'));
    it.equipId = e->id; it.priority = "Medium"; it.status = "Planned"; it.desc = "Proposed preventive maintenance";
    for (const auto &t : techs) if (t.active && t.certified) { it.tech = t.name; break; }
    if (it.tech.isEmpty()) { showMessage(this, QMessageBox::Warning, "Cannot schedule", "No active staff member is available in the current preview data to assign this intervention."); return; }
    it.plannedStart = next > QDateTime::currentDateTime() ? next : QDateTime::currentDateTime().addDays(1);
    it.plannedEnd = it.plannedStart.addSecs(7200);
    data.prepend(it); selId = it.id; refresh();
}


