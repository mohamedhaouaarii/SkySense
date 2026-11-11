
#pragma once
#include "apptheme.h"
#include <QtWidgets>
#include <algorithm>

namespace StaffDesign {
inline QWidget *dialogOwner(QWidget *widget)
{
    QWidget *top=widget->window();
    if (auto *proxy=top->graphicsProxyWidget())
        if (proxy->scene() && !proxy->scene()->views().isEmpty()) return proxy->scene()->views().constFirst()->window();
    return top;
}
inline QIcon notificationBell(bool unread)
{
    QPixmap image(64,64);
    image.fill(Qt::transparent);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor ink=AppTheme::dark()?QColor("#d9d6ff"):QColor("#373255");
    p.setPen(QPen(ink,3.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    QPainterPath bell;
    bell.moveTo(16,43);bell.cubicTo(21,38,20,34,20,27);
    bell.cubicTo(20,11,44,11,44,27);bell.cubicTo(44,34,43,38,48,43);
    bell.closeSubpath();p.drawPath(bell);
    p.drawArc(QRectF(26,41,12,11),180*16,180*16);
    if(unread){
        p.setPen(QPen(QColor("#ffffff"),2));
        p.setBrush(QColor("#d94a68"));p.drawEllipse(QPointF(48,15),8,8);
    }
    return QIcon(image);
}
class Masthead final : public QFrame {
public:
    using QFrame::QFrame;
    void fitControls() {
        const bool staff=property("staffMode").toBool();
        const bool compact=staff && width()<1130;
        if(auto *row=qobject_cast<QHBoxLayout*>(layout())){
            row->setContentsMargins(compact?16:34,12,compact?12:24,12);
            row->setSpacing(compact?7:16);
        }
        if(auto *title=findChild<QLabel*>("staffHeadingText")){
            const int size=compact?21:26;
            title->setStyleSheet(QString("font:700 %1px 'Segoe UI';color:#211a43;background:transparent;").arg(size));
            title->setMinimumWidth(title->fontMetrics().horizontalAdvance(title->text())+3);
        }
        if(auto *icon=findChild<QLabel*>("workspaceCalendarIcon"))icon->setVisible(!compact);
        if(auto *date=findChild<QDateEdit*>("workspaceReportDate"))date->setFixedWidth(compact?140:165);
        if(auto *search=findChild<QLineEdit*>("workspaceSearch"))search->setMinimumWidth(compact?110:180);
    }
protected:
    void resizeEvent(QResizeEvent *e) override {QFrame::resizeEvent(e);fitControls();}
    void paintEvent(QPaintEvent *e) override {
        QFrame::paintEvent(e);
        if(!property("staffMode").toBool()) return;
        QPainter p(this);
        const bool dark=AppTheme::dark();
        QLinearGradient sky(0,0,width(),height());
        sky.setColorAt(0,dark?QColor("#22223c"):QColor("#f2f0fc"));
        sky.setColorAt(1,dark?QColor("#2c3853"):QColor("#d6e7fc"));
        p.fillRect(rect(),sky);
        static const QPixmap landscape(":/images/weather_landscape.png");
        QRect band(0,0,width(),height());
        if(!landscape.isNull()){
            p.setOpacity(dark?.12:.13);
            p.drawPixmap(band,landscape,QRect(0,landscape.height()/3,landscape.width(),landscape.height()/3));
            p.setOpacity(1);
        }
    }
};

class ElidingLabel final : public QLabel {
public:
    using QLabel::QLabel;
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setFont(font());p.setPen(palette().color(foregroundRole()));
        p.drawText(contentsRect(),Qt::AlignLeft|Qt::AlignVCenter,
                   fontMetrics().elidedText(text(),Qt::ElideRight,contentsRect().width()));
    }
};
inline QString initials(QString first,QString last,const QString &display = {})
{
    if (first.trimmed().isEmpty() && last.trimmed().isEmpty()) {
        const QStringList parts=display.simplified().split(' ',Qt::SkipEmptyParts);
        first=parts.value(0);
        if (parts.size()>1) last=parts.last();
    }
    auto initial=[](QString s) {
        s=s.trimmed();
        QTextBoundaryFinder boundary(QTextBoundaryFinder::Grapheme,s);
        return s.left(qMax(0, boundary.toNextBoundary())).toUpper();
    };
    const QString value=initial(first)+initial(last);
    return value.isEmpty()?QStringLiteral("—"):value;
}
inline QLabel *label(QWidget *parent,const QString &value,const char *kind)
{
    const QByteArray type(kind);
    QLabel *l=(type=="value" || type=="contact")?static_cast<QLabel*>(new ElidingLabel(value,parent)):new QLabel(value,parent);
    l->setProperty("type",kind);
    l->setTextFormat(Qt::PlainText);
    return l;
}
inline QFrame *card(QWidget *parent,const char *kind="panel")
{
    auto *f=new QFrame(parent);
    f->setProperty("card",kind);
    return f;
}
inline QPushButton *action(QWidget *parent,const QString &title,const char *id)
{
    auto *b=new QPushButton(title,parent);
    b->setObjectName(id);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}
inline QVBoxLayout *interior(QWidget *card,int padding=14)
{
    auto *l=new QVBoxLayout(card);
    l->setContentsMargins(padding,padding,padding,padding);
    l->setSpacing(8);
    return l;
}

class Ring final : public QWidget {
    int enabled=-1;
public:
    explicit Ring(QWidget *parent):QWidget(parent) { setFixedSize(72,72); }
    void setCount(int value) { enabled=value; update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        const QRectF circle=QRectF(rect()).adjusted(6,6,-6,-6);
        p.setPen(AppTheme::pen(p,QPen(QColor("#dfe5ee"),6,Qt::SolidLine,Qt::RoundCap)));
        p.drawEllipse(circle);
        if(enabled>0){
            p.setPen(AppTheme::pen(p,QPen(QColor("#16976c"),6,Qt::SolidLine,Qt::RoundCap)));
            p.drawArc(circle,90*16,-enabled*90*16);
        }
        QFont f("Segoe UI");f.setPixelSize(18);f.setBold(true);p.setFont(f);
        p.setPen(AppTheme::pen(p,QColor("#253052")));
        p.drawText(rect(),Qt::AlignCenter,enabled<0?QStringLiteral("—"):QString("%1/4").arg(enabled));
    }
};

class DirectoryCards final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &,const QModelIndex &) const override {return {232,77};}
    void paint(QPainter *p,const QStyleOptionViewItem &opt,const QModelIndex &idx) const override {
        p->save();p->setRenderHint(QPainter::Antialiasing);
        const QRect box=opt.rect.adjusted(2,2,-2,-2);
        const bool selected=opt.state & QStyle::State_Selected;
        p->setBrush(AppTheme::brush(*p,selected?QColor("#eeebff"):QColor("#fafaff")));
        p->setPen(AppTheme::pen(*p,selected?QColor("#8a79cf"):QColor("#e1deef")));
        p->drawRoundedRect(box,9,9);
        QRect chip(box.left()+8,box.center().y()-18,36,36);
        p->setBrush(AppTheme::brush(*p,QColor("#e8e3fa")));p->setPen(Qt::NoPen);p->drawRoundedRect(chip,11,11);
        QFont f("Segoe UI");f.setBold(true);f.setPixelSize(13);p->setFont(f);
        p->setPen(AppTheme::pen(*p,QColor("#413383")));
        p->drawText(chip,Qt::AlignCenter,idx.data(Qt::UserRole+1).toString());
        QRect line=box.adjusted(51,9,-6,-9);line.setHeight(18);
        p->drawText(line,Qt::AlignVCenter,p->fontMetrics().elidedText(idx.data().toString(),Qt::ElideRight,line.width()));
        f.setBold(false);f.setPixelSize(12);p->setFont(f);line.translate(0,17);
        p->setPen(AppTheme::pen(*p,QColor("#56516e")));
        p->drawText(line,Qt::AlignVCenter,p->fontMetrics().elidedText(idx.data(Qt::UserRole+2).toString(),Qt::ElideRight,line.width()));
        line.translate(0,16);p->drawText(line,Qt::AlignVCenter,idx.data(Qt::UserRole+3).toString());
        p->restore();
    }
};

