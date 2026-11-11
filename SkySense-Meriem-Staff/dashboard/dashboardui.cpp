#include "dashboardui.h"
#include "dashboarddata.h"
#include "connection.h"
#include "apptheme.h"
#include "appsession.h"
#include <QAbstractButton>
#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QTimer>
#include <QVariantAnimation>
#include <QtMath>

// Tableau de bord de la partie Equipement : uniquement des donnees de la table EQUIPEMENT.
namespace {
QColor ink() { return QColor(AppTheme::dark() ? "#edf4ff" : "#142942"); }
QColor muted() { return QColor(AppTheme::dark() ? "#9eb3cb" : "#62778c"); }
QColor edge() { return QColor(AppTheme::dark() ? "#2b405a" : "#e0e8f1"); }
QColor surface() { return QColor(AppTheme::dark() ? "#18283d" : "#ffffff"); }
const QColor cyan("#19b7d0"), blue("#3989f9"), green("#27bf91"), amber("#efab45"), red("#f07889"), violet("#9681f7");

// Meme couleur par type que la gestion Equipement.
QColor typeTint(const QString &type)
{
    static const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
    static const QStringList palette{"#2f6fe0","#0ea5a4","#0284c7","#f59e0b","#7c3aed","#db2777"};
    const int index=types.indexOf(type);
    return QColor(index>=0?palette[index]:QStringLiteral("#2f6fe0"));
}
void words(QPainter &p, QRectF r, const QString &s, int size, QColor color,
           int weight = QFont::Normal, int alignment = Qt::AlignLeft | Qt::AlignVCenter) {
    QFont font("Segoe UI"); font.setPixelSize(size); font.setWeight(QFont::Weight(weight));
    p.setFont(font); p.setPen(color);
    p.drawText(r, alignment, p.fontMetrics().elidedText(s, Qt::ElideRight, int(r.width())));
}
// 1 equipement, 2 horloge, 5 en ligne, 6 hors ligne, 7 maintenance
void drawIcon(QPainter &p, QPointF at, int kind, QColor color) {
    p.save(); p.translate(at); p.setPen(QPen(color,1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
    if(kind==1) {
        p.drawRoundedRect(QRectF(-8,-8,16,16),3,3); p.drawRect(QRectF(-3,-3,6,6));
        for(int a:{-5,0,5}) {p.drawLine(QPointF(a,-11),QPointF(a,-8));p.drawLine(QPointF(a,8),QPointF(a,11));}
    } else if(kind==2) {
        p.drawEllipse(QPointF(0,0),8,8);p.drawLine(QPointF(0,-5),QPointF(0,0));p.drawLine(QPointF(0,0),QPointF(4,3));
    } else if(kind==5 || kind==6) {
        p.drawArc(QRectF(-10,-7,20,20),40*16,100*16); p.drawArc(QRectF(-6,-3,12,12),40*16,100*16);
        p.setBrush(color); p.drawEllipse(QPointF(0,6),1.6,1.6); p.setBrush(Qt::NoBrush);
        if(kind==6) p.drawLine(QPointF(-8,-8),QPointF(8,8));
    } else {
        QPainterPath path; path.moveTo(4,-9); path.cubicTo(-2,-11,-4,-5,-1,-2); path.lineTo(-8,5); path.lineTo(-5,8);
        path.lineTo(2,1); path.cubicTo(7,4,11,-2,9,-6); path.lineTo(5,-2); path.lineTo(2,-5); path.closeSubpath(); p.drawPath(path);
    }
    p.restore();
}
class Panel : public QWidget {
public:
    explicit Panel(QWidget *parent=nullptr):QWidget(parent) { setMinimumWidth(0); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        p.setPen(edge());p.setBrush(surface());p.drawRoundedRect(rect().adjusted(1,1,-1,-1),18,18);
    }
};
// Pastille ronde teintee avec une icone (meme style que les cartes Equipement).
class IconChip final : public QWidget {
    int kind; QColor color;
public:
    IconChip(int symbol,QColor tint):kind(symbol),color(tint) { setFixedSize(40,40); setAttribute(Qt::WA_TransparentForMouseEvents); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        QColor fill=color; fill.setAlpha(AppTheme::dark()?52:30); QColor ring=color; ring.setAlpha(AppTheme::dark()?120:80);
        p.setPen(QPen(ring,1.4)); p.setBrush(fill); p.drawEllipse(QRectF(rect()).adjusted(1.5,1.5,-1.5,-1.5));
        drawIcon(p,QPointF(width()/2.,height()/2.),kind,AppTheme::dark()?color.lighter(140):color.darker(108));
    }
};
// Pastille de statut d'un equipement.
class StatusPill final : public QWidget {
    QString text; QColor tone;
public:
    StatusPill(QString value,QColor color):text(std::move(value)),tone(color) {
        setFixedSize(112,28);setAttribute(Qt::WA_TransparentForMouseEvents);setAccessibleName(text);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        const QColor shown=AppTheme::dark()?tone.lighter(135):tone.darker(118);
        QColor fill=tone;fill.setAlpha(AppTheme::dark()?34:24);QColor border=tone;border.setAlpha(110);
        p.setBrush(fill);p.setPen(border);p.drawRoundedRect(rect().adjusted(1,1,-1,-1),13,13);
        p.setPen(Qt::NoPen);p.setBrush(tone);p.drawEllipse(QPointF(15,height()/2.),3.5,3.5);
        words(p,{24,0,qreal(width()-30),qreal(height())},text,11,shown,QFont::DemiBold,Qt::AlignCenter);
    }
};
// Jauge horizontale avec pourcentage.
class ShareBar final : public QWidget {
    QColor color; qreal ratio=-1;
public:
    explicit ShareBar(QColor tint):color(tint) { setFixedHeight(16); setMinimumWidth(160); }
    void setRatio(qreal value) { ratio=value; update(); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF track(0,5,width()-48,7);
        QColor rail=color; rail.setAlpha(AppTheme::dark()?55:38);
        p.setPen(Qt::NoPen); p.setBrush(rail); p.drawRoundedRect(track,3.5,3.5);
        if(ratio<0) return;
        if(ratio>0) {
            QLinearGradient fill(track.topLeft(),track.topRight()); fill.setColorAt(0,color.lighter(125)); fill.setColorAt(1,color);
            p.setBrush(fill); p.drawRoundedRect(QRectF(track.x(),track.y(),qMax(7.0,track.width()*ratio),track.height()),3.5,3.5);
        }
        words(p,{qreal(width()-44),0,44,16},QString("%1%").arg(qRound(ratio*100)),11,AppTheme::dark()?color.lighter(140):color.darker(115),QFont::DemiBold,Qt::AlignRight|Qt::AlignVCenter);
    }
};
// Carte KPI : barre d'accent en haut, valeur, et jauge avec la part reelle.
class Metric final : public QAbstractButton {
public:
    QString title, number="—", note="Waiting for connection";
    QColor accent; int kind; qreal ratio=-1;
    Metric(QString name,QColor tint,int symbol):title(name),accent(tint),kind(symbol) {
        setMinimumHeight(148);setMinimumWidth(170);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        setCursor(Qt::PointingHandCursor);setFocusPolicy(Qt::StrongFocus);setAccessibleName("Open equipment: "+title);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(underMouse()||hasFocus()?accent:edge(),hasFocus()?2:1));p.setBrush(surface());
        p.drawRoundedRect(rect().adjusted(1,1,-1,-1),16,16);
        {
            p.save(); QPainterPath clip; clip.addRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),16,16); p.setClipPath(clip);
            QLinearGradient bar(0,0,width(),0); bar.setColorAt(0,accent); QColor fade=accent; fade.setAlpha(60); bar.setColorAt(1,fade);
            p.setPen(Qt::NoPen); p.setBrush(bar); p.drawRect(QRectF(1,1,width()-2,4)); p.restore();
        }
        QColor wash=accent;wash.setAlpha(AppTheme::dark()?40:26);QColor ring=accent;ring.setAlpha(AppTheme::dark()?120:80);
        p.setPen(QPen(ring,1.4));p.setBrush(wash);p.drawEllipse(QRectF(width()-56,16,38,38));
        drawIcon(p,{qreal(width()-37),35},kind,AppTheme::dark()?accent.lighter(140):accent.darker(108));
        words(p,{18,16,qreal(width()-80),23},title,13,muted(),QFont::DemiBold);
        words(p,{18,43,qreal(width()-36),43},number,32,ink(),QFont::DemiBold);
        words(p,{18,94,qreal(width()-36),20},note,11,muted());
        if(ratio>=0) {
            const QRectF track(18,124,width()-78,6);
            QColor rail=accent; rail.setAlpha(AppTheme::dark()?55:38);
            p.setPen(Qt::NoPen); p.setBrush(rail); p.drawRoundedRect(track,3,3);
            if(ratio>0) {
                QLinearGradient fill(track.topLeft(),track.topRight()); fill.setColorAt(0,accent.lighter(125)); fill.setColorAt(1,accent);
                p.setBrush(fill); p.drawRoundedRect(QRectF(track.x(),track.y(),qMax(6.0,track.width()*ratio),track.height()),3,3);
            }
            words(p,{qreal(width()-56),116,40,22},QString("%1%").arg(qRound(ratio*100)),11,AppTheme::dark()?accent.lighter(140):accent.darker(115),QFont::DemiBold,Qt::AlignRight|Qt::AlignVCenter);
        }
    }
    void enterEvent(QEnterEvent *e) override {QAbstractButton::enterEvent(e);update();}
    void leaveEvent(QEvent *e) override {QAbstractButton::leaveEvent(e);update();}
};
class OrbitBanner final : public QWidget {
public:
    DashboardSnapshot snapshot; qreal phase=0; QTimer motion;
    OrbitBanner() {
        setFixedHeight(186);
        motion.setInterval(60);
        connect(&motion,&QTimer::timeout,this,[this]{if(isVisible()){phase+=0.018;update();}});
    }
    void showEvent(QShowEvent *e) override {QWidget::showEvent(e);motion.start();}
    void hideEvent(QHideEvent *e) override {motion.stop();QWidget::hideEvent(e);}
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient background(0,0,width(),height());background.setColorAt(0,QColor("#102d50"));background.setColorAt(1,QColor("#104c67"));
        p.setPen(Qt::NoPen);p.setBrush(background);p.drawRoundedRect(rect(),20,20);
        const int left=26, textWidth=qMax(240,width()-330);
        words(p,{qreal(left),20,qreal(textWidth),20},"SKYSENSE  /  EQUIPMENT",11,QColor("#8fdfef"),QFont::DemiBold);
        words(p,{qreal(left),49,qreal(textWidth),40},"Your equipment, in focus.",width()<780?25:32,Qt::white,QFont::DemiBold);
        words(p,{qreal(left),96,qreal(textWidth),24},"Status, follow-up and freshness of every device, in one view.",12,QColor("#c2dbe9"));
        p.setBrush(snapshot.available?green:amber);p.drawEllipse(QPointF(left+4,151),4,4);
        words(p,{qreal(left+16),137,qreal(textWidth-16),27},
              snapshot.available?QString("Oracle connected  ·  %1 equipment items in the EQUIPEMENT table").arg(snapshot.equipment):
              Connection::createInstance().isConnected()?"Oracle connected  ·  Overview unavailable":
              "Connect to Oracle to see your equipment",11,QColor("#d2eaf4"));
        if(width()<610)return;
        const QPointF c(width()-145,91);
        QRadialGradient glow(c,110);glow.setColorAt(0,QColor(35,206,230,55));glow.setColorAt(1,Qt::transparent);
        p.setBrush(glow);p.drawEllipse(c,112,88);
        p.setBrush(Qt::NoBrush);
        for(int i=0;i<3;++i) {
            p.save();p.translate(c);p.rotate(-22+i*55);
            p.setPen(QPen(QColor(105,218,237,60+i*12),1));p.drawEllipse(QRectF(-108,-41,216,82));p.restore();
        }
        QRadialGradient sphere(c-QPointF(12,15),67);sphere.setColorAt(0,QColor("#42d5e3"));sphere.setColorAt(0.45,QColor("#137baf"));sphere.setColorAt(1,QColor("#0c3358"));
        p.setBrush(sphere);p.setPen(QPen(QColor("#5cbedb"),1));p.drawEllipse(c,48,48);
        p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor(169,237,255,80),1));
        p.drawEllipse(QRectF(c.x()-48,c.y()-18,96,36));p.drawEllipse(QRectF(c.x()-20,c.y()-48,40,96));
        p.drawLine(c-QPointF(48,0),c+QPointF(48,0));
        const int kinds[]={1,5,7,2};
        for(int i=0;i<4;++i) {
            const qreal a=phase+i*M_PI/2;
            const QPointF n=c+QPointF(qCos(a)*106,qSin(a)*61);
            p.setPen(QPen(QColor(100,220,240,50),1));p.drawLine(c,n);
            p.setBrush(QColor("#153c5d"));p.setPen(QColor("#63c9e2"));p.drawEllipse(n,15,15);
            p.save();p.translate(n);p.scale(.65,.65);drawIcon(p,{0,0},kinds[i],QColor("#d5f9ff"));p.restore();
        }
    }
};
class HealthChart final : public QWidget {
public:
    DashboardSnapshot snapshot; qreal progress=1;
    HealthChart(){setMinimumHeight(228);setAccessibleName("Equipment by recorded status");}
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        const qreal d=qMin(148.,width()*.38);const QRectF ring(8,28,d,d);
        p.setPen(QPen(edge(),14));p.drawArc(ring,0,360*16);
        const int counts[]={snapshot.online,snapshot.offline,snapshot.servicing,snapshot.other};
        const QColor colors[]={green,red,amber,violet};
        qreal angle=90;
        if(snapshot.available && snapshot.equipment>0) for(int i=0;i<4;++i) {
            const qreal span=360.*counts[i]/snapshot.equipment*progress;
            p.setPen(QPen(colors[i],14,Qt::SolidLine,Qt::FlatCap));p.drawArc(ring,qRound(angle*16),-qRound(span*16));angle-=span;
        }
        const QString ratio=snapshot.available&&snapshot.equipment?QString::number(qRound(100.*snapshot.online/snapshot.equipment))+"%":"—";
        words(p,{ring.x(),ring.center().y()-23,d,35},ratio,29,ink(),QFont::DemiBold,Qt::AlignCenter);
        words(p,{ring.x(),ring.center().y()+13,d,19},"online",11,muted(),QFont::Normal,Qt::AlignCenter);
        const QStringList labels{"Online","Offline","Maintenance","Other"};
        const int x=int(d)+35;
        for(int i=0;i<4;++i) {
            p.setBrush(colors[i]);p.setPen(Qt::NoPen);p.drawEllipse(QPointF(x,44+i*36),4,4);
            words(p,{qreal(x+13),qreal(32+i*36),qreal(width()-x-50),24},labels[i],12,muted());
            words(p,{qreal(width()-38),qreal(32+i*36),32,24},snapshot.available?QString::number(counts[i]):"—",13,ink(),QFont::DemiBold,Qt::AlignRight|Qt::AlignVCenter);
        }
        words(p,{8,201,qreal(width()-16),20},snapshot.available?
              QString("%1 equipment items in total").arg(snapshot.equipment):"Status data appears after a successful refresh.",11,muted());
    }
};
// Repartition des equipements par type (couleur du type).
class TypeChart final : public QWidget {
public:
    DashboardSnapshot snapshot; qreal progress=1;
    TypeChart(){setMinimumHeight(228);setAccessibleName("Equipment by type");}
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        if(!snapshot.available||snapshot.types.isEmpty()) {
            drawIcon(p,{width()/2.,78},1,blue);
            words(p,{0,106,qreal(width()),26},snapshot.available?"No equipment registered yet":"Your equipment types will appear here",13,muted(),QFont::Normal,Qt::AlignCenter);
            return;
        }
        const int shown=qMin(5,int(snapshot.types.size()));
        const int maximum=qMax(1,snapshot.types.first().count);
        const int step=shown>4?37:46;
        for(int i=0;i<shown;++i) {
            const auto &t=snapshot.types[i];const int y=8+i*step;const QColor tint=typeTint(t.type);
            p.setPen(Qt::NoPen);p.setBrush(tint);p.drawEllipse(QPointF(5,y+11),4,4);
            words(p,{16,qreal(y),qreal(width()-60),22},t.type,12,ink(),QFont::DemiBold);
            words(p,{qreal(width()-42),qreal(y),40,22},QString::number(t.count),12,muted(),QFont::Normal,Qt::AlignRight|Qt::AlignVCenter);
            const QRectF track(0,y+26,width()-2,5);p.setBrush(edge());p.drawRoundedRect(track,2,2);
            QLinearGradient g(track.topLeft(),track.topRight());g.setColorAt(0,tint.lighter(125));g.setColorAt(1,tint);p.setBrush(g);
            p.drawRoundedRect(QRectF(track.x(),track.y(),track.width()*t.count/maximum*progress,5),2,2);
        }
        words(p,{0,201,qreal(width()),20},QString("%1 equipment type%2  ·  Count per type").arg(snapshot.types.size()).arg(snapshot.types.size()>1?"s":""),11,muted());
    }
};
QLabel *label(const QString &text,int size=13,bool bold=false) {
    auto *l=new QLabel(text);l->setTextFormat(Qt::PlainText);
    l->setStyleSheet(QString("background:transparent;color:#536c86;font-family:'Segoe UI';font-size:%1px;font-weight:%2;border:0;").arg(size).arg(bold?600:400));
    return l;
}
QLabel *title(const QString &text,int size) {
    auto *l=label(text,size,true);
    l->setStyleSheet(QString("background:transparent;color:#172f4d;font-family:'Segoe UI';font-size:%1px;font-weight:600;border:0;").arg(size));
    return l;
}
QPushButton *button(const QString &text) {
    auto *b=new QPushButton(text);b->setCursor(Qt::PointingHandCursor);b->setMinimumHeight(34);
    b->setStyleSheet("QPushButton{background:#edf5fd;color:#1a668c;border:1px solid #d8e7f2;border-radius:9px;padding:6px 14px;font-size:12px;font-weight:600;}QPushButton:hover{background:#d9eefb;border-color:#66accf;}QPushButton:focus{border:2px solid #287cbd;}");
    return b;
}
}

