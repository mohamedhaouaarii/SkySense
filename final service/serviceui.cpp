#include "serviceui.h"
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTime>
#include <QTimeEdit>
#include <QResizeEvent>
#include <QResource>
#include <QScreen>
#include <QVariantAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>
#include <QVector>
#include <QTextEdit>
#include <QTimer>
#include <functional>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtMath>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

namespace {
const QColor navy("#091b40"), muted("#5d77ad"), blue("#0785ff");

void symbol(QPainter &p, const QString &name, QColor color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
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

void playClickSound()
{
#ifdef Q_OS_WIN
    static const QByteArray wave=[]() {
        QFile file(":/assets/sounds/ui_click.wav");
        return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray();
    }();
    if(!wave.isEmpty()) PlaySoundA(reinterpret_cast<LPCSTR>(wave.constData()),nullptr,SND_MEMORY|SND_ASYNC|SND_NODEFAULT);
#else
    QApplication::beep();
#endif
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
    QObject::connect(b,&QPushButton::clicked,b,[]() { playClickSound(); });
    return b;
}

QFrame *card(const char *role="card")
{
    auto *f=new QFrame; f->setProperty("role",role); return f;
}

QLabel *pill(const QString &value, const QString &tone="green")
{
    auto *l=text(value,"pill"); l->setProperty("tone",tone); l->setAlignment(Qt::AlignCenter); return l;
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

class Glyph final : public QWidget {
    QString name; QColor color; bool bubble;
public:
    Glyph(QString n,QColor c=blue,int size=28,bool circle=false):name(n),color(c),bubble(circle) { setFixedSize(size,size); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(bubble) { QColor bg=color; bg.setAlpha(22); p.setPen(Qt::NoPen); p.setBrush(bg); p.drawEllipse(rect()); }
        qreal margin=bubble?width()*.25:2; p.translate(margin,margin); p.scale((width()-2*margin)/24.0,(height()-2*margin)/24.0); symbol(p,name,color);
    }
};

class Sidebar final : public QWidget {
    QPixmap photo{":/assets/backgrounds/services_alps.png"};
public:
    Sidebar() {
        setObjectName("servicesSidebar"); setFixedWidth(270);
        auto *column=new QVBoxLayout(this); column->setContentsMargins(10,20,10,18); column->setSpacing(4);
        auto *dots=new QLabel("<span style='color:#ff4854'>●</span> &nbsp;<span style='color:#ffbd35'>●</span> &nbsp;<span style='color:#20c777'>●</span>");
        dots->setStyleSheet("font-size:15px"); column->addWidget(dots);
        auto *brand=new QLabel; brand->setObjectName("sidebarBrand"); brand->setFixedSize(250,98);
        brand->setPixmap(QPixmap(":/assets/logo/skysense_logo.png").scaled(244,88,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter); column->addWidget(brand); column->addSpacing(15);
        const QStringList names{"Dashboard","Stations","Equipment","Maintenance","Staff","Services"};
        const QStringList icons{"home","pin","equipment","tool","people","grid"};
        for(int i=0;i<names.size();++i) {
            auto *nav=button(names[i],icons[i]); nav->setProperty("nav",true); nav->setProperty("active",i==5);
            nav->setIcon(icon(icons[i],QColor("#e4f3ff"))); nav->setIconSize(QSize(25,25)); nav->setFixedHeight(50); column->addWidget(nav);
        }
        column->addStretch();
        auto *statusBox=new QVBoxLayout; statusBox->setContentsMargins(16,12,4,0); statusBox->setSpacing(5);
        statusBox->addWidget(text("System Status","sidebarStatus"));
        auto *statusRow=new QHBoxLayout; auto *dot=text(QString::fromUtf8("●"),"greenDot"); dot->setFixedWidth(16);
        statusRow->addWidget(dot); statusRow->addWidget(text("All Systems Operational","sidebarStatus")); statusBox->addLayout(statusRow);
        statusBox->addWidget(text("Last updated: Apr 24, 2025 10:42","sidebarMuted"));
        auto *status=new QWidget; status->setLayout(statusBox); column->addWidget(status);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.fillRect(rect(),QColor("#071a33")); p.setRenderHint(QPainter::SmoothPixmapTransform);
        QRectF area(0,height()*.35,width(),height()*.65); QSizeF size=photo.size(); size.scale(area.size(),Qt::KeepAspectRatioByExpanding);
        p.save(); p.setClipRect(area); p.drawPixmap(QRectF((width()-size.width())*.15,area.top()+(area.height()-size.height())/2,size.width(),size.height()),photo,photo.rect()); p.restore();
        QLinearGradient g(0,area.top(),0,height()); g.setColorAt(0,QColor(7,26,51,255)); g.setColorAt(.34,QColor(7,26,51,100)); g.setColorAt(.7,QColor(7,26,51,22)); g.setColorAt(1,QColor(7,26,51,225)); p.fillRect(area,g);
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

struct ServiceRecord {
    QString id, title, type, stationCode, stationName, assignee, date, startTime, endTime, priority, status;
    QString description;
    QStringList checklist;
};

QVector<ServiceRecord> serviceRecords{
    {"SRV-001","Routine Maintenance","Maintenance","ALP-03","Alpine Ridge","James Wilson","Apr 24, 2025","09:00","12:00","High","In Progress",{},{}},
    {"SRV-002","Sensor Calibration","Calibration","COA-01","Coastal Bay","Sophie Chen","Apr 24, 2025","12:30","15:00","Medium","Scheduled",{},{}},
    {"SRV-003","Camera Inspection","Inspection","FOR-12","Pine Forest","Miguel Santos","Apr 25, 2025","09:00","12:00","Low","Completed",{},{}},
    {"SRV-004","Weather Station Repair","Repair","VAL-07","Green Valley","Emma Davis","Apr 25, 2025","14:00","17:00","High","Overdue",{},{}},
    {"SRV-005","Data Connectivity Check","Connectivity","ALP-03","Alpine Ridge","Ryan Kim","Apr 25, 2025","10:00","12:00","Medium","Scheduled",{},{}},
    {"SRV-006","Software Update","Software","DES-05","Desert View","Olivia Martinez","Apr 26, 2025","13:00","16:00","Low","Scheduled",{},{}}
};

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
public:
    explicit ServiceDelegate(QObject *parent=nullptr):QStyledItemDelegate(parent),rowOpacity(serviceRecords.size(),1.0) {}
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
        p->fillRect(r,checked?QColor("#e5f4ff"):hovered?QColor("#f2f8ff"):Qt::white); p->setPen(QColor("#e5eef9")); p->drawLine(r.bottomLeft(),r.bottomRight());
        QFont font("Segoe UI"); font.setPixelSize(13); p->setFont(font); p->setPen(navy);
        if(col==0) {
            if(checked) p->fillRect(QRect(r.left(),r.top(),3,r.height()),blue);
            QRectF box(r.center().x()-8,r.center().y()-8,16,16); p->setPen(QPen(checked?blue:QColor("#8ca7d3"),1.3)); p->setBrush(checked?blue:Qt::white); p->drawRoundedRect(box,3,3);
            if(checked) { p->translate(box.topLeft()); p->scale(16.0/24,16.0/24); symbol(*p,"check",Qt::white); }
        } else if(col==1) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(9,0,-3,0),Qt::AlignVCenter,record.id);
        } else if(col==2) {
            p->save(); p->translate(r.left()+8,r.center().y()-11); symbol(*p,glyphForType(record.type),blue); p->restore();
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(39,0,-4,0),Qt::AlignVCenter,record.title);
        } else if(col==3) {
            p->setPen(muted); p->drawText(r.adjusted(8,0,-3,0),Qt::AlignVCenter,record.type);
        } else if(col==4) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(8,3,-3,-r.height()/2+2),Qt::AlignVCenter,record.stationCode);
            font.setBold(false); p->setFont(font); p->setPen(muted); p->drawText(r.adjusted(8,r.height()/2-1,-3,-3),Qt::AlignVCenter,record.stationName);
        } else if(col==5) {
            const QString initials=record.assignee.split(' ').value(0).left(1)+record.assignee.split(' ').value(1).left(1);
            QColor avatar[]={QColor("#e3efff"),QColor("#fbe9e1"),QColor("#e8e4fb"),QColor("#f8e9f0"),QColor("#e5f5ec"),QColor("#f2eddf")};
            p->setPen(Qt::NoPen); p->setBrush(avatar[row]); p->drawEllipse(QRectF(r.left()+7,r.center().y()-13,26,26));
            p->setPen(QColor("#28517c")); p->drawText(QRectF(r.left()+7,r.center().y()-13,26,26),Qt::AlignCenter,initials);
            p->setPen(navy); p->drawText(r.adjusted(39,0,-2,0),Qt::AlignVCenter,record.assignee);
        } else if(col==6) {
            p->save(); p->translate(r.left()+7,r.center().y()-10); symbol(*p,"calendar",muted); p->restore();
            p->setPen(muted); p->drawText(r.adjusted(34,3,-2,-r.height()/2+1),Qt::AlignVCenter,record.date);
            p->drawText(r.adjusted(34,r.height()/2-1,-2,-3),Qt::AlignVCenter,record.startTime+" - "+record.endTime);
        } else if(col==7 || col==8) {
            const QString value=col==7?record.priority:record.status; QColor bg,fg;
            if(col==7) { bg=value=="High"?QColor("#fde5e6"):value=="Medium"?QColor("#fff1d4"):QColor("#e5f8ed"); fg=value=="High"?QColor("#dc4551"):value=="Medium"?QColor("#bb8113"):QColor("#23985d"); }
            else { bg=value=="Completed"?QColor("#ddf7e9"):value=="Overdue"?QColor("#ffe4e7"):value=="In Progress"?QColor("#e4f1ff"):QColor("#e9f3ff"); fg=value=="Completed"?QColor("#13834f"):value=="Overdue"?QColor("#d34756"):QColor("#2474c6"); }
            QRectF badge(r.left()+4,r.center().y()-13,r.width()-8,26); p->setPen(Qt::NoPen); p->setBrush(bg); p->drawRoundedRect(badge,13,13); p->setPen(fg); p->drawText(badge,Qt::AlignCenter,value);
        } else {
            p->setPen(Qt::NoPen); p->setBrush(navy); for(int i=-1;i<=1;++i) p->drawEllipse(QPointF(r.center().x()+i*7,r.center().y()),1.5,1.5);
        }
        p->restore();
    }
};

