#include <cmath>
#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QCursor>
#include <QtMath>
#include <QPushButton>
#include <QVariantAnimation>
#include <QEnterEvent>

class LoginAction final : public QPushButton {
    QVariantAnimation transition;
    qreal hover = 0;
    void animateTo(qreal target) {
        transition.stop(); transition.setStartValue(hover); transition.setEndValue(target); transition.start();
    }
public:
    explicit LoginAction(const QString &text, QWidget *parent=nullptr) : QPushButton(text,parent) {
        transition.setDuration(180); transition.setEasingCurve(QEasingCurve::OutCubic);
        connect(&transition,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){hover=v.toReal();update();});
    }
protected:
    void enterEvent(QEnterEvent *e) override { QPushButton::enterEvent(e);animateTo(1); }
    void leaveEvent(QEvent *e) override { QPushButton::leaveEvent(e);animateTo(0); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        QRectF box=QRectF(rect()).adjusted(1,1,-1,-1);
        QLinearGradient fill(box.topLeft(),box.bottomRight());
        fill.setColorAt(0,QColor(12+int(hover*15),139+int(hover*20),249));
        fill.setColorAt(.52,QColor(5,111+int(hover*18),222));fill.setColorAt(1,QColor(9,67,165));
        p.setPen(QPen(QColor(37,153,244),1));p.setBrush(isDown()?QBrush(QColor("#095bb8")):QBrush(fill));
        p.drawRoundedRect(box,13,13);
        p.setPen(QPen(QColor(202,241,255,95+int(hover*55)),1));
        p.drawLine(QPointF(16,2),QPointF(width()-16,2));
        if(hasFocus()) {p.setPen(QPen(QColor("#e7f7ff"),1.4,Qt::DashLine));p.setBrush(Qt::NoBrush);p.drawRoundedRect(box.adjusted(4,4,-4,-4),9,9);}
        QFont f=font();f.setPixelSize(16);f.setBold(true);p.setFont(f);
        const QString caption=text().trimmed(); const int tw=p.fontMetrics().horizontalAdvance(caption);
        const int x=(width()-tw-36)/2; const int dy=isDown()?1:0;
        icon().paint(&p,QRect(x,height()/2-12+dy,24,24));
        p.setPen(Qt::white);p.drawText(QRect(x+36,dy,tw,height()),Qt::AlignVCenter,caption);
    }
};

