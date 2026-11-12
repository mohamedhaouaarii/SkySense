#include "sidebar.h"
#include "apptheme.h"
#include "connection.h"
#include <QDateTime>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {
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
    } else if (name == "people") {
        p.drawEllipse(QRectF(8,2,8,8)); path.moveTo(3,22); path.cubicTo(3,9,21,9,21,22); p.drawPath(path);
        p.drawArc(QRectF(17,4,6,7),-90*16,180*16); p.drawArc(QRectF(17,13,10,14),0,90*16);
    } else if (name == "grid") {
        for(int x : {3,14}) for(int y : {3,14}) p.drawRoundedRect(QRectF(x,y,7,7),1,1);
    }
}

QIcon icon(const QString &name, QColor color)
{
    QPixmap px(48,48); px.fill(Qt::transparent); QPainter p(&px); p.scale(2,2); symbol(p,name,color); p.end(); px.setDevicePixelRatio(2); return QIcon(px);
}

QLabel *text(const QString &value, const char *role)
{
    auto *label=new QLabel(value); label->setProperty("role",role); label->setWordWrap(true); return label;
}

class Sidebar final : public QWidget {
    std::function<void(const QString &)> navigate;
    QStringList navigationNames;
    QList<QPushButton *> navigationButtons;
public:
    Sidebar(std::function<void(const QString &)> onNavigate, const QString &initialSection, const QStringList &sections)
        : navigate(std::move(onNavigate)), navigationNames(sections) {
        setObjectName("servicesSidebar"); setFixedWidth(270);
        auto *column=new QVBoxLayout(this); column->setContentsMargins(10,20,10,18); column->setSpacing(4);
        auto *brand=new QLabel; brand->setObjectName("sidebarBrand"); brand->setFixedSize(250,98);
        brand->setPixmap(QPixmap(":/assets/logo/skysense_logo.png").scaled(244,88,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter);
        auto *brandGlow=new QGraphicsDropShadowEffect(brand);
        brandGlow->setBlurRadius(20); brandGlow->setOffset(0,2); brandGlow->setColor(QColor(25,150,255,105));
        brand->setGraphicsEffect(brandGlow);
        column->addWidget(brand); column->addSpacing(15);
        static const QHash<QString,QString> icons{{"Dashboard","home"},{"Stations","pin"},{"Equipment","equipment"},
                                                  {"Maintenance","tool"},{"Staff","people"},{"Services","grid"}};
        for(int i=0;i<navigationNames.size();++i) {
            auto *nav=new QPushButton(navigationNames[i]); nav->setCursor(Qt::PointingHandCursor);
            nav->setProperty("nav",true); nav->setProperty("active",navigationNames[i]==initialSection);
            nav->setIcon(icon(icons.value(navigationNames[i],"grid"),QColor("#e4f3ff"))); nav->setIconSize(QSize(25,25)); nav->setFixedHeight(50);
            column->addWidget(nav); navigationButtons.append(nav);
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
        auto *themeRow = new QHBoxLayout; themeRow->setContentsMargins(16, 12, 16, 10);
        auto *workspaceTheme = new ThemeSwitch(this); workspaceTheme->setObjectName("workspaceThemeSwitch");
        themeRow->addWidget(workspaceTheme); themeRow->addStretch(); column->addLayout(themeRow);
        auto *statusBox=new QVBoxLayout; statusBox->setContentsMargins(16,12,4,0); statusBox->setSpacing(5);
        statusBox->addWidget(text("System Status","sidebarStatus"));
        const bool oracleConnected = Connection::createInstance().isConnected();
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
} // namespace

QWidget *createSkySenseSidebar(std::function<void(const QString &)> navigate, const QString &initialSection,
                               const QStringList &sections)
{
    auto *sidebar = new Sidebar(std::move(navigate), initialSection, sections);
    QFile stylesheet(":/styles/services.qss");
    if(stylesheet.open(QIODevice::ReadOnly)) sidebar->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    return sidebar;
}
