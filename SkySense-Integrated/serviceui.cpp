#include "apptheme.h"
#include "serviceui.h"
#include "connection.h"
#include "servicemodel.h"
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTime>
#include <QTimeEdit>
#include <QToolButton>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QResource>
#include <QScreen>
#include <QSet>
#include <QVariantAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QShowEvent>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QStringList>
#include <QVector>
#include <QTextEdit>
#include <QTimer>
#include <functional>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QUrl>
#include <algorithm>
#include <QtMath>

namespace {
const QColor navy("#091b40"), muted("#5d77ad"), blue("#0785ff");

void symbol(QPainter &p, const QString &name, QColor color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(AppTheme::pen(p, QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
    p.setBrush(AppTheme::brush(p, Qt::NoBrush));
    QPainterPath path;
    if (name == "home") {
        path.moveTo(2,11); path.lineTo(12,2); path.lineTo(22,11); path.moveTo(5,9); path.lineTo(5,22); path.lineTo(19,22); path.lineTo(19,9); p.drawPath(path); p.drawRect(QRectF(10,15,4,7));
    } else if (name == "pin") {
        p.drawEllipse(QRectF(5,2,14,14)); p.drawEllipse(QRectF(9,6,6,6)); p.drawLine(6,13,12,23); p.drawLine(12,23,18,13);
    } else if (name == "equipment") {
        for (int y : {4,14}) { p.drawRoundedRect(QRectF(3,y,18,6),1,1); p.drawPoint(6,y+3); p.drawLine(16,y+3,18,y+3); }
    } else if (name == "tool") {
        path.moveTo(15,3); path.cubicTo(6,0,8,10,10,11); path.lineTo(2,19); path.lineTo(5,22); path.lineTo(14,13); path.cubicTo(23,15,25,6,21,4); path.lineTo(17,8); path.lineTo(14,6); path.closeSubpath(); p.drawPath(path);
    } else if (name == "people" || name == "user") {
        p.drawEllipse(QRectF(8,2,8,8)); path.moveTo(3,22); path.cubicTo(3,9,21,9,21,22); p.drawPath(path);
        if (name == "people") { p.drawArc(QRectF(17,4,6,7),-90*16,180*16); p.drawArc(QRectF(17,13,10,14),0,90*16); }
    } else if (name == "grid") {
        for(int x : {3,14}) for(int y : {3,14}) p.drawRoundedRect(QRectF(x,y,7,7),1,1);
    } else if (name == "search") {
        p.drawEllipse(QRectF(3,3,13,13)); p.drawLine(15,15,22,22);
    } else if (name == "calendar") {
        p.drawRoundedRect(QRectF(3,5,18,17),1,1); p.drawLine(3,10,21,10); p.drawLine(7,2,7,7); p.drawLine(17,2,17,7); p.drawLine(9,15,14,15); p.drawLine(14,15,14,19);
    } else if (name == "down") {
        p.drawLine(7,10,12,15); p.drawLine(12,15,17,10);
    } else if (name == "export") {
        p.drawLine(12,2,12,15); p.drawLine(7,10,12,15); p.drawLine(12,15,17,10); p.drawLine(3,16,3,21); p.drawLine(3,21,21,21); p.drawLine(21,21,21,16);
    } else if (name == "plus") {
        p.drawLine(3,12,21,12); p.drawLine(12,3,12,21);
    } else if (name == "wrench") {
        path.moveTo(15,3); path.cubicTo(8,1,6,7,10,11); path.lineTo(3,18); path.lineTo(6,21); path.lineTo(13,14); path.cubicTo(19,17,23,11,21,6); path.lineTo(17,10); path.lineTo(14,8); path.closeSubpath(); p.drawPath(path);
    } else if (name == "check") {
        p.drawLine(5,12,10,17); p.drawLine(10,17,20,6);
    } else if (name == "clock") {
        p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(12,6,12,12); p.drawLine(12,12,17,14);
    } else if (name == "info") {
        p.drawEllipse(QRectF(2,2,20,20)); p.drawPoint(QPointF(12,7)); p.drawLine(12,11,12,17);
    } else if (name == "warning") {
        path.moveTo(12,2); path.lineTo(22,21); path.lineTo(2,21); path.closeSubpath(); p.drawPath(path);
        p.drawLine(12,8,12,14); p.drawPoint(QPointF(12,18));
    } else if (name == "description") {
        path.moveTo(6,2); path.lineTo(15,2); path.lineTo(20,7); path.lineTo(20,22); path.lineTo(6,22); path.closeSubpath(); p.drawPath(path); p.drawLine(15,2,15,7); p.drawLine(15,7,20,7); p.drawLine(9,12,17,12); p.drawLine(9,16,17,16);
    } else if (name == "checklist") {
        p.drawRoundedRect(QRectF(4,3,17,19),2,2); p.drawLine(8,8,10,10); p.drawLine(10,10,13,6); p.drawLine(14,8,18,8); p.drawLine(8,15,10,17); p.drawLine(10,17,13,13); p.drawLine(14,15,18,15);
    } else if (name == "external") {
        p.drawLine(13,3,21,3); p.drawLine(21,3,21,11); p.drawLine(21,3,11,13); path.moveTo(9,5); path.lineTo(3,5); path.lineTo(3,21); path.lineTo(19,21); path.lineTo(19,15); p.drawPath(path);
    } else if (name == "camera") {
        p.drawRoundedRect(QRectF(2,6,20,15),2,2); p.drawEllipse(QRectF(8,9,8,8)); p.drawLine(7,6,8,3); p.drawLine(8,3,16,3); p.drawLine(16,3,17,6);
    } else if (name == "gear") {
        p.drawEllipse(QRectF(6,6,12,12)); p.drawEllipse(QRectF(10,10,4,4));
        for(int i=0;i<8;++i) { qreal a=i*M_PI/4; p.drawLine(QPointF(12+7*qCos(a),12+7*qSin(a)),QPointF(12+10*qCos(a),12+10*qSin(a))); }
    } else if (name == "chart") {
        p.drawLine(3,21,3,3); p.drawLine(3,21,22,21);
        p.drawLine(7,18,7,14); p.drawLine(12,18,12,10); p.drawLine(17,18,17,6); p.drawLine(22,18,22,2);
    } else if (name == "edit") {
        path.moveTo(4,17); path.lineTo(16,5); path.lineTo(20,9); path.lineTo(8,21); path.lineTo(3,22); path.closeSubpath(); p.drawPath(path);
        p.drawLine(14,7,18,11);
    } else if (name == "trash") {
        p.drawLine(5,7,19,7); p.drawLine(9,4,15,4); p.drawLine(7,8,8,21); p.drawLine(17,8,16,21);
        p.drawLine(8,21,16,21); p.drawLine(10,10,10,18); p.drawLine(14,10,14,18);
    }
}

QIcon icon(const QString &name, QColor color=blue)
{
    QPixmap px(48,48); px.fill(Qt::transparent); QPainter p(&px); p.scale(2,2); symbol(p,name,color); p.end(); px.setDevicePixelRatio(2); return QIcon(px);
}

QLabel *text(const QString &value, const char *role="body")
{
    auto *label=new QLabel(value); label->setProperty("role",role); label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred); return label;
}

QPushButton *button(const QString &title, const QString &glyph, bool primary=false)
{
    auto *b=new QPushButton(title);
    if (!glyph.isEmpty()) b->setIcon(icon(glyph,primary?Qt::white:navy));
    b->setIconSize(QSize(20,20)); b->setProperty("primary",primary);
    b->setCursor(Qt::PointingHandCursor); b->setMinimumHeight(42);
    return b;
}

QFrame *card(const char *role="card")
{
    auto *f=new QFrame; f->setProperty("role",role); return f;
}

QLineEdit *search(const QString &placeholder)
{
    auto *edit=new QLineEdit; edit->setPlaceholderText(placeholder); edit->setMinimumWidth(160); edit->setFixedHeight(42);
    edit->addAction(icon("search",muted),QLineEdit::LeadingPosition); return edit;
}

class Selector final : public QComboBox {
protected:
    void paintEvent(QPaintEvent *event) override {
        QComboBox::paintEvent(event); QPainter p(this); p.translate(width()-23,height()/2-8); p.scale(16.0/24,16.0/24); symbol(p,"down",muted);
    }
};

QComboBox *selector(const QStringList &values)
{
    auto *combo=new Selector; combo->addItems(values); combo->setFixedHeight(48); combo->setMinimumWidth(118); return combo;
}

class ServiceReportCalendar final : public QCalendarWidget {
protected:
    void paintCell(QPainter *painter,const QRect &rect,QDate day) const override {
        painter->save(); painter->setRenderHint(QPainter::Antialiasing);
        const bool selected=day==selectedDate(), today=day==QDate::currentDate();
        const bool inMonth=day.month()==monthShown() && day.year()==yearShown();
        const QRectF cell=QRectF(rect).adjusted(3,3,-3,-3);
        painter->fillRect(rect,AppTheme::brush(*painter, Qt::white));
        painter->setPen(AppTheme::pen(*painter, today&&!selected?QPen(blue,1.3):QPen(Qt::NoPen))); painter->setBrush(AppTheme::brush(*painter, selected?blue:today?QColor("#eef6ff"):Qt::white));
        painter->drawRoundedRect(cell,8,8);
        QFont font("Segoe UI"); font.setPixelSize(14); font.setBold(selected||today); painter->setFont(font);
        painter->setPen(AppTheme::pen(*painter, selected?Qt::white:!inMonth?QColor("#a1aec0"):day.dayOfWeek()>5?QColor("#62738c"):navy));
        painter->drawText(rect,Qt::AlignCenter,QString::number(day.day())); painter->restore();
    }
};

class ServiceReportDateEdit final : public QDateEdit {
    QFrame *popup{}; ServiceReportCalendar *calendar{}; QComboBox *month{}; QSpinBox *year{};
public:
    explicit ServiceReportDateEdit(QWidget *parent=nullptr):QDateEdit(QDate::currentDate(),parent) {
        setLocale(QLocale(QLocale::English,QLocale::UnitedStates)); setDisplayFormat("ddd, dd MMM yyyy"); setDateRange(QDate(1900,1,1),QDate(2100,12,31));
        setButtonSymbols(QAbstractSpinBox::NoButtons); setCursor(Qt::PointingHandCursor);
        popup=new QFrame(this,Qt::Popup); popup->setObjectName("serviceReportCalendarPopup");
        popup->setStyleSheet("QFrame#serviceReportCalendarPopup { background:white; border:1px solid #c9def7; border-radius:12px; } QCalendarWidget QAbstractItemView { background:white; selection-background-color:#0785ff; selection-color:white; }");
        auto *box=new QVBoxLayout(popup); box->setContentsMargins(20,18,20,18); box->setSpacing(14);
        auto *title=new QLabel("Choose report date"); title->setProperty("role","detailTitle"); box->addWidget(title);
        auto *hint=new QLabel("Choose a month and year, then select a day."); hint->setProperty("role","smallMuted"); box->addWidget(hint);
        auto *navigation=new QHBoxLayout; auto *previous=new QToolButton; previous->setText("‹"); previous->setAccessibleName("Previous month");
        month=new Selector; month->setObjectName("serviceReportCalendarMonth"); for(int i=1;i<=12;++i) month->addItem(QLocale(QLocale::English).monthName(i));
        year=new QSpinBox; year->setObjectName("serviceReportCalendarYear"); year->setRange(1900,2100); year->setFixedWidth(94); year->setAlignment(Qt::AlignCenter);
        auto *next=new QToolButton; next->setText("›"); next->setAccessibleName("Next month");
        previous->setFixedWidth(34); next->setFixedWidth(34); previous->setStyleSheet("padding:0;font-size:22px;"); next->setStyleSheet("padding:0;font-size:22px;");
        for(QWidget *w:QList<QWidget *>{previous,month,year,next}) { w->setMinimumHeight(40); w->setCursor(Qt::PointingHandCursor); }
        navigation->addWidget(previous); navigation->addWidget(month,1); navigation->addWidget(year); navigation->addWidget(next); box->addLayout(navigation);
        calendar=new ServiceReportCalendar; calendar->setObjectName("serviceReportCalendar"); calendar->setNavigationBarVisible(false); calendar->setLocale(QLocale(QLocale::English)); calendar->setFirstDayOfWeek(Qt::Monday); calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        QTextCharFormat weekday; weekday.setForeground(navy); weekday.setBackground(QColor("#eef6ff")); QTextCharFormat weekend=weekday; weekend.setForeground(QColor("#c83c4d"));
        for(Qt::DayOfWeek day:{Qt::Monday,Qt::Tuesday,Qt::Wednesday,Qt::Thursday,Qt::Friday}) calendar->setWeekdayTextFormat(day,weekday);
        calendar->setWeekdayTextFormat(Qt::Saturday,weekend); calendar->setWeekdayTextFormat(Qt::Sunday,weekend); calendar->setDateRange(minimumDate(),maximumDate()); calendar->setFixedSize(350,280); box->addWidget(calendar);
        auto *footer=new QHBoxLayout; auto *todayLabel=new QLabel(QDate::currentDate().toString("dd MMM yyyy")); todayLabel->setProperty("role","smallMuted"); auto *today=button("Today","",true); today->setMinimumHeight(38); footer->addWidget(todayLabel); footer->addStretch(); footer->addWidget(today); box->addLayout(footer);
        QObject::connect(month,&QComboBox::currentIndexChanged,popup,[this](int value){calendar->setCurrentPage(year->value(),value+1);});
        QObject::connect(year,&QSpinBox::valueChanged,popup,[this](int value){calendar->setCurrentPage(value,month->currentIndex()+1);});
        QObject::connect(calendar,&QCalendarWidget::currentPageChanged,popup,[this](int y,int m){QSignalBlocker a(month),b(year);month->setCurrentIndex(m-1);year->setValue(y);});
        QObject::connect(previous,&QToolButton::clicked,calendar,&QCalendarWidget::showPreviousMonth); QObject::connect(next,&QToolButton::clicked,calendar,&QCalendarWidget::showNextMonth);
        QObject::connect(calendar,&QCalendarWidget::clicked,this,[this](const QDate &value){setDate(value);popup->hide();setFocus();}); QObject::connect(calendar,&QCalendarWidget::activated,this,[this](const QDate &value){setDate(value);popup->hide();});
        QObject::connect(today,&QPushButton::clicked,this,[this]{setDate(QDate::currentDate());popup->hide();});
    }
    void openCalendar() {
        calendar->setSelectedDate(date()); calendar->setCurrentPage(date().year(),date().month()); { QSignalBlocker a(month),b(year); month->setCurrentIndex(date().month()-1); year->setValue(date().year()); }
        popup->adjustSize(); const QRect available=screen()->availableGeometry(); QPoint position=mapToGlobal(QPoint(width()-popup->width(),height()+6)); position.setX(qBound(available.left(),position.x(),qMax(available.left(),available.right()-popup->width()))); if(position.y()+popup->height()>available.bottom()) position.setY(mapToGlobal(QPoint(0,-popup->height()-6)).y()); popup->move(position); popup->show(); calendar->setFocus();
    }
protected:
    void mousePressEvent(QMouseEvent *event) override { if(event->button()==Qt::LeftButton) {openCalendar();event->accept();} else QDateEdit::mousePressEvent(event); }
    void keyPressEvent(QKeyEvent *event) override { if(event->key()==Qt::Key_F4 || (event->key()==Qt::Key_Down && event->modifiers().testFlag(Qt::AltModifier))) {openCalendar();event->accept();} else QDateEdit::keyPressEvent(event); }
};

class Glyph final : public QWidget {
    QString name; QColor color; bool bubble;
public:
    Glyph(QString n,QColor c=blue,int size=28,bool circle=false):name(n),color(c),bubble(circle) { setFixedSize(size,size); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(bubble) { QColor bg=color; bg.setAlpha(22); p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, bg)); p.drawEllipse(rect()); }
        qreal margin=bubble?width()*.25:2; p.translate(margin,margin); p.scale((width()-2*margin)/24.0,(height()-2*margin)/24.0); symbol(p,name,color);
    }
};

