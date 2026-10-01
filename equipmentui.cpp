#include "equipmentui.h"
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtMath>

namespace {
const QColor navy("#091b40"), muted("#5d77ad"), blue("#0785ff");
void symbol(QPainter &p, const QString &name, QColor color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
    QPainterPath path;
    if (name == "equipment") {
        for (int y : {4, 14}) { p.drawRoundedRect(QRectF(3,y,18,6),1,1); p.drawPoint(6,y+3); p.drawLine(16,y+3,18,y+3); }
    } else if (name == "home") {
        path.moveTo(2,11); path.lineTo(12,2); path.lineTo(22,11); path.moveTo(5,9); path.lineTo(5,22); path.lineTo(19,22); path.lineTo(19,9); p.drawPath(path); p.drawRect(QRectF(10,15,4,7));
    } else if (name == "pin") {
        p.drawEllipse(QRectF(5,2,14,14)); p.drawEllipse(QRectF(9,6,6,6)); p.drawLine(6,13,12,23); p.drawLine(12,23,18,13);
    } else if (name == "people" || name == "user") {
        p.drawEllipse(QRectF(8,2,8,8)); path.moveTo(3,22); path.cubicTo(3,9,21,9,21,22); p.drawPath(path);
        if (name == "people") { p.drawArc(QRectF(17,4,6,7),-90*16,180*16); p.drawArc(QRectF(17,13,10,14),0,90*16); }
    } else if (name == "grid") {
        for(int x : {3,14}) for(int y : {3,14}) p.drawRoundedRect(QRectF(x,y,7,7),1,1);
    } else if (name == "tool") {
        path.moveTo(15,3); path.cubicTo(6,0,8,10,10,11); path.lineTo(2,19); path.lineTo(5,22); path.lineTo(14,13); path.cubicTo(23,15,25,6,21,4); path.lineTo(17,8); path.lineTo(14,6); path.closeSubpath(); p.drawPath(path);
    } else if (name == "search") {
        p.drawEllipse(QRectF(3,3,13,13)); p.drawLine(15,15,22,22);
    } else if (name == "calendar") {
        p.drawRoundedRect(QRectF(3,5,18,17),1,1); p.drawLine(3,10,21,10); p.drawLine(7,2,7,7); p.drawLine(17,2,17,7); p.drawLine(9,15,14,15); p.drawLine(14,15,14,19);
    } else if (name == "down") {
        p.drawLine(7,10,12,15); p.drawLine(12,15,17,10);
    } else if (name == "export") {
        p.drawLine(12,2,12,15); p.drawLine(7,10,12,15); p.drawLine(12,15,17,10); p.drawLine(3,16,3,21); p.drawLine(3,21,21,21); p.drawLine(21,21,21,16);
    } else if (name == "plus") { p.drawLine(3,12,21,12); p.drawLine(12,3,12,21);
    } else if (name == "wifi" || name == "offline") {
        p.drawArc(QRectF(0,4,24,20),35*16,110*16); p.drawArc(QRectF(5,9,14,12),35*16,110*16); p.drawEllipse(QPointF(12,20),1,1);
        if(name == "offline") p.drawLine(2,2,22,22);
    } else if (name == "gear") {
        p.drawEllipse(QRectF(5,5,14,14)); p.drawEllipse(QRectF(9,9,6,6));
        for(int i=0;i<8;++i) { qreal a=i*M_PI/4; p.drawLine(QPointF(12+8*qCos(a),12+8*qSin(a)),QPointF(12+11*qCos(a),12+11*qSin(a))); }
    } else if (name == "thermometer") {
        path.moveTo(9,16); path.lineTo(9,5); path.cubicTo(9,0,15,0,15,5); path.lineTo(15,16); path.cubicTo(22,24,2,24,9,16); p.drawPath(path); p.drawLine(12,7,12,19); p.drawLine(15,8,18,8); p.drawLine(15,12,18,12);
    } else if (name == "drop" || name == "rain") {
        path.moveTo(12,2); path.cubicTo(9,8,4,11,4,16); path.cubicTo(4,25,20,25,20,16); path.cubicTo(20,11,15,8,12,2); p.drawPath(path);
    } else if (name == "wind" || name == "anemometer") {
        p.drawLine(12,6,12,23); p.drawLine(12,12,4,14); p.drawLine(12,12,20,14);
        p.drawEllipse(QPointF(12,4),2.5,2.5); p.drawEllipse(QPointF(3,15),2.5,2.5); p.drawEllipse(QPointF(21,15),2.5,2.5);
    } else if (name == "sun") {
        p.drawEllipse(QRectF(7,7,10,10));
        for(int i=0;i<8;++i) { qreal a=i*M_PI/4; p.drawLine(QPointF(12+8*qCos(a),12+8*qSin(a)),QPointF(12+11*qCos(a),12+11*qSin(a))); }
    } else if (name == "gauge") {
        p.drawArc(QRectF(2,3,20,20),0,180*16); p.drawLine(2,13,5,21); p.drawLine(5,21,19,21); p.drawLine(19,21,22,13); p.drawLine(12,14,17,8);
        p.drawLine(6,8,7,10); p.drawLine(12,5,12,7);
    } else if (name == "camera") {
        p.drawRoundedRect(QRectF(2,6,20,15),2,2); p.drawEllipse(QRectF(8,9,8,8)); p.drawLine(7,6,8,3); p.drawLine(8,3,16,3); p.drawLine(16,3,17,6);
    } else if (name == "check") { p.drawLine(5,12,10,17); p.drawLine(10,17,20,6);
    } else if (name == "bulb") {
        p.drawEllipse(QRectF(6,2,12,14)); p.drawLine(9,16,9,20); p.drawLine(15,16,15,20); p.drawLine(9,20,15,20); p.drawLine(10,23,14,23);
    } else if (name == "external") {
        p.drawLine(13,3,21,3); p.drawLine(21,3,21,11); p.drawLine(21,3,11,13); path.moveTo(9,5); path.lineTo(3,5); path.lineTo(3,21); path.lineTo(19,21); path.lineTo(19,15); p.drawPath(path);
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
    auto *b=new QPushButton(title); b->setIcon(icon(glyph,primary?Qt::white:navy)); b->setIconSize(QSize(20,20));
    b->setProperty("primary",primary); b->setCursor(Qt::PointingHandCursor); b->setMinimumHeight(42); return b;
}
QFrame *card(const char *role="card")
{
    auto *f=new QFrame; f->setProperty("role",role); return f;
}
QLabel *pill(const QString &status)
{
    auto *l=text(status,"pill"); l->setProperty("status",status); l->setAlignment(Qt::AlignCenter); l->setFixedSize(status=="Maintenance"?110:84,28); return l;
}
QLineEdit *search(const QString &placeholder)
{
    auto *edit=new QLineEdit; edit->setPlaceholderText(placeholder); edit->setMinimumWidth(160); edit->setFixedHeight(40);
    edit->addAction(icon("search",muted),QLineEdit::LeadingPosition); return edit;
}
class Selector final : public QComboBox {
protected:
    void paintEvent(QPaintEvent *event) override {
        QComboBox::paintEvent(event);
        QPainter p(this); p.translate(width()-23, height()/2-8); p.scale(16.0/24,16.0/24); symbol(p,"down",muted);
    }
};
QComboBox *selector(const QString &title, bool small=false)
{
    auto *combo=new Selector; combo->addItem(title); combo->setProperty("small",small);
    combo->setFixedHeight(small?25:40); combo->setMinimumWidth(small?108:124); return combo;
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
    QPixmap photo{":/assets/backgrounds/login_background.png"};
public:
    Sidebar() {
        setObjectName("equipmentSidebar"); setFixedWidth(270);
        auto *column=new QVBoxLayout(this); column->setContentsMargins(10,22,10,24); column->setSpacing(4);
        auto *brand=new QLabel; brand->setObjectName("sidebarBrand"); brand->setFixedSize(250,105);
        brand->setPixmap(QPixmap(":/assets/logo/skysense_logo.png").scaled(244,90,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter); column->addWidget(brand); column->addSpacing(18);
        const QStringList names{"Dashboard","Stations","Equipment","Maintenance","Staff","Services"};
        const QStringList icons{"home","pin","equipment","tool","people","grid"};
        for(int i=0;i<names.size();++i) {
            auto *nav=button(names[i],icons[i]); nav->setProperty("nav",true); nav->setProperty("active",i==2);
            nav->setIcon(icon(icons[i],QColor("#e4f3ff"))); nav->setIconSize(QSize(25,25)); nav->setFixedHeight(50); column->addWidget(nav);
        }
        column->addStretch(); auto *status=new QWidget; auto *statusBox=new QVBoxLayout(status); statusBox->setContentsMargins(16,12,4,0); statusBox->setSpacing(5);
        statusBox->addWidget(text("System Status","sidebarStatus"));
        auto *line=new QHBoxLayout; auto *dot=text("●","greenDot"); dot->setFixedWidth(16); line->addWidget(dot); line->addWidget(text("All Systems Operational","sidebarStatus")); statusBox->addLayout(line);
        statusBox->addWidget(text("Last updated: Apr 24, 2025 10:42","sidebarMuted")); column->addWidget(status);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.fillRect(rect(),QColor("#071a33")); p.setRenderHint(QPainter::SmoothPixmapTransform);
        QRectF area(0,height()*.36,width(),height()*.64); QSizeF size=photo.size(); size.scale(area.size(),Qt::KeepAspectRatioByExpanding);
        p.save(); p.setClipRect(area); p.drawPixmap(QRectF((width()-size.width())*.15,area.top()+(area.height()-size.height())/2,size.width(),size.height()),photo,photo.rect()); p.restore();
        QLinearGradient g(0,area.top(),0,height()); g.setColorAt(0,QColor(7,26,51,255)); g.setColorAt(.35,QColor(7,26,51,80)); g.setColorAt(.72,QColor(7,26,51,10)); g.setColorAt(1,QColor(7,26,51,235)); p.fillRect(area,g);
    }
};
class MiniBars final : public QWidget {
    QColor color;
public: explicit MiniBars(QColor c):color(c) { setFixedSize(30,42); }
protected: void paintEvent(QPaintEvent *) override { QPainter p(this); p.setRenderHint(QPainter::Antialiasing); color.setAlpha(65); p.setBrush(color); p.setPen(Qt::NoPen); for(int i=0;i<4;++i) p.drawRoundedRect(QRectF(i*width()/4.0,32-i*9,qMin(4.0,width()/6.0),10+i*9),2,2); }
};
QWidget *metricCard(const QString &title,const QString &value,const QString &total,const QString &trend,const QString &glyph,const QColor &accent,const char *tone)
{
    auto *f=card("metric"); f->setProperty("tone",tone); f->setMinimumHeight(118);
    auto *row=new QHBoxLayout(f); row->setContentsMargins(14,16,12,14); row->setSpacing(12);
    row->addWidget(new Glyph(glyph,accent,48,true),0,Qt::AlignTop);
    auto *copy=new QVBoxLayout; copy->setSpacing(4); copy->addWidget(text(title,"metricTitle"));
    auto *numbers=new QHBoxLayout; numbers->setSpacing(6); auto *valueLabel=text(value,"number"); valueLabel->setObjectName("metricValue"); auto *totalLabel=text(total,"muted"); totalLabel->setObjectName("metricTotal"); numbers->addWidget(valueLabel); numbers->addWidget(totalLabel,1,Qt::AlignBottom); copy->addLayout(numbers);
    auto *change=text(QString::fromUtf8("↗  ")+trend,"trend"); change->setProperty("tone",tone); copy->addWidget(change); row->addLayout(copy,1); row->addWidget(new MiniBars(accent),0,Qt::AlignVCenter); return f;
}
const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
const QStringList typeIcons{"thermometer","anemometer","drop","sun","gauge","camera"};
const QStringList stations{"ALP-03\nAlpine Ridge","COA-01\nCoastal Bay","FOR-12\nPine Forest","VAL-07\nGreen Valley","ALP-03\nAlpine Ridge","DES-05\nDesert View"};
const QStringList units{QString::fromUtf8("—"),"m/s","mm",QString::fromUtf8("W/m²"),"hPa",QString::fromUtf8("—")};
const QStringList times{"10:38","10:36","10:20","10:41","10:15","10:37"};
const QStringList statuses{"Online","Online","Offline","Online","Maintenance","Online"};
struct EquipmentRecord {
    QString id;
    QString type;
    QString station;
    QString unit;
    QString updated;
    QString status;
    QString model;
    QString serialNumber;
};
QVector<EquipmentRecord> equipmentRecords{
    {"EQP-001",types[0],stations[0],units[0],"Apr 24, 2025 10:38",statuses[0],"Vaisala WXT536","W5536-78421"},
    {"EQP-002",types[1],stations[1],units[1],"Apr 24, 2025 10:36",statuses[1],"WindSonic 2","WS-20481"},
    {"EQP-003",types[2],stations[2],units[2],"Apr 24, 2025 10:20",statuses[2],"RG3-M","RG-12003"},
    {"EQP-004",types[3],stations[3],units[3],"Apr 24, 2025 10:41",statuses[3],"CMP11","CMP-09114"},
    {"EQP-005",types[4],stations[4],units[4],"Apr 24, 2025 10:15",statuses[4],"PTB330","PTB-55210"},
    {"EQP-006",types[5],stations[5],units[5],"Apr 24, 2025 10:37",statuses[5],"SkyCam S2","SC-38201"}
};
QString unitForType(const QString &type)
{
    const int index=types.indexOf(type);
    return index>=0?units[index]:QString();
}
bool showAddEquipmentDialog(QWidget *parent, EquipmentRecord *record)
{
    QDialog dialog(parent);
    dialog.setObjectName("addEquipmentDialog");
    dialog.setWindowTitle("Add Equipment — SkySense");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumSize(650,680);
    dialog.resize(720,760);
    dialog.setSizeGripEnabled(true);
    auto *layout=new QVBoxLayout(&dialog);
    layout->setContentsMargins(30,26,30,24);
    layout->setSpacing(18);

    const int nextId=equipmentRecords.size()+1;
    const QString generatedId=QString("EQP-%1").arg(nextId,3,10,QChar('0'));
    auto *header=new QHBoxLayout;
    header->setSpacing(16);
    header->addWidget(new Glyph("plus",blue,54,true),0,Qt::AlignTop);
    auto *heading=new QVBoxLayout;
    heading->setSpacing(5);
    heading->addWidget(text("Add Equipment","dialogTitle"));
    heading->addWidget(text("Register a device and assign it to a weather station.","dialogSubtitle"));
    header->addLayout(heading,1);
    auto *idBadge=new QFrame;
    idBadge->setProperty("role","idBadge");
    idBadge->setMinimumWidth(112);
    auto *idLayout=new QVBoxLayout(idBadge);
    idLayout->setContentsMargins(14,10,14,10);
    idLayout->setSpacing(3);
    idLayout->addWidget(text("AUTO ID","idCaption"));
    auto *idValue=text(generatedId,"idValue");
    idValue->setObjectName("generatedEquipmentId");
    idLayout->addWidget(idValue);
    header->addWidget(idBadge,0,Qt::AlignTop);
    layout->addLayout(header);

    auto *separator=new QFrame;
    separator->setProperty("role","dialogSeparator");
    separator->setFixedHeight(1);
    layout->addWidget(separator);

    auto *type=new Selector;
    type->setFixedHeight(46);
    type->setMinimumWidth(0);
    type->addItems(types);
    auto *station=new Selector;
    station->setFixedHeight(46);
    station->setMinimumWidth(0);
    for(const QString &entry:stations)
        station->addItem(entry.section('\n',0,0)+" — "+entry.section('\n',1,1),entry);
    auto *unit=new QLineEdit(unitForType(type->currentText()));
    unit->setReadOnly(true);
    auto *status=new Selector;
    status->setFixedHeight(46);
    status->setMinimumWidth(0);
    status->addItems({"Online","Offline","Maintenance"});
    auto *model=new QLineEdit;
    model->setPlaceholderText("e.g. Vaisala WXT536");
    auto *serial=new QLineEdit;
    serial->setPlaceholderText("Enter the unique serial number");
    QObject::connect(type,&QComboBox::currentTextChanged,&dialog,[unit](const QString &value){unit->setText(unitForType(value));});

    auto field=[](const QString &caption,QWidget *editor,const QString &hint=QString()) {
        auto *container=new QWidget;
        container->setObjectName("dialogField");
        auto *column=new QVBoxLayout(container);
        column->setContentsMargins(0,0,0,0);
        column->setSpacing(7);
        auto *label=text(caption,"fieldLabel");
        column->addWidget(label);
        editor->setMinimumHeight(46);
        column->addWidget(editor);
        if(!hint.isEmpty()) column->addWidget(text(hint,"fieldHint"));
        return container;
    };
    auto *details=card("dialogSection");
    auto *detailsLayout=new QVBoxLayout(details);
    detailsLayout->setContentsMargins(20,18,20,20);
    detailsLayout->setSpacing(15);
    detailsLayout->addWidget(text("Equipment details","dialogSectionTitle"));
    detailsLayout->addWidget(text("Choose the device type, station, and operating status.","dialogSubtitle"));
    auto *grid=new QGridLayout;
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(16);
    grid->addWidget(field("Equipment Type",type),0,0);
    grid->addWidget(field("Weather Station",station),0,1);
    grid->addWidget(field("Measurement Unit",unit),1,0);
    grid->addWidget(field("Current Status",status),1,1);
    grid->addWidget(field("Model",model),2,0,1,2);
    grid->addWidget(field("Serial Number",serial,"Serial numbers must be unique."),3,0,1,2);
    grid->setColumnStretch(0,1);
    grid->setColumnStretch(1,1);
    detailsLayout->addLayout(grid);
    layout->addWidget(details,1);

    auto *footer=new QHBoxLayout;
    footer->setSpacing(12);
    footer->addWidget(text("The new device will appear in the equipment list.","fieldHint"),1);
    auto *cancelButton=new QPushButton("Cancel");
    cancelButton->setObjectName("dialogCancelButton");
    cancelButton->setMinimumHeight(48);
    auto *saveButton=new QPushButton("Add Equipment");
    saveButton->setObjectName("dialogSaveButton");
    saveButton->setProperty("primary",true);
    saveButton->setIcon(icon("plus",Qt::white));
    saveButton->setIconSize(QSize(18,18));
    saveButton->setMinimumHeight(48);
    saveButton->setMinimumWidth(176);
    footer->addWidget(cancelButton);
    footer->addWidget(saveButton);
    layout->addLayout(footer);

    QObject::connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::reject);
    QObject::connect(saveButton,&QPushButton::clicked,&dialog,[&dialog,model,serial]{
        const QString serialValue=serial->text().trimmed();
        if(model->text().trimmed().isEmpty()||serialValue.isEmpty()) {
            QMessageBox::warning(&dialog,"Required information","Enter both the model and serial number.");
            return;
        }
        for(const auto &existing:equipmentRecords) {
            if(existing.serialNumber.compare(serialValue,Qt::CaseInsensitive)==0) {
                QMessageBox::warning(&dialog,"Serial number already exists","Each equipment item must have a unique serial number.");
                return;
            }
        }
        dialog.accept();
    });
    if(dialog.exec()!=QDialog::Accepted)
        return false;

    record->id=generatedId;
    record->type=type->currentText();
    record->station=station->currentData().toString();
    record->unit=unit->text();
    record->updated=QDateTime::currentDateTime().toString("MMM d, yyyy HH:mm");
    record->status=status->currentText();
    record->model=model->text().trimmed();
    record->serialNumber=serial->text().trimmed();
    return true;
}
class EquipmentDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &o,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); QRect r=o.rect; int row=index.row(), col=index.column();
        p->fillRect(r,row==0?QColor("#e5f4ff"):Qt::white); p->setPen(QColor("#e5eef9")); p->drawLine(r.bottomLeft(),r.bottomRight());
        QFont font("Segoe UI"); font.setPixelSize(13); p->setFont(font); p->setPen(navy);
        const EquipmentRecord data=equipmentRecords.value(row);
        if(col==0) {
            if(row==0) p->fillRect(QRect(r.left(),r.top(),3,r.height()),blue);
            QRectF box(r.center().x()-8,r.center().y()-8,16,16); p->setPen(QPen(row==0?blue:QColor("#8ca7d3"),1.3)); p->setBrush(row==0?blue:Qt::white); p->drawRoundedRect(box,3,3);
            if(row==0) { p->translate(box.topLeft()); p->scale(16.0/24,16.0/24); symbol(*p,"check",Qt::white); }
        } else if(col==1) { font.setBold(true); p->setFont(font); p->drawText(r.adjusted(10,0,-4,0),Qt::AlignVCenter,data.id);
        } else if(col==2) {
            const int typeIndex=qMax(0,types.indexOf(data.type));
            p->save(); p->translate(r.left()+8,r.center().y()-12); symbol(*p,typeIcons[typeIndex],typeIndex==3?QColor("#f5aa00"):blue); p->restore(); p->drawText(r.adjusted(44,0,-4,0),Qt::AlignVCenter,data.type);
        } else if(col==3) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(10,3,-4,-r.height()/2+2),Qt::AlignVCenter,data.station.section('\n',0,0));
            font.setBold(false); p->setFont(font); p->setPen(muted); p->drawText(r.adjusted(10,r.height()/2-1,-4,-3),Qt::AlignVCenter,data.station.section('\n',1,1));
        } else if(col==4) { p->setPen(muted); p->drawText(r.adjusted(10,0,0,0),Qt::AlignVCenter,data.unit);
        } else if(col==5) {
            const QColor dot=data.status=="Offline"?QColor("#ff304a"):data.status=="Maintenance"?QColor("#ffb000"):QColor("#00b66a");
            p->setPen(Qt::NoPen); p->setBrush(dot); p->drawEllipse(QPointF(r.left()+14,r.center().y()),4.5,4.5);
            p->setPen(muted); p->drawText(r.adjusted(30,0,0,0),Qt::AlignVCenter,data.updated);
        } else if(col==6) {
            const bool offline=data.status=="Offline", maintenance=data.status=="Maintenance";
            QRectF badge(r.center().x()-53,r.center().y()-14,106,28); p->setPen(Qt::NoPen); p->setBrush(offline?QColor("#fee2e2"):maintenance?QColor("#fef3c7"):QColor("#dcfce7")); p->drawRoundedRect(badge,14,14);
            p->setPen(offline?QColor("#ef4444"):maintenance?QColor("#d97706"):QColor("#16a34a")); p->drawText(badge,Qt::AlignCenter,data.status);
        } else { p->setPen(Qt::NoPen); p->setBrush(navy); for(int i=-1;i<=1;++i) p->drawEllipse(QPointF(r.center().x()+i*7,r.center().y()),1.5,1.5); }
        p->restore();
    }
};
class TableHeader final : public QHeaderView {
public: explicit TableHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) {}
protected:
    void paintSection(QPainter *p,const QRect &rect,int section) const override {
        p->save(); QHeaderView::paintSection(p,rect,section); p->restore(); p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(QColor("#8aa6d2"),1.2));
        if(section==0) { p->setBrush(Qt::white); p->drawRoundedRect(QRectF(rect.center().x()-8,rect.center().y()-8,16,16),3,3); }
        else if(section<7) { int x=rect.left()+fontMetrics().horizontalAdvance(model()->headerData(section,Qt::Horizontal).toString())+20; int y=rect.center().y(); p->drawLine(x,y-2,x+3,y-5); p->drawLine(x+3,y-5,x+6,y-2); p->drawLine(x,y+2,x+3,y+5); p->drawLine(x+3,y+5,x+6,y+2); }
        p->restore();
    }
};
class EquipmentTable final : public QTableWidget {
public:
    EquipmentTable():QTableWidget(equipmentRecords.size(),8) {
        setObjectName("equipmentTable"); setHorizontalHeader(new TableHeader(this)); setHorizontalHeaderLabels({"","ID","Type","Station","Unit","Latest Update","Status","Actions"});
        verticalHeader()->hide(); verticalHeader()->setDefaultSectionSize(38); horizontalHeader()->setFixedHeight(38); horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); setItemDelegate(new EquipmentDelegate(this)); setShowGrid(false); setSelectionMode(QAbstractItemView::NoSelection); setEditTriggers(QAbstractItemView::NoEditTriggers); setFocusPolicy(Qt::NoFocus); setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        for(int r=0;r<equipmentRecords.size();++r) populateRow(r);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); setMinimumWidth(0);
    }
    void addEquipment(const EquipmentRecord &record) {
        equipmentRecords.append(record);
        const int row=rowCount();
        setRowCount(row+1);
        populateRow(row);
        setVerticalScrollBarPolicy(rowCount()>6?Qt::ScrollBarAsNeeded:Qt::ScrollBarAlwaysOff);
        updateTableGeometry();
        updateGeometry();
        viewport()->update();
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QTableWidget::resizeEvent(event); updateTableGeometry();
    }