class Metric final : public QFrame {
    QString caption, glyph;
    QColor accent;
    int value=0;
    QString detail;
    QList<int> joins;
public:
    Metric(QWidget *parent,QString title,QString mark,QColor color):
        QFrame(parent),caption(std::move(title)),glyph(std::move(mark)),accent(color)
    { setProperty("card","metric");setFixedHeight(90); }
    void setValue(int n,const QString &hint,const QList<int> &series) {
        value=n;detail=hint;joins=series;setAccessibleName(caption);setAccessibleDescription(QString::number(n)+" "+hint);
        setToolTip("Current roster, accumulated by join date");update();
    }
protected:
    void paintEvent(QPaintEvent *e) override {
        QFrame::paintEvent(e);
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        QColor wash=accent;wash.setAlpha(24);
        p.setPen(Qt::NoPen);p.setBrush(wash);p.drawRoundedRect(QRectF(13,21,44,44),14,14);
        p.setPen(AppTheme::pen(p,QPen(accent,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)));
        p.setBrush(Qt::NoBrush);
        if(glyph=="layers"){
            for(int dy : {0,5,10}) { QPolygonF path;path<<QPointF(23,36+dy)<<QPointF(35,30+dy)<<QPointF(47,36+dy)<<QPointF(35,42+dy);p.drawPolygon(path); }
        }else{
            p.drawEllipse(QPointF(33,37),4,4);p.drawArc(QRectF(24,44,17,13),0,180*16);
            if(glyph=="all"){ p.drawEllipse(QPointF(43,38),3,3);p.drawArc(QRectF(41,44,8,10),0,110*16); }
            else{p.drawLine(41,44,48,44);if(glyph=="active")p.drawLine(44.5,40.5,44.5,47.5);}
        }
        QFont f("Segoe UI");f.setPixelSize(28);f.setBold(true);p.setFont(f);p.setPen(AppTheme::pen(p,QColor("#1c1940")));
        p.drawText(QRect(68,12,width()-82,39),Qt::AlignVCenter,QString::number(value));
        f.setPixelSize(14);p.setFont(f);p.drawText(QRect(69,51,width()-82,25),Qt::AlignVCenter,caption);
        if(joins.size()>1 && width()>=240 && joins.last()>0) {
            const QRectF chart(width()-76,53,61,17);QPolygonF path;
            for(int i=0;i<joins.size();++i)
                path.append({chart.left()+chart.width()*i/(joins.size()-1),
                             chart.bottom()-chart.height()*joins[i]/joins.last()});
            p.setPen(AppTheme::pen(p,QPen(accent,1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)));
            p.setBrush(Qt::NoBrush);p.drawPolyline(path);
        }
        if(!detail.isEmpty()){
            f.setPixelSize(11);p.setFont(f);
            int w=p.fontMetrics().horizontalAdvance(detail)+15;
            QRect pill(width()-w-12,12,w,23);
            p.setPen(Qt::NoPen);p.setBrush(wash);p.drawRoundedRect(pill,10,10);
            p.setPen(AppTheme::pen(p,accent));p.drawText(pill,Qt::AlignCenter,detail);
        }
    }
};
}