class Sidebar final : public QWidget {

    std::function<void(const QString &)> navigate;
    QStringList navigationNames;
    QList<QPushButton *> navigationButtons;
public:
    explicit Sidebar(std::function<void(const QString &)> onNavigate = {}, const QString &initialSection = "Services")
        : navigate(std::move(onNavigate)) {
        setObjectName("servicesSidebar"); setFixedWidth(270);
        auto *column=new QVBoxLayout(this); column->setContentsMargins(10,20,10,18); column->setSpacing(4);
        auto *brand=new QLabel; brand->setObjectName("sidebarBrand"); brand->setFixedSize(250,98);
        brand->setPixmap(QPixmap(":/assets/logo/skysense_logo.png").scaled(244,88,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter);
        auto *brandGlow=new QGraphicsDropShadowEffect(brand);
        brandGlow->setBlurRadius(20); brandGlow->setOffset(0,2); brandGlow->setColor(QColor(25,150,255,105));
        brand->setGraphicsEffect(brandGlow);
        column->addWidget(brand); column->addSpacing(15);
        const QStringList names{"Dashboard","Stations","Equipment","Maintenance","Staff","Services"};
        navigationNames=names;
        const QStringList icons{"home","pin","equipment","tool","people","grid"};
        for(int i=0;i<names.size();++i) {
            auto *nav=button(names[i],icons[i]); nav->setProperty("nav",true); nav->setProperty("active",names[i]==initialSection);
            nav->setIcon(icon(icons[i],QColor("#e4f3ff"))); nav->setIconSize(QSize(25,25)); nav->setFixedHeight(50); column->addWidget(nav);
            navigationButtons.append(nav);
            QObject::connect(nav,&QPushButton::clicked,this,[this,i]() {
                for(int index=0;index<navigationButtons.size();++index) {
                    navigationButtons[index]->setProperty("active",index==i);
                    navigationButtons[index]->style()->unpolish(navigationButtons[index]);
                    navigationButtons[index]->style()->polish(navigationButtons[index]);
                }
                if(navigate) navigate(navigationNames.value(i));
            });
        }
        column->addStretch();
        auto *themeRow = new QHBoxLayout;
        themeRow->setContentsMargins(16, 12, 16, 10);
        auto *workspaceTheme = new ThemeSwitch(this);
        workspaceTheme->setObjectName("workspaceThemeSwitch");
        themeRow->addWidget(workspaceTheme); themeRow->addStretch();
        column->addLayout(themeRow);
        auto *statusBox=new QVBoxLayout; statusBox->setContentsMargins(16,12,4,0); statusBox->setSpacing(5);
        statusBox->addWidget(text("System Status","sidebarStatus"));
        const bool oracleConnected = QSqlDatabase::database().isOpen();
        auto *statusRow=new QHBoxLayout; auto *dot=text(QString::fromUtf8("●"),"greenDot"); dot->setFixedWidth(16);
        dot->setStyleSheet(oracleConnected ? "color: #00c778;" : "color: #ff6b6b;");
        if (oracleConnected) {
            auto *pulse = new QGraphicsOpacityEffect(dot); pulse->setOpacity(1.0); dot->setGraphicsEffect(pulse);
            auto *pulseAnimation = new QPropertyAnimation(pulse,"opacity",dot); pulseAnimation->setDuration(1050);
            pulseAnimation->setStartValue(1.0); pulseAnimation->setKeyValueAt(0.5,0.42); pulseAnimation->setEndValue(1.0);
            pulseAnimation->setLoopCount(-1); pulseAnimation->start();
        }
        statusRow->addWidget(dot);
        statusRow->addWidget(text(oracleConnected ? "Oracle connected" : "Oracle disconnected","sidebarStatus"));
        statusBox->addLayout(statusRow);
        statusBox->addWidget(text("Last checked: " + QDateTime::currentDateTime().toString("dd MMM yyyy HH:mm"),"sidebarMuted"));
        auto *status=new QWidget; status->setLayout(statusBox); column->addWidget(status);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient base(0,0,width(),height());
        base.setColorAt(0,QColor("#091c35"));base.setColorAt(.6,QColor("#102b49"));base.setColorAt(1,QColor("#071629"));
        p.fillRect(rect(),base);
        const QPointF center(width()*.84,height()*.7);
        QRadialGradient aura(center,width()*1.15);
        aura.setColorAt(0,QColor(20,156,195,45));aura.setColorAt(1,Qt::transparent);p.fillRect(rect(),aura);
        p.save();p.setClipRect(QRectF(0,height()*.56,width(),height()*.25));
        p.translate(center);p.rotate(-24);p.setBrush(Qt::NoBrush);
        for(int i=0;i<6;++i){p.setPen(QPen(QColor(101,215,238,18+i*3),.8));p.drawEllipse(QPointF(),70+i*24,26+i*12);}
        p.restore();
        p.setPen(QPen(QColor(121,188,224,35),1));p.drawLine(width()-1,0,width()-1,height());
        QLinearGradient footer(0,height()-175,0,height());footer.setColorAt(0,Qt::transparent);footer.setColorAt(1,QColor(3,13,29,180));p.fillRect(rect(),footer);
    }
};

QWidget *metricCard(const QString &title,const QString &value,const QString &total,const QString &trend,
                    const QString &glyph,const QColor &accent,const char *tone)
{
    auto *f=card("metric"); f->setProperty("tone",tone); f->setMinimumHeight(118);
    auto *row=new QHBoxLayout(f); row->setContentsMargins(14,14,12,12); row->setSpacing(11);
    row->addWidget(new Glyph(glyph,accent,46,true),0,Qt::AlignTop);
    auto *copy=new QVBoxLayout; copy->setSpacing(3); copy->addWidget(text(title,"metricTitle"));
    auto *numbers=new QHBoxLayout; numbers->setSpacing(5); auto *count=text(value,"number"); count->setProperty("role","metricCount"); numbers->addWidget(count); auto *totalLabel=text(total,"muted"); totalLabel->setProperty("role","metricTotal"); numbers->addWidget(totalLabel,1,Qt::AlignBottom); copy->addLayout(numbers);
    auto *change=text(QString::fromUtf8("↗  ")+trend,"trend"); change->setProperty("tone",tone); copy->addWidget(change);
    row->addLayout(copy,1); row->addWidget(new Glyph("chart",accent,30),0,Qt::AlignVCenter); return f;
}

// ServiceRecord now lives in servicemodel.h (shared with the Oracle model).

QVector<ServiceRecord> serviceRecords;

struct ServiceStationChoice { int id=0; QString code,name; };
struct ServicePersonChoice { int id=0; QString name; };
QVector<ServiceStationChoice> serviceStationChoices;
QVector<ServicePersonChoice> servicePersonChoices;
int minutesBetween(const QString &from,const QString &to);







QString nextServiceId()
{
    int next=1;
    for(const auto &record:serviceRecords) {
        bool ok=false;
        const int number=record.id.mid(4).toInt(&ok);
        if(ok) next=qMax(next,number+1);
    }
    return QString("SRV-%1").arg(next,3,10,QChar('0'));
}

QString serviceLoadError;   // filled when the Oracle SELECT fails (shown once on the Services page)

// Loads the Services list from Oracle (table SKYSENSE_SERVICES).
// Demo rows are used ONLY when Oracle is not connected (--skip-oracle) and are never written to the database.
void initialiseServiceData()
{
    serviceStationChoices={{1,"ALP-03","Alpine Ridge"},{2,"COA-01","Coastal Bay"},{3,"FOR-12","Pine Forest"},{4,"VAL-07","Green Valley"},{5,"DES-05","Desert View"}};
    servicePersonChoices={{1,"Alex Martin"},{2,"Sophie Chen"},{3,"James Wilson"},{4,"Emma Davis"},{5,"Ryan Kim"}};
    serviceRecords.clear(); serviceLoadError.clear();
    if(QSqlDatabase::database().isOpen()) {
        QVector<ServiceRecord> loaded;
        if(ServiceModel().loadAll(loaded,&serviceLoadError)) serviceRecords=loaded;
        return;
    }
    auto addDemoService=[](const QString &id,const QString &title,const QString &type,int stationIndex,int personIndex,
                           const QString &date,const QString &priority,const QString &status,double price) {
        ServiceRecord record;
        record.id=id; record.title=title; record.type=type;
        const auto station=serviceStationChoices.at(stationIndex); const auto person=servicePersonChoices.at(personIndex);
        record.stationCode=station.code; record.stationName=station.name; record.stationId=station.id;
        record.assignee=person.name; record.personnelId=person.id;
        record.date=date; record.startTime="09:00"; record.endTime="12:00";
        record.priority=priority; record.status=status; record.quantity=1; record.unitPrice=price;
        record.description=title+" at "+station.name;
        record.checklist={"Inspect equipment","Check communication","Validate readings"};
        record.checklistDone={status=="Completed",status=="Completed",status=="Completed"};
        if(status=="Completed") record.actualEndTime="11:30";
        serviceRecords.append(record);
    };
    addDemoService("SRV-001","Routine Maintenance","Maintenance",0,2,"Oct 20, 2026","High","In Progress",150);
    addDemoService("SRV-002","Sensor Calibration","Calibration",1,1,"Oct 21, 2026","Medium","Scheduled",95);
    addDemoService("SRV-003","Camera Inspection","Inspection",2,0,"Oct 18, 2026","Low","Completed",80);
    addDemoService("SRV-004","Weather Station Repair","Repair",3,3,"Oct 16, 2026","High","Overdue",220);
    addDemoService("SRV-005","Data Connectivity Check","Connectivity",0,4,"Oct 22, 2026","Medium","Scheduled",70);
}


// ---- Shared helpers for the two innovative métiers (history reuse + performance) ----
int minutesBetween(const QString &from,const QString &to)
{
    const QTime a=QTime::fromString(from,"HH:mm"), b=QTime::fromString(to,"HH:mm");
    return (a.isValid()&&b.isValid())?a.secsTo(b)/60:0;
}
int plannedMinutes(const ServiceRecord &r) { return minutesBetween(r.startTime,r.endTime); }
int actualMinutes(const ServiceRecord &r) { return r.actualEndTime.isEmpty()?-1:minutesBetween(r.startTime,r.actualEndTime); }
QString durationText(int minutes)
{
    const int m=qAbs(minutes);
    return QString("%1 h %2").arg(m/60).arg(m%60,2,10,QChar('0'));
}
QString gapText(int minutes) { return QString("%1%2 min").arg(minutes>0?"+":minutes<0?"-":"").arg(qAbs(minutes)); }

// I2 — a service is late if it is Overdue, or Completed after its planned end time.
bool finishedLate(const ServiceRecord &r)
{
    return r.status=="Overdue" || (r.status=="Completed" && actualMinutes(r)>plannedMinutes(r));
}
struct PerformanceStats { int completed=0, overdue=0, onTime=0, measured=0, plannedSum=0, actualSum=0; };
PerformanceStats performanceFor(const QString &type=QString())
{
    PerformanceStats s;
    for(const auto &r:serviceRecords) {
        if(!type.isEmpty() && r.type!=type) continue;
        if(r.status=="Overdue") ++s.overdue;
        else if(r.status=="Completed") {
            ++s.completed;
            if(!finishedLate(r)) ++s.onTime;
            if(actualMinutes(r)>=0) { ++s.measured; s.plannedSum+=plannedMinutes(r); s.actualSum+=actualMinutes(r); }
        }
    }
    return s;
}
// On-time rate = completed on time / (completed + overdue). Returns -1 when there is nothing to measure.
int onTimeRate(const PerformanceStats &s)
{
    const int total=s.completed+s.overdue;
    return total>0?qRound(100.0*s.onTime/total):-1;
}

bool writeServicesPdf(const QString &path,const QVector<ServiceRecord> &records,QString *error,const QDate &reportDate=QDate::currentDate())
{
    if(records.isEmpty()) {
        if(error) *error="There are no visible services to export.";
        return false;
    }

    QPdfWriter pdf(path);
    pdf.setTitle("SkySense services report");
    pdf.setCreator("SkySense Smart Weather Management");
    pdf.setResolution(144);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setPageMargins(QMarginsF(12,12,12,12),QPageLayout::Millimeter);

    QPainter painter(&pdf);
    if(!painter.isActive()) {
        if(error) *error="The PDF file could not be created at the selected location.";
        return false;
    }

    const QColor dark("#091b40"),secondary("#5d77ad"),line("#dfe9f6"),surface("#f5f9ff"),accent("#0785ff");
    const QRect page=pdf.pageLayout().paintRectPixels(pdf.resolution());
    const int contentLeft=page.left(),contentRight=page.right(),contentWidth=page.width();
    const int footerHeight=34,rowHeight=54,headerRowHeight=42;
    int pageNumber=1,y=0;

    auto useFont=[&](int pixels,QFont::Weight weight=QFont::Normal,const QColor &color=QColor("#091b40")) {
        QFont font("Segoe UI"); font.setPixelSize(pixels); font.setWeight(weight); painter.setFont(font); painter.setPen(AppTheme::pen(painter, color));
    };
    auto footer=[&] {
        painter.setPen(AppTheme::pen(painter, line)); painter.drawLine(contentLeft,page.bottom()-footerHeight,contentRight,page.bottom()-footerHeight);
        useFont(11,QFont::Normal,secondary);
        painter.drawText(QRect(contentLeft,page.bottom()-footerHeight+8,contentWidth,20),Qt::AlignLeft|Qt::AlignVCenter,
                         "SkySense • Smart Weather Management");
        painter.drawText(QRect(contentLeft,page.bottom()-footerHeight+8,contentWidth,20),Qt::AlignRight|Qt::AlignVCenter,
                         QString("Page %1").arg(pageNumber));
    };
    auto reportHeader=[&](bool includeSummary) {
        painter.fillRect(page,AppTheme::brush(painter, Qt::white));
        QPixmap logo(":/assets/logo/skysense_logo.png");
        if(!logo.isNull()) painter.drawPixmap(QRect(contentLeft,y,190,66),logo.scaled(190,66,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        useFont(25,QFont::Bold,dark);
        painter.drawText(QRect(contentLeft+215,y+2,contentWidth-215,34),Qt::AlignLeft|Qt::AlignVCenter,"Services Operations Report");
        useFont(12,QFont::Normal,secondary);
        painter.drawText(QRect(contentLeft+215,y+38,contentWidth-215,22),Qt::AlignLeft|Qt::AlignVCenter,
                          QString("Report date %1 • %2 service records").arg(QLocale(QLocale::English).toString(reportDate,"dd MMMM yyyy")).arg(records.size()));
        y+=78; painter.fillRect(QRect(contentLeft,y,contentWidth,3),AppTheme::brush(painter, accent)); y+=18;
        if(includeSummary) {
            const QStringList labels{"Total","Scheduled","In Progress","Completed","Overdue"};
            QVector<int> values{int(records.size()),0,0,0,0};
            for(const auto &record:records) {
                if(record.status=="Scheduled") ++values[1];
                else if(record.status=="In Progress") ++values[2];
                else if(record.status=="Completed") ++values[3];
                else if(record.status=="Overdue") ++values[4];
            }
            const int gap=12,cardWidth=(contentWidth-gap*4)/5;
            for(int i=0;i<labels.size();++i) {
                const QRect card(contentLeft+i*(cardWidth+gap),y,cardWidth,70);
                painter.setPen(AppTheme::pen(painter, QPen(line,1))); painter.setBrush(AppTheme::brush(painter, i==0?QColor("#eef7ff"):surface)); painter.drawRoundedRect(card,10,10);
                useFont(12,QFont::DemiBold,secondary); painter.drawText(card.adjusted(14,8,-10,-36),Qt::AlignLeft|Qt::AlignVCenter,labels[i]);
                useFont(24,QFont::Bold,i==0?accent:dark); painter.drawText(card.adjusted(14,30,-10,-5),Qt::AlignLeft|Qt::AlignVCenter,QString::number(values[i]));
            }
            y+=86;
            int completed=0, checklistTasks=0, late=0;
            for(const auto &record:records) {
                if(record.status=="Completed") ++completed;
                checklistTasks+=record.checklist.size();
                if(finishedLate(record)) ++late;
            }
            const QRect summary(contentLeft,y,contentWidth,48);
            painter.setPen(AppTheme::pen(painter, QPen(line,1))); painter.setBrush(AppTheme::brush(painter, QColor("#f8fbff"))); painter.drawRoundedRect(summary,8,8);
            useFont(12,QFont::DemiBold,dark); painter.drawText(summary.adjusted(14,5,-14,-25),Qt::AlignLeft|Qt::AlignVCenter,"Completion summary");
            useFont(11,QFont::Normal,secondary);
            painter.drawText(summary.adjusted(14,23,-14,-4),Qt::AlignLeft|Qt::AlignVCenter,
                QString("%1 of %2 services completed • %3 local checklist task%4 • %5 late service%6")
                    .arg(completed).arg(records.size()).arg(checklistTasks).arg(checklistTasks==1?"":"s")
                    .arg(late).arg(late==1?"":"s"));
            y+=60;
        }
    };

    const QStringList headings{"ID","Service","Type","Station","Assignee","Schedule","Priority","Status"};
    const QVector<int> weights{7,20,11,13,14,16,8,11};
    QVector<int> widths; int used=0;
    for(int i=0;i<weights.size();++i) {
        const int width=i==weights.size()-1?contentWidth-used:contentWidth*weights[i]/100;
        widths.append(width); used+=width;
    }
    auto tableHeader=[&] {
        int x=contentLeft; painter.fillRect(QRect(contentLeft,y,contentWidth,headerRowHeight),AppTheme::brush(painter, QColor("#eaf4ff")));
        useFont(12,QFont::DemiBold,dark);
        for(int i=0;i<headings.size();++i) {
            painter.drawText(QRect(x+8,y,widths[i]-16,headerRowHeight),Qt::AlignLeft|Qt::AlignVCenter,headings[i]);
            x+=widths[i];
        }
        y+=headerRowHeight;
    };

    y=page.top(); reportHeader(true); tableHeader();
    for(int index=0;index<records.size();++index) {
        if(y+rowHeight>page.bottom()-footerHeight-8) {
            footer(); pdf.newPage(); ++pageNumber; y=page.top(); reportHeader(false); tableHeader();
        }
        const auto &record=records[index];
        if(index%2) painter.fillRect(QRect(contentLeft,y,contentWidth,rowHeight),AppTheme::brush(painter, QColor("#f8fbff")));
        painter.setPen(AppTheme::pen(painter, line)); painter.drawLine(contentLeft,y+rowHeight,contentRight,y+rowHeight);
        const QStringList values{
            record.id,record.title,record.type,record.stationCode+"\n"+record.stationName,record.assignee,
            record.date+"\n"+record.startTime+" – "+record.endTime,record.priority,record.status
        };
        int x=contentLeft; useFont(11,QFont::Normal,dark);
        for(int column=0;column<values.size();++column) {
            QFont font=painter.font(); font.setBold(column==0||column==1); painter.setFont(font);
            QColor valueColor=dark;
            if(column==6) valueColor=record.priority=="High"?QColor("#d9363e"):record.priority=="Medium"?QColor("#b77900"):QColor("#168653");
            if(column==7) valueColor=record.status=="Overdue"?QColor("#d9363e"):record.status=="Completed"?QColor("#168653"):QColor("#246ec5");
            painter.setPen(AppTheme::pen(painter, valueColor));
            painter.drawText(QRect(x+8,y+5,widths[column]-16,rowHeight-10),Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap,values[column]);
            x+=widths[column];
        }
        y+=rowHeight;
    }
    footer(); painter.end();
    if(!QFileInfo::exists(path)||QFileInfo(path).size()==0) {
        if(error) *error="The PDF writer did not produce a valid file.";
        return false;
    }
    return true;
}

QString glyphForType(const QString &type)
{
    if (type=="Maintenance") return "wrench";
    if (type=="Calibration") return "gear";
    if (type=="Inspection") return "camera";
    if (type=="Repair") return "tool";
    if (type=="Connectivity") return "equipment";
    return "check";
}

class ServiceDelegate final : public QStyledItemDelegate {
    QVector<qreal> rowOpacity;
    qreal pulseLevel=1.0;
public:
    explicit ServiceDelegate(QObject *parent=nullptr):QStyledItemDelegate(parent),rowOpacity(serviceRecords.size(),1.0) {}
    void setPulseLevel(qreal value) { pulseLevel=value; }
    void setRowOpacity(int row,qreal opacity) {
        if(row>=rowOpacity.size()) rowOpacity.resize(row+1,1.0);
        rowOpacity[row]=opacity;
    }
    void paint(QPainter *p,const QStyleOptionViewItem &o,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); const QRect r=o.rect; const int row=index.row(), col=index.column();
        p->setOpacity(p->opacity()*(row<rowOpacity.size()?rowOpacity[row]:1.0));
        const bool checked=index.siblingAtColumn(0).data(Qt::CheckStateRole).toInt()==Qt::Checked;
        const ServiceRecord &record=serviceRecords[row];
        const bool hovered=(o.state & QStyle::State_MouseOver) && !checked;
        p->fillRect(r,AppTheme::brush(*p, checked?QColor("#e5f4ff"):hovered?QColor("#f2f8ff"):Qt::white)); p->setPen(AppTheme::pen(*p, QColor("#e5eef9"))); p->drawLine(r.bottomLeft(),r.bottomRight());
        QFont font("Segoe UI"); font.setPixelSize(13); p->setFont(font); p->setPen(AppTheme::pen(*p, navy));
        if(col==0) {
            if(checked) p->fillRect(QRect(r.left(),r.top(),3,r.height()),AppTheme::brush(*p, blue));
            QRectF box(r.center().x()-8,r.center().y()-8,16,16); p->setPen(AppTheme::pen(*p, QPen(checked?blue:QColor("#8ca7d3"),1.3))); p->setBrush(AppTheme::brush(*p, checked?blue:Qt::white)); p->drawRoundedRect(box,3,3);
            if(checked) { p->translate(box.topLeft()); p->scale(16.0/24,16.0/24); symbol(*p,"check",Qt::white); }
        } else if(col==1) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(9,0,-3,0),Qt::AlignVCenter,record.id);
        } else if(col==2) {
            p->save(); p->translate(r.left()+8,r.center().y()-11); symbol(*p,glyphForType(record.type),blue); p->restore();
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(39,0,-4,0),Qt::AlignVCenter,record.title);
        } else if(col==3) {
            p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(8,0,-3,0),Qt::AlignVCenter,record.type);
        } else if(col==4) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(8,3,-3,-r.height()/2+2),Qt::AlignVCenter,record.stationCode);
            font.setBold(false); p->setFont(font); p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(8,r.height()/2-1,-3,-3),Qt::AlignVCenter,record.stationName);
        } else if(col==5) {
            const QString initials=record.assignee.split(' ').value(0).left(1)+record.assignee.split(' ').value(1).left(1);
            const QColor avatar[]={QColor("#e3efff"),QColor("#fbe9e1"),QColor("#e8e4fb"),QColor("#f8e9f0"),QColor("#e5f5ec"),QColor("#f2eddf")};
            p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, avatar[row%6])); p->drawEllipse(QRectF(r.left()+7,r.center().y()-13,26,26));
            p->setPen(AppTheme::pen(*p, QColor("#28517c"))); p->drawText(QRectF(r.left()+7,r.center().y()-13,26,26),Qt::AlignCenter,initials);
            p->setPen(AppTheme::pen(*p, navy)); p->drawText(r.adjusted(39,0,-2,0),Qt::AlignVCenter,record.assignee);
        } else if(col==6) {
            p->save(); p->translate(r.left()+7,r.center().y()-10); symbol(*p,"calendar",muted); p->restore();
            p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(34,3,-2,-r.height()/2+1),Qt::AlignVCenter,record.date);
            p->drawText(r.adjusted(34,r.height()/2-1,-2,-3),Qt::AlignVCenter,record.startTime+" - "+record.endTime);
        } else if(col==7 || col==8) {
            const QString value=col==7?record.priority:record.status; QColor bg,fg;
            if(col==7) { bg=value=="High"?QColor("#fde5e6"):value=="Medium"?QColor("#fff1d4"):QColor("#e5f8ed"); fg=value=="High"?QColor("#dc4551"):value=="Medium"?QColor("#bb8113"):QColor("#23985d"); }
            else { bg=value=="Completed"?QColor("#ddf7e9"):value=="Overdue"?QColor("#ffe4e7"):value=="In Progress"?QColor("#e4f1ff"):QColor("#e9f3ff"); fg=value=="Completed"?QColor("#13834f"):value=="Overdue"?QColor("#d34756"):QColor("#2474c6"); }
            QRectF badge(r.left()+4,r.center().y()-13,r.width()-8,26);
            if(col==8) {
                QColor halo=fg; halo.setAlpha(qRound(7+40*pulseLevel));
                p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, halo));
                p->drawRoundedRect(badge.adjusted(-2,-2,2,2),15,15);
            }
            p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, bg)); p->drawRoundedRect(badge,13,13); p->setPen(AppTheme::pen(*p, fg)); p->drawText(badge,Qt::AlignCenter,value);
        } else {
            p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, navy)); for(int i=-1;i<=1;++i) p->drawEllipse(QPointF(r.center().x()+i*7,r.center().y()),1.5,1.5);
        }
        p->restore();
    }
};