class ServiceHeader final : public QHeaderView {
public:
    explicit ServiceHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) {}
protected:
    void paintSection(QPainter *p,const QRect &rect,int section) const override {
        QHeaderView::paintSection(p,rect,section); p->save(); p->setRenderHint(QPainter::Antialiasing); p->setPen(QPen(QColor("#8aa6d2"),1.2));
        if(section==0) { p->setBrush(Qt::white); p->drawRoundedRect(QRectF(rect.center().x()-8,rect.center().y()-8,16,16),3,3); }
        else if(section>0 && section<9) { int x=rect.left()+fontMetrics().horizontalAdvance(model()->headerData(section,Qt::Horizontal).toString())+20; int y=rect.center().y(); p->drawLine(x,y-2,x+3,y-5); p->drawLine(x+3,y-5,x+6,y-2); p->drawLine(x,y+2,x+3,y+5); p->drawLine(x+3,y+5,x+6,y+2); }
        p->restore();
    }
};

class ServiceTable final : public QTableWidget {
    std::function<void()> changeCallback;
public:
    ServiceTable():QTableWidget(serviceRecords.size(),10) {
        setObjectName("serviceTable"); setHorizontalHeader(new ServiceHeader(this)); viewport()->setMouseTracking(true); setMouseTracking(true);
        setHorizontalHeaderLabels({"","ID","Title","Type","Station","Assignee","Scheduled Date","Priority","Status","Actions"});
        verticalHeader()->hide(); verticalHeader()->setDefaultSectionSize(42); horizontalHeader()->setFixedHeight(40); horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); setItemDelegate(new ServiceDelegate(this)); setShowGrid(false); setSelectionMode(QAbstractItemView::NoSelection); setEditTriggers(QAbstractItemView::NoEditTriggers); setFocusPolicy(Qt::NoFocus);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        for(int r=0;r<serviceRecords.size();++r) populateRow(r);
        connect(this,&QTableWidget::cellClicked,this,[this](int row,int column) {
            if(column!=0) return;
            auto *check=item(row,0);
            if(!check) return;
            check->setCheckState(check->checkState()==Qt::Checked?Qt::Unchecked:Qt::Checked);
            viewport()->update();
        });
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); setMinimumWidth(0);
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
    void setChangeCallback(std::function<void()> callback) { changeCallback=std::move(callback); }
    int visibleServiceCount() const { int count=0; for(int row=0;row<rowCount();++row) if(!isRowHidden(row)) ++count; return count; }
    int serviceCount() const { return serviceRecords.size(); }
    int serviceCountWithStatus(const QString &status) const { int count=0; for(const ServiceRecord &record:serviceRecords) if(record.status==status) ++count; return count; }
    void applyFilters(const QString &query,const QString &type,const QString &station,const QString &status) {
        const QString needle=query.trimmed();
        for(int row=0;row<rowCount();++row) {
            const ServiceRecord &r=serviceRecords[row];
            const QString haystack=(r.id+" "+r.title+" "+r.stationCode+" "+r.stationName+" "+r.assignee).toCaseFolded();
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
protected:
    void resizeEvent(QResizeEvent *event) override {
        QTableWidget::resizeEvent(event); const int widths[]{42,86,171,122,116,139,155,88,112,56};
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
    QTextEdit *descriptionEdit=nullptr;
    QLabel *descriptionCount=nullptr,*stationCodePreview=nullptr,*stationNamePreview=nullptr,*datePreview=nullptr,*timePreview=nullptr;
    QVBoxLayout *checklistLayout=nullptr;
    QStringList checklistItems;
    bool editingMode=false;
    bool deleteRequestedFlag=false;
public:
    explicit AddServiceDialog(QWidget *parent=nullptr,const ServiceRecord *existing=nullptr):QDialog(parent),editingMode(existing!=nullptr) {
        setObjectName("addServiceDialog");
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
        const QStringList stationCodes{"ALP-03","COA-01","FOR-12","VAL-07","DES-05"};
        const QStringList stationNames{"Alpine Ridge","Coastal Bay","Pine Forest","Green Valley","Desert View"};
        for(int i=0;i<stationCodes.size();++i) {
            stationCombo->addItem(stationCodes[i]+" — "+stationNames[i]);
            stationCombo->setItemData(i+1,stationCodes[i],Qt::UserRole);
            stationCombo->setItemData(i+1,stationNames[i],Qt::UserRole+1);
        }
        assigneeCombo=selector({"Select assignee","James Wilson","Sophie Chen","Miguel Santos","Emma Davis","Ryan Kim","Olivia Martinez"});
        dateEdit=new QDateEdit(QDate(2025,4,24)); dateEdit->setDisplayFormat("MMM dd, yyyy"); dateEdit->setCalendarPopup(true); dateEdit->setFixedHeight(48);
        startEdit=new QTimeEdit(QTime(8,0)); startEdit->setDisplayFormat("HH:mm"); startEdit->setFixedHeight(48);
        endEdit=new QTimeEdit(QTime(10,0)); endEdit->setDisplayFormat("HH:mm"); endEdit->setFixedHeight(48);
        priorityCombo=selector({"High","Medium","Low"}); priorityCombo->setCurrentText("Medium");
        statusCombo=selector({"Scheduled","In Progress","Completed","Overdue"});
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
        for(int column=0;column<3;++column) detailsGrid->setColumnStretch(column,1);
        cardLayout->addLayout(detailsGrid);

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
            for(const QString &item:existing->checklist) addChecklistRow(item);
        } else {
            addChecklistRow("Check equipment status and perform visual inspection");
            addChecklistRow("Calibrate sensors and verify readings");
            addChecklistRow("Update maintenance logs and reports");
        }
        auto *addCheck=button("Add checklist item","plus");
        addCheck->setProperty("link",true);
        addCheck->setFixedHeight(34);
        connect(addCheck,&QPushButton::clicked,this,[this]() { addChecklistRow(QString()); });
        cardLayout->addWidget(addCheck,0,Qt::AlignLeft);
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
        record.assignee=assigneeCombo->currentText();
        record.date=dateEdit->date().toString("MMM dd, yyyy");
        record.startTime=startEdit->time().toString("HH:mm");
        record.endTime=endEdit->time().toString("HH:mm");
        record.priority=priorityCombo->currentText();
        record.status=statusCombo->currentText();
        record.description=descriptionEdit->toPlainText();
        record.checklist=checklistItems;
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
    void addChecklistRow(const QString &value) {
        auto *row=new QWidget;
        row->setProperty("role","checklistRow");
        auto *layout=new QHBoxLayout(row);
        layout->setContentsMargins(8,3,8,3);
        layout->setSpacing(8);
        auto *done=new QCheckBox;
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
        connect(remove,&QPushButton::clicked,this,[this,row]() { playClickSound(); checklistLayout->removeWidget(row); row->deleteLater(); });
    }
    void createService() {
        if(titleEdit->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Service title required","Enter a title for this service."); titleEdit->setFocus(); return; }
        if(stationCombo->currentIndex()==0) { QMessageBox::warning(this,"Station required","Select a weather station for this service."); stationCombo->setFocus(); return; }
        if(assigneeCombo->currentIndex()==0) { QMessageBox::warning(this,"Assignee required","Select a staff member to assign this service to."); assigneeCombo->setFocus(); return; }
        if(endEdit->time()<=startEdit->time()) { QMessageBox::warning(this,"Schedule time invalid","End time must be later than start time."); endEdit->setFocus(); return; }
        checklistItems.clear();
        for(int i=0;i<checklistLayout->count();++i) {
            auto *row=checklistLayout->itemAt(i)->widget();
            if(!row) continue;
            auto *edit=row->findChild<QLineEdit *>();
            if(edit && !edit->text().trimmed().isEmpty()) checklistItems.append(edit->text().trimmed());
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
        connect(close,&QPushButton::clicked,this,[this]() { playClickSound(); reject(); });
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
    const QStringList fields{"Service ID|SRV-001","Type|Maintenance","Station|ALP-03 (Alpine Ridge)","Scheduled|Apr 24, 2025 09:00–12:00","Assigned To|James Wilson","Priority|High"};
    for(const auto &field:fields) { auto *row=new QHBoxLayout; row->addWidget(text(field.section('|',0,0),"smallMuted")); row->addStretch(); row->addWidget(text(field.section('|',1,1),"smallValue")); v->addLayout(row); }
    v->addWidget(text("Routine maintenance check including calibration, system health verification, and component inspection.","recommendationCopy")); return box;
}

QWidget *checklist()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(7); v->addWidget(text("Service Checklist","detailTitle"));
    const QStringList tasks{"Visual inspection of equipment|09:15","Clean solar panels|09:40","Check sensor alignment|10:20","Verify calibration readings|Pending","Test data transmission|Pending"};
    for(int i=0;i<tasks.size();++i) { auto *row=new QHBoxLayout; auto *mark=text(i<3?QString::fromUtf8("●"):QString::fromUtf8("○"),i<3?"doneDot":"todoDot"); mark->setFixedWidth(15); row->addWidget(mark); row->addWidget(text(tasks[i].section('|',0,0),"smallValue"),1); row->addWidget(text(tasks[i].section('|',1,1),"smallMuted")); v->addLayout(row); } return box;
}

QWidget *timeline()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(7); v->addWidget(text("Service Timeline","detailTitle"));
    const QStringList events{"Service scheduled|Apr 24, 2025 08:00","Technician on site|Apr 24, 2025 09:12","Maintenance in progress|Apr 24, 2025 09:15","Service completion|Pending","Report submitted|Pending"};
    for(int i=0;i<events.size();++i) { auto *row=new QHBoxLayout; auto *mark=text(i<3?QString::fromUtf8("●"):QString::fromUtf8("○"),i<3?"timelineDone":"timelinePending"); mark->setFixedWidth(15); row->addWidget(mark); auto *copy=new QVBoxLayout; copy->setSpacing(0); copy->addWidget(text(events[i].section('|',0,0),"smallValue")); copy->addWidget(text(events[i].section('|',1,1),"smallMuted")); row->addLayout(copy,1); v->addLayout(row); } return box;
}

QWidget *stationInfo()
{
    auto *box=new QWidget; auto *v=new QVBoxLayout(box); v->setContentsMargins(0,0,0,0); v->setSpacing(5); v->addWidget(text("Station Information","detailTitle"));
    auto *badge=new QHBoxLayout; badge->addWidget(new Glyph("pin",blue,30,true)); auto *copy=new QVBoxLayout; copy->setSpacing(0); copy->addWidget(text("ALP-03","smallValue")); copy->addWidget(text("Alpine Ridge","smallMuted")); badge->addLayout(copy,1); v->addLayout(badge);
    v->addWidget(text("2,450 m altitude","smallMuted")); auto *online=new QHBoxLayout; online->addWidget(text(QString::fromUtf8("●"),"greenDot")); online->addWidget(text("Online","smallValue")); online->addStretch(); v->addLayout(online);
    auto *link=button("View Station","external"); link->setProperty("link",true); v->addWidget(link,0,Qt::AlignLeft); v->addStretch(); return box;
}

class ServicesPage final : public QWidget {
    QGridLayout *detailGrid=nullptr; QList<QWidget *> detailPanels; QVBoxLayout *body=nullptr; ServiceTable *serviceTable=nullptr; int detailColumns=0;
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
        globalSearch=search("Search services by ID, title, station, or assignee..."); globalSearch->setMaximumWidth(540); bar->addWidget(globalSearch,1); bar->addStretch();
        bar->addWidget(new Glyph("calendar",navy,24)); bar->addWidget(text("Thu, Apr 24, 2025","topDate")); bar->addWidget(new Glyph("down",muted,18));
        auto *divider=new QFrame; divider->setProperty("role","divider"); divider->setFixedSize(1,35); bar->addWidget(divider); bar->addWidget(new Glyph("user",muted,40,true));
        auto *user=new QVBoxLayout; user->setSpacing(1); user->addWidget(text("Alex Martin","detailTitle")); user->addWidget(text("Operations Manager","smallMuted")); bar->addLayout(user); bar->addWidget(new Glyph("down",navy,18)); outer->addWidget(top);

        auto *scroll=new QScrollArea; scroll->setObjectName("servicesScroll"); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); outer->addWidget(scroll,1);
        auto *content=new QWidget; content->setObjectName("servicesContent"); body=new QVBoxLayout(content); body->setContentsMargins(28,16,24,8); body->setSpacing(10); body->setSizeConstraint(QLayout::SetMinimumSize);

        auto *headerWidget=new QWidget; introWidgets.append(headerWidget); auto *header=new QHBoxLayout(headerWidget); header->setContentsMargins(0,0,0,0); auto *copy=new QVBoxLayout; copy->setSpacing(0); copy->addWidget(text("Services","pageTitle")); copy->addWidget(text("Manage and track field services, maintenance visits, and support activities.","subtitle")); header->addLayout(copy,1); header->addSpacing(12);
        header->addWidget(button("Export PDF","export"));
        auto *deleteButton=button("Delete","trash"); deleteButton->setObjectName("deleteServiceAction"); header->addWidget(deleteButton);
        auto *editButton=button("Edit","edit"); editButton->setObjectName("editServiceAction"); header->addWidget(editButton);
        auto *addButton=button("Add Service","plus",true); header->addWidget(addButton); body->addWidget(headerWidget);

        auto *metricsWidget=new QWidget; auto *metrics=new QHBoxLayout(metricsWidget); metrics->setContentsMargins(0,0,0,0); metrics->setSpacing(12);
        for(QWidget *metric:{metricCard("Total Services","46","/ 50 this month","+12% from last month","wrench",blue,"blue"),metricCard("Completed","32","/ 50 this month","+18% from last month","check",QColor("#00b863"),"green"),metricCard("In Progress","8","/ 50 this month","+2% from last month","clock",QColor("#ff3859"),"red"),metricCard("Overdue","6","/ 50 this month","+3 from last month","camera",QColor("#7948ff"),"purple")}) {
            metrics->addWidget(metric,1); introWidgets.append(metric);
            QLabel *countLabel=nullptr;
            const auto labels=metric->findChildren<QLabel *>();
            for(QLabel *label:labels) {
                if(label->property("role").toString()=="metricCount") countLabel=label;
            }
            metricCounts.append(countLabel);
        }
        body->addWidget(metricsWidget);

        auto *filtersWidget=new QWidget; introWidgets.append(filtersWidget); auto *filters=new QHBoxLayout(filtersWidget); filters->setContentsMargins(0,0,0,0); filters->setSpacing(10); filterSearch=search("Search by ID, title, station, or assignee..."); filters->addWidget(filterSearch,3);
        typeFilter=selector({"All Types","Maintenance","Calibration","Inspection","Repair","Connectivity","Software"}); filters->addWidget(typeFilter,1);
        stationFilter=selector({"All Stations","ALP-03","COA-01","FOR-12","VAL-07","DES-05"}); filters->addWidget(stationFilter,1);
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
        connect(deleteButton,&QPushButton::clicked,this,[this]() { openDeleteService(); });
        connect(editButton,&QPushButton::clicked,this,[this]() { openEditService(); });

        auto *details=card("health"); introWidgets.append(details); auto *healthBox=new QVBoxLayout(details); healthBox->setContentsMargins(12,10,12,10); healthBox->setSpacing(9);
        auto *healthHead=new QHBoxLayout; healthHead->setSpacing(11); healthHead->addWidget(new Glyph("wrench",blue,44,true)); auto *healthCopy=new QVBoxLayout; healthCopy->setSpacing(1);
        healthCopy->addWidget(text("SRV-001  –  Routine Maintenance","sectionTitle")); healthCopy->addWidget(text("Standard maintenance visit for weather station ALP-03.","smallMuted")); healthHead->addLayout(healthCopy,1);
        healthHead->addWidget(pill("In Progress","blue")); healthHead->addWidget(text("Last updated 1 hour ago","smallMuted")); healthHead->addWidget(button("View Full Details","external")); healthBox->addLayout(healthHead);
        detailGrid=new QGridLayout; detailGrid->setSpacing(14); detailPanels={detailInfo(),checklist(),timeline(),stationInfo()}; healthBox->addLayout(detailGrid); body->addWidget(details); body->addStretch(); scroll->setWidget(content);
        adapt();
        QTimer::singleShot(130,this,[this]() { animatePageContents(); });
    }
protected:
    void resizeEvent(QResizeEvent *event) override { QWidget::resizeEvent(event); adapt(); }
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
        if(result==QDialog::Accepted && serviceTable) serviceTable->addService(dialog.service());
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
            if(dialog.deleteRequested()) serviceTable->removeService(row);
            else serviceTable->updateService(row,dialog.service());
        }
    }
    void openDeleteService() {
        const int row=serviceTable?serviceTable->checkedRow():-1;
        if(row<0) {
            QMessageBox::information(this,"Select a service","Select a service using its checkbox, then choose Delete.");
            return;
        }
        const ServiceRecord record=serviceTable->serviceAt(row);
        if(confirmDeleteService(window(),record) && serviceTable) serviceTable->removeService(row);
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
    root->setObjectName("servicesRoot"); auto *layout=new QHBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0); layout->addWidget(new Sidebar); layout->addWidget(new ServicesPage(root),1);
    QFile stylesheet(":/styles/services.qss"); if(stylesheet.open(QIODevice::ReadOnly)) root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}