// Native, projected sphere: no browser, network service or database dependency.
class LoginScene final : public QWidget {
    QTimer timer;
    qreal phase = 0;
    QPointF offset;
public:
    explicit LoginScene(QWidget *parent = nullptr) : QWidget(parent) {
        timer.setInterval(33);
        connect(&timer, &QTimer::timeout, this, [this] {
            phase += .003;
            const QPoint pt = mapFromGlobal(QCursor::pos());
            const QPointF target = rect().contains(pt)
                ? QPointF((qreal(pt.x()) / qMax(1,width()) - .5) * 12,
                          (qreal(pt.y()) / qMax(1,height()) - .5) * 8) : QPointF();
            offset += (target - offset) * .07;
            update();
        });
    }
protected:
    void showEvent(QShowEvent *event) override { QWidget::showEvent(event); timer.start(); }
    void hideEvent(QHideEvent *event) override { timer.stop(); QWidget::hideEvent(event); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#0b1830"));
        const QWidget *area = findChild<QWidget *>("sceneSpace");
        const QRectF zone = area ? QRectF(area->mapTo(this,QPoint(0,0)),area->size())
                                 : QRectF(0,height()*.4,width()*.6,height()*.55);
        const QPointF c = zone.center() + offset;
        const qreal r = qMin(zone.height() * .42, zone.width() * .25);
        QRadialGradient atmosphere(c, r * 2.7);
        atmosphere.setColorAt(0, QColor(50,153,198,118));
        atmosphere.setColorAt(.5, QColor(29,91,151,78));
        atmosphere.setColorAt(1, QColor(4,12,27,0));
        p.fillRect(rect(), atmosphere);
        QRadialGradient rightGlow(QPointF(width()*.94,height()*.7),width()*.5);
        rightGlow.setColorAt(0,QColor(53,112,183,82)); rightGlow.setColorAt(1,QColor(9,23,46,0));
        p.fillRect(rect(),rightGlow);
        for(int i=0;i<70;++i) {
            qreal x = qreal((i*137+31)%997)/997*width();
            qreal y = qreal((i*89+17)%503)/503*height();
            p.setPen(Qt::NoPen); p.setBrush(QColor(156,201,234,18+(i%4)*9));
            p.drawEllipse(QPointF(x,y),i%7==0?1.4:.65,i%7==0?1.4:.65);
        }
        p.save(); p.translate(c); p.rotate(-19);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(99,182,227,35),1));
        p.drawEllipse(QPointF(),r*1.65,r*.47);
        p.setPen(QPen(QColor(143,208,245,20),1));
        p.drawEllipse(QPointF(),r*1.86,r*.54);
        p.restore();
        // Deep shadow beneath the sphere, with a bright rim on the lit side.
        QRadialGradient glow(c,r*1.18);
        glow.setColorAt(0,QColor(47,191,228,0)); glow.setColorAt(.83,QColor(47,191,228,0));
        glow.setColorAt(.85,QColor(47,191,228,60)); glow.setColorAt(1,QColor(47,191,228,0));
        p.setPen(Qt::NoPen); p.setBrush(glow); p.drawEllipse(c,r*1.18,r*1.18);
        QRadialGradient sphere(c-QPointF(r*.38,r*.48),r*1.7);
        sphere.setColorAt(0,QColor("#326b89")); sphere.setColorAt(.32,QColor("#123e5a"));
        sphere.setColorAt(.65,QColor("#071c36")); sphere.setColorAt(1,QColor("#020815"));
        p.setBrush(sphere); p.setPen(QPen(QColor(144,225,242,105),1.3)); p.drawEllipse(c,r,r);
        auto project = [&](qreal lat,qreal lon, qreal &z) {
            const qreal x=qCos(lat)*qSin(lon+phase);
            const qreal yy=qSin(lat), zz=qCos(lat)*qCos(lon+phase);
            const qreal tilt=.25;
            z=zz*qCos(tilt)-yy*qSin(tilt);
            return QPointF(c.x()+r*x,c.y()-r*(yy*qCos(tilt)+zz*qSin(tilt)));
        };
        for(int grid=0;grid<26;++grid) {
            QPainterPath path; bool started=false;
            for(int j=0;j<=180;++j) {
                const bool latitude=grid<9;
                qreal lat=latitude ? (grid-4)*.29 : -M_PI_2+M_PI*j/180;
                qreal lon=latitude ? 2*M_PI*j/180 : (grid-9)*2*M_PI/17;
                qreal z; QPointF pt=project(lat,lon,z);
                if(z>0) { if(!started) path.moveTo(pt); else path.lineTo(pt); started=true; }
                else started=false;
            }
            p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(114,217,237,42),.8)); p.drawPath(path);
        }
        // Flowing atmospheric ribbons on the front surface.
        for(int band=0;band<7;++band) {
            QPainterPath path; bool started=false;
            for(int j=0;j<=220;++j) {
                qreal lon=2*M_PI*j/220;
                qreal lat=-.85+band*.27 + .10*qSin(lon*3+band*.7)+.035*qSin(lon*8);
                qreal z; QPointF pt=project(lat,lon,z);
                if(z>.05) { if(!started)path.moveTo(pt);else path.lineTo(pt);started=true;} else started=false;
            }
            p.setPen(QPen(QColor(130,221,233,30+band*3),3.5,Qt::SolidLine,Qt::RoundCap));p.drawPath(path);
            p.setPen(QPen(QColor(176,239,247,70),.7));p.drawPath(path);
        }
        p.save();p.translate(c);p.rotate(-19);
        QRectF orbit(-r*1.65,-r*.47,r*3.3,r*.94);
        p.setPen(QPen(QColor(121,218,250,150),1.4));p.setBrush(Qt::NoBrush);
        p.drawArc(orbit,180*16,180*16);
        qreal a=phase*.7;
        QPointF node(qCos(a)*r*1.65,qSin(a)*r*.47);
        QRadialGradient light(node,12);light.setColorAt(0,QColor(174,237,255,190));light.setColorAt(1,QColor(82,201,255,0));
        p.setPen(Qt::NoPen);p.setBrush(light);p.drawEllipse(node,12,12);
        p.setBrush(QColor("#d8f7ff"));p.drawEllipse(node,2.5,2.5);p.restore();
        // Weather instruments float around the globe, within its reserved layout area.
        p.save();p.setClipRect(zone.adjusted(0,4,0,-4));
        const qreal unit=qBound(27.0,r*.22,46.0);
        for(int i=0;i<3;++i){
            const qreal theta=-2.55+i*2.12+.055*qSin(phase*2+i);
            QPointF pos=c+QPointF(qCos(theta)*r*1.40,qSin(theta)*r*.81);
            pos.ry()+=qSin(phase*3+i*2)*7;
            p.setPen(QPen(QColor(101,211,242,48),1,Qt::DashLine));p.drawLine(c+(pos-c)*.78,pos);
            p.save();p.translate(pos);p.rotate(qSin(phase*2+i)*4);
            QRadialGradient halo(QPointF(),unit*1.5);halo.setColorAt(0,QColor(66,191,245,65));halo.setColorAt(1,Qt::transparent);
            p.setPen(Qt::NoPen);p.setBrush(halo);p.drawEllipse(QPointF(),unit*1.5,unit*1.5);
            QRectF panel(-unit,-unit,unit*2,unit*2);
            QLinearGradient glass(panel.topLeft(),panel.bottomRight());glass.setColorAt(0,QColor(74,134,169,245));glass.setColorAt(.42,QColor(22,63,94,245));glass.setColorAt(1,QColor(8,29,55,250));
            p.setBrush(glass);p.setPen(QPen(QColor(134,225,245,140),1));p.drawRoundedRect(panel,unit*.38,unit*.38);
            p.setPen(QPen(QColor(215,247,255,100),1));p.drawLine(QPointF(-unit*.62,-unit*.87),QPointF(unit*.50,-unit*.87));
            p.scale(unit/40,unit/40);p.setPen(QPen(QColor("#a3efff"),2.4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.setBrush(Qt::NoBrush);
            if(i==0){ // sensor mast and rotating anemometer
                p.drawLine(QPointF(0,-5),QPointF(0,23));p.drawLine(QPointF(-11,23),QPointF(11,23));
                p.save();p.translate(0,-9);p.rotate(phase*180);
                for(int arm=0;arm<3;++arm){p.rotate(120);p.drawLine(QPointF(),QPointF(19,0));p.setBrush(QColor("#56d7f5"));p.drawEllipse(QPointF(20,0),5,3);p.setBrush(Qt::NoBrush);}p.restore();
            }else if(i==1){ // volumetric cloud with drifting rainfall
                QLinearGradient cloud(0,-20,0,9);cloud.setColorAt(0,QColor("#eefcff"));cloud.setColorAt(1,QColor("#64abc9"));
                QPainterPath shape;shape.setFillRule(Qt::WindingFill);shape.addEllipse(QRectF(-24,-8,30,18));shape.addEllipse(QRectF(-12,-20,28,29));shape.addEllipse(QRectF(5,-9,23,20));
                p.setPen(Qt::NoPen);p.setBrush(cloud);p.drawPath(shape.simplified());
                for(int drop=0;drop<3;++drop){qreal y=13+std::fmod(phase*30+drop*5,12.0);p.setPen(QPen(QColor(113,225,255,200),2,Qt::SolidLine,Qt::RoundCap));p.drawLine(QPointF(-13+drop*13,y),QPointF(-16+drop*13,y+6));}
            }else{ // scanning weather radar
                p.setPen(QPen(QColor(111,220,241,100),1));p.drawEllipse(QPointF(),24,24);p.drawEllipse(QPointF(),13,13);p.drawLine(-24,0,24,0);p.drawLine(0,-24,0,24);
                p.save();p.rotate(phase*140);QConicalGradient scan(QPointF(),0);scan.setColorAt(0,QColor(91,243,208,170));scan.setColorAt(.18,Qt::transparent);scan.setColorAt(1,Qt::transparent);p.setBrush(scan);p.setPen(Qt::NoPen);p.drawEllipse(QPointF(),24,24);p.setPen(QPen(QColor("#8affdc"),2));p.drawLine(0,0,24,0);p.restore();
                p.setBrush(QColor("#baffeb"));p.setPen(Qt::NoPen);p.drawEllipse(QPointF(11,-12),2.5,2.5);
            }
            p.restore();
        }
        p.restore();
    }
};