class ServiceHeader final : public QHeaderView {
public:
    explicit ServiceHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) {}
protected:
    void paintSection(QPainter *p,const QRect &rect,int section) const override {
        QHeaderView::paintSection(p,rect,section); p->save(); p->setRenderHint(QPainter::Antialiasing); p->setPen(AppTheme::pen(*p, QPen(QColor("#8aa6d2"),1.2)));
        if(section==0) { p->setBrush(AppTheme::brush(*p, Qt::white)); p->drawRoundedRect(QRectF(rect.center().x()-8,rect.center().y()-8,16,16),3,3); }
        else if(section>0 && section<9) { int x=rect.left()+fontMetrics().horizontalAdvance(model()->headerData(section,Qt::Horizontal).toString())+20; int y=rect.center().y(); p->drawLine(x,y-2,x+3,y-5); p->drawLine(x+3,y-5,x+6,y-2); p->drawLine(x,y+2,x+3,y+5); p->drawLine(x+3,y+5,x+6,y+2); }
        p->restore();
    }
};

class ServiceTable final : public QTableWidget {
    std::function<void()> changeCallback;
    QHash<int,bool> ascendingSort;
public:
    ServiceTable():QTableWidget(serviceRecords.size(),10) {
        setObjectName("serviceTable"); setHorizontalHeader(new ServiceHeader(this)); viewport()->setMouseTracking(true); setMouseTracking(true);
        setHorizontalHeaderLabels({"","ID","Title","Type","Station","Assignee","Scheduled Date","Priority","Status","Actions"});
        verticalHeader()->hide(); verticalHeader()->setDefaultSectionSize(42); horizontalHeader()->setFixedHeight(40); horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); setItemDelegate(new ServiceDelegate(this)); setShowGrid(false); setSelectionMode(QAbstractItemView::NoSelection); setEditTriggers(QAbstractItemView::NoEditTriggers); setFocusPolicy(Qt::NoFocus);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        for(int r=0;r<serviceRecords.size();++r) populateRow(r);
        connect(horizontalHeader(),&QHeaderView::sectionClicked,this,[this](int column) {
            if(column>0 && column<9) sortRecords(column);
        });
        connect(this,&QTableWidget::cellClicked,this,[this](int row,int column) {
            if(column!=0) return;
            auto *check=item(row,0);
            if(!check) return;
            check->setCheckState(check->checkState()==Qt::Checked?Qt::Unchecked:Qt::Checked);
            viewport()->update();
        });
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); setMinimumWidth(0);
        auto *statusPulse=new QTimer(this);
        statusPulse->setInterval(75);
        connect(statusPulse,&QTimer::timeout,this,[this,phase=0.0]() mutable {
            if(!isVisible()) return;
            phase+=0.17;
            static_cast<ServiceDelegate *>(itemDelegate())->setPulseLevel((1.0+qSin(phase))*0.5);
            viewport()->update();
        });
        statusPulse->start();
    }
    void addService(const ServiceRecord &record) {
        serviceRecords.append(record);
        const int row=serviceRecords.size()-1;
        setRowCount(serviceRecords.size());
        populateRow(row);
        setRowHeight(row,42);
        updateHeight();
        viewport()->update();
        if(changeCallback) changeCallback();
    }
    int checkedRow() const {
        for(int row=0;row<rowCount();++row) {
            const auto *check=item(row,0);
            if(!isRowHidden(row) && check && check->checkState()==Qt::Checked) return row;
        }
        return -1;
    }
    ServiceRecord serviceAt(int row) const { return serviceRecords.value(row); }
    QVector<ServiceRecord> visibleServices() const {
        QVector<ServiceRecord> visible;
        for(int row=0;row<rowCount();++row) if(!isRowHidden(row)) visible.append(serviceRecords[row]);
        return visible;
    }
    void setChangeCallback(std::function<void()> callback) { changeCallback=std::move(callback); }
    int visibleServiceCount() const { int count=0; for(int row=0;row<rowCount();++row) if(!isRowHidden(row)) ++count; return count; }
    int serviceCount() const { return serviceRecords.size(); }
    int serviceCountWithStatus(const QString &status) const { int count=0; for(const ServiceRecord &record:serviceRecords) if(record.status==status) ++count; return count; }
    void applyFilters(const QString &query,const QString &type,const QString &station,const QString &status) {
        const QString needle=query.trimmed();
        for(int row=0;row<rowCount();++row) {
            const ServiceRecord &r=serviceRecords[row];
            const QString haystack=(r.id+" "+r.title+" "+r.type+" "+r.status+" "+r.stationCode+" "+r.stationName+" "+r.assignee).toCaseFolded();
            const bool matchesText=needle.isEmpty() || haystack.contains(needle.toCaseFolded());
            const bool matchesType=type=="All Types" || r.type==type;
            const bool matchesStation=station=="All Stations" || r.stationCode==station;
            const bool matchesStatus=status=="All Statuses" || r.status==status;
            setRowHidden(row,!(matchesText&&matchesType&&matchesStation&&matchesStatus));
        }
        updateHeight(); viewport()->update();
    }
    void animateRows(int delayMs,int stepMs) {
        auto *delegate=static_cast<ServiceDelegate *>(itemDelegate());
        for(int row=0;row<rowCount();++row) {
            delegate->setRowOpacity(row,0.0);
            QTimer::singleShot(delayMs+row*stepMs,this,[this,delegate,row]() {
                auto *animation=new QVariantAnimation(this);
                animation->setDuration(430);
                animation->setStartValue(0.0);
                animation->setEndValue(1.0);
                animation->setEasingCurve(QEasingCurve::OutCubic);
                connect(animation,&QVariantAnimation::valueChanged,this,[this,delegate,row](const QVariant &value) {
                    delegate->setRowOpacity(row,value.toReal());
                    viewport()->update();
                });
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            });
        }
    }
    void updateService(int row,const ServiceRecord &record) {
        if(row<0 || row>=serviceRecords.size()) return;
        serviceRecords[row]=record;
        clearContents();
        for(int current=0;current<rowCount();++current) populateRow(current);
        if(item(row,0)) item(row,0)->setCheckState(Qt::Checked);
        viewport()->update(); if(changeCallback) changeCallback();
    }
    void removeService(int row) {
        if(row<0 || row>=serviceRecords.size()) return;
        serviceRecords.removeAt(row);
        setRowCount(serviceRecords.size());
        clearContents();
        for(int current=0;current<rowCount();++current) { populateRow(current); setRowHeight(current,42); }
        updateHeight();
        viewport()->update(); if(changeCallback) changeCallback();
    }
    void sortRecords(int column) {
        const bool ascending=!ascendingSort.value(column,false);
        ascendingSort[column]=ascending;
        QSet<QString> checkedIds;
        for(int row=0;row<rowCount();++row) {
            if(auto *check=item(row,0); check && check->checkState()==Qt::Checked)
                checkedIds.insert(serviceRecords[row].id);
        }
        auto priorityValue=[](const QString &priority) {
            if(priority=="High") return 3;
            if(priority=="Medium") return 2;
            return 1;
        };
        auto lessForColumn=[column,priorityValue](const ServiceRecord &left,const ServiceRecord &right) {
            switch(column) {
            case 1: return left.id.compare(right.id,Qt::CaseInsensitive)<0;
            case 2: return left.title.compare(right.title,Qt::CaseInsensitive)<0;
            case 3: return left.type.compare(right.type,Qt::CaseInsensitive)<0;
            case 4: return left.stationCode.compare(right.stationCode,Qt::CaseInsensitive)<0;
            case 5: return left.assignee.compare(right.assignee,Qt::CaseInsensitive)<0;
            case 6: return QDate::fromString(left.date,"MMM dd, yyyy")<QDate::fromString(right.date,"MMM dd, yyyy");
            case 7: return priorityValue(left.priority)<priorityValue(right.priority);
            case 8: return left.status.compare(right.status,Qt::CaseInsensitive)<0;
            default: return false;
            }
        };
        std::sort(serviceRecords.begin(),serviceRecords.end(),[&](const ServiceRecord &left,const ServiceRecord &right) {
            return ascending ? lessForColumn(left,right) : lessForColumn(right,left);
        });
        clearContents();
        for(int row=0;row<rowCount();++row) {
            populateRow(row);
            setRowHeight(row,42);
            if(checkedIds.contains(serviceRecords[row].id)) item(row,0)->setCheckState(Qt::Checked);
        }
        if(changeCallback) changeCallback();
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QTableWidget::resizeEvent(event); const int widths[]{42,86,190,116,108,130,152,88,112,56};
        int total=0; for(int n:widths) total+=n; int extra=qMax(0,viewport()->width()-total);
        for(int c=0;c<10;++c) setColumnWidth(c,widths[c]+(c==2||c==4||c==5||c==6?extra/4:0));
        updateHeight();
    }