private:
    void updateTableGeometry() {
        const int widths[]{44,114,184,186,102,235,156,78};
        int total=0; for(int n:widths) total+=n; int extra=qMax(0,viewport()->width()-total);
        for(int c=0;c<8;++c) setColumnWidth(c,widths[c]+(c==2||c==3||c==5?extra/3:0));
        const int visibleRows=qMin(rowCount(),6);
        int h=38+visibleRows*38+4+(rowCount()>6||viewport()->width()<total?14:0); if(height()!=h) setFixedHeight(h);
    }
    void populateRow(int row) {
        const auto &data=equipmentRecords[row];
        const QString values[]{"",data.id,data.type,data.station,data.unit,data.updated,data.status,""};
        for(int column=0;column<8;++column) setItem(row,column,new QTableWidgetItem(values[column]));
    }
};
class TrendChart final : public QWidget {
public: TrendChart() { setMinimumSize(235,142); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); QFont f("Segoe UI"); f.setPixelSize(10); p.setFont(f);
        QRectF plot(32,20,width()-40,height()-45);
        for(int i=0;i<5;++i) { qreal y=plot.bottom()-plot.height()*i/4; p.setPen(QColor("#e8f0fa")); p.drawLine(QPointF(plot.left(),y),QPointF(plot.right(),y)); p.setPen(muted); p.drawText(QRectF(0,y-7,28,14),Qt::AlignRight,QString::number(i*10)+QString::fromUtf8("°C")); }
        const QStringList ticks{"12 AM","4 AM","8 AM","12 PM","4 PM","8 PM"};
        for(int i=0;i<6;++i) { qreal x=plot.left()+plot.width()*i/6; p.setPen(QColor("#e8f0fa")); p.drawLine(QPointF(x,plot.top()),QPointF(x,plot.bottom())); p.setPen(muted); p.drawText(QRectF(x-18,plot.bottom()+8,40,16),Qt::AlignCenter,ticks[i]); }
        const QVector<qreal> values{11,14,16,14,15,20,17,15,18,22,24,22,20,21,23,24.8,23,22,20,21,23,22,19,20,21,24};
        QPainterPath line; QVector<QPointF> points; for(int i=0;i<values.size();++i) { QPointF q(plot.left()+plot.width()*i/(values.size()-1),plot.bottom()-plot.height()*values[i]/40); points<<q; if(i==0) line.moveTo(q); else line.lineTo(q); }
        QPainterPath fill=line; fill.lineTo(plot.bottomRight()); fill.lineTo(plot.bottomLeft()); fill.closeSubpath(); QLinearGradient gradient(0,plot.top(),0,plot.bottom()); gradient.setColorAt(0,QColor(0,145,255,75)); gradient.setColorAt(1,QColor(0,145,255,6)); p.fillPath(fill,gradient); p.setPen(QPen(blue,1.7)); p.drawPath(line); p.setBrush(blue); for(const auto &q:points) p.drawEllipse(q,1.5,1.5);
        QPointF selected=points[15]; p.setPen(QPen(blue,1,Qt::DashLine)); p.drawLine(selected,QPointF(selected.x(),plot.bottom())); p.setPen(Qt::white); p.drawEllipse(selected,4.5,4.5);
        QRectF tip(qBound(plot.left(),selected.x()-38,plot.right()-80),qMax(0.0,selected.y()-51),80,39); p.setPen(Qt::NoPen); p.setBrush(QColor("#28384e")); p.drawRoundedRect(tip,4,4); p.setPen(Qt::white); p.drawText(tip.adjusted(7,4,-3,-18),Qt::AlignLeft,QString::fromUtf8("24.8°C")); f.setPixelSize(9); p.setFont(f); p.drawText(tip.adjusted(7,21,-3,-2),Qt::AlignLeft,"Apr 24, 14:00");
    }
};
QWidget *information()
{
    auto *f=card("detail"); auto *grid=new QGridLayout(f); grid->setContentsMargins(12,10,12,10); grid->setHorizontalSpacing(12); grid->setVerticalSpacing(4);
    QStringList labels{"Equipment ID","Type","Model","Serial Number","Station","Installation Date","Firmware Version"};
    QStringList values{"EQP-001","Weather Station","Vaisala WXT536","W5536-78421","ALP-03 (Alpine Ridge)","Jan 14, 2024","v3.2.1"};
    for(int i=0;i<labels.size();++i) { grid->addWidget(text(labels[i],"smallMuted"),i,0); grid->addWidget(text(values[i],"smallMuted"),i,1); } grid->setColumnStretch(1,1); return f;
}
QWidget *readings()
{
    auto *f=card("detail"); auto *v=new QVBoxLayout(f); v->setContentsMargins(12,8,12,8); v->setSpacing(4);
    auto *head=new QHBoxLayout; head->addWidget(text("Latest Readings","detailTitle"),1); head->addWidget(selector("Last 24 Hours",true)); v->addLayout(head);
    QStringList labels{"Temperature","Humidity","Wind Speed","Wind Direction","Pressure","Precipitation"};
    QStringList values{QString::fromUtf8("24.8 °C"),"68 %","12.4 m/s",QString::fromUtf8("215° (SW)"),"1012.3 hPa","0.0 mm"};
    QStringList glyphs{"thermometer","drop","wind","anemometer","gauge","rain"};
    for(int i=0;i<6;++i) { auto *row=new QHBoxLayout; row->setSpacing(8); row->addWidget(new Glyph(glyphs[i],blue,20)); row->addWidget(text(labels[i],"smallMuted"),1); auto *value=text(values[i],"readingValue"); value->setAlignment(Qt::AlignRight); row->addWidget(value); v->addLayout(row); } return f;
}
QWidget *trend()
{
    auto *f=card("detail"); auto *v=new QVBoxLayout(f); v->setContentsMargins(12,8,12,8); v->setSpacing(4);
    auto *head=new QHBoxLayout; head->addWidget(text("Temperature Trend","detailTitle"),1); head->addWidget(selector("Last 24 Hours",true)); v->addLayout(head); v->addWidget(new TrendChart,1); return f;
}
QWidget *recommendations()
{
    auto *f=card("detail"); auto *v=new QVBoxLayout(f); v->setContentsMargins(12,8,12,8); v->setSpacing(4); v->addWidget(text("Status & Recommendations","detailTitle"));
    QStringList titles{"Equipment Status","Calibration","Recommendations"}; QStringList lines{"Normal operation\nAll systems functioning properly.","Valid (82 days left)\nLast calibrated: Feb 1, 2025","No action required\nEquipment is operating normally."}; QStringList glyphs{"check","gear","bulb"}; QColor colors[]{QColor("#00b864"),QColor("#0d47a1"),QColor("#ffaf00")};
    for(int i=0;i<3;++i) { auto *row=new QHBoxLayout; row->setSpacing(10); row->addWidget(new Glyph(glyphs[i],colors[i],32,true),0,Qt::AlignTop); auto *copy=new QVBoxLayout; copy->setSpacing(1); copy->addWidget(text(titles[i],"detailTitle")); copy->addWidget(text(lines[i],"recommendationCopy")); row->addLayout(copy,1); v->addLayout(row); } return f;
}
class EquipmentPage final : public QWidget {
    QGridLayout *details=nullptr; QList<QWidget *> detailCards; QVBoxLayout *body=nullptr; QList<QWidget *> metrics; int detailColumns=0;
public:
    explicit EquipmentPage(QWidget *parent):QWidget(parent) {
        setObjectName("equipmentPage"); auto *outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
        auto *top=card("topbar"); auto *bar=new QHBoxLayout(top); bar->setContentsMargins(28,12,24,12); bar->setSpacing(20); auto *globalSearch=search("Search equipment, types, stations, or serial numbers..."); globalSearch->setMaximumWidth(500); bar->addWidget(globalSearch,1); bar->addStretch();
        bar->addWidget(new Glyph("calendar",navy,24)); bar->addWidget(text("Thu, Apr 24, 2025","topDate")); bar->addWidget(new Glyph("down",muted,18)); auto *divider=new QFrame; divider->setProperty("role","divider"); divider->setFixedSize(1,35); bar->addWidget(divider); bar->addWidget(new Glyph("user",muted,42,true));
        auto *user=new QVBoxLayout; user->setSpacing(1); user->addWidget(text("Alex Martin","detailTitle")); user->addWidget(text("Operations Manager","smallMuted")); bar->addLayout(user); bar->addWidget(new Glyph("down",navy,18)); outer->addWidget(top);
        auto *scroll=new QScrollArea; scroll->setObjectName("equipmentScroll"); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); outer->addWidget(scroll,1);
        auto *content=new QWidget; content->setObjectName("equipmentContent"); body=new QVBoxLayout(content); body->setContentsMargins(28,16,24,6); body->setSpacing(10); body->setSizeConstraint(QLayout::SetMinimumSize);
        auto *header=new QHBoxLayout; auto *copy=new QVBoxLayout; copy->setSpacing(0); copy->addWidget(text("Equipment","pageTitle")); copy->addWidget(text("Monitor weather equipment health, connectivity, calibration, and latest readings.","subtitle")); header->addLayout(copy,1); header->addSpacing(12); header->addWidget(button("Export PDF","export")); auto *addButton=button("Add Equipment","plus",true); addButton->setObjectName("addEquipmentButton"); header->addWidget(addButton); body->addLayout(header);
        auto *cards=new QHBoxLayout; cards->setSpacing(12);
        metrics={metricCard("Registered Equipment","210","/ 250 limit","+12 this month","equipment",blue,"blue"),metricCard("Online Equipment","187","/ 210 total","+8 this month","wifi",QColor("#00b863"),"green"),metricCard("Offline Equipment","18","/ 210 total","+3 this month","offline",QColor("#ff3859"),"red"),metricCard("Calibration Due","36","/ 210 total","+5 this month","gear",QColor("#7948ff"),"purple")};
        for(auto *m:metrics) cards->addWidget(m,1);
        body->addLayout(cards);
        auto *filters=new QHBoxLayout; filters->setSpacing(12); filters->addWidget(search("Search equipment by ID, type, station, or serial number..."),3); filters->addWidget(selector("All Types"),1); filters->addWidget(selector("All Stations"),1); filters->addWidget(selector("All Statuses"),1); auto *clear=button("Clear",""); clear->setProperty("link",true); filters->addWidget(clear); body->addLayout(filters);
        auto *equipmentTable=new EquipmentTable; body->addWidget(equipmentTable);
        QObject::connect(addButton,&QPushButton::clicked,this,[this,equipmentTable]{
            EquipmentRecord record;
            if(!showAddEquipmentDialog(this,&record)) return;
            equipmentTable->addEquipment(record);
            ++registeredCount;
            if(record.status=="Online") ++onlineCount;
            if(record.status=="Offline") ++offlineCount;
            refreshMetrics();
        });
        auto *health=card(); health->setObjectName("equipmentHealth"); auto *healthBox=new QVBoxLayout(health); healthBox->setContentsMargins(10,10,10,10); healthBox->setSpacing(10);
        auto *healthHead=new QHBoxLayout; healthHead->setSpacing(12); healthHead->addWidget(new Glyph("thermometer",blue,50,true)); auto *healthCopy=new QVBoxLayout; healthCopy->setSpacing(1); healthCopy->addWidget(text("Equipment Health","sectionTitle")); healthCopy->addWidget(text("Real-time status and performance details for EQP-001.","smallMuted")); healthHead->addLayout(healthCopy,1); healthHead->addWidget(pill("Online")); healthHead->addWidget(text("Last updated 2 min ago","smallMuted")); healthHead->addWidget(button("View Full Details","external")); healthBox->addLayout(healthHead);
        details=new QGridLayout; details->setSpacing(10); detailCards={information(),readings(),trend(),recommendations()}; healthBox->addLayout(details); body->addWidget(health); body->addStretch(); scroll->setWidget(content);
        adapt();
    }
