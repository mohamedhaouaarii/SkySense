#include <QPainterPath>
#include "apptheme.h"
#include <QApplication>
#include <QStyle>
#include <QSettings>
#include <QRegularExpression>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <QGraphicsView>
#include <QImage>
#include <QtMath>

namespace GestionStation {

AppTheme::AppTheme(QObject *parent):QObject(parent){}
AppTheme *AppTheme::instance(){static AppTheme *theme=new AppTheme(qApp);return theme;}
bool AppTheme::dark(){return instance()->enabled;}
void AppTheme::initialize(bool persist){
    persistence=persist; lightPalette=qApp->palette();
    enabled=QSettings().value("appearance/darkMode",false).toBool();
    qApp->installEventFilter(this);
}
QColor AppTheme::color(QColor c,Role role){
    if(!dark() || !c.isValid() || c.alpha()==0)return c;
    const int alpha=c.alpha(); const qreal l=c.lightnessF(),sat=c.hslSaturationF();
    QColor out=c;
    if(role==Role::Surface){
        if(l>.82){
            // Neutral cards and pale semantic badges share depth, retaining their hue.
            out=QColor("#18283e");
            if(sat>.18 && l<.965)out=QColor::fromHslF(c.hslHueF()<0?.60:c.hslHueF(),qMin(.43,sat*.65),.16);
        }else if(l>.64 && sat<.55)out=QColor("#263a54");
    }else if(role==Role::Border){
        if(l>.62)out=QColor("#344b67");
        else if(sat<.3 && l<.35)out=QColor("#3c526c");
    }else{
        if(l<.26)out=QColor("#e5eef9");
        else if(l<.68){
            if(sat>.60)out=QColor::fromHslF(c.hslHueF(),qMin(.86,sat),qMax(.66,l));
            else out=QColor("#afc0d6");
        }
    }
    out.setAlpha(alpha);return out;
}
static bool themePaintDevice(const QPainter &p){return p.device() && p.device()->devType()==QInternal::Widget;}
QPen AppTheme::pen(const QPainter &p,QPen source){
    if(dark() && themePaintDevice(p) && source.style()!=Qt::NoPen){
        const QColor c=source.color();
        const bool paleBorder=c.lightnessF()>.84 && !(c.red()>250 && c.green()>250 && c.blue()>250);
        source.setColor(color(c,paleBorder?Role::Border:Role::Text));
    }return source;
}
static QBrush themeBrush(QBrush b){
    if(b.gradient()){
        QGradient g=*b.gradient();QGradientStops stops=g.stops();
        for(auto &stop:stops)stop.second=AppTheme::color(stop.second,AppTheme::Role::Surface);
        g.setStops(stops);QBrush out(g);out.setTransform(b.transform());return out;
    }
    if(b.style()!=Qt::NoBrush && b.texture().isNull())b.setColor(AppTheme::color(b.color(),AppTheme::Role::Surface));
    return b;
}
QBrush AppTheme::brush(const QPainter &p,QBrush source){return dark() && themePaintDevice(p)?themeBrush(source):source;}

QString AppTheme::themedStyle(const QString &source) const{
    if(!enabled)return source;
    static const QRegularExpression declaration(R"(([\w-]+)\s*:\s*([^;{}]+))");
    static const QRegularExpression token(R"(#[0-9a-fA-F]{3,8}\b|rgba?\([^)]*\)|\b(?:white|black|navy|gray|grey|silver|red|green|blue)\b)",QRegularExpression::CaseInsensitiveOption);
    QString result;int last=0;auto matches=declaration.globalMatch(source);
    while(matches.hasNext()){
        const auto m=matches.next();QString key=m.captured(1).toLower(),value=m.captured(2);
        Role role=Role::Text;
        const bool relevant=key.contains("background")||key.contains("color")||key.startsWith("border");
        if(key.contains("background"))role=Role::Surface;else if(key.startsWith("border"))role=Role::Border;
        if(relevant){
            QString converted;int end=0;auto colors=token.globalMatch(value);
            while(colors.hasNext()){
                auto cm=colors.next();QString literal=cm.captured();QColor input;
                if(literal.startsWith("rgb",Qt::CaseInsensitive)){
                    auto parts=literal.mid(literal.indexOf('(')+1).chopped(1).split(',');
                    if(parts.size()>=3){
                        auto channel=[](QString t){t=t.trimmed();return t.endsWith('%')?qRound(t.chopped(1).toDouble()*2.55):t.toInt();};
                        int a=255;if(parts.size()==4){QString av=parts[3].trimmed();a=av.contains('.')?qRound(av.toDouble()*255):channel(av);}
                        input=QColor(channel(parts[0]),channel(parts[1]),channel(parts[2]),qBound(0,a,255));
                    }
                }else input=QColor(literal);
                QColor output=color(input,role);
                QString replacement=literal;
                if(output.isValid())replacement=output.alpha()==255?output.name():QString("rgba(%1,%2,%3,%4)").arg(output.red()).arg(output.green()).arg(output.blue()).arg(output.alpha());
                converted+=value.mid(end,cm.capturedStart()-end)+replacement;end=cm.capturedEnd();
            }
            value=converted+value.mid(end);
        }
        result+=source.mid(last,m.capturedStart()-last)+m.captured(1)+":"+value;last=m.capturedEnd();
    }
    return result+source.mid(last);
}
static QPixmap themedIconPixmap(QPixmap pm){
    QImage image=pm.toImage().convertToFormat(QImage::Format_ARGB32);
    for(int y=0;y<image.height();++y){auto *row=reinterpret_cast<QRgb*>(image.scanLine(y));for(int x=0;x<image.width();++x){
        QColor c=QColor::fromRgba(row[x]);if(c.alpha())row[x]=AppTheme::color(c,AppTheme::Role::Text).rgba();
    }}
    QPixmap result=QPixmap::fromImage(image);result.setDevicePixelRatio(pm.devicePixelRatio());return result;
}
static QIcon themedIcon(const QIcon &original){
    QIcon out;for(int size:{16,20,24,28,32,48,64})out.addPixmap(themedIconPixmap(original.pixmap(size,size)));
    return out;
}
void AppTheme::applyIcons(QWidget *w){
    auto updateIcon=[this](QObject *object,QIcon current,auto setter){
        if(current.isNull())return;
        const qint64 applied=object->property("themeAppliedIcon").toLongLong();
        if(current.cacheKey()!=applied)object->setProperty("themeOriginalIcon",current);
        QIcon original=object->property("themeOriginalIcon").value<QIcon>();
        const bool state=object->property("themeIconDark").toBool();
        if(current.cacheKey()==applied && state==enabled)return;
        QIcon next=enabled?themedIcon(original):original;
        object->setProperty("themeAppliedIcon",next.cacheKey());object->setProperty("themeIconDark",enabled);setter(next);
    };
    if(auto *b=qobject_cast<QAbstractButton*>(w))updateIcon(b,b->icon(),[b](QIcon i){b->setIcon(i);});
    for(auto *a:w->actions())updateIcon(a,a->icon(),[a](QIcon i){a->setIcon(i);});
    if(auto *l=qobject_cast<QLabel*>(w)){
        // Rich-text labels can specify their own colors outside QSS.
        const QString text=l->text();
        if(text.contains("style=")){
            if(!l->property("themeTextKnown").toBool() || text!=l->property("themeAppliedText").toString())l->setProperty("themeSourceText",text);
            l->setProperty("themeTextKnown",true);
            QString next=l->property("themeSourceText").toString();
            if(enabled){
                static const QRegularExpression cssColor("color:(#[0-9a-fA-F]{3,8})");
                auto matches=cssColor.globalMatch(next);QList<QRegularExpressionMatch> found;
                while(matches.hasNext())found.append(matches.next());
                for(auto it=found.crbegin();it!=found.crend();++it)next.replace(it->capturedStart(1),it->capturedLength(1),color(QColor(it->captured(1)),Role::Text).name());
            }
            l->setProperty("themeAppliedText",next);if(next!=text)l->setText(next);
        }
        QPixmap pm=l->pixmap();
        if(pm.isNull() || pm.deviceIndependentSize().width()>64 || pm.deviceIndependentSize().height()>64)return;
        const qint64 applied=l->property("themeAppliedPixmap").toLongLong();
        if(pm.cacheKey()!=applied)l->setProperty("themeOriginalPixmap",pm);
        if(pm.cacheKey()==applied && l->property("themePixmapDark").toBool()==enabled)return;
        QPixmap original=l->property("themeOriginalPixmap").value<QPixmap>();
        QPixmap next=enabled?themedIconPixmap(original):original;
        l->setProperty("themeAppliedPixmap",next.cacheKey());l->setProperty("themePixmapDark",enabled);l->setPixmap(next);
    }
}
void AppTheme::applyWidget(QWidget *w){
    if(!w)return;
    const QString current=w->styleSheet();
    if(!w->property("themeStyleKnown").toBool() || current!=w->property("themeAppliedStyle").toString())w->setProperty("themeSourceStyle",current);
    w->setProperty("themeStyleKnown",true);
    const QString desired=themedStyle(w->property("themeSourceStyle").toString());
    w->setProperty("themeAppliedStyle",desired);
    if(current!=desired)w->setStyleSheet(desired);
    if(!w->property("themeOriginalPalette").isValid())w->setProperty("themeOriginalPalette",w->testAttribute(Qt::WA_SetPalette)?w->palette().resolve(lightPalette):lightPalette);
    QPalette palette=w->property("themeOriginalPalette").value<QPalette>();
    if(enabled){
        palette.setColor(QPalette::Window,QColor("#101c2e"));palette.setColor(QPalette::Base,QColor("#15243a"));
        palette.setColor(QPalette::AlternateBase,QColor("#1c2e46"));palette.setColor(QPalette::Text,QColor("#e5eef9"));
        palette.setColor(QPalette::WindowText,QColor("#e5eef9"));palette.setColor(QPalette::Button,QColor("#223751"));
        palette.setColor(QPalette::ButtonText,QColor("#e5eef9"));palette.setColor(QPalette::Highlight,QColor("#2879db"));
        palette.setColor(QPalette::HighlightedText,Qt::white);palette.setColor(QPalette::PlaceholderText,QColor("#90a6c1"));
        palette.setColor(QPalette::ToolTipBase,QColor("#20344e"));palette.setColor(QPalette::ToolTipText,QColor("#e5eef9"));
        palette.setColor(QPalette::Disabled,QPalette::Text,QColor("#7f93ad"));palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor("#7f93ad"));
    }
    if(w->palette()!=palette)w->setPalette(palette);
    if(auto *view=qobject_cast<QGraphicsView*>(w)){
        if(!view->property("themeSourceBackground").isValid())view->setProperty("themeSourceBackground",view->backgroundBrush());
        auto b=view->property("themeSourceBackground").value<QBrush>();view->setBackgroundBrush(enabled?themeBrush(b):b);
    }
    applyIcons(w);w->update();
}
void AppTheme::applyAll(){
    if(busy)return;
    busy=true;
    const auto roots=qApp->topLevelWidgets();for(auto *root:roots)root->setUpdatesEnabled(false);
    for(auto *w:qApp->allWidgets())applyWidget(w);
    for(auto *root:roots)root->setUpdatesEnabled(true);
    busy=false;
}
void AppTheme::setDark(bool value){
    if(enabled==value)return;
    enabled=value;
    if(persistence)QSettings().setValue("appearance/darkMode",enabled);
    applyAll();emit changed(enabled);
}
bool AppTheme::eventFilter(QObject *object,QEvent *event){
    if(busy)return false;
    auto *w=qobject_cast<QWidget*>(object);if(!w)return false;
    if(event->type()==QEvent::Polish || event->type()==QEvent::Show){busy=true;applyWidget(w);busy=false;}
    else if(event->type()==QEvent::StyleChange && !w->property("themePending").toBool()){
        w->setProperty("themePending",true);
        QTimer::singleShot(0,w,[this,w]{w->setProperty("themePending",false);if(!busy){busy=true;applyWidget(w);busy=false;}});
    }else if(event->type()==QEvent::Paint){busy=true;applyIcons(w);busy=false;}
    return false;
}