private:
    void populateRow(int row) {
        const ServiceRecord &record=serviceRecords[row];
        const QString values[]{"",record.id,record.title,record.type,record.stationCode+"\n"+record.stationName,record.assignee,record.date+"\n"+record.startTime+" - "+record.endTime,record.priority,record.status,""};
        for(int column=0;column<10;++column) {
            auto *item=new QTableWidgetItem(values[column]);
            if(column==0) { item->setCheckState(Qt::Unchecked); item->setFlags(Qt::ItemIsEnabled); }
            setItem(row,column,item);
        }
    }
    void updateHeight() {
        int total=0,visible=0; const int widths[]{42,86,171,122,116,139,155,88,112,56}; for(int n:widths) total+=n;
        for(int row=0;row<rowCount();++row) if(!isRowHidden(row)) ++visible;
        int h=40+visible*42+4+(viewport()->width()<total?14:0); if(height()!=h) setFixedHeight(h);
    }
};

struct ServiceChartItem { QString label; int value; QColor color; };

class ServiceBarChart final : public QWidget {
    QVector<ServiceChartItem> items;
public:
    explicit ServiceBarChart(QVector<ServiceChartItem> values, QWidget *parent=nullptr)
        : QWidget(parent), items(std::move(values))
    { setMinimumHeight(60+items.size()*44); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(items.isEmpty()) return;
        int total=0, maximum=1;
        for(const auto &item:items) { total+=item.value; maximum=qMax(maximum,item.value); }
        QFont labelFont("Segoe UI"); labelFont.setPixelSize(15); labelFont.setWeight(QFont::DemiBold);
        QFont valueFont("Segoe UI"); valueFont.setPixelSize(16); valueFont.setWeight(QFont::Bold);
        QFont percentFont("Segoe UI"); percentFont.setPixelSize(13);
        int labelW=0; const QFontMetrics lm(labelFont);
        for(const auto &item:items) labelW=qMax(labelW,lm.horizontalAdvance(item.label));
        labelW+=14;
        const int valueW=86, gap=10;
        const int trackX=labelW+gap, trackW=qMax(40,width()-trackX-valueW-gap);
        const int rowH=qBound(30,height()/items.size(),52), barH=qMin(24,rowH-12);
        const int used=rowH*items.size(), top=qMax(0,(height()-used)/2);
        for(int i=0;i<items.size();++i) {
            const auto &item=items[i];
            const int y=top+i*rowH, cy=y+rowH/2;
            p.setFont(labelFont); p.setPen(AppTheme::pen(p, QColor("#17294f")));
            p.drawText(QRect(0,y,labelW,rowH),Qt::AlignLeft|Qt::AlignVCenter,item.label);
            const QRectF track(trackX,cy-barH/2.0,trackW,barH);
            p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, QColor("#edf3fb"))); p.drawRoundedRect(track,barH/2.0,barH/2.0);
            if(item.value>0) {
                const qreal w=qMax<qreal>(barH,trackW*qreal(item.value)/maximum);
                p.setBrush(AppTheme::brush(p, item.color)); p.drawRoundedRect(QRectF(trackX,track.top(),w,barH),barH/2.0,barH/2.0);
            }
            const int pct=total>0?qRound(100.0*item.value/total):0;
            p.setFont(valueFont); p.setPen(AppTheme::pen(p, QColor("#0d2250")));
            p.drawText(QRect(trackX+trackW+gap,y,32,rowH),Qt::AlignRight|Qt::AlignVCenter,QString::number(item.value));
            p.setFont(percentFont); p.setPen(AppTheme::pen(p, QColor("#6a7fa3")));
            p.drawText(QRect(trackX+trackW+gap+34,y,valueW-34,rowH),Qt::AlignRight|Qt::AlignVCenter,QString("%1%").arg(pct));
        }
    }
};