protected:
    void resizeEvent(QResizeEvent *e) override { QWidget::resizeEvent(e); adapt(); }
private:
    int registeredCount=210, onlineCount=187, offlineCount=18;
    void refreshMetrics() {
        auto setValue=[this](int index,const QString &value,const QString &total){
            auto *valueLabel=metrics[index]->findChild<QLabel *>("metricValue");
            auto *totalLabel=metrics[index]->findChild<QLabel *>("metricTotal");
            if(valueLabel) valueLabel->setText(value);
            if(totalLabel) totalLabel->setText(total);
        };
        setValue(0,QString::number(registeredCount),"/ 250 limit");
        setValue(1,QString::number(onlineCount),QString("/ %1 total").arg(registeredCount));
        setValue(2,QString::number(offlineCount),QString("/ %1 total").arg(registeredCount));
        setValue(3,"36",QString("/ %1 total").arg(registeredCount));
    }
    void adapt() {
        if(!details) return;
        const int columns=width()<1120?2:4;
        if(columns!=detailColumns) { for(auto *w:detailCards) details->removeWidget(w); for(int i=0;i<4;++i) { details->setColumnStretch(i,0); details->setColumnMinimumWidth(i,0); } for(int i=0;i<4;++i) details->addWidget(detailCards[i],i/columns,i%columns); for(int i=0;i<columns;++i) details->setColumnStretch(i,1); detailColumns=columns; }
        bool narrow=width()<1000; body->setContentsMargins(narrow?16:28,16,narrow?16:24,6);
        for(auto *m:metrics) {
            auto *row=qobject_cast<QHBoxLayout *>(m->layout()); row->setContentsMargins(narrow?8:14,14,narrow?8:12,14); row->setSpacing(narrow?6:12);
            for(auto *w:m->findChildren<QWidget *>(QString(),Qt::FindDirectChildrenOnly)) {
                if(auto *glyph=dynamic_cast<Glyph *>(w)) glyph->setFixedSize(narrow?28:48,narrow?28:48);
                if(auto *bars=dynamic_cast<MiniBars *>(w)) bars->setFixedWidth(narrow?12:30);
            }
        }
    }
};
}
void setupEquipmentUi(QWidget *root)
{
    root->setObjectName("equipmentRoot"); auto *layout=new QHBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0); layout->addWidget(new Sidebar); layout->addWidget(new EquipmentPage(root),1);
    QFile stylesheet(":/styles/equipment.qss"); if(stylesheet.open(QIODevice::ReadOnly)) root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}