struct DashboardPage::Content {
    DashboardPage *owner;
    DashboardSnapshot snapshot;
    std::function<void(const QString &)> navigate;
    OrbitBanner *banner=new OrbitBanner;
    HealthChart *health=new HealthChart;
    TypeChart *types=new TypeChart;
    QVector<Metric*> metrics;
    QLabel *updated=label("Waiting for Oracle",11), *notice=label(""), *freshness=label(""), *watchSummary=label(""), *greeting=label("",12);
    ShareBar *freshnessBar=new ShareBar(green);
    QComboBox *filter=new QComboBox;
    QGridLayout *metricGrid=new QGridLayout, *chartGrid=new QGridLayout;
    QVBoxLayout *watchRows=new QVBoxLayout;
    Panel *healthPanel=new Panel, *typePanel=new Panel;
    int columns=0;
    QVariantAnimation entrance;
    Content(DashboardPage *page,std::function<void(const QString &)> nav):owner(page),navigate(std::move(nav)){}
    void fillWatchList() {
        while(auto *item=watchRows->takeAt(0)){delete item->widget();delete item;}
        int shown=0,total=0;
        for(const auto &item:snapshot.watchList) {
            const QString status=item.status.toCaseFolded();
            if(filter->currentIndex()==1 && status!="offline") continue;
            if(filter->currentIndex()==2 && status!="maintenance") continue;
            if(filter->currentIndex()==3 && !(item.ageDays<0||item.ageDays>=staleDays)) continue;
            ++total;if(shown++>=5)continue;
            auto *row=button("");row->setMinimumHeight(66);row->setObjectName("dashboardWatchRow");
            row->setStyleSheet("QPushButton{background:#f6f9fd;border:1px solid #e6edf5;border-radius:10px;}QPushButton:hover{background:#e9f3fc;border-color:#7eb6d3;}QPushButton:focus{border:2px solid #287cbd;}");
            auto *h=new QHBoxLayout(row);h->setContentsMargins(12,9,14,9);h->setSpacing(12);
            h->addWidget(new IconChip(1,typeTint(item.type)));
            auto *texts=new QVBoxLayout;texts->setSpacing(2);
            auto *name=title(item.id+"  ·  "+item.type,13);name->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
            texts->addWidget(name);
            QString detail=item.model.isEmpty()?QString():item.model+"  ·  ";
            detail+=item.follower.isEmpty()?QStringLiteral("Not followed"):"Followed by "+item.follower;
            detail+="  ·  "+(item.updated.isValid()?"Updated "+QLocale(QLocale::English).toString(item.updated,"dd MMM yyyy"):QStringLiteral("Never updated"));
            texts->addWidget(label(detail,11));
            if(item.ageDays<0||item.ageDays>=staleDays) {
                auto *age=label("",11,true);age->setTextFormat(Qt::RichText); // couleur visible dans les deux themes
                age->setText(QString("<span style='color:#e5566c;font-weight:600;'>%1</span>")
                             .arg(item.ageDays<0?QStringLiteral("No update recorded"):QString("%1 days without update").arg(item.ageDays)));
                texts->addWidget(age);
            }
            h->addLayout(texts,1);
            const bool offline=status=="offline", servicing=status=="maintenance";
            h->addWidget(new StatusPill(offline?"Offline":servicing?"Maintenance":"Needs update",offline?red:amber));
            h->addWidget(label("  →",18));
            row->setAccessibleName(item.id+": "+item.type+", "+item.status);
            row->setToolTip("Open the Equipment page");
            for(auto *l:row->findChildren<QLabel*>()) l->setAttribute(Qt::WA_TransparentForMouseEvents);
            QObject::connect(row,&QPushButton::clicked,owner,[this]{navigate("Equipment");});
            watchRows->addWidget(row);
        }
        if(!total) {
            auto *empty=label(snapshot.available?"All equipment in this view is online and up to date.":"Connect to Oracle to see the equipment to watch.",13);
            empty->setMinimumHeight(72);empty->setAlignment(Qt::AlignCenter);watchRows->addWidget(empty);
        } else {
            watchRows->addWidget(label(QString("Showing %1 of %2 equipment items  ·  Offline first, then maintenance, then the oldest updates.").arg(qMin(5,total)).arg(total),11));
        }
    }
    void render() {
        banner->snapshot=snapshot;banner->update();health->snapshot=snapshot;types->snapshot=snapshot;
        const bool ok=snapshot.available&&snapshot.equipment>0;
        const int counts[]={snapshot.equipment,snapshot.online,snapshot.offline,snapshot.servicing};
        const QStringList notes{QString("%1 equipment type%2").arg(snapshot.types.size()).arg(snapshot.types.size()>1?"s":""),
                                "Operational","Needs attention","Service in progress"};
        for(int i=0;i<metrics.size();++i){
            metrics[i]->number=snapshot.available?QString::number(counts[i]):"—";
            metrics[i]->note=snapshot.available?notes[i]:"Waiting for connection";
            metrics[i]->ratio=i==0||!ok?-1:qreal(counts[i])/snapshot.equipment;
            metrics[i]->update();
        }
        updated->setText(snapshot.available?"Updated "+snapshot.refreshed.toString("HH:mm:ss"):"Data unavailable");
        const int hour=QTime::currentTime().hour();
        const QString hello=hour<12?"Good morning":hour<18?"Good afternoon":"Good evening";
        greeting->setText(QString("%1, %2  ·  %3").arg(hello,AppSession::displayName(),
                          QLocale(QLocale::English).toString(QDate::currentDate(),"dddd, dd MMMM yyyy")));
        watchSummary->setText(snapshot.available?QString("%1 to watch  ·  %2 offline  ·  %3 in maintenance  ·  %4 not updated for %5 days or more")
                              .arg(snapshot.attention).arg(snapshot.offline).arg(snapshot.servicing).arg(snapshot.stale).arg(staleDays)
                              :"Offline, in maintenance or not updated recently");
        const int fresh=snapshot.equipment-snapshot.stale;
        freshness->setText(snapshot.available?QString("%1 of %2 equipment items updated in the last %3 days").arg(fresh).arg(snapshot.equipment).arg(staleDays)
                           :"Update freshness appears after connecting to Oracle.");
        freshnessBar->setRatio(ok?qreal(fresh)/snapshot.equipment:-1);
        fillWatchList();
        entrance.stop();entrance.setDuration(650);entrance.setStartValue(0.);entrance.setEndValue(1.);
        entrance.setEasingCurve(QEasingCurve::OutCubic);entrance.start();
    }
};