class ServicePerformanceDialog final : public QDialog {
    static QTableWidget *makeTable(const QStringList &headers,const QVector<QStringList> &rows,QWidget *parent) {
        auto *table=new QTableWidget(rows.size(),headers.size(),parent);
        table->setHorizontalHeaderLabels(headers);
        table->verticalHeader()->hide(); table->setShowGrid(false); table->setAlternatingRowColors(true);
        table->setSelectionMode(QAbstractItemView::NoSelection); table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setFocusPolicy(Qt::NoFocus);
        table->setStyleSheet("QTableWidget{background:#fff;alternate-background-color:#f6faff;border:1px solid #dfeaf7;border-radius:10px;font-size:14px;color:#17294f;}"
                             "QHeaderView::section{background:#eef4fc;color:#38507c;font-weight:700;font-size:13px;border:none;padding:8px;}"
                             "QTableWidget::item{padding:6px;}");
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        for(int r=0;r<rows.size();++r) {
            table->setRowHeight(r,38);
            for(int c=0;c<rows[r].size();++c) table->setItem(r,c,new QTableWidgetItem(rows[r][c]));
        }
        return table;
    }
public:
    explicit ServicePerformanceDialog(QWidget *parent):QDialog(parent) {
        setObjectName("serviceStatisticsDialog");
        setWindowTitle("Service Performance — SkySense");
        setMinimumSize(980,640); resize(1100,720);
        QFile stylesheet(":/styles/services.qss");
        if(stylesheet.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
        auto *root=new QVBoxLayout(this); root->setContentsMargins(24,20,24,20); root->setSpacing(12);
        root->addWidget(text("Service Performance","statsTitle"));
        root->addWidget(text("On-time rate and gap between planned and actual duration, by service type.","statsSubtitle"));

        const PerformanceStats all=performanceFor();
        const int rate=onTimeRate(all);
        const int avgGap=all.measured>0?qRound(double(all.actualSum-all.plannedSum)/all.measured):0;
        int lateCount=0; for(const auto &r:serviceRecords) if(finishedLate(r)) ++lateCount;
        auto *kpis=new QHBoxLayout; kpis->setSpacing(12);
        auto addKpi=[&](const QString &label,const QString &value) {
            auto *panel=new QFrame; panel->setObjectName("serviceStatisticsPanel");
            auto *l=new QVBoxLayout(panel); l->setContentsMargins(20,14,20,14); l->setSpacing(2);
            l->addWidget(text(label,"smallMuted")); l->addWidget(text(value,"statsChartTitle"));
            kpis->addWidget(panel,1);
        };
        addKpi("On-time rate",rate>=0?QString("%1%").arg(rate):QString("—"));
        addKpi("Average gap (actual vs planned)",all.measured>0?gapText(avgGap):QString("—"));
        addKpi("Late services",QString::number(lateCount));
        root->addLayout(kpis);

        root->addWidget(text("By service type","formSectionTitle"));
        const QStringList types{"Maintenance","Calibration","Inspection","Repair","Connectivity","Software"};
        QVector<QStringList> typeRows;
        for(const QString &type:types) {
            const PerformanceStats st=performanceFor(type);
            const int tr=onTimeRate(st);
            const int plannedAvg=st.measured>0?st.plannedSum/st.measured:0, actualAvg=st.measured>0?st.actualSum/st.measured:0;
            typeRows.append({type,QString::number(st.completed),QString::number(st.overdue),tr>=0?QString("%1%").arg(tr):QString("—"),
                             st.measured>0?durationText(plannedAvg):QString("—"),st.measured>0?durationText(actualAvg):QString("—"),
                             st.measured>0?gapText(actualAvg-plannedAvg):QString("—")});
        }
        auto *byType=makeTable({"Type","Completed","Overdue","On-time rate","Avg planned","Avg actual","Avg gap"},typeRows,this);
        byType->setMinimumHeight(40+types.size()*38+6);
        root->addWidget(byType);

        root->addWidget(text("Late services (flagged)","formSectionTitle"));
        QVector<QStringList> lateRows;
        for(const auto &r:serviceRecords) {
            if(!finishedLate(r)) continue;
            const QString delay=r.status=="Overdue"?QString("Overdue"):gapText(actualMinutes(r)-plannedMinutes(r))+" late";
            lateRows.append({r.id,r.title,r.type,r.stationCode,r.assignee,delay});
        }
        if(lateRows.isEmpty()) root->addWidget(text("No late service. Every completed service finished on time.","statsCaption"));
        else root->addWidget(makeTable({"ID","Service","Type","Station","Assignee","Delay"},lateRows,this),1);
        if(lateRows.isEmpty()) root->addStretch(1);

        auto *footer=new QHBoxLayout;
        footer->addWidget(text("A service is late when it is Overdue, or Completed after its planned end time.","statsFootnote"),1);
        auto *close=button("Close","",false); close->setMinimumWidth(112); connect(close,&QPushButton::clicked,this,&QDialog::accept);
        footer->addWidget(close); root->addLayout(footer);
    }
};

class ServiceStatisticsDialog final : public QDialog {
public:
    ServiceStatisticsDialog(const QVector<ServiceChartItem> &statuses,
                            const QVector<ServiceChartItem> &types,
                            const QVector<ServiceChartItem> &priorities, QWidget *parent)
        : QDialog(parent) {
        setObjectName("serviceStatisticsDialog");
        setWindowTitle("Service Statistics — SkySense");
        setMinimumSize(1020,560); resize(1200,640);
        QFile stylesheet(":/styles/services.qss");
        if(stylesheet.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
        auto *root=new QVBoxLayout(this); root->setContentsMargins(24,20,24,20); root->setSpacing(14);
        auto *heading=new QVBoxLayout; heading->setSpacing(3);
        heading->addWidget(text("Service Statistics","statsTitle"));
        heading->addWidget(text("A live overview of service status, type and priority distribution.","statsSubtitle"));
        root->addLayout(heading);
        auto *charts=new QHBoxLayout; charts->setSpacing(14);
        auto addPanel=[&](const QString &title,const QString &caption,const QVector<ServiceChartItem> &items) {
            auto *panel=new QFrame; panel->setObjectName("serviceStatisticsPanel");
            auto *layout=new QVBoxLayout(panel); layout->setContentsMargins(22,20,22,18); layout->setSpacing(5);
            int total=0; for(const auto &item:items) total+=item.value;
            layout->addWidget(text(title,"statsChartTitle"));
            layout->addWidget(text(caption,"statsCaption"));
            auto *totalLabel=text(QString("Total: %1 service%2").arg(total).arg(total==1?"":"s"),"statsTotal");
            layout->addWidget(totalLabel); layout->addSpacing(6);
            layout->addWidget(new ServiceBarChart(items,panel),1);
            charts->addWidget(panel,1);
        };
        addPanel("Services by status","Number of current services in each status.",statuses);
        addPanel("Services by type","How the service workload is distributed by type.",types);
        addPanel("Services by priority","Priority levels requiring operational attention.",priorities);
        root->addLayout(charts,1);
        auto *footer=new QHBoxLayout;
        footer->addWidget(text("Charts reflect the services currently shown in the system.","statsFootnote"),1);
        auto *performance=button("Performance","clock",false); performance->setObjectName("servicePerformanceAction");
        QObject::connect(performance,&QPushButton::clicked,this,[this]() { ServicePerformanceDialog dialog(this); dialog.exec(); });
        footer->addWidget(performance);
        auto *close=button("Close","",false); close->setMinimumWidth(112); QObject::connect(close,&QPushButton::clicked,this,&QDialog::accept); footer->addWidget(close);
        root->addLayout(footer);
    }
};

class ServiceExportSelectDialog final : public QDialog {
    QTableWidget *list=nullptr; QPushButton *exportButton=nullptr; QLabel *counter=nullptr;
    QVector<ServiceRecord> records;
    bool bulk=false;
    void refreshState() {
        int n=0; for(int r=0;r<list->rowCount();++r) if(list->item(r,0)->checkState()==Qt::Checked) ++n;
        counter->setText(QString("%1 of %2 selected").arg(n).arg(records.size()));
        exportButton->setEnabled(n>0);
        exportButton->setText(n>0?QString("Export %1 service%2").arg(n).arg(n==1?"":"s"):QString("Export"));
    }
    void setAll(bool on) {
        bulk=true; for(int r=0;r<list->rowCount();++r) list->item(r,0)->setCheckState(on?Qt::Checked:Qt::Unchecked);
        bulk=false; refreshState();
    }
public:
    ServiceExportSelectDialog(const QVector<ServiceRecord> &source,QWidget *parent):QDialog(parent),records(source) {
        setObjectName("serviceStatisticsDialog");
        setWindowTitle("Export Services — SkySense");
        setMinimumSize(820,480); resize(960,560);
        QFile stylesheet(":/styles/services.qss");
        if(stylesheet.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
        auto *root=new QVBoxLayout(this); root->setContentsMargins(24,20,24,20); root->setSpacing(12);
        root->addWidget(text("Choose services to export","statsTitle"));
        root->addWidget(text("Tick one or more services. Only the selected ones will be included in the PDF.","statsSubtitle"));
        auto *tools=new QHBoxLayout; tools->setSpacing(8);
        auto *all=button("Select all","",false); auto *none=button("Clear selection","",false);
        all->setMinimumHeight(36); none->setMinimumHeight(36);
        counter=text("","statsCaption"); counter->setWordWrap(false);
        tools->addWidget(all); tools->addWidget(none); tools->addStretch(1); tools->addWidget(counter);
        root->addLayout(tools);
        list=new QTableWidget(records.size(),6,this);
        list->setHorizontalHeaderLabels({"","ID","Service","Station","Priority","Status"});
        list->verticalHeader()->hide(); list->setShowGrid(false); list->setAlternatingRowColors(true);
        list->setSelectionMode(QAbstractItemView::NoSelection); list->setEditTriggers(QAbstractItemView::NoEditTriggers);
        list->setFocusPolicy(Qt::NoFocus);
        list->setStyleSheet("QTableWidget{background:#fff;alternate-background-color:#f6faff;border:1px solid #dfeaf7;border-radius:10px;font-size:14px;color:#17294f;}"
                            "QHeaderView::section{background:#eef4fc;color:#38507c;font-weight:700;font-size:13px;border:none;padding:8px;}"
                            "QTableWidget::item{padding:6px;}");
        auto *header=list->horizontalHeader(); header->setSectionResizeMode(QHeaderView::ResizeToContents);
        header->setSectionResizeMode(2,QHeaderView::Stretch); header->setSectionResizeMode(3,QHeaderView::Stretch);
        header->setStretchLastSection(false);
        for(int r=0;r<records.size();++r) {
            const auto &rec=records[r];
            auto *check=new QTableWidgetItem; check->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable); check->setCheckState(Qt::Unchecked);
            list->setItem(r,0,check);
            const QStringList cols{rec.id,rec.title,rec.stationCode+" — "+rec.stationName,rec.priority,rec.status};
            for(int c=0;c<cols.size();++c) { auto *it=new QTableWidgetItem(cols[c]); it->setFlags(Qt::ItemIsEnabled); list->setItem(r,c+1,it); }
            list->setRowHeight(r,40);
        }
        root->addWidget(list,1);
        auto *footer=new QHBoxLayout; footer->addStretch(1);
        auto *cancel=button("Cancel","",false); cancel->setMinimumWidth(112);
        exportButton=button("Export","export",true); exportButton->setMinimumWidth(180);
        footer->addWidget(cancel); footer->addWidget(exportButton); root->addLayout(footer);
        connect(list,&QTableWidget::cellClicked,this,[this](int row,int col) {
            if(col==0) return;
            auto *it=list->item(row,0); it->setCheckState(it->checkState()==Qt::Checked?Qt::Unchecked:Qt::Checked);
        });
        connect(list,&QTableWidget::itemChanged,this,[this](QTableWidgetItem *) { if(!bulk) refreshState(); });
        connect(all,&QPushButton::clicked,this,[this]() { setAll(true); });
        connect(none,&QPushButton::clicked,this,[this]() { setAll(false); });
        connect(cancel,&QPushButton::clicked,this,&QDialog::reject);
        connect(exportButton,&QPushButton::clicked,this,&QDialog::accept);
        refreshState();
    }
    QVector<ServiceRecord> selected() const {
        QVector<ServiceRecord> out;
        for(int r=0;r<list->rowCount();++r) if(list->item(r,0)->checkState()==Qt::Checked) out.append(records[r]);
        return out;
    }
};

QWidget *formField(const QString &title,QWidget *control,bool required=true)
{
    auto *field=new QWidget; auto *layout=new QVBoxLayout(field); layout->setContentsMargins(0,0,0,0); layout->setSpacing(5);
    layout->addWidget(text(required?title+" *":title,"fieldLabel")); layout->addWidget(control); return field;
}

bool confirmDeleteService(QWidget *parent,const ServiceRecord &record);

class AddServiceDialog final : public QDialog {
    QLineEdit *idEdit=nullptr,*titleEdit=nullptr;
    QComboBox *typeCombo=nullptr,*stationCombo=nullptr,*assigneeCombo=nullptr,*priorityCombo=nullptr,*statusCombo=nullptr;
    QDateEdit *dateEdit=nullptr;
    QTimeEdit *startEdit=nullptr,*endEdit=nullptr;
    QTimeEdit *actualEndEdit=nullptr;
    QLabel *durationPreview=nullptr;
    double keptQuantity=1.0,keptUnitPrice=0.0;   // not shown in the form; kept as stored in Oracle
    QTextEdit *descriptionEdit=nullptr;
    QLabel *descriptionCount=nullptr,*stationCodePreview=nullptr,*stationNamePreview=nullptr,*datePreview=nullptr,*timePreview=nullptr;
    QVBoxLayout *checklistLayout=nullptr;
    QStringList checklistItems;
    QList<QCheckBox *> checklistChecks;
    QPushButton *addChecklistButton=nullptr;
    bool editingMode=false;
    bool deleteRequestedFlag=false;
public:
    explicit AddServiceDialog(QWidget *parent=nullptr,const ServiceRecord *existing=nullptr):QDialog(parent),editingMode(existing!=nullptr) {
        setObjectName("addServiceDialog");
        if(existing) { keptQuantity=existing->quantity; keptUnitPrice=existing->unitPrice; }
        setWindowTitle(editingMode?"Edit Service — SkySense":"Add Service — SkySense");
        setWindowModality(Qt::WindowModal);
        setWindowFlags(Qt::Dialog|Qt::WindowTitleHint|Qt::WindowCloseButtonHint);
        QScreen *screen=QApplication::primaryScreen();
        if(parent) {
            if(QScreen *parentScreen=QApplication::screenAt(parent->mapToGlobal(parent->rect().center()))) screen=parentScreen;
        }
        const QRect available=screen?screen->availableGeometry():QRect(0,0,1280,800);
        const int maxWidth=qMax(760,available.width()-64);
        const int maxHeight=qMax(620,available.height()-84);
        setMaximumSize(maxWidth,maxHeight);
        setMinimumSize(qMin(editingMode?900:760,maxWidth),qMin(editingMode?620:580,maxHeight));
        resize(qMin(editingMode?1120:860,maxWidth),qMin(editingMode?790:740,maxHeight));
        QFile dialogStylesheet(":/styles/services.qss");
        if(dialogStylesheet.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(dialogStylesheet.readAll()));

        auto *root=new QVBoxLayout(this);
        root->setContentsMargins(22,18,22,18);
        root->setSpacing(14);

        auto *heading=new QHBoxLayout;
        heading->setSpacing(18);
        auto *mark=new QLabel(editingMode?QString::fromUtf8("✎"):"+");
        mark->setObjectName("serviceFormPlus");
        mark->setAlignment(Qt::AlignCenter);
        mark->setFixedSize(66,66);
        heading->addWidget(mark);
        auto *headingCopy=new QVBoxLayout;
        headingCopy->setSpacing(3);
        headingCopy->addWidget(text(editingMode?"Edit Service":"Add Service","dialogTitle"));
        headingCopy->addWidget(text(editingMode?"Update the details of this field service.":"Register and schedule a new field service.","dialogSubtitle"));
        heading->addLayout(headingCopy,1);
        auto *idCard=new QWidget;
        idCard->setObjectName("autoIdCard");
        auto *idLayout=new QVBoxLayout(idCard);
        idLayout->setContentsMargins(16,9,16,9);
        idLayout->setSpacing(2);
        idLayout->addWidget(text(editingMode?"SERVICE ID":"AUTO ID","smallMuted"));
        idEdit=new QLineEdit(existing?existing->id:nextServiceId());
        idEdit->setObjectName("serviceAutoId");
        idEdit->setReadOnly(true);
        idLayout->addWidget(idEdit);
        heading->addWidget(idCard,0,Qt::AlignVCenter);
        root->addLayout(heading);

        auto *divider=new QFrame;
        divider->setObjectName("serviceFormDivider");
        divider->setFixedHeight(1);
        root->addWidget(divider);

        auto *scroll=new QScrollArea;
        scroll->setObjectName("serviceFormScroll");
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto *card=new QFrame;
        card->setObjectName("serviceDetailsCard");
        auto *cardLayout=new QVBoxLayout(card);
        cardLayout->setContentsMargins(24,22,24,22);
        cardLayout->setSpacing(14);
        cardLayout->addWidget(text("Service details","serviceCardTitle"));
        cardLayout->addWidget(text(editingMode?"Review and update the service assignment, schedule, and status.":"Choose the service type, station, schedule, and status.","dialogSubtitle"));

        auto *detailsGrid=new QGridLayout;
        detailsGrid->setContentsMargins(0,6,0,0);
        detailsGrid->setHorizontalSpacing(18);
        detailsGrid->setVerticalSpacing(14);
        titleEdit=new QLineEdit;
        titleEdit->setPlaceholderText("Enter service title");
        typeCombo=selector({"Maintenance","Calibration","Inspection","Repair","Connectivity","Software"});
        stationCombo=selector({"Select station"});
        for(const auto &station:serviceStationChoices) {
            stationCombo->addItem(station.code+" — "+station.name);
            const int index=stationCombo->count()-1;
            stationCombo->setItemData(index,station.code,Qt::UserRole);
            stationCombo->setItemData(index,station.name,Qt::UserRole+1);
            stationCombo->setItemData(index,station.id,Qt::UserRole+2);
        }
        assigneeCombo=selector({"Select assignee"});
        for(const auto &person:servicePersonChoices) { assigneeCombo->addItem(person.name); assigneeCombo->setItemData(assigneeCombo->count()-1,person.id,Qt::UserRole); }
        dateEdit=new QDateEdit(QDate::currentDate()); dateEdit->setDisplayFormat("MMM dd, yyyy"); dateEdit->setCalendarPopup(true); dateEdit->setFixedHeight(48);
        startEdit=new QTimeEdit(QTime(8,0)); startEdit->setDisplayFormat("HH:mm"); startEdit->setFixedHeight(48);
        endEdit=new QTimeEdit(QTime(10,0)); endEdit->setDisplayFormat("HH:mm"); endEdit->setFixedHeight(48);
        priorityCombo=selector({"High","Medium","Low"}); priorityCombo->setCurrentText("Medium");
        statusCombo=selector({"Scheduled","In Progress","Completed","Overdue"});
        actualEndEdit=new QTimeEdit(QTime(10,0)); actualEndEdit->setDisplayFormat("HH:mm"); actualEndEdit->setFixedHeight(48);
        durationPreview=text(QString(),"statsTotal"); durationPreview->setWordWrap(false);
        if(existing) {
            titleEdit->setText(existing->title);
            typeCombo->setCurrentText(existing->type);
            for(int i=1;i<stationCombo->count();++i) if(stationCombo->itemData(i,Qt::UserRole).toString()==existing->stationCode) stationCombo->setCurrentIndex(i);
            assigneeCombo->setCurrentText(existing->assignee);
            const QDate savedDate=QDate::fromString(existing->date,"MMM dd, yyyy"); if(savedDate.isValid()) dateEdit->setDate(savedDate);
            const QTime savedStart=QTime::fromString(existing->startTime,"HH:mm"); if(savedStart.isValid()) startEdit->setTime(savedStart);
            const QTime savedEnd=QTime::fromString(existing->endTime,"HH:mm"); if(savedEnd.isValid()) endEdit->setTime(savedEnd);
            priorityCombo->setCurrentText(existing->priority);
            statusCombo->setCurrentText(existing->status);
            const QTime savedActual=QTime::fromString(existing->actualEndTime,"HH:mm");
            actualEndEdit->setTime(savedActual.isValid()?savedActual:endEdit->time());
        }
        detailsGrid->addWidget(formField("Title",titleEdit),0,0);
        detailsGrid->addWidget(formField("Type",typeCombo),0,1);
        detailsGrid->addWidget(formField("Station",stationCombo),1,0);
        detailsGrid->addWidget(formField("Assignee",assigneeCombo),1,1);
        detailsGrid->addWidget(formField("Scheduled Date",dateEdit),2,0);
        detailsGrid->addWidget(formField("Start Time",startEdit),2,1);
        detailsGrid->addWidget(formField("End Time",endEdit),2,2);
        detailsGrid->addWidget(formField("Priority",priorityCombo),3,0);
        detailsGrid->addWidget(formField("Status",statusCombo),3,1);
        detailsGrid->addWidget(formField("Actual End Time",actualEndEdit,false),3,2);
        detailsGrid->addWidget(durationPreview,5,0,1,3);
        for(int column=0;column<3;++column) detailsGrid->setColumnStretch(column,1);
        cardLayout->addLayout(detailsGrid);
        // I2 (performance): live comparison between planned and actual duration.
        auto updateDuration=[this]() {
            const bool done=statusCombo->currentText()=="Completed";
            actualEndEdit->setEnabled(done);
            const QString from=startEdit->time().toString("HH:mm");
            const int planned=minutesBetween(from,endEdit->time().toString("HH:mm"));
            QString info=QString("Planned duration: %1").arg(durationText(planned));
            if(done) {
                const int actual=minutesBetween(from,actualEndEdit->time().toString("HH:mm"));
                info+=QString("   •   Actual: %1 (%2 vs planned)").arg(durationText(actual),gapText(actual-planned));
            }
            durationPreview->setText(info);
        };
        connect(statusCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,[updateDuration](int) { updateDuration(); });
        connect(startEdit,&QTimeEdit::timeChanged,this,[updateDuration](const QTime &) { updateDuration(); });
        connect(endEdit,&QTimeEdit::timeChanged,this,[this,updateDuration](const QTime &t) {
            if(statusCombo->currentText()!="Completed") actualEndEdit->setTime(t);
            updateDuration();
        });
        connect(actualEndEdit,&QTimeEdit::timeChanged,this,[updateDuration](const QTime &) { updateDuration(); });
        updateDuration();

        descriptionEdit=new QTextEdit;
        descriptionEdit->setPlaceholderText("Enter service description...");
        descriptionEdit->setMinimumHeight(74);
        descriptionEdit->setMaximumHeight(92);
        if(existing) descriptionEdit->setPlainText(existing->description);
        auto *descriptionField=new QVBoxLayout;
        descriptionField->setSpacing(5);
        descriptionField->addWidget(text("Description","fieldLabel"));
        descriptionField->addWidget(descriptionEdit);
        descriptionCount=text(QString("%1/500").arg(descriptionEdit->toPlainText().size()),"characterCount");
        descriptionCount->setAlignment(Qt::AlignRight);
        descriptionField->addWidget(descriptionCount);
        cardLayout->addLayout(descriptionField);
        connect(descriptionEdit,&QTextEdit::textChanged,this,[this]() {
            QString value=descriptionEdit->toPlainText();
            if(value.size()>500) { value.truncate(500); descriptionEdit->setPlainText(value); }
            descriptionCount->setText(QString("%1/500").arg(value.size()));
        });

        cardLayout->addWidget(text("Service Checklist","formSectionTitle"));
        checklistLayout=new QVBoxLayout;
        checklistLayout->setContentsMargins(0,0,0,0);
        checklistLayout->setSpacing(7);
        cardLayout->addLayout(checklistLayout);
        if(existing) {
            for(int i=0;i<existing->checklist.size();++i) {
                addChecklistRow(existing->checklist[i],true);
                if(i<checklistChecks.size() && i<existing->checklistDone.size()) checklistChecks[i]->setChecked(existing->checklistDone[i]);
            }
        } else {
            addChecklistRow("Check equipment status and perform visual inspection");
            addChecklistRow("Calibrate sensors and verify readings");
            addChecklistRow("Update maintenance logs and reports");
        }
        addChecklistButton=button("Add checklist item","plus");
        addChecklistButton->setObjectName("addChecklistItem");
        addChecklistButton->setProperty("link",true);
        addChecklistButton->setFixedHeight(34);
        connect(addChecklistButton,&QPushButton::clicked,this,[this]() { addChecklistRow(QString()); });
        cardLayout->addWidget(addChecklistButton,0,Qt::AlignLeft);
        scroll->setWidget(card);

        if(editingMode) {
            auto *columns=new QHBoxLayout;
            columns->setSpacing(14);
            columns->addWidget(scroll,3);
            auto *summary=new QFrame;
            summary->setObjectName("serviceSummary");
            summary->setMinimumWidth(245);
            summary->setMaximumWidth(320);
            auto *summaryLayout=new QVBoxLayout(summary);
            summaryLayout->setContentsMargins(18,18,18,18);
            summaryLayout->setSpacing(13);
            auto *stationHeading=new QHBoxLayout;
            stationHeading->addWidget(new Glyph("pin",blue,42,true));
            stationHeading->addWidget(text("Station Information","detailTitle"));
            stationHeading->addStretch();
            summaryLayout->addLayout(stationHeading);
            stationCodePreview=text(existing->stationCode,"summaryStrong");
            stationNamePreview=text(existing->stationName,"summaryText");
            summaryLayout->addWidget(stationCodePreview);
            summaryLayout->addWidget(stationNamePreview);
            auto *separator=new QFrame; separator->setFrameShape(QFrame::HLine); separator->setProperty("role","formSeparator"); summaryLayout->addWidget(separator);
            summaryLayout->addWidget(text("Selected Date & Time","detailTitle"));
            datePreview=text(dateEdit->date().toString("MMM dd, yyyy"),"summaryText");
            timePreview=text(startEdit->time().toString("HH:mm")+" – "+endEdit->time().toString("HH:mm"),"summaryText");
            summaryLayout->addWidget(datePreview);
            summaryLayout->addWidget(timePreview);
            summaryLayout->addStretch();
            columns->addWidget(summary,1);
            root->addLayout(columns,1);
            connect(stationCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]() { updateSummary(); });
            connect(dateEdit,&QDateEdit::dateChanged,this,[this]() { updateSummary(); });
            connect(startEdit,&QTimeEdit::timeChanged,this,[this]() { updateSummary(); });
            connect(endEdit,&QTimeEdit::timeChanged,this,[this]() { updateSummary(); });
        } else {
            root->addWidget(scroll,1);
        }