ThemeSwitch::ThemeSwitch(QWidget *parent):QAbstractButton(parent){
    setObjectName("themeSwitch");setFixedSize(148,34);setCursor(Qt::PointingHandCursor);setCheckable(true);setFocusPolicy(Qt::StrongFocus);
    setAccessibleName("Dark mode");setChecked(AppTheme::dark());position=isChecked()?1:0;
    animation.setDuration(180);animation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&animation,&QVariantAnimation::valueChanged,this,[this](QVariant v){position=v.toReal();update();});
    connect(AppTheme::instance(),&AppTheme::changed,this,[this](bool dark){
        setChecked(dark);setToolTip(dark?"Switch to light mode":"Switch to dark mode");
        animation.stop();animation.setStartValue(position);animation.setEndValue(dark?1.:0.);animation.start();
    });
    setToolTip(isChecked()?"Switch to light mode":"Switch to dark mode");
}
void ThemeSwitch::nextCheckState(){AppTheme::instance()->setDark(!AppTheme::dark());}
void ThemeSwitch::paintEvent(QPaintEvent*){
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);const bool dark=AppTheme::dark();
    p.setPen(QPen(dark?QColor("#3a536f"):QColor("#c5d6e8"),1));p.setBrush(dark?QColor("#0d1a2c"):QColor("#e8eff7"));
    p.drawRoundedRect(QRectF(.5,.5,width()-1,height()-1),17,17);
    QRectF active(3+position*71,3,71,28);QLinearGradient material(active.topLeft(),active.bottomLeft());
    material.setColorAt(0,dark?QColor("#344e70"):QColor("#ffffff"));material.setColorAt(1,dark?QColor("#243c5b"):QColor("#f4f8fd"));
    p.setPen(QPen(dark?QColor("#52749b"):QColor("#ffffff"),1));p.setBrush(material);p.drawRoundedRect(active,14,14);
    for(int side=0;side<2;++side){
        const bool selected=(side==1)==dark;QColor ink=selected?(dark?QColor("#e6f4ff"):QColor("#173c66")):(dark?QColor("#a3b7d1"):QColor("#536d8d"));
        p.setPen(QPen(ink,1.4,Qt::SolidLine,Qt::RoundCap));p.setBrush(Qt::NoBrush);QPointF c(17+side*71,17);
        if(!side){p.drawEllipse(c,3.3,3.3);for(int i=0;i<8;++i){qreal a=i*M_PI/4;p.drawLine(c+QPointF(qCos(a),qSin(a))*5.3,c+QPointF(qCos(a),qSin(a))*6.7);}}
        else{QPainterPath moon;moon.addEllipse(c,6,6);QPainterPath cut;cut.addEllipse(c+QPointF(3,-2),5.5,5.5);p.setPen(Qt::NoPen);p.setBrush(ink);p.drawPath(moon.subtracted(cut));}
        QFont f("Segoe UI");f.setPixelSize(11);f.setWeight(selected?QFont::DemiBold:QFont::Normal);p.setFont(f);p.setPen(ink);p.drawText(QRect(29+side*71,0,42,34),Qt::AlignVCenter,side?"Dark":"Light");
    }
    if(hasFocus()){p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor("#48aaff"),1.5,Qt::DashLine));p.drawRoundedRect(QRectF(2,2,width()-4,height()-4),15,15);}
}

} // namespace GestionStation