DashboardPage::DashboardPage(std::function<void(const QString &)> navigate,QWidget *parent)
    :QWidget(parent),content(std::make_unique<Content>(this,std::move(navigate))) {
    auto &c=*content;setObjectName("dashboardPage");
    setStyleSheet("QWidget#dashboardPage{background:#f2f6fb;}QScrollArea{background:transparent;border:0;}");
    auto *root=new QVBoxLayout(this);root->setContentsMargins(0,0,0,0);
    auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);root->addWidget(scroll);
    auto *body=new QWidget;body->setObjectName("dashboardBody");body->setStyleSheet("QWidget#dashboardBody{background:#f2f6fb;}");
    scroll->setWidget(body);
    auto *layout=new QVBoxLayout(body);layout->setContentsMargins(26,22,26,26);layout->setSpacing(17);
    auto *head=new QHBoxLayout;
    auto *titles=new QVBoxLayout;titles->setSpacing(2);titles->addWidget(title("Equipment overview",25));titles->addWidget(c.greeting);
    head->addLayout(titles);head->addStretch();head->addWidget(c.updated);
    auto *refreshButton=button("↻  Refresh");refreshButton->setObjectName("dashboardRefreshButton");
    refreshButton->setToolTip("Reload the equipment from Oracle");
    head->addWidget(refreshButton);layout->addLayout(head);
    connect(refreshButton,&QPushButton::clicked,this,[this]{refresh();});
    c.notice->setWordWrap(true);c.notice->setStyleSheet("background:#fff3de;color:#865918;border:1px solid #efdbb4;border-radius:9px;padding:10px;font-size:12px;");
    c.notice->hide();layout->addWidget(c.notice);layout->addWidget(c.banner);

    const QStringList names{"Registered Equipment","Online","Offline","In Maintenance"};
    const QColor colors[]={blue,green,red,amber};
    const int kinds[]={1,5,6,7};
    c.metricGrid->setSpacing(12);layout->addLayout(c.metricGrid);
    for(int i=0;i<names.size();++i) {
        auto *tile=new Metric(names[i],colors[i],kinds[i]);tile->setObjectName(QString("dashboardMetric%1").arg(i));c.metrics.append(tile);
        connect(tile,&QAbstractButton::clicked,this,[this]{content->navigate("Equipment");});
    }
    auto configurePanel=[&](Panel *panel,const QString &heading,const QString &subtitle,QWidget *chart){
        auto *v=new QVBoxLayout(panel);v->setContentsMargins(20,17,20,12);v->setSpacing(4);
        auto *h=new QHBoxLayout;h->addWidget(title(heading,16));h->addStretch();
        auto *open=button("Open →");open->setAccessibleName("Open the Equipment page");h->addWidget(open);
        connect(open,&QPushButton::clicked,this,[this]{content->navigate("Equipment");});
        v->addLayout(h);v->addWidget(label(subtitle,11));v->addWidget(chart);
    };
    configurePanel(c.healthPanel,"Equipment status","Recorded state across the entire fleet",c.health);
    configurePanel(c.typePanel,"Equipment by type","How the fleet is split between device types",c.types);
    c.chartGrid->setSpacing(16);layout->addLayout(c.chartGrid);

    auto *watch=new Panel;auto *wLayout=new QVBoxLayout(watch);wLayout->setContentsMargins(20,18,20,18);wLayout->setSpacing(12);
    auto *wHeader=new QHBoxLayout;auto *wTitles=new QVBoxLayout;wTitles->setSpacing(3);
    wTitles->addWidget(title("Needs attention",17));wTitles->addWidget(c.watchSummary);
    wHeader->addLayout(wTitles,1);c.filter->addItems({"All","Offline","Maintenance","Needs update"});c.filter->setMinimumHeight(35);
    c.filter->setAccessibleName("Filter the equipment to watch");c.filter->setObjectName("dashboardWatchFilter");
    c.filter->setStyleSheet("QComboBox{background:#edf5fd;border:1px solid #d8e7f2;border-radius:8px;padding:6px 30px 6px 14px;color:#245675;min-width:130px;font-size:12px;}"
                            "QComboBox::drop-down{subcontrol-origin:padding;subcontrol-position:center right;width:26px;border:none;background:transparent;}"
                            "QComboBox::down-arrow{image:url(:/assets/icons/chevron_down.svg);width:12px;height:12px;}");
    wHeader->addWidget(c.filter);wLayout->addLayout(wHeader);wLayout->addLayout(c.watchRows);layout->addWidget(watch);

    auto *freshPanel=new Panel;auto *fLayout=new QHBoxLayout(freshPanel);fLayout->setContentsMargins(20,14,20,14);fLayout->setSpacing(16);
    fLayout->addWidget(new IconChip(2,green));
    auto *fTexts=new QVBoxLayout;fTexts->setSpacing(6);
    fTexts->addWidget(title("Update freshness",15));fTexts->addWidget(c.freshnessBar);fTexts->addWidget(c.freshness);
    fLayout->addLayout(fTexts,1);
    auto *openEquipment=button("Open equipment →");fLayout->addWidget(openEquipment);
    connect(openEquipment,&QPushButton::clicked,this,[this]{content->navigate("Equipment");});
    layout->addWidget(freshPanel);
    layout->addWidget(label(QString("Counts cover every record of the EQUIPEMENT table. An update of %1 days or more is flagged, as on the Equipment page.").arg(staleDays),11));
    layout->addStretch();
    connect(c.filter,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{content->fillWatchList();});
    connect(&c.entrance,&QVariantAnimation::valueChanged,this,[this](QVariant value){
        content->health->progress=value.toReal();content->types->progress=value.toReal();
        content->health->update();content->types->update();
    });
    connect(AppTheme::instance(),&AppTheme::changed,this,[this]{update();for(auto *child:findChildren<QWidget*>())child->update();});
    c.render();
}
DashboardPage::~DashboardPage()=default;
void DashboardPage::refresh() {
    QString error;
    const bool success=loadDashboardSnapshot(Connection::createInstance().database(),&content->snapshot,&error);
    content->notice->setVisible(!success);
    content->notice->setText(Connection::createInstance().isConnected()?
        "The overview could not be refreshed. Check the database connection, then select Refresh.":
        "Oracle is disconnected. Connect when starting SkySense, then return to the dashboard.");
    content->notice->setToolTip(error);
    content->render();
}
void DashboardPage::showEvent(QShowEvent *event) {QWidget::showEvent(event);refresh();}
void DashboardPage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    const int cols=width()>=900?4:2;
    auto &c=*content;
    const int mode=cols*10+(width()>=800?2:1);
    if(c.columns==mode)return;
    c.columns=mode;
    for(auto *m:c.metrics)c.metricGrid->removeWidget(m);
    for(int i=0;i<c.metrics.size();++i)c.metricGrid->addWidget(c.metrics[i],i/cols,i%cols);
    for(int i=0;i<4;++i)c.metricGrid->setColumnStretch(i,i<cols?1:0);
    c.chartGrid->removeWidget(c.healthPanel);c.chartGrid->removeWidget(c.typePanel);
    c.chartGrid->addWidget(c.healthPanel,0,0);
    c.chartGrid->addWidget(c.typePanel,width()>=800?0:1,width()>=800?1:0);
    c.chartGrid->setColumnStretch(0,1);c.chartGrid->setColumnStretch(1,width()>=800?1:0);
}