        auto *footer=new QHBoxLayout;
        auto *cancel=button("Cancel",""); cancel->setObjectName("cancelService");
        if(editingMode) {
            auto *remove=button("Delete","trash"); remove->setObjectName("deleteEditedService");
            footer->addStretch();
            footer->addWidget(remove);
            auto *save=button("Save Changes","check",true); save->setObjectName("createService");
            footer->addWidget(cancel); footer->addWidget(save);
            connect(remove,&QPushButton::clicked,this,[this]() {
                if(confirmDeleteService(this,service())) {
                    deleteRequestedFlag=true; accept();
                }
            });
            connect(save,&QPushButton::clicked,this,&AddServiceDialog::createService);
        } else {
            footer->addWidget(text("The new service will appear in the Services list.","dialogSubtitle"),1);
            auto *create=button("Add Service","plus",true); create->setObjectName("createService");
            footer->addWidget(cancel); footer->addWidget(create);
            connect(create,&QPushButton::clicked,this,&AddServiceDialog::createService);
        }
        root->addLayout(footer);
        connect(cancel,&QPushButton::clicked,this,&QDialog::reject);
    }

    ServiceRecord service() const {
        ServiceRecord record;
        record.id=idEdit->text().trimmed();
        record.title=titleEdit->text().trimmed();
        record.type=typeCombo->currentText();
        record.stationCode=stationCombo->currentData(Qt::UserRole).toString();
        record.stationName=stationCombo->currentData(Qt::UserRole+1).toString();
        record.stationId=stationCombo->currentData(Qt::UserRole+2).toInt();
        record.assignee=assigneeCombo->currentText();
        record.personnelId=assigneeCombo->currentData(Qt::UserRole).toInt();
        record.date=dateEdit->date().toString("MMM dd, yyyy");
        record.startTime=startEdit->time().toString("HH:mm");
        record.endTime=endEdit->time().toString("HH:mm");
        record.priority=priorityCombo->currentText();
        record.status=statusCombo->currentText();
        record.description=descriptionEdit->toPlainText();
        record.checklist=checklistItems;
        for(const QCheckBox *check:checklistChecks) record.checklistDone.append(check&&check->isChecked());
        record.quantity=keptQuantity; record.unitPrice=keptUnitPrice;
        record.actualEndTime=statusCombo->currentText()=="Completed"?actualEndEdit->time().toString("HH:mm"):QString();
        return record;
    }
    bool deleteRequested() const { return deleteRequestedFlag; }
private:
    void updateSummary() {
        const int index=stationCombo->currentIndex();
        if(index>0) {
            stationCodePreview->setText(stationCombo->currentData(Qt::UserRole).toString());
            stationNamePreview->setText(stationCombo->currentData(Qt::UserRole+1).toString());
        }
        datePreview->setText(dateEdit->date().toString("MMM dd, yyyy"));
        timePreview->setText(startEdit->time().toString("HH:mm")+" – "+endEdit->time().toString("HH:mm"));
    }
    void addChecklistRow(const QString &value,bool legacy=false) {
        if(!legacy && checklistLayout->count()>=3) {
            QMessageBox::information(this,"Checklist limit","A service can contain a maximum of 3 local checklist tasks.");
            return;
        }
        auto *row=new QWidget;
        row->setProperty("role","checklistRow");
        auto *layout=new QHBoxLayout(row);
        layout->setContentsMargins(8,3,8,3);
        layout->setSpacing(8);
        auto *done=new QCheckBox;
        done->setObjectName("serviceChecklistDone");
        auto *item=new QLineEdit(value);
        item->setPlaceholderText("Enter checklist item");
        item->setProperty("checklistInput",true);
        auto *remove=new QPushButton(QString::fromUtf8("×"));
        remove->setProperty("role","removeChecklist");
        remove->setFixedSize(28,28);
        layout->addWidget(done);
        layout->addWidget(item,1);
        layout->addWidget(remove);
        checklistLayout->addWidget(row);
        checklistChecks.append(done);
        connect(remove,&QPushButton::clicked,this,[this,row,done]() {
            checklistChecks.removeAll(done);
            checklistLayout->removeWidget(row);
            row->deleteLater();
            updateChecklistButton();
        });
        updateChecklistButton();
    }
    void updateChecklistButton() {
        if(!addChecklistButton) return;
        const int count=checklistLayout->count();
        addChecklistButton->setEnabled(count<3);
        addChecklistButton->setText(count<3?"Add checklist item": "Maximum of 3 checklist tasks");
    }
    void createService() {
        if(titleEdit->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Service title required","Enter a title for this service."); titleEdit->setFocus(); return; }
        if(stationCombo->currentIndex()==0) { QMessageBox::warning(this,"Station required","Select a weather station for this service."); stationCombo->setFocus(); return; }
        if(assigneeCombo->currentIndex()==0) { QMessageBox::warning(this,"Assignee required","Select a staff member to assign this service to."); assigneeCombo->setFocus(); return; }
        if(endEdit->time()<=startEdit->time()) { QMessageBox::warning(this,"Schedule time invalid","End time must be later than start time."); endEdit->setFocus(); return; }
        if(statusCombo->currentText()=="Completed" && actualEndEdit->time()<=startEdit->time()) { QMessageBox::warning(this,"Actual end time invalid","The actual end time must be later than the start time."); actualEndEdit->setFocus(); return; }
        if(checklistLayout->count()==0) { QMessageBox::warning(this,"Checklist required","Add at least one local checklist task before saving this service."); return; }
        if(checklistLayout->count()>3) { QMessageBox::warning(this,"Checklist limit","This older service has more than three checklist tasks. Remove the extra tasks before saving."); return; }
        checklistItems.clear();
        for(int i=0;i<checklistLayout->count();++i) {
            auto *row=checklistLayout->itemAt(i)->widget();
            if(!row) continue;
            auto *edit=row->findChild<QLineEdit *>();
            if(edit && !edit->text().trimmed().isEmpty()) checklistItems.append(edit->text().trimmed());
        }
        if(checklistItems.isEmpty()) { QMessageBox::warning(this,"Checklist required","Enter at least one checklist task before saving this service."); return; }
        if(statusCombo->currentText()=="Completed") {
            const bool allDone=std::all_of(checklistChecks.cbegin(),checklistChecks.cend(),[](const QCheckBox *check) { return check && check->isChecked(); });
            if(!allDone) {
                QMessageBox::warning(this,"Complete the checklist","Mark every checklist task as completed before setting the service status to Completed.");
                return;
            }
        }
        accept();
    }
};
class DeleteServiceDialog final : public QDialog {
public:
    explicit DeleteServiceDialog(QWidget *parent,const ServiceRecord &record):QDialog(parent) {
        setObjectName("deleteServiceDialog");
        setWindowTitle("Delete Service — SkySense");
        setWindowModality(Qt::WindowModal);
        setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        QScreen *screen=parent?QApplication::screenAt(parent->mapToGlobal(parent->rect().center())):QApplication::primaryScreen();
        if(!screen) screen=QApplication::primaryScreen();
        const QRect available=screen?screen->availableGeometry():QRect(0,0,1280,800);
        const int maxWidth=qMax(760,available.width()-64);
        const int maxHeight=qMax(600,available.height()-90);
        setMaximumSize(maxWidth,maxHeight);
        setMinimumSize(qMin(900,maxWidth),qMin(620,maxHeight));
        resize(qMin(1080,maxWidth),qMin(820,maxHeight));
        QFile stylesheet(":/styles/services.qss");
        if(stylesheet.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
        auto *shadow=new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(24); shadow->setOffset(0,8); shadow->setColor(QColor(8,26,58,75));
        setGraphicsEffect(shadow);

        auto *root=new QVBoxLayout(this);
        root->setContentsMargins(20,18,20,18);
        root->setSpacing(14);
        auto *surface=new QWidget;
        surface->setObjectName("deleteServiceSurface");
        auto *surfaceLayout=new QVBoxLayout(surface);
        surfaceLayout->setContentsMargins(22,18,22,18);
        surfaceLayout->setSpacing(14);

        auto *heading=new QHBoxLayout;
        heading->setSpacing(18);
        heading->addWidget(new Glyph("trash",QColor("#ff4055"),72,true));
        auto *headingCopy=new QVBoxLayout;
        headingCopy->setSpacing(3);
        headingCopy->addWidget(text("Delete Service","deleteDialogTitle"));
        headingCopy->addWidget(text("Are you sure you want to delete this service? This action cannot be undone.","dialogSubtitle"));
        heading->addLayout(headingCopy,1);
        auto *close=new QPushButton(QString::fromUtf8("×"));
        close->setObjectName("dialogClose");
        close->setFixedSize(36,36);
        close->setCursor(Qt::PointingHandCursor);
        heading->addWidget(close,0,Qt::AlignTop);
        surfaceLayout->addLayout(heading);
        connect(close,&QPushButton::clicked,this,[this]() { reject(); });
        auto *divider=new QFrame; divider->setObjectName("serviceFormDivider"); divider->setFixedHeight(1); surfaceLayout->addWidget(divider);

        auto *columns=new QHBoxLayout;
        columns->setSpacing(16);
        auto *scroll=new QScrollArea;
        scroll->setObjectName("serviceFormScroll");
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto *details=new QFrame;
        details->setObjectName("deleteServiceDetailsCard");
        auto *detailsLayout=new QVBoxLayout(details);
        detailsLayout->setContentsMargins(18,14,18,16);
        detailsLayout->setSpacing(12);
        auto *detailsHeading=new QHBoxLayout;
        detailsHeading->addWidget(new Glyph("description",blue,24));
        detailsHeading->addWidget(text("Service Details","formSectionTitle"));
        detailsHeading->addStretch();
        detailsLayout->addLayout(detailsHeading);

        auto *grid=new QGridLayout;
        grid->setContentsMargins(0,0,0,0);
        grid->setHorizontalSpacing(20);
        grid->setVerticalSpacing(10);
        auto addValue=[&](const QString &label,const QString &value,int row,int col,int span=1) {
            auto *block=new QWidget;
            auto *layout=new QVBoxLayout(block);
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(3);
            layout->addWidget(text(label,"smallMuted"));
            layout->addWidget(text(value.isEmpty()?"—":value,"smallValue"));
            grid->addWidget(block,row,col,1,span);
        };
        addValue("Service ID",record.id,0,0);
        addValue("Title",record.title,0,1);
        addValue("Type",record.type,1,0);
        addValue("Station",record.stationCode+" — "+record.stationName,1,1);
        addValue("Assignee",record.assignee,2,0,2);
        detailsLayout->addLayout(grid);
        auto *scheduleDivider=new QFrame; scheduleDivider->setProperty("role","formSeparator"); scheduleDivider->setFixedHeight(1); detailsLayout->addWidget(scheduleDivider);
        auto *schedule=new QGridLayout;
        schedule->setContentsMargins(0,0,0,0);
        schedule->setHorizontalSpacing(18);
        auto addSchedule=[&](const QString &label,const QString &value,int col) {
            auto *block=new QWidget;
            auto *layout=new QVBoxLayout(block);
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(3);
            layout->addWidget(text(label,"smallMuted"));
            layout->addWidget(text(value,"smallValue"));
            schedule->addWidget(block,0,col);
        };
        addSchedule("Scheduled Date",record.date,0);
        addSchedule("Start Time",record.startTime,1);
        addSchedule("End Time",record.endTime,2);
        detailsLayout->addLayout(schedule);
        auto *statusGrid=new QGridLayout;
        statusGrid->setContentsMargins(0,0,0,0);
        statusGrid->setHorizontalSpacing(20);
        auto addStatus=[&](const QString &label,const QString &value,int col) {
            auto *block=new QWidget;
            auto *layout=new QVBoxLayout(block);
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(3);
            layout->addWidget(text(label,"smallMuted"));
            layout->addWidget(text(value,"smallValue"));
            statusGrid->addWidget(block,0,col);
        };
        addStatus("Priority",record.priority,0);
        addStatus("Status",record.status,1);
        detailsLayout->addWidget(new QFrame);
        detailsLayout->addLayout(statusGrid);
        auto *descriptionDivider=new QFrame; descriptionDivider->setProperty("role","formSeparator"); descriptionDivider->setFixedHeight(1); detailsLayout->addWidget(descriptionDivider);
        detailsLayout->addWidget(text("Description","smallMuted"));
        detailsLayout->addWidget(text(record.description.isEmpty()?"—":record.description,"smallValue"));
        auto *checkHeading=new QHBoxLayout;
        checkHeading->addWidget(new Glyph("checklist",blue,21));
        checkHeading->addWidget(text("Service Checklist","formSectionTitle"));
        checkHeading->addStretch();
        detailsLayout->addLayout(checkHeading);
        if(record.checklist.isEmpty()) {
            detailsLayout->addWidget(text("—","smallMuted"));
        } else {
            for(const QString &item:record.checklist) {
                auto *row=new QHBoxLayout;
                row->addWidget(new Glyph("check",blue,18));
                row->addWidget(text(item,"smallValue"),1);
                detailsLayout->addLayout(row);
            }
        }
        scroll->setWidget(details);
        columns->addWidget(scroll,3);

        auto *side=new QVBoxLayout;
        side->setSpacing(14);
        auto *station=new QFrame;
        station->setObjectName("serviceSummary");
        station->setMinimumWidth(230);
        station->setMaximumWidth(300);
        auto *stationLayout=new QVBoxLayout(station);
        stationLayout->setContentsMargins(16,16,16,16);
        stationLayout->setSpacing(9);
        auto *stationHeading=new QHBoxLayout;
        stationHeading->addWidget(new Glyph("pin",blue,30));
        stationHeading->addWidget(text("Station Information","detailTitle"));
        stationHeading->addStretch();
        stationLayout->addLayout(stationHeading);
        stationLayout->addWidget(text(record.stationCode,"summaryStrong"));
        stationLayout->addWidget(text(record.stationName,"summaryText"));
        auto *sideDivider=new QFrame; sideDivider->setProperty("role","formSeparator"); sideDivider->setFixedHeight(1); stationLayout->addWidget(sideDivider);
        auto *timeHeading=new QHBoxLayout;
        timeHeading->addWidget(new Glyph("calendar",blue,26));
        timeHeading->addWidget(text("Selected Date & Time","detailTitle"));
        timeHeading->addStretch();
        stationLayout->addLayout(timeHeading);
        stationLayout->addWidget(text(record.date,"summaryText"));
        stationLayout->addWidget(text(record.startTime+" – "+record.endTime,"summaryText"));
        side->addWidget(station);

        auto *warning=new QWidget;
        warning->setObjectName("deleteServiceWarning");
        auto *warningLayout=new QVBoxLayout(warning);
        warningLayout->setContentsMargins(14,12,14,12);
        warningLayout->setSpacing(7);
        auto *warningTitle=new QHBoxLayout;
        warningTitle->addWidget(new Glyph("warning",QColor("#f04455"),25));
        warningTitle->addWidget(text("This action is permanent","deleteWarningTitle"));
        warningTitle->addStretch();
        warningLayout->addLayout(warningTitle);
        warningLayout->addWidget(text("The service will be removed from the list and cannot be recovered.","smallMuted"));
        side->addWidget(warning);
        side->addStretch();
        columns->addLayout(side,1);
        surfaceLayout->addLayout(columns,1);

        auto *footer=new QHBoxLayout;
        footer->addStretch();
        auto *cancel=button("Cancel",""); cancel->setObjectName("cancelService");
        auto *confirm=button("Delete Service","trash"); confirm->setObjectName("confirmDeleteService");
        footer->addWidget(cancel);
        footer->addWidget(confirm);
        surfaceLayout->addLayout(footer);
        connect(cancel,&QPushButton::clicked,this,&QDialog::reject);
        connect(confirm,&QPushButton::clicked,this,&QDialog::accept);
        root->addWidget(surface);
    }
};

bool confirmDeleteService(QWidget *parent,const ServiceRecord &record)
{
    if(!parent) return false;
    auto *overlay=new QWidget(parent);
    overlay->setObjectName("modalDimmer");
    overlay->setAttribute(Qt::WA_StyledBackground,true);
    overlay->setStyleSheet("background:rgba(8,26,58,145)");
    overlay->setGeometry(parent->rect());
    overlay->show();
    overlay->raise();
    DeleteServiceDialog dialog(parent,record);
    dialog.setWindowOpacity(0.0);
    QTimer::singleShot(0,&dialog,[&dialog]() {
        auto *fade=new QPropertyAnimation(&dialog,"windowOpacity",&dialog);
        fade->setDuration(240);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    });
    const QPoint center=parent->mapToGlobal(parent->rect().center());
    dialog.move(center-QPoint(dialog.width()/2,dialog.height()/2));
    dialog.raise();
    const bool confirmed=dialog.exec()==QDialog::Accepted;
    overlay->hide();
    overlay->deleteLater();
    return confirmed;
}
QWidget *detailInfo()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(5);
    v->addWidget(text("Service Details","detailTitle"));
    const QStringList fields{"Service ID|—","Type|—","Station|—","Scheduled|—","Assigned To|—","Priority|—"};
    for(const auto &field:fields) { auto *row=new QHBoxLayout; row->addWidget(text(field.section('|',0,0),"smallMuted")); row->addStretch(); row->addWidget(text(field.section('|',1,1),"smallValue")); v->addLayout(row); }
    v->addWidget(text("Choose a service to view its saved details.","recommendationCopy")); return box;
}

QWidget *checklist()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(7); v->addWidget(text("Service Checklist","detailTitle"));
    v->addWidget(text("No service selected.","recommendationCopy")); return box;
}

QWidget *timeline()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(7); v->addWidget(text("Completion Summary","detailTitle"));
    auto *headline=text("No completion data selected","smallValue"); v->addWidget(headline);
    v->addWidget(text("Select a service to review its saved checklist and status.","smallMuted"));
    const QStringList items{"Completed tasks|—","Pending tasks|—","Current status|—","Planned duration|—"};
    for(const QString &item:items) { auto *row=new QHBoxLayout; row->addWidget(text(item.section('|',0,0),"smallMuted")); row->addStretch(); row->addWidget(text(item.section('|',1,1),"smallValue")); v->addLayout(row); }
    v->addStretch(); return box;
}

QWidget *stationInfo()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(5); v->addWidget(text("Station Information","detailTitle"));
    v->addWidget(text("No station selected.","smallMuted"));
    auto *link=button("View Station","external"); link->setProperty("link",true); v->addWidget(link,0,Qt::AlignLeft); v->addStretch(); return box;
}

class ServicesPage final : public QWidget {
    QGridLayout *detailGrid=nullptr; QList<QWidget *> detailPanels; QVBoxLayout *body=nullptr; ServiceTable *serviceTable=nullptr; QDateEdit *reportDatePicker=nullptr; int detailColumns=0;
    QLineEdit *filterSearch=nullptr,*globalSearch=nullptr;
    QComboBox *typeFilter=nullptr,*stationFilter=nullptr,*statusFilter=nullptr;
    QWidget *emptyState=nullptr;
    QVector<QLabel *> metricCounts;
    QVector<QWidget *> introWidgets;
    bool introStarted=false;
public:
    explicit ServicesPage(QWidget *parent):QWidget(parent) {
        setObjectName("servicesPage"); auto *outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
        auto *top=card("topbar"); introWidgets.append(top); auto *bar=new QHBoxLayout(top); bar->setContentsMargins(28,10,24,10); bar->setSpacing(16);
        globalSearch=search("Search services by ID, title, type, status, station, or assignee..."); globalSearch->setObjectName("serviceGlobalSearch"); globalSearch->setMaximumWidth(540); bar->addWidget(globalSearch,1); bar->addStretch();
        bar->addWidget(new Glyph("calendar",navy,24));
        reportDatePicker=new ServiceReportDateEdit;
        reportDatePicker->setObjectName("serviceReportDatePicker");
        reportDatePicker->setLocale(QLocale(QLocale::English,QLocale::UnitedStates));
        reportDatePicker->setDisplayFormat("ddd, dd MMM yyyy");
        reportDatePicker->setToolTip("Choose the report date for the Services PDF.");
        reportDatePicker->setFixedHeight(38);
        reportDatePicker->setMinimumWidth(176);
        reportDatePicker->setCursor(Qt::PointingHandCursor);
        bar->addWidget(reportDatePicker);
        auto *divider=new QFrame; divider->setProperty("role","divider"); divider->setFixedSize(1,35); bar->addWidget(divider); bar->addWidget(new Glyph("user",muted,40,true));
        auto *user=new QVBoxLayout; user->setSpacing(1); user->addWidget(text("Alex Martin","detailTitle")); user->addWidget(text("Operations Manager","smallMuted")); bar->addLayout(user); bar->addWidget(new Glyph("down",navy,18)); top->hide(); outer->addWidget(top);

        auto *scroll=new QScrollArea; scroll->setObjectName("servicesScroll"); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); outer->addWidget(scroll,1);
        auto *content=new QWidget; content->setObjectName("servicesContent"); body=new QVBoxLayout(content); body->setContentsMargins(28,16,24,8); body->setSpacing(10); body->setSizeConstraint(QLayout::SetMinimumSize);

        auto *headerWidget=new QWidget; introWidgets.append(headerWidget); auto *header=new QHBoxLayout(headerWidget); header->setContentsMargins(0,0,0,0); auto *copy=new QVBoxLayout; copy->setSpacing(0); copy->addWidget(text("Services","pageTitle")); copy->addWidget(text("Manage and track field services, maintenance visits, and support activities.","subtitle")); header->addLayout(copy,1); header->addSpacing(12);
        auto *statisticsButton=button("Statistics","chart"); statisticsButton->setObjectName("serviceStatisticsAction"); header->addWidget(statisticsButton);
        auto *exportButton=button("Export PDF","export"); exportButton->setObjectName("exportServicesAction"); exportButton->setToolTip("Choose which services to export to a PDF report."); header->addWidget(exportButton);
        auto *deleteButton=button("Delete","trash"); deleteButton->setObjectName("deleteServiceAction"); header->addWidget(deleteButton);
        auto *editButton=button("Edit","edit"); editButton->setObjectName("editServiceAction"); header->addWidget(editButton);
        auto *addButton=button("Add Service","plus",true); header->addWidget(addButton); body->addWidget(headerWidget);

        auto *metricsWidget=new QWidget; auto *metrics=new QHBoxLayout(metricsWidget); metrics->setContentsMargins(0,0,0,0); metrics->setSpacing(12);
        const QString dataSource = QSqlDatabase::database().isOpen() ? "saved in Oracle" : "in preview mode";
        for(QWidget *metric:{metricCard("Total Services","0",dataSource,"","wrench",blue,"blue"),metricCard("Completed","0",dataSource,"","check",QColor("#00b863"),"green"),metricCard("In Progress","0",dataSource,"","clock",QColor("#ff3859"),"red"),metricCard("Overdue","0",dataSource,"","camera",QColor("#7948ff"),"purple")}) {
            metrics->addWidget(metric,1); introWidgets.append(metric);
            QLabel *countLabel=nullptr;
            const auto labels=metric->findChildren<QLabel *>();
            for(QLabel *label:labels) {
                if(label->property("role").toString()=="metricCount") countLabel=label;
            }
            metricCounts.append(countLabel);
        }
        body->addWidget(metricsWidget);

        auto *filtersWidget=new QWidget; introWidgets.append(filtersWidget); auto *filters=new QHBoxLayout(filtersWidget); filters->setContentsMargins(0,0,0,0); filters->setSpacing(10); filterSearch=search("Search by ID, title, type, status, station, or assignee..."); filters->addWidget(filterSearch,3);
        typeFilter=selector({"All Types","Maintenance","Calibration","Inspection","Repair","Connectivity","Software"}); filters->addWidget(typeFilter,1);
        stationFilter=selector({"All Stations"});
        for(const auto &station:serviceStationChoices) stationFilter->addItem(station.code);
        filters->addWidget(stationFilter,1);
        statusFilter=selector({"All Statuses","In Progress","Scheduled","Completed","Overdue"}); filters->addWidget(statusFilter,1);
        auto *clear=button("Clear",""); clear->setProperty("link",true); filters->addWidget(clear); body->addWidget(filtersWidget);
        serviceTable=new ServiceTable; body->addWidget(serviceTable);
        emptyState=new QWidget; emptyState->setObjectName("servicesEmptyState"); emptyState->setMinimumHeight(190); emptyState->setVisible(false);
        auto *emptyLayout=new QVBoxLayout(emptyState); emptyLayout->setContentsMargins(20,20,20,20); emptyLayout->setSpacing(7); emptyLayout->setAlignment(Qt::AlignCenter);
        auto *emptyIcon=new Glyph("search",QColor("#7d99c0"),36,true); emptyLayout->addWidget(emptyIcon,0,Qt::AlignCenter);
        emptyLayout->addWidget(text("No services found","emptyStateTitle"),0,Qt::AlignCenter);
        emptyLayout->addWidget(text("Try a different search or adjust your filters.","smallMuted"),0,Qt::AlignCenter);
        auto *emptyClear=button("Clear filters",""); emptyClear->setProperty("link",true); emptyLayout->addWidget(emptyClear,0,Qt::AlignCenter); body->addWidget(emptyState);
        auto applyFilters=[this]() {
            if(!serviceTable) return;
            serviceTable->applyFilters(filterSearch->text(),typeFilter->currentText(),stationFilter->currentText(),statusFilter->currentText());
            emptyState->setVisible(serviceTable->visibleServiceCount()==0);
            if(emptyState->isVisible()) serviceTable->hide(); else serviceTable->show();
            updateMetricCards();
        };
        auto clearFilters=[this,applyFilters]() {
            filterSearch->clear(); typeFilter->setCurrentIndex(0); stationFilter->setCurrentIndex(0); statusFilter->setCurrentIndex(0); applyFilters();
        };
        connect(filterSearch,&QLineEdit::textChanged,this,[this,applyFilters](const QString &value) { if(globalSearch->text()!=value) globalSearch->setText(value); applyFilters(); });
        connect(globalSearch,&QLineEdit::textChanged,this,[this](const QString &value) { if(filterSearch->text()!=value) filterSearch->setText(value); });
        connect(typeFilter,qOverload<int>(&QComboBox::currentIndexChanged),this,[applyFilters](int) { applyFilters(); });
        connect(stationFilter,qOverload<int>(&QComboBox::currentIndexChanged),this,[applyFilters](int) { applyFilters(); });
        connect(statusFilter,qOverload<int>(&QComboBox::currentIndexChanged),this,[applyFilters](int) { applyFilters(); });
        connect(clear,&QPushButton::clicked,this,[clearFilters]() { clearFilters(); });
        connect(emptyClear,&QPushButton::clicked,this,[clearFilters]() { clearFilters(); });
        serviceTable->setChangeCallback([this,applyFilters]() { applyFilters(); });
        applyFilters();
        connect(addButton,&QPushButton::clicked,this,[this]() { openAddService(); });
        connect(statisticsButton,&QPushButton::clicked,this,[this]() { openStatistics(); });
        connect(exportButton,&QPushButton::clicked,this,[this]() { exportServices(); });
        connect(deleteButton,&QPushButton::clicked,this,[this]() { openDeleteService(); });
        connect(editButton,&QPushButton::clicked,this,[this]() { openEditService(); });

        auto *details=card("health"); introWidgets.append(details); auto *healthBox=new QVBoxLayout(details); healthBox->setContentsMargins(12,10,12,10); healthBox->setSpacing(9);
        auto *healthHead=new QHBoxLayout; healthHead->setSpacing(11); healthHead->addWidget(new Glyph("wrench",blue,44,true)); auto *healthCopy=new QVBoxLayout; healthCopy->setSpacing(1);
        healthCopy->addWidget(text("Service completion summary","sectionTitle")); healthCopy->addWidget(text("Select a service to review its recorded tasks and completion status.","smallMuted")); healthHead->addLayout(healthCopy,1);
        healthBox->addLayout(healthHead);
        detailGrid=new QGridLayout; detailGrid->setSpacing(14); detailPanels={detailInfo(),checklist(),timeline(),stationInfo()}; healthBox->addLayout(detailGrid); body->addWidget(details); body->addStretch(); scroll->setWidget(content);
        adapt();
    }
protected:
    void resizeEvent(QResizeEvent *event) override { QWidget::resizeEvent(event); adapt(); }
    void showEvent(QShowEvent *event) override {
        QWidget::showEvent(event);
        if(!serviceLoadError.isEmpty()) {
            const QString message=serviceLoadError; serviceLoadError.clear();
            QTimer::singleShot(250,this,[this,message]() {
                QMessageBox::warning(window(),"Services table not ready",
                    "The Services list could not be loaded from Oracle (table SKYSENSE_SERVICES).\n"
                    "Run service_sqlplus.sql once in SQL*Plus, then restart SkySense.\n\n"+message);
            });
        }
        if(property("integratedMode").toBool()) { introStarted=true; return; }
        if(!introStarted) QTimer::singleShot(130,this,[this]() { animatePageContents(); });
    }
private:
    void animateWidget(QWidget *widget,int delayMs) {
        auto *effect=new QGraphicsOpacityEffect(widget);
        effect->setOpacity(0.0);
        widget->setGraphicsEffect(effect);
        QTimer::singleShot(delayMs,this,[this,effect]() {
            auto *animation=new QPropertyAnimation(effect,"opacity",this);
            animation->setDuration(520);
            animation->setStartValue(0.0);
            animation->setEndValue(1.0);
            animation->setEasingCurve(QEasingCurve::OutCubic);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
    void animatePageContents() {
        if(introStarted) return;
        introStarted=true;
        int delay=0;
        for(QWidget *widget:introWidgets) { animateWidget(widget,delay); delay+=125; }
        if(serviceTable) serviceTable->animateRows(delay+30,115);
    }
    void updateMetricCards() {
        if(metricCounts.size()<4 || !serviceTable) return;
        metricCounts[0]->setText(QString::number(serviceTable->serviceCount()));
        metricCounts[1]->setText(QString::number(serviceTable->serviceCountWithStatus("Completed")));
        metricCounts[2]->setText(QString::number(serviceTable->serviceCountWithStatus("In Progress")));
        metricCounts[3]->setText(QString::number(serviceTable->serviceCountWithStatus("Overdue")));
    }
    void exportServices() {
        const QVector<ServiceRecord> candidates=serviceTable?serviceTable->visibleServices():QVector<ServiceRecord>{};
        if(candidates.isEmpty()) {
            QMessageBox::information(this,"Nothing to export","No services match the current search and filters.");
            return;
        }
        ServiceExportSelectDialog picker(candidates,window());
        if(picker.exec()!=QDialog::Accepted) return;
        const QVector<ServiceRecord> records=picker.selected();
        if(records.isEmpty()) return;
        QFileDialog dialog(window(),"Export services report",QDir::homePath()+"/SkySense_Services_"+QDate::currentDate().toString("yyyy-MM-dd")+".pdf","PDF document (*.pdf)");
        dialog.setObjectName("serviceExportFileDialog"); dialog.setAcceptMode(QFileDialog::AcceptSave); dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setDefaultSuffix("pdf"); dialog.setOption(QFileDialog::DontUseNativeDialog,true);
        if(dialog.exec()!=QDialog::Accepted||dialog.selectedFiles().isEmpty()) return;
        QString path=dialog.selectedFiles().first(); if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
        QString error;
        const QDate reportDate=reportDatePicker?reportDatePicker->date():QDate::currentDate();
        if(!writeServicesPdf(path,records,&error,reportDate)) {
            QMessageBox::critical(this,"Export failed",error);
            return;
        }
        QMessageBox result(window()); result.setWindowTitle("Services exported"); result.setIcon(QMessageBox::Information);
        result.setText(QString("%1 service record%2 exported successfully.").arg(records.size()).arg(records.size()==1?"":"s"));
        result.setInformativeText(QString("Report date: %1\n%2").arg(reportDate.toString("dd MMM yyyy"),path));
        auto *open=result.addButton("Open PDF",QMessageBox::ActionRole); result.addButton(QMessageBox::Close); result.exec();
        if(result.clickedButton()==open&&!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
            QMessageBox::information(this,"PDF saved","The report is saved at:\n"+path);
    }
    

    void openAddService() {
        QWidget *owner=window();
        auto *overlay=new QWidget(owner); overlay->setObjectName("modalDimmer"); overlay->setAttribute(Qt::WA_StyledBackground,true);
        overlay->setStyleSheet("background:rgba(8,26,58,145)"); overlay->setGeometry(owner->rect()); overlay->show(); overlay->raise();
        AddServiceDialog dialog(owner);
        dialog.setWindowOpacity(0.0);
        QTimer::singleShot(0,&dialog,[&dialog]() {
            auto *fade=new QPropertyAnimation(&dialog,"windowOpacity",&dialog);
            fade->setDuration(220); fade->setStartValue(0.0); fade->setEndValue(1.0);
            fade->setEasingCurve(QEasingCurve::OutCubic); fade->start(QAbstractAnimation::DeleteWhenStopped);
        });
        const QPoint center=owner->mapToGlobal(owner->rect().center()); dialog.move(center-QPoint(dialog.width()/2,dialog.height()/2)); dialog.raise();
        const int result=dialog.exec(); overlay->hide(); overlay->deleteLater();
        if(result==QDialog::Accepted && serviceTable) {
            ServiceRecord record=dialog.service(); QString error;
            if(!ServiceModel().insert(record,&error)) {
                QMessageBox::critical(window(),"Oracle error","The service could not be saved in Oracle.\n\n"+error);
                return;
            }
            serviceTable->addService(record);
        }
    }
    void openEditService() {
        const int row=serviceTable?serviceTable->checkedRow():-1;
        if(row<0) {
            QMessageBox::information(this,"Select a service","Select a service using its checkbox, then choose Edit.");
            return;
        }
        QWidget *owner=window();
        auto *overlay=new QWidget(owner); overlay->setObjectName("modalDimmer"); overlay->setAttribute(Qt::WA_StyledBackground,true);
        overlay->setStyleSheet("background:rgba(8,26,58,145)"); overlay->setGeometry(owner->rect()); overlay->show(); overlay->raise();
        const ServiceRecord existing=serviceTable->serviceAt(row);
        AddServiceDialog dialog(owner,&existing);
        dialog.setWindowOpacity(0.0);
        QTimer::singleShot(0,&dialog,[&dialog]() {
            auto *fade=new QPropertyAnimation(&dialog,"windowOpacity",&dialog);
            fade->setDuration(260); fade->setStartValue(0.0); fade->setEndValue(1.0);
            fade->setEasingCurve(QEasingCurve::OutCubic); fade->start(QAbstractAnimation::DeleteWhenStopped);
        });
        const QPoint center=owner->mapToGlobal(owner->rect().center()); dialog.move(center-QPoint(dialog.width()/2,dialog.height()/2)); dialog.raise();
        const int result=dialog.exec(); overlay->hide(); overlay->deleteLater();
        if(result==QDialog::Accepted && serviceTable) {
            if(dialog.deleteRequested()) {
                QString error;
                if(!ServiceModel().remove(existing.id,&error)) {
                    QMessageBox::critical(window(),"Oracle error","The service could not be deleted from Oracle.\n\n"+error);
                    return;
                }
                serviceTable->removeService(row);
            } else {
                const ServiceRecord updated=dialog.service(); QString error;
                if(!ServiceModel().update(updated,&error)) {
                    QMessageBox::critical(window(),"Oracle error","The service could not be updated in Oracle.\n\n"+error);
                    return;
                }
                serviceTable->updateService(row,updated);
            }
        }
    }
    void openDeleteService() {
        const int row=serviceTable?serviceTable->checkedRow():-1;
        if(row<0) {
            QMessageBox::information(this,"Select a service","Select a service using its checkbox, then choose Delete.");
            return;
        }
        const ServiceRecord record=serviceTable->serviceAt(row);
        if(confirmDeleteService(window(),record) && serviceTable) {
            QString error;
            if(!ServiceModel().remove(record.id,&error)) {
                QMessageBox::critical(window(),"Oracle error","The service could not be deleted from Oracle.\n\n"+error);
                return;
            }
            serviceTable->removeService(row);
        }
    }
    void openStatistics() {
        if(!serviceTable) return;
        const QStringList statuses{"Scheduled","In Progress","Completed","Overdue"};
        const QStringList types{"Maintenance","Calibration","Inspection","Repair","Connectivity","Software"};
        const QStringList priorities{"High","Medium","Low"};
        const QColor statusColors[]{QColor("#5489e8"),QColor("#24a6d8"),QColor("#18a66a"),QColor("#e14d5b")};
        const QColor typeColors[]{QColor("#5489e8"),QColor("#7948ff"),QColor("#18a66a"),QColor("#e14d5b"),QColor("#24a6d8"),QColor("#e7a128")};
        const QColor priorityColors[]{QColor("#e14d5b"),QColor("#e7a128"),QColor("#18a66a")};
        QVector<ServiceChartItem> byStatus,byType,byPriority;
        for(int i=0;i<statuses.size();++i) {
            int count=0; for(int row=0;row<serviceTable->rowCount();++row) if(serviceTable->serviceAt(row).status==statuses[i]) ++count;
            byStatus.append({statuses[i],count,statusColors[i]});
        }
        for(int i=0;i<types.size();++i) {
            int count=0; for(int row=0;row<serviceTable->rowCount();++row) if(serviceTable->serviceAt(row).type==types[i]) ++count;
            byType.append({types[i],count,typeColors[i]});
        }
        for(int i=0;i<priorities.size();++i) {
            int count=0; for(int row=0;row<serviceTable->rowCount();++row) if(serviceTable->serviceAt(row).priority==priorities[i]) ++count;
            byPriority.append({priorities[i],count,priorityColors[i]});
        }
        ServiceStatisticsDialog dialog(byStatus,byType,byPriority,window());
        dialog.exec();
    }
    void adapt() {
        if(!detailGrid) return;
        const int columns=width()<1120?2:4;
        if(columns!=detailColumns) {
            for(auto *w:detailPanels) detailGrid->removeWidget(w);
            for(int i=0;i<4;++i) { detailGrid->setColumnStretch(i,0); detailGrid->setColumnMinimumWidth(i,0); }
            for(int i=0;i<4;++i) detailGrid->addWidget(detailPanels[i],i/columns,i%columns);
            for(int i=0;i<columns;++i) detailGrid->setColumnStretch(i,1);
            detailColumns=columns;
        }
        const bool narrow=width()<1000; body->setContentsMargins(narrow?16:28,16,narrow?16:24,8);
    }
};
}

void setupServiceUi(QWidget *root)
{
    
        initialiseServiceData();
    
    root->setObjectName("servicesRoot"); auto *layout=new QHBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0); layout->addWidget(new Sidebar); layout->addWidget(new ServicesPage(root),1);
    QFile stylesheet(":/styles/services.qss"); if(stylesheet.open(QIODevice::ReadOnly)) root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}

QWidget *createSkySenseSidebar(std::function<void(const QString &)> navigate,const QString &initialSection)
{
    auto *sidebar = new Sidebar(std::move(navigate),initialSection);
    QFile stylesheet(":/styles/services.qss");
    if(stylesheet.open(QIODevice::ReadOnly)) sidebar->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    return sidebar;
}

void setupServiceContent(QWidget *root)
{
    
        initialiseServiceData();
    
    root->setObjectName("servicesContentRoot");
    auto *layout=new QVBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0);
    auto *page=new ServicesPage(root);
    page->setProperty("integratedMode",true);
    layout->addWidget(page);
    QFile stylesheet(":/styles/services.qss"); if(stylesheet.open(QIODevice::ReadOnly)) root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}

bool exportServicesReportPdf(const QString &filePath,QString *error)
{
    return writeServicesPdf(filePath,serviceRecords,error);
}
