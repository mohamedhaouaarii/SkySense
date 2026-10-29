#include "apptheme.h"
#include "equipmentui.h"
#include "equipmentdata.h"
#include "connection.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <cmath>
#include <QSignalBlocker>
#include <QScreen>
#include <QCloseEvent>
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDateEdit>
#include <QEnterEvent>
#include <QCalendarWidget>
#include <QSpinBox>
#include <QLocale>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QImageReader>
#include <QStandardPaths>
#include <QBuffer>
#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLinearGradient>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTextCharFormat>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtMath>
#include <algorithm>
#include <functional>

namespace {
const QColor navy("#091b40"), muted("#5d77ad"), blue("#0785ff");
qreal equipmentStatusPulse=0.0;
void symbol(QPainter &p, const QString &name, QColor color)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(AppTheme::pen(p, QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin))); p.setBrush(AppTheme::brush(p, Qt::NoBrush));
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
    } else if (name == "chart") {
        p.drawLine(3,21,21,21); p.drawLine(4,21,4,4);
        p.drawRoundedRect(QRectF(7,13,3,6),1,1); p.drawRoundedRect(QRectF(12,9,3,10),1,1); p.drawRoundedRect(QRectF(17,5,3,14),1,1);
    } else if (name == "edit") {
        path.moveTo(4,17); path.lineTo(15,6); path.lineTo(19,10); path.lineTo(8,21); path.lineTo(3,22); path.closeSubpath();
        p.drawPath(path); p.drawLine(13,8,17,12); p.drawLine(4,17,8,21); p.drawLine(3,22,8,21);
    } else if (name == "delete") {
        p.drawLine(4,7,20,7); p.drawLine(9,4,15,4); p.drawLine(6,8,7,21); p.drawLine(18,8,17,21);
        p.drawLine(7,21,17,21); p.drawLine(10,10,10,18); p.drawLine(14,10,14,18);
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
    } else if (name == "replace") {
        p.drawLine(4,8,17,8); p.drawLine(14,4,18,8); p.drawLine(18,8,14,12);
        p.drawLine(20,16,7,16); p.drawLine(10,12,6,16); p.drawLine(6,16,10,20);
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
class PremiumButton final : public QPushButton {
    QGraphicsDropShadowEffect *shadow = nullptr;
    QPropertyAnimation *blurAnimation = nullptr;
    QPropertyAnimation *offsetAnimation = nullptr;

    void animateShadow(qreal blur, const QPointF &offset)
    {
        blurAnimation->stop();
        blurAnimation->setStartValue(shadow->blurRadius());
        blurAnimation->setEndValue(blur);
        blurAnimation->start();
        offsetAnimation->stop();
        offsetAnimation->setStartValue(shadow->offset());
        offsetAnimation->setEndValue(offset);
        offsetAnimation->start();
    }

public:
    explicit PremiumButton(const QString &title, QWidget *parent = nullptr)
        : QPushButton(title, parent)
    {
        shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(12);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(9, 27, 64, 48));
        setGraphicsEffect(shadow);
        blurAnimation = new QPropertyAnimation(shadow, "blurRadius", this);
        blurAnimation->setDuration(150);
        blurAnimation->setEasingCurve(QEasingCurve::OutCubic);
        offsetAnimation = new QPropertyAnimation(shadow, "offset", this);
        offsetAnimation->setDuration(150);
        offsetAnimation->setEasingCurve(QEasingCurve::OutCubic);
    }

protected:
    void enterEvent(QEnterEvent *event) override
    {
        QPushButton::enterEvent(event);
        animateShadow(20, QPointF(0, 6));
    }
    void leaveEvent(QEvent *event) override
    {
        QPushButton::leaveEvent(event);
        animateShadow(12, QPointF(0, 3));
    }
    void mousePressEvent(QMouseEvent *event) override
    {
        QPushButton::mousePressEvent(event);
        animateShadow(7, QPointF(0, 1));
    }
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        QPushButton::mouseReleaseEvent(event);
        const bool hovered = rect().contains(event->pos());
        animateShadow(hovered ? 20 : 12, QPointF(0, hovered ? 6 : 3));
    }
};

QPushButton *button(const QString &title, const QString &glyph, bool primary=false)
{
    auto *b=new PremiumButton(title); b->setIcon(icon(glyph,primary?Qt::white:navy)); b->setIconSize(QSize(20,20));
    b->setProperty("primary",primary); b->setCursor(Qt::PointingHandCursor); b->setMinimumHeight(42); return b;
}
QFrame *card(const char *role="card")
{
    auto *f=new QFrame; f->setProperty("role",role);
    auto *shadow = new QGraphicsDropShadowEffect(f);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(18, 48, 88, 24));
    f->setGraphicsEffect(shadow);
    return f;
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
    QString name; QColor color; bool bubble; bool animated=false;
    QTimer pulseTimer; qreal pulse=.5; int pulseTick=0; qreal phase=0;
public:
    Glyph(QString n,QColor c=blue,int size=28,bool circle=false,bool animate=false)
        :name(n),color(c),bubble(circle),animated(animate) {
        setFixedSize(size,size);
        if(animated) {
            static int sequence=0;
            phase=(sequence++%4)*1.57079632679;
            pulseTimer.setInterval(34);
            QObject::connect(&pulseTimer,&QTimer::timeout,this,[this]{
                pulse=.5+.5*qSin(pulseTick*.14+phase);
                ++pulseTick;
                update();
            });
        }
    }
protected:
    void showEvent(QShowEvent *event) override { QWidget::showEvent(event); if(animated)pulseTimer.start(); }
    void hideEvent(QHideEvent *event) override { pulseTimer.stop(); QWidget::hideEvent(event); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(bubble) {
            QLinearGradient surface(0, 0, width(), height());
            surface.setColorAt(0, color.lighter(165));
            surface.setColorAt(0.55, color.lighter(125));
            surface.setColorAt(1, color.darker(108));
            p.setPen(AppTheme::pen(p, QPen(QColor(255,255,255,205), 1)));
            p.setBrush(AppTheme::brush(p, surface));
            p.drawEllipse(rect().adjusted(1,1,-1,-1));
            p.setPen(AppTheme::pen(p, QPen(QColor(255,255,255,95), 1)));
            p.drawArc(rect().adjusted(4,3,-4,-4), 25*16, 130*16);
        }
        qreal margin=bubble?width()*.25:2;
        p.translate(margin,margin);
        p.scale((width()-2*margin)/24.0,(height()-2*margin)/24.0);
        QColor ink=color;
        if(animated) {
            ink=AppTheme::dark()?color.lighter(100+qRound(28*pulse)):color.darker(118-qRound(18*pulse));
            p.setOpacity(.42+.58*pulse);
        }
        symbol(p,name,ink);
    }
};
class MiniBars final : public QWidget {
    QColor color;
    qreal currentProgress=0.0;
public:
    explicit MiniBars(QColor c):color(c) { setFixedSize(58,9); setToolTip("Share of total equipment"); }
    qreal progress() const { return currentProgress; }
    void setProgress(qreal value) { currentProgress=qBound(0.0,value,1.0); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF track=QRectF(rect()).adjusted(0.5,1.5,-0.5,-1.5);
        p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, QColor(color.red(),color.green(),color.blue(),28))); p.drawRoundedRect(track,3.5,3.5);
        if(currentProgress<=0.0) return;
        QRectF fill=track; fill.setWidth(qMax(7.0,track.width()*currentProgress));
        QLinearGradient gradient(fill.topLeft(),fill.topRight()); gradient.setColorAt(0,color.lighter(125)); gradient.setColorAt(1,color);
        p.setBrush(AppTheme::brush(p, gradient)); p.drawRoundedRect(fill,3.5,3.5);
    }
};
QWidget *metricCard(const QString &title,const QString &value,const QString &total,const QString &trend,const QString &glyph,const QColor &accent,const char *tone)
{
    auto *f=card("metric"); f->setProperty("tone",tone); f->setMinimumHeight(118);
    auto *row=new QHBoxLayout(f); row->setContentsMargins(14,16,12,14); row->setSpacing(12);
    row->addWidget(new Glyph(glyph,accent,48,true,true),0,Qt::AlignTop);
    auto *copy=new QVBoxLayout; copy->setSpacing(4); copy->addWidget(text(title,"metricTitle"));
    auto *numbers=new QHBoxLayout; numbers->setSpacing(6); auto *valueLabel=text(value,"number"); valueLabel->setObjectName("metricValue"); auto *totalLabel=text(total,"muted"); totalLabel->setObjectName("metricTotal"); numbers->addWidget(valueLabel); numbers->addWidget(totalLabel,1,Qt::AlignBottom); copy->addLayout(numbers);
    auto *change=text(trend,"trend"); change->setProperty("tone",tone); copy->addWidget(change); row->addLayout(copy,1);
    auto *insight=new QVBoxLayout; insight->setSpacing(5); insight->setAlignment(Qt::AlignCenter);
    auto *share=text("—%","share"); share->setObjectName("metricShare"); share->setAlignment(Qt::AlignRight|Qt::AlignVCenter); insight->addWidget(share);
    auto *meter=new MiniBars(accent); meter->setObjectName("metricMeter"); insight->addWidget(meter); row->addLayout(insight,0); return f;
}
const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
const QStringList typeIcons{"thermometer","anemometer","drop","sun","gauge","camera"};
const QStringList units{QString::fromUtf8("—"),"m/s","mm",QString::fromUtf8("W/m²"),"hPa",QString::fromUtf8("—")};
EquipmentStore inventory;
const QVector<EquipmentRecord> &equipmentRecords=inventory.records();

QString unitForType(const QString &type)
{
    const int index=types.indexOf(type);
    return index>=0?units[index]:QString();
}
QPixmap equipmentPhoto(const QString &encoded,const QString &type,const QSize &size)
{
    QPixmap photo;
    if(!encoded.isEmpty()) photo.loadFromData(QByteArray::fromBase64(encoded.toLatin1()),"PNG");
    if(!photo.isNull()) return photo.scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    QPixmap placeholder(size); placeholder.fill(QColor("#edf6ff"));
    QPainter painter(&placeholder); painter.setRenderHint(QPainter::Antialiasing);
    const int side=qMin(size.width(),size.height())/2;
    painter.translate((size.width()-side)/2,(size.height()-side)/2);
    painter.scale(side/24.0,side/24.0);
    symbol(painter,typeIcons.value(types.indexOf(type),"equipment"),blue);
    return placeholder;
}

bool readEquipmentPhoto(const QString &path,QString *encoded,QString *error)
{
    if(QFileInfo(path).size()>20*1024*1024) { *error="Choose an image smaller than 20 MB."; return false; }
    QImageReader reader(path); reader.setAutoTransform(true);
    const QSize size=reader.size();
    if(!size.isValid() || qint64(size.width())*size.height()>60000000) { *error="Choose a valid image up to 60 megapixels."; return false; }
    reader.setScaledSize(size.scaled(QSize(1000,1000),Qt::KeepAspectRatio));
    QImage photo=reader.read();
    if(photo.isNull()) { *error="This image could not be opened. Choose a PNG, JPG or WebP image."; return false; }
    photo=photo.scaled(QSize(1000,1000),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    QByteArray bytes; QBuffer buffer(&bytes); buffer.open(QIODevice::WriteOnly);
    if(!photo.save(&buffer,"PNG")) { *error="The image could not be prepared."; return false; }
    *encoded=QString::fromLatin1(bytes.toBase64());
    return true;
}

class EquipmentDialog final : public QDialog {
public:
    using QDialog::QDialog;
    bool dirty=false;
    void reject() override {
        if(dirty) {
            QMessageBox box(this); box.setWindowTitle("Unsaved equipment details");
            box.setText("Discard the changes in this form?");
            auto *discard=box.addButton("Discard changes",QMessageBox::DestructiveRole);
            auto *keep=box.addButton("Continue editing",QMessageBox::RejectRole);
            box.setDefaultButton(keep); box.setEscapeButton(keep); box.exec();
            if(box.clickedButton()!=discard) return;
        }
        QDialog::reject();
    }
};
bool showAddEquipmentDialog(QWidget *parent, EquipmentRecord *record, bool editing=false,
                            std::function<bool(const EquipmentRecord &)> save={})
{
    EquipmentDialog dialog(parent);
    dialog.setObjectName("addEquipmentDialog");
    const QString originalId=editing?record->id:QString();
    dialog.setWindowTitle(editing?"Edit Equipment — SkySense":"Add Equipment — SkySense");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumSize(620,560);
    dialog.resize(700,qMin(720,QApplication::primaryScreen()->availableGeometry().height()-70));
    dialog.setSizeGripEnabled(true);
    auto *layout=new QVBoxLayout(&dialog);
    layout->setContentsMargins(24,18,24,18);
    layout->setSpacing(12);

    const QString generatedId=inventory.nextId();
    auto *header=new QHBoxLayout;
    header->setSpacing(16);
    header->addWidget(new Glyph("plus",blue,54,true),0,Qt::AlignTop);
    auto *heading=new QVBoxLayout;
    heading->setSpacing(5);
    heading->addWidget(text(editing?"Edit Equipment":"Add Equipment","dialogTitle"));
    heading->addWidget(text(editing?"Update the device details and station assignment.":"Register a device and assign it to a weather station.","dialogSubtitle"));
    header->addLayout(heading,1);
    auto *idBadge=new QFrame;
    idBadge->setProperty("role","idBadge");
    idBadge->setMinimumWidth(112);
    auto *idLayout=new QVBoxLayout(idBadge);
    idLayout->setContentsMargins(14,10,14,10);
    idLayout->setSpacing(3);
    idLayout->addWidget(text(editing?"EQUIPMENT ID":"AUTO ID","idCaption"));
    auto *idValue=text(editing?originalId:generatedId,"idValue");
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
    type->setObjectName("equipmentType"); type->addItems(types);
    auto *station=new Selector;
    station->setFixedHeight(46);
    station->setMinimumWidth(0);
    station->insertItem(0,"Reserve / unassigned",QString());
    QString stationLoadError;
    const QStringList availableStations = inventory.stationNames(&stationLoadError);
    for (const QString &name : availableStations) station->addItem(name,name);
    station->setEditable(false);
    station->setCurrentIndex(station->count() > 1 ? 1 : 0);
    auto *unit=new QLineEdit(unitForType(type->currentText()));
    unit->setReadOnly(true);
    auto *location=new QLineEdit;
    location->setObjectName("equipmentLocation");
    location->setMaxLength(150);
    location->setPlaceholderText("e.g. North tower, rooftop");
    auto *status=new Selector;
    status->setFixedHeight(46);
    status->setMinimumWidth(0);
    status->setObjectName("equipmentStatus"); status->addItems({"Online","Offline","Maintenance"});
    auto *model=new QLineEdit; model->setObjectName("equipmentModel"); model->setMaxLength(100);
    model->setPlaceholderText("e.g. Vaisala WXT536");
    auto *serial=new QLineEdit; serial->setObjectName("equipmentSerial"); serial->setMaxLength(80);
    serial->setPlaceholderText("Enter the unique serial number");
    QObject::connect(type,&QComboBox::currentTextChanged,&dialog,[unit](const QString &value){unit->setText(unitForType(value));});
    if(editing) {
        type->setCurrentText(record->type);
        bool stationSet=false;
        for(int i=0;i<station->count();++i) if(station->itemData(i).toString().compare(record->station,Qt::CaseInsensitive)==0) {station->setCurrentIndex(i);stationSet=true;break;}
        if(!stationSet) station->setCurrentText(record->station);
        status->setCurrentText(record->status);
        model->setText(record->model);
        serial->setText(record->serialNumber);
        location->setText(record->location);
    }

    for(auto *editor:{model,serial,location}) QObject::connect(editor,&QLineEdit::textEdited,&dialog,[&dialog]{dialog.dirty=true;});
    for(QComboBox *editor:{type,station,status}) QObject::connect(editor,&QComboBox::currentIndexChanged,&dialog,[&dialog]{dialog.dirty=true;});
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
    QString photoData=editing?record->photoBase64:QString();
    auto *photoRow=new QHBoxLayout; photoRow->setSpacing(18);
    auto *photoPreview=new QLabel; photoPreview->setObjectName("equipmentPhotoPreview");
    photoPreview->setFixedSize(156,124); photoPreview->setAlignment(Qt::AlignCenter);
    photoPreview->setStyleSheet("background:#edf6ff;border:1px solid #d4e6f8;border-radius:10px;padding:6px;");
    photoRow->addWidget(photoPreview);
    auto *photoActions=new QVBoxLayout; photoActions->setSpacing(8);
    photoActions->addWidget(text("Equipment photo","dialogSectionTitle"));
    auto *photoHint=text("","fieldHint"); photoHint->setWordWrap(true); photoActions->addWidget(photoHint);
    auto *photoButtons=new QHBoxLayout;
    auto *choosePhoto=button("Choose photo","camera"); choosePhoto->setObjectName("chooseEquipmentPhoto");
    auto *removePhoto=button("Remove",""); removePhoto->setObjectName("removeEquipmentPhoto");
    photoButtons->addWidget(choosePhoto); photoButtons->addWidget(removePhoto); photoButtons->addStretch();
    photoActions->addLayout(photoButtons); photoRow->addLayout(photoActions,1); detailsLayout->addLayout(photoRow);
    auto refreshPhoto=[&photoData,photoPreview,photoHint,removePhoto,type]{
        photoPreview->setPixmap(equipmentPhoto(photoData,type->currentText(),QSize(142,110)));
        removePhoto->setEnabled(!photoData.isEmpty());
        photoHint->setText(photoData.isEmpty()?"Type illustration · Add your equipment photo.\nPNG, JPG or WebP, up to 20 MB.":"Photo saved with this equipment.\nThe original file stays unchanged.");
    };
    refreshPhoto();
    QObject::connect(type,&QComboBox::currentTextChanged,&dialog,[refreshPhoto]{refreshPhoto();});
    QObject::connect(choosePhoto,&QPushButton::clicked,&dialog,[&dialog,&photoData,refreshPhoto]{
        const QString path=QFileDialog::getOpenFileName(&dialog,"Choose equipment photo",QString(),"Images (*.png *.jpg *.jpeg *.webp)");
        if(path.isEmpty()) return;
        QString encoded,error;
        if(!readEquipmentPhoto(path,&encoded,&error)) { QMessageBox::warning(&dialog,"Photo unavailable",error); return; }
        photoData=encoded; dialog.dirty=true; refreshPhoto();
    });
    QObject::connect(removePhoto,&QPushButton::clicked,&dialog,[&dialog,&photoData,refreshPhoto]{photoData.clear();dialog.dirty=true;refreshPhoto();});
    detailsLayout->addWidget(text("Equipment details","dialogSectionTitle"));
    detailsLayout->addWidget(text("Enter the equipment identity, station assignment, physical location, and current status.","dialogSubtitle"));
    auto *grid=new QGridLayout;
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(16);
    grid->addWidget(field("Equipment Type",type),0,0);
    grid->addWidget(field("Station",station,"Select Reserve / unassigned when the equipment has no station."),0,1);
    grid->addWidget(field("Measurement Unit",unit),1,0);
    grid->addWidget(field("Current Status",status),1,1);
    grid->addWidget(field("Model",model),2,0,1,2);
    grid->addWidget(field("Serial Number",serial,"Serial numbers must be unique."),3,0,1,2);
    grid->addWidget(field("Physical Location",location,"This is the equipment emplacement inside or near the station."),4,0,1,2);
    grid->setColumnStretch(0,1);
    grid->setColumnStretch(1,1);
    detailsLayout->addLayout(grid);
    auto *formScroll=new QScrollArea; formScroll->setWidgetResizable(true); formScroll->setFrameShape(QFrame::NoFrame); formScroll->setWidget(details); layout->addWidget(formScroll,1);

    auto *footer=new QHBoxLayout;
    footer->setSpacing(12);
    footer->addStretch(1);
    auto *cancelButton=new QPushButton("Cancel");
    cancelButton->setObjectName("dialogCancelButton");
    cancelButton->setMinimumHeight(48);
    auto *saveButton=new QPushButton(editing?"Save Changes":"Add Equipment");
    saveButton->setObjectName("dialogSaveButton");
    saveButton->setProperty("primary",true);
    saveButton->setIcon(icon("plus",Qt::white));
    saveButton->setIconSize(QSize(18,18));
    saveButton->setMinimumHeight(48);
    saveButton->setMinimumWidth(176);
    footer->addWidget(cancelButton);
    footer->addWidget(saveButton);
    layout->addLayout(footer);

    auto *validation=text("","fieldHint"); validation->setObjectName("validationMessage"); validation->setWordWrap(true); validation->setStyleSheet("color:#c03945;"); layout->insertWidget(layout->count()-1,validation);
    QObject::connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::reject);
    for(auto *editor:{model,serial,location}) QObject::connect(editor,&QLineEdit::textChanged,validation,&QLabel::clear);
    QObject::connect(saveButton,&QPushButton::clicked,&dialog,[&dialog,&photoData,model,serial,location,originalId,validation,type,station,unit,status,record,editing,generatedId,save]{
        const QString serialValue=serial->text().trimmed();
        if(model->text().trimmed().isEmpty()||serialValue.isEmpty()) {
            validation->setText("Enter both the model and serial number.");
            (model->text().trimmed().isEmpty()?model:serial)->setFocus();
            return;
        }
        for(const auto &existing:equipmentRecords) {
            if(existing.id!=originalId && existing.serialNumber.compare(serialValue,Qt::CaseInsensitive)==0) {
                validation->setText("This serial number already exists. Enter a unique serial number.");
                serial->setFocus(); serial->selectAll();
                return;
            }
        }
        EquipmentRecord candidate;
        candidate.id=editing?originalId:generatedId;
        candidate.type=type->currentText();
        candidate.station=station->currentIndex()==0?QString():station->currentData().toString();
        if(station->currentIndex()<0) candidate.station=station->currentText().trimmed();
        candidate.unit=unit->text(); candidate.updated=QDateTime::currentDateTime().toString(Qt::ISODate);
        candidate.status=status->currentText(); candidate.model=model->text().trimmed();
        candidate.serialNumber=serialValue;
        candidate.photoBase64=photoData;
        candidate.location=location->text().trimmed();
        if(save && !save(candidate)) return;
        *record=candidate;
        dialog.accept();
    });
    return dialog.exec()==QDialog::Accepted;
}

class EquipmentDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &o,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); QRect r=o.rect; int col=index.column();
        p->fillRect(r,AppTheme::brush(*p, o.state.testFlag(QStyle::State_Selected)?QColor("#e5f4ff"):Qt::white)); p->setPen(AppTheme::pen(*p, QColor("#e5eef9"))); p->drawLine(r.bottomLeft(),r.bottomRight());
        QFont font("Segoe UI"); font.setPixelSize(13); p->setFont(font); p->setPen(AppTheme::pen(*p, navy));
        EquipmentRecord data;
        const QString id=index.siblingAtColumn(1).data().toString();
        for(const auto &record:equipmentRecords) if(record.id==id) { data=record; break; }
        const bool selected=o.state.testFlag(QStyle::State_Selected);
        if(col==0) {
            if(selected) p->fillRect(QRect(r.left(),r.top(),3,r.height()),AppTheme::brush(*p, blue));
            QRectF box(r.center().x()-8,r.center().y()-8,16,16); p->setPen(AppTheme::pen(*p, QPen(selected?blue:QColor("#8ca7d3"),1.3))); p->setBrush(AppTheme::brush(*p, selected?blue:Qt::white)); p->drawRoundedRect(box,3,3);
            if(selected) { p->translate(box.topLeft()); p->scale(16.0/24,16.0/24); symbol(*p,"check",Qt::white); }
        } else if(col==1) { font.setBold(true); p->setFont(font); p->drawText(r.adjusted(10,0,-4,0),Qt::AlignVCenter,data.id);
        } else if(col==2) {
            const int typeIndex=qMax(0,types.indexOf(data.type));
            p->save(); p->translate(r.left()+8,r.center().y()-12); symbol(*p,typeIcons[typeIndex],typeIndex==3?QColor("#f5aa00"):blue); p->restore(); p->drawText(r.adjusted(44,0,-4,0),Qt::AlignVCenter,data.type);
        } else if(col==3) {
            font.setBold(true); p->setFont(font); p->drawText(r.adjusted(10,3,-4,-r.height()/2+2),Qt::AlignVCenter,data.station.section('\n',0,0));
            font.setBold(false); p->setFont(font); p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(10,r.height()/2-1,-4,-3),Qt::AlignVCenter,data.station.section('\n',1,1));
        } else if(col==4) { p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(10,0,0,0),Qt::AlignVCenter,data.unit);
        } else if(col==5) {
            const QColor dot=data.status=="Offline"?QColor("#ff304a"):data.status=="Maintenance"?QColor("#ffb000"):QColor("#00b66a");
            QColor halo=dot; halo.setAlpha(qRound(9+21*equipmentStatusPulse)); p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, halo)); p->drawEllipse(QPointF(r.left()+14,r.center().y()),8,8);
            p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, dot)); p->drawEllipse(QPointF(r.left()+14,r.center().y()),4.5,4.5);
            p->setPen(AppTheme::pen(*p, muted)); p->drawText(r.adjusted(30,0,0,0),Qt::AlignVCenter,QDateTime::fromString(data.updated,Qt::ISODate).toString("dd MMM yyyy HH:mm"));
        } else if(col==6) {
            const bool offline=data.status=="Offline", maintenance=data.status=="Maintenance";
            QRectF badge(r.center().x()-53,r.center().y()-14,106,28);
            QColor halo=offline?QColor("#ef4444"):maintenance?QColor("#d97706"):QColor("#16a34a");
            halo.setAlpha(qRound(10+19*equipmentStatusPulse));
            p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, halo)); p->drawRoundedRect(badge.adjusted(-2,-2,2,2),16,16);
            if(AppTheme::dark()) {
                const QColor ink=offline?QColor("#ffb7c1"):maintenance?QColor("#ffe09a"):QColor("#91f5c4");
                const QColor surface=offline?QColor("#452332"):maintenance?QColor("#40351e"):QColor("#143e35");
                QColor edge=ink;edge.setAlpha(110);
                p->setPen(QPen(edge,1));p->setBrush(surface);p->drawRoundedRect(badge,14,14);
                font.setWeight(QFont::DemiBold);p->setFont(font);p->setPen(ink);
            } else {
                p->setBrush(offline?QColor("#fee2e2"):maintenance?QColor("#fef3c7"):QColor("#dcfce7"));p->drawRoundedRect(badge,14,14);
                p->setPen(offline?QColor("#b91c1c"):maintenance?QColor("#92400e"):QColor("#15803d"));
            }
            p->drawText(badge,Qt::AlignCenter,data.status);
        }
        p->restore();
    }
};
enum class PdfExportScope { Cancelled, All, Selected };

PdfExportScope choosePdfExportScope(QWidget *parent) {
    QDialog dialog(parent); dialog.setObjectName("equipmentExportScopeDialog"); dialog.setWindowTitle("Export equipment report"); dialog.setMinimumSize(510,270); dialog.setAttribute(Qt::WA_StyledBackground,true);
    dialog.setStyleSheet("QDialog#equipmentExportScopeDialog { background:#ffffff; color:#091b40; } QLabel#exportScopeTitle { color:#091b40; font-size:22px; font-weight:700; } QLabel#exportScopeHint { color:#5d77ad; font-size:13px; } QPushButton { min-height:38px; }");
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(30,28,30,26); layout->setSpacing(12);
    auto *title=new QLabel("What would you like to export?"); title->setObjectName("exportScopeTitle"); layout->addWidget(title);
    auto *hint=new QLabel("Export the complete equipment inventory, or choose only the equipment records needed for this report."); hint->setObjectName("exportScopeHint"); hint->setWordWrap(true); layout->addWidget(hint);
    layout->addSpacing(6); auto *buttons=new QHBoxLayout; buttons->setSpacing(10);
    auto *all=button("Export all equipment","export",true); all->setObjectName("exportAllEquipmentButton");
    auto *selected=button("Choose equipment","",false); selected->setObjectName("chooseEquipmentForExportButton");
    auto *cancel=button("Cancel","",false); cancel->setObjectName("cancelEquipmentExportScopeButton");
    buttons->addWidget(all,1); buttons->addWidget(selected,1); buttons->addWidget(cancel); layout->addLayout(buttons);
    PdfExportScope result=PdfExportScope::Cancelled;
    QObject::connect(all,&QPushButton::clicked,&dialog,[&]{result=PdfExportScope::All;dialog.accept();});
    QObject::connect(selected,&QPushButton::clicked,&dialog,[&]{result=PdfExportScope::Selected;dialog.accept();});
    QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    return dialog.exec()==QDialog::Accepted?result:PdfExportScope::Cancelled;
}

QVector<EquipmentRecord> chooseEquipmentForPdf(QWidget *parent,const QVector<EquipmentRecord> &records) {
    QDialog dialog(parent); dialog.setObjectName("exportEquipmentSelectionDialog"); dialog.setWindowTitle("Choose equipment for PDF export"); dialog.setMinimumSize(580,510); dialog.setAttribute(Qt::WA_StyledBackground,true);
    dialog.setStyleSheet("QDialog#exportEquipmentSelectionDialog { background:#ffffff; color:#091b40; } QListWidget { background:#ffffff; color:#17366f; border:1px solid #d7e5f7; border-radius:8px; padding:4px; } QListWidget::item { min-height:29px; padding:6px; } QListWidget::item:hover { background:#f1f7ff; } QListWidget::item:alternate { background:#f8fbff; }");
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(28,24,28,24); layout->setSpacing(12);
    auto *title=text("Choose equipment for the PDF","sectionTitle"); title->setObjectName("exportSelectionTitle"); layout->addWidget(title);
    auto *hint=text("Select the equipment records to include. The report will contain only the selected equipment and its related monitoring profile.","smallMuted"); hint->setWordWrap(true); layout->addWidget(hint);
    auto *searchField=search("Search by ID, type, station, model or serial number..."); searchField->setObjectName("exportEquipmentSearch"); searchField->setClearButtonEnabled(true); layout->addWidget(searchField);
    auto *list=new QListWidget; list->setObjectName("exportEquipmentSelectionList"); list->setAlternatingRowColors(true); list->setSelectionMode(QAbstractItemView::NoSelection); layout->addWidget(list,1);
    for(const auto &record:records) {
        const QString label=QString("%1  |  %2  |  %3  |  %4").arg(record.id,record.type,record.station.section('\n',0,0),record.model);
        auto *item=new QListWidgetItem(label,list); item->setData(Qt::UserRole,record.id); item->setData(Qt::UserRole+1,(record.id+" "+record.type+" "+record.station+" "+record.model+" "+record.serialNumber).toLower());
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable); item->setCheckState(Qt::Unchecked); item->setToolTip(record.serialNumber+" - "+record.status);
    }
    auto *selectionInfo=text("0 equipment selected","smallMuted"); selectionInfo->setObjectName("exportSelectionCount"); layout->addWidget(selectionInfo);
    auto *actions=new QHBoxLayout; auto *selectVisible=button("Select visible",""); selectVisible->setObjectName("selectVisibleEquipmentButton"); auto *clearSelection=button("Clear selection",""); clearSelection->setObjectName("clearExportSelectionButton"); auto *cancel=button("Cancel",""); cancel->setObjectName("cancelEquipmentExportButton"); auto *exportSelected=button("Export selected", "export",true); exportSelected->setObjectName("confirmSelectedEquipmentExportButton"); exportSelected->setEnabled(false);
    actions->addWidget(selectVisible); actions->addWidget(clearSelection); actions->addStretch(); actions->addWidget(cancel); actions->addWidget(exportSelected); layout->addLayout(actions);
    auto updateSelection=[&]{ int count=0; for(int i=0;i<list->count();++i) if(list->item(i)->checkState()==Qt::Checked) ++count; selectionInfo->setText(QString("%1 equipment selected").arg(count)); exportSelected->setEnabled(count>0); exportSelected->setText(count?QString("Export %1 selected").arg(count):"Export selected"); };
    QObject::connect(list,&QListWidget::itemChanged,&dialog,[&](QListWidgetItem *){updateSelection();});
    QObject::connect(searchField,&QLineEdit::textChanged,&dialog,[&](const QString &value){ const QString query=value.trimmed().toLower(); for(int i=0;i<list->count();++i) list->item(i)->setHidden(!query.isEmpty() && !list->item(i)->data(Qt::UserRole+1).toString().contains(query)); });
    QObject::connect(selectVisible,&QPushButton::clicked,&dialog,[&]{ for(int i=0;i<list->count();++i) if(!list->item(i)->isHidden()) list->item(i)->setCheckState(Qt::Checked); });
    QObject::connect(clearSelection,&QPushButton::clicked,&dialog,[&]{ for(int i=0;i<list->count();++i) list->item(i)->setCheckState(Qt::Unchecked); });
    QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject); QObject::connect(exportSelected,&QPushButton::clicked,&dialog,&QDialog::accept);
    if(dialog.exec()!=QDialog::Accepted) return {};
    QSet<QString> ids; for(int i=0;i<list->count();++i) if(list->item(i)->checkState()==Qt::Checked) ids.insert(list->item(i)->data(Qt::UserRole).toString());
    QVector<EquipmentRecord> selected; for(const auto &record:records) if(ids.contains(record.id)) selected.append(record); return selected;
}

class EquipmentTable final : public QTableWidget {
public:
    QString query, typeFilter, stationFilter, statusFilter;
    int sortColumn=1;
    bool ascending=true;
    std::function<void(const QString &)> changed;
    std::function<void()> selectionChanged;
    EquipmentTable():QTableWidget(0,8) {
        setObjectName("equipmentTable");
        setHorizontalHeaderLabels({"","ID","Type","Station","Unit","Last Update","Status","Actions"});
        verticalHeader()->hide(); verticalHeader()->setDefaultSectionSize(44);
        horizontalHeader()->setFixedHeight(38);
        horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        horizontalHeader()->setSectionsClickable(true);
        horizontalHeader()->setSortIndicatorShown(true);
        horizontalHeader()->setSortIndicator(1,Qt::AscendingOrder);
        setItemDelegate(new EquipmentDelegate(this)); setShowGrid(false);
        setSelectionMode(QAbstractItemView::SingleSelection); setSelectionBehavior(QAbstractItemView::SelectRows);
        setEditTriggers(QAbstractItemView::NoEditTriggers); setFocusPolicy(Qt::StrongFocus);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); setMinimumWidth(0);
        auto *statusAnimation=new QVariantAnimation(this); statusAnimation->setStartValue(0.0); statusAnimation->setKeyValueAt(.5,1.0);
        statusAnimation->setEndValue(0.0); statusAnimation->setDuration(2400); statusAnimation->setLoopCount(-1);
        statusAnimation->setEasingCurve(QEasingCurve::InOutSine);
        QObject::connect(statusAnimation,&QVariantAnimation::valueChanged,this,[this](const QVariant &value){equipmentStatusPulse=value.toReal(); viewport()->update();});
        statusAnimation->start();
        QObject::connect(horizontalHeader(),&QHeaderView::sectionClicked,this,[this](int column){
            if(column<1 || column>6) return;
            ascending=column==sortColumn?!ascending:true; sortColumn=column;
            horizontalHeader()->setSortIndicator(column,ascending?Qt::AscendingOrder:Qt::DescendingOrder);
            refresh();
        });
        QObject::connect(this,&QTableWidget::itemSelectionChanged,this,[this]{if(selectionChanged) selectionChanged();});
        QObject::connect(this,&QTableWidget::cellDoubleClicked,this,[this](int,int){editEquipment(selectedId());});
        refresh();
    }
    QString selectedId() const {
        return currentRow()>=0 && item(currentRow(),1) && !selectedItems().isEmpty() ? item(currentRow(),1)->text() : QString();
    }
    void refresh(const QString &preferredId=QString()) {
        const QString keep=preferredId.isEmpty()?selectedId():preferredId;
        const auto visible=filterEquipment(equipmentRecords,query,typeFilter,stationFilter,statusFilter,sortColumn,ascending);
        {
            QSignalBlocker block(this);
            setRowCount(0); setRowCount(visible.size());
            int selected=-1;
            for(int row=0;row<visible.size();++row) {
                const auto &r=visible[row];
                const QStringList fields{"",r.id,r.type,r.station.trimmed().isEmpty()?QStringLiteral("Reserve / unassigned"):r.station,r.unit,r.updated,r.status,""};
                for(int c=0;c<8;++c) {
                    auto *cell=new QTableWidgetItem(fields[c]);
                    cell->setToolTip(r.id+" | "+r.model+" | "+r.serialNumber);
                    setItem(row,c,cell);
                }
                auto *actions=new QWidget;
                auto *layout=new QHBoxLayout(actions); layout->setContentsMargins(3,3,3,3); layout->setSpacing(4);
                for(const QString &name:{QString("edit"),QString("delete")}) {
                    auto *action=new QToolButton;
                    action->setObjectName(name=="edit"?"editEquipmentButton":"deleteEquipmentButton");
                    action->setIcon(icon(name,name=="edit"?blue:QColor("#e5484d")));
                    action->setFixedSize(30,30); action->setToolTip(name+" "+r.id);
                    action->setAccessibleName(name+" "+r.id);
                    const QString id=r.id;
                    QObject::connect(action,&QToolButton::clicked,this,[this,id,name]{
                        if(name=="edit") editEquipment(id); else deleteEquipment(id);
                    });
                    layout->addWidget(action);
                }
                layout->addStretch(); setCellWidget(row,7,actions);
                if(r.id==keep) selected=row;
            }
            if(selected>=0) selectRow(selected);
            else { clearSelection(); setCurrentCell(-1,-1); }
        }
        updateTableGeometry();
        if(selectionChanged) selectionChanged();
    }
    bool commit(const QVector<EquipmentRecord> &candidate,const QString &message,const QString &select=QString()) {
        QString error;
        if(!inventory.save(candidate,&error)) { QMessageBox::critical(this,"Changes were not saved",error); return false; }
        refresh(select);
        if(changed) changed(message);
        return true;
    }
    void addEquipment(const EquipmentRecord &r) {
        auto candidate=equipmentRecords; candidate.append(r);
        commit(candidate,r.id+" added and saved.",r.id);
    }
    void editEquipment(const QString &id) {
        const int index=findRecord(id); if(index<0) return;
        EquipmentRecord record=equipmentRecords[index];
        showAddEquipmentDialog(this,&record,true,[this,index,id](const EquipmentRecord &updated){
            auto candidate=equipmentRecords; candidate[index]=updated;
            return commit(candidate,id+" updated and saved.",id);
        });
    }
    void deleteEquipment(const QString &id) {
        const int index=findRecord(id); if(index<0) return;
        const auto record=equipmentRecords[index];
        QMessageBox confirmation(this);
        confirmation.setObjectName("deleteEquipmentConfirmation");
        confirmation.setWindowTitle("Confirm equipment deletion");
        confirmation.setIcon(QMessageBox::Warning); confirmation.setTextFormat(Qt::PlainText);
        confirmation.setIconPixmap(equipmentPhoto(record.photoBase64,record.type,QSize(160,140)));
        confirmation.setText("Delete equipment "+record.id+"?");
        confirmation.setInformativeText(QString("Model: %1\nSerial number: %2\nStation: %3\nStatus: %4\n\nThis removes the equipment from the saved inventory. This action cannot be undone.")
            .arg(record.model,record.serialNumber,record.station.simplified(),record.status));
        auto *remove=confirmation.addButton("Delete equipment",QMessageBox::DestructiveRole);
        remove->setObjectName("confirmDeleteEquipmentButton");
        auto *cancel=confirmation.addButton("Keep equipment",QMessageBox::RejectRole);
        cancel->setObjectName("cancelDeleteEquipmentButton");
        confirmation.setStyleSheet(
            "QMessageBox{background:#ffffff;} QMessageBox QLabel{color:#173052;background:transparent;padding:3px;}"
            "QPushButton{min-height:36px;border-radius:8px;padding:0 18px;font-weight:600;}"
            "QPushButton#confirmDeleteEquipmentButton{color:white;background:#e5484d;border:1px solid #d9363e;}"
            "QPushButton#confirmDeleteEquipmentButton:hover{background:#d9363e;}"
            "QPushButton#cancelDeleteEquipmentButton{color:#334b70;background:#f6f9fe;border:1px solid #d0ddef;}"
            "QPushButton#cancelDeleteEquipmentButton:hover{background:#edf4fc;border-color:#adc4e1;}");
        confirmation.setDefaultButton(cancel); confirmation.setEscapeButton(cancel);
        confirmation.exec();
        if(confirmation.clickedButton()!=remove) return;
        auto candidate=equipmentRecords; candidate.removeAt(index);
        commit(candidate,id+" deleted. Inventory saved.");
    }
protected:
    void resizeEvent(QResizeEvent *e) override { QTableWidget::resizeEvent(e); updateTableGeometry(); }
private:
    int findRecord(const QString &id) const {
        for(int i=0;i<equipmentRecords.size();++i) if(equipmentRecords[i].id==id) return i;
        return -1;
    }
    void updateTableGeometry() {
        const int widths[]{36,100,170,160,74,196,126,84};
        const bool narrow=viewport()->width()<900;
        setColumnHidden(4,narrow); setColumnHidden(5,narrow);
        int total=0; for(int c=0;c<8;++c) if(!isColumnHidden(c)) total+=widths[c];
        const int extra=qMax(0,viewport()->width()-total);
        for(int c=0;c<8;++c) setColumnWidth(c,widths[c]+(c==2||c==3?extra/2:0));
        setFixedHeight(40+qMax(2,qMin(rowCount(),8))*44+18);
    }
};

class EquipmentBarChart final : public QWidget {
    Q_PROPERTY(qreal reveal READ reveal WRITE setReveal)
    QString title;
    QVector<QPair<QString,int>> values;
    QColor accent;
    qreal animationProgress=0.0;
public:
    EquipmentBarChart(QString chartTitle, QVector<QPair<QString,int>> chartValues, QColor color, QWidget *parent=nullptr)
        : QWidget(parent), title(std::move(chartTitle)), values(std::move(chartValues)), accent(color) {
        setMinimumHeight(245); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        setAccessibleName(title);
        QStringList description;
        for (const auto &item : values) description << QString("%1: %2").arg(item.first).arg(item.second);
        setAccessibleDescription(description.join("; "));
        auto *animation=new QVariantAnimation(this); animation->setStartValue(0.0); animation->setEndValue(1.0);
        animation->setDuration(650); animation->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){setReveal(v.toReal());});
        QTimer::singleShot(90,this,[animation]{animation->start();});
    }
    qreal reveal() const { return animationProgress; }
    void setReveal(qreal value) { animationProgress=qBound(0.0,value,1.0); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(),AppTheme::brush(p,  Qt::transparent));
        QFont titleFont("Segoe UI",10,QFont::DemiBold); p.setFont(titleFont); p.setPen(AppTheme::pen(p, navy));
        p.drawText(QRectF(16,13,width()-32,24),Qt::AlignLeft|Qt::AlignVCenter,title);
        if (values.isEmpty()) {
            p.setPen(AppTheme::pen(p, muted)); p.drawText(rect().adjusted(12,46,-12,-12), Qt::AlignCenter, "No equipment data to display");
            return;
        }
        const int maximum=qMax(1, std::max_element(values.cbegin(),values.cend(),[](const auto &a,const auto &b){ return a.second<b.second; })->second);
        QFont small("Segoe UI"); small.setPixelSize(10); p.setFont(small);
        const qreal rowHeight=qMin(31.0,(height()-56.0)/qMax(1,values.size()));
        const qreal labelWidth=qMin(150.0,qMax(92.0,width()*.34));
        const qreal valueWidth=34.0, gap=10.0;
        const qreal barLeft=labelWidth+gap, barWidth=qMax(20.0,width()-barLeft-valueWidth-22.0);
        for(int i=0;i<values.size();++i) {
            const qreal y=47+i*rowHeight;
            QString label=values[i].first;
            QFontMetrics fm(small); while(label.size()>4 && fm.horizontalAdvance(label)>labelWidth-8) label.chop(1);
            if(label!=values[i].first) label+=QString::fromUtf8("…");
            p.setPen(AppTheme::pen(p, navy)); p.drawText(QRectF(16,y,labelWidth,rowHeight),Qt::AlignLeft|Qt::AlignVCenter,label);
            QRectF track(barLeft,y+rowHeight*.31,barWidth, rowHeight*.38);
            p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, QColor("#edf3fb"))); p.drawRoundedRect(track,5,5);
            QRectF r(track.left(),track.top(),track.width()*values[i].second/maximum*animationProgress,track.height());
            QLinearGradient fill(r.topLeft(),r.topRight()); fill.setColorAt(0,accent.lighter(112)); fill.setColorAt(1,accent);
            p.setBrush(AppTheme::brush(p, fill)); p.drawRoundedRect(r,5,5);
            p.setPen(AppTheme::pen(p, navy)); p.drawText(QRectF(track.right()+8,y,valueWidth,rowHeight),Qt::AlignRight|Qt::AlignVCenter,QString::number(values[i].second));
        }
    }
};

class EquipmentStatusRing final : public QWidget {
    Q_PROPERTY(qreal reveal READ reveal WRITE setReveal)
    int online=0,offline=0,maintenance=0,total=0;
    qreal animationProgress=0.0;
public:
    EquipmentStatusRing(int onlineCount,int offlineCount,int maintenanceCount,QWidget *parent=nullptr)
        : QWidget(parent),online(onlineCount),offline(offlineCount),maintenance(maintenanceCount),
          total(onlineCount+offlineCount+maintenanceCount) {
        setMinimumSize(208,190); setMaximumWidth(250); setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
        setAccessibleName("Equipment status distribution");
        setAccessibleDescription(QString("%1 total: %2 online, %3 offline, %4 in maintenance")
                                 .arg(total).arg(online).arg(offline).arg(maintenance));
        auto *animation=new QVariantAnimation(this); animation->setStartValue(0.0); animation->setEndValue(1.0);
        animation->setDuration(850); animation->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){setReveal(v.toReal());});
        QTimer::singleShot(60,this,[animation]{animation->start();});
    }
    qreal reveal() const { return animationProgress; }
    void setReveal(qreal value) { animationProgress=qBound(0.0,value,1.0); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const qreal side=qMin(width()-20,height()-18), x=(width()-side)/2, y=(height()-side)/2;
        const QRectF ring(x,y,side,side); const qreal penWidth=qMax(13.0,side*.12);
        p.setPen(AppTheme::pen(p, QPen(QColor("#eaf0f8"),penWidth,Qt::SolidLine,Qt::RoundCap)));
        p.drawArc(ring,90*16,-360*16);
        if(total>0) {
            const int counts[]{online,offline,maintenance};
            const QColor colors[]{QColor("#18b878"),QColor("#ef5263"),QColor("#ed9b2f")};
            int start=90*16; const int full=qRound(360.0*16*animationProgress);
            int consumed=0;
            for(int i=0;i<3;++i) {
                const int span=(i==2?full-consumed:qRound(full*qreal(counts[i])/total)); consumed+=span;
                if(span<=0) continue;
                p.setPen(AppTheme::pen(p, QPen(colors[i],penWidth,Qt::SolidLine,Qt::RoundCap))); p.drawArc(ring,start,-span);
                start-=span;
            }
        }
        QFont numberFont("Segoe UI",22,QFont::Bold); p.setFont(numberFont); p.setPen(AppTheme::pen(p, QColor("#10284f")));
        p.drawText(QRectF(ring.left(),ring.center().y()-22,ring.width(),32),Qt::AlignCenter,QString::number(total));
        QFont caption("Segoe UI",9,QFont::DemiBold); p.setFont(caption); p.setPen(AppTheme::pen(p, QColor("#7183a0")));
        p.drawText(QRectF(ring.left(),ring.center().y()+9,ring.width(),20),Qt::AlignCenter,"TOTAL DEVICES");
    }
};

void showEquipmentStatistics(QWidget *parent, const QVector<EquipmentRecord> &records)
{
    QMap<QString,int> byStatus, byType, byStation;
    int attention=0,assigned=0,stale=0;
    for(const auto &record:records) {
        ++byStatus[record.status]; ++byType[record.type];
        const QString station=record.station.section('\n',0,0).trimmed();
        ++byStation[station.isEmpty()?QStringLiteral("Reserve / unassigned"):station];
        if(!station.isEmpty()) ++assigned;
        const EquipmentAssessment assessment=assessEquipment(record);
        if(assessment.needsReview) ++attention;
        if(assessment.daysSinceUpdate>=EquipmentUpdateReviewDays || assessment.daysSinceUpdate<0) ++stale;
    }
    const auto pairs=[](const QMap<QString,int> &map) { QVector<QPair<QString,int>> result; for(auto it=map.cbegin();it!=map.cend();++it) result.append({it.key(),it.value()}); return result; };
    QVector<QPair<QString,int>> stationValues=pairs(byStation);
    std::sort(stationValues.begin(),stationValues.end(),[](const auto &a,const auto &b){return a.second==b.second?a.first<b.first:a.second>b.second;});
    if(stationValues.size()>6) {
        int other=0; for(int i=5;i<stationValues.size();++i) other+=stationValues[i].second;
        stationValues.resize(5); stationValues.append({QStringLiteral("Other stations"),other});
    }
    QDialog dialog(parent); dialog.setObjectName("equipmentStatisticsDialog"); dialog.setWindowTitle("Equipment insights | SkySense"); dialog.setMinimumSize(820,650); dialog.resize(980,720);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(26,22,26,22); layout->setSpacing(12);
    auto *heading=new QHBoxLayout; heading->addWidget(new Glyph("chart",blue,42,true));
    auto *headingCopy=new QVBoxLayout; headingCopy->setSpacing(2);
    headingCopy->addWidget(text("Equipment Insights","dialogTitle"));
    auto *intro=text("A live operational snapshot from the equipment records currently loaded.","subtitle"); intro->setWordWrap(true); headingCopy->addWidget(intro);
    heading->addLayout(headingCopy,1); layout->addLayout(heading);

    auto *hero=card("detail"); hero->setObjectName("equipmentInsightsHero");
    auto *heroLayout=new QHBoxLayout(hero); heroLayout->setContentsMargins(18,14,18,14); heroLayout->setSpacing(22);
    heroLayout->addWidget(new EquipmentStatusRing(byStatus.value("Online"),byStatus.value("Offline"),byStatus.value("Maintenance")),0,Qt::AlignVCenter);
    auto *legend=new QVBoxLayout; legend->setSpacing(10);
    legend->addWidget(text("Status distribution","detailTitle"));
    const QStringList statusNames{"Online","Offline","Maintenance"};
    const QColor statusColors[]{QColor("#18b878"),QColor("#ef5263"),QColor("#ed9b2f")};
    for(int i=0;i<statusNames.size();++i) {
        auto *row=new QHBoxLayout; row->setSpacing(8);
        auto *dot=new QLabel; dot->setFixedSize(10,10); dot->setStyleSheet(QString("background:%1;border-radius:5px;").arg(statusColors[i].name()));
        row->addWidget(dot); row->addWidget(text(statusNames[i],"body"),1);
        const int count=byStatus.value(statusNames[i]);
        auto *value=text(QString("%1  ·  %2%").arg(count).arg(records.isEmpty()?0:qRound(100.0*count/records.size())),"detailTitle");
        value->setAlignment(Qt::AlignRight|Qt::AlignVCenter); row->addWidget(value); legend->addLayout(row);
    }
    legend->addStretch();
    auto *statusSummary=text(records.isEmpty()?"Add equipment to start monitoring operational status."
        :QString("%1 of %2 devices need a review based on status or update freshness.").arg(attention).arg(records.size()),"smallMuted");
    statusSummary->setWordWrap(true); legend->addWidget(statusSummary); heroLayout->addLayout(legend,1);
    auto *divider=new QFrame; divider->setFrameShape(QFrame::VLine); divider->setStyleSheet("color:#e5edf7;"); heroLayout->addWidget(divider);
    auto *insights=new QVBoxLayout; insights->setSpacing(7); insights->addWidget(text("Action overview","detailTitle"));
    const QString freshnessLine=QString("%1 need an update review").arg(stale);
    const QString assignmentLine=QString("%1 assigned  ·  %2 reserve").arg(assigned).arg(qMax(0,records.size()-assigned));
    for(const auto &item:QVector<QPair<QString,QString>>{{"Review queue",QString("%1 device(s)").arg(attention)},
            {"Update freshness",freshnessLine},{"Station coverage",assignmentLine}}) {
        auto *tile=new QFrame; tile->setProperty("role","insightTile"); auto *tileLayout=new QVBoxLayout(tile); tileLayout->setContentsMargins(10,6,10,6); tileLayout->setSpacing(1);
        tileLayout->addWidget(text(item.first,"smallMuted")); tileLayout->addWidget(text(item.second,"detailTitle")); insights->addWidget(tile);
    }
    insights->addStretch(); heroLayout->addLayout(insights,1); layout->addWidget(hero);

    auto *charts=new QGridLayout; charts->setHorizontalSpacing(12); charts->setVerticalSpacing(12);
    auto *byTypeCard=card("detail"); auto *typeLayout=new QVBoxLayout(byTypeCard); typeLayout->setContentsMargins(12,10,12,12); typeLayout->addWidget(new EquipmentBarChart("Equipment by type",pairs(byType),QColor("#287bea")));
    auto *byStationCard=card("detail"); auto *stationLayout=new QVBoxLayout(byStationCard); stationLayout->setContentsMargins(12,10,12,12); stationLayout->addWidget(new EquipmentBarChart("Equipment by station · top 5 + reserve",stationValues,QColor("#8560e8")));
    charts->addWidget(byTypeCard,0,0); charts->addWidget(byStationCard,0,1); charts->setColumnStretch(0,1); charts->setColumnStretch(1,1); layout->addLayout(charts,1);
    auto *footer=new QHBoxLayout; footer->addWidget(text("Counts reflect current records; no historical trend is inferred.","smallMuted"),1);
    auto *close=button("Close",""); close->setObjectName("closeEquipmentStatisticsButton"); footer->addWidget(close); layout->addLayout(footer);
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); dialog.exec();
}


class EquipmentDetailsPanel final : public QFrame {
public:
    explicit EquipmentDetailsPanel(QWidget *parent=nullptr):QFrame(parent) {
        setObjectName("equipmentHealth"); setProperty("role","card");
        auto *box=new QVBoxLayout(this); box->setContentsMargins(14,12,14,12); box->setSpacing(10);
        auto *head=new QHBoxLayout; head->setSpacing(12);
        head->addWidget(new Glyph("thermometer",blue,50,true));
        auto *copy=new QVBoxLayout; copy->setSpacing(1); copy->addWidget(text("Equipment Health","sectionTitle"));
        subtitle=text("Select an equipment item to view its recorded details.","smallMuted"); copy->addWidget(subtitle); head->addLayout(copy,1);
        state=text("No selection","smallMuted"); state->setObjectName("equipmentHealthStatus"); state->setProperty("role","equipmentStatus"); state->setAlignment(Qt::AlignCenter); state->setFixedSize(124,30);
        auto *statusGlow=new QGraphicsDropShadowEffect(state); statusGlow->setBlurRadius(14); statusGlow->setOffset(0,1); statusGlow->setColor(QColor(22,163,74,38)); state->setGraphicsEffect(statusGlow); head->addWidget(state);
        statusPulse=new QPropertyAnimation(statusGlow,"blurRadius",this); statusPulse->setDuration(1500);
        statusPulse->setStartValue(10.0); statusPulse->setKeyValueAt(.5,19.0); statusPulse->setEndValue(10.0);
        statusPulse->setEasingCurve(QEasingCurve::InOutSine); statusPulse->setLoopCount(-1);
        updated=text("","smallMuted"); head->addWidget(updated); box->addLayout(head);

        cardsGrid=new QGridLayout; cardsGrid->setSpacing(10); box->addLayout(cardsGrid);
        auto *identity=card("detail"); identity->setObjectName("equipmentIdentityCard");
        auto *identityGrid=new QGridLayout(identity); identityGrid->setContentsMargins(12,10,12,10); identityGrid->setHorizontalSpacing(12); identityGrid->setVerticalSpacing(4);
        photoPreview=new QLabel("Select an equipment item");
        photoPreview->setObjectName("equipmentPhotoPreview");
        photoPreview->setAlignment(Qt::AlignCenter);
        photoPreview->setFixedSize(132,84);
        photoPreview->setStyleSheet("background:#edf6ff;border:1px solid #d9e6f5;border-radius:8px;color:#5d77ad;padding:4px;");
        auto *photoRow=new QHBoxLayout; photoRow->setSpacing(12); photoRow->addWidget(photoPreview);
        photoCaption=text("Select an item to view its photo.", "smallMuted");
        photoCaption->setWordWrap(true); photoRow->addWidget(photoCaption,1,Qt::AlignVCenter);
        identityGrid->addLayout(photoRow,0,0,1,2);
        const QStringList labels{"Equipment ID","Type","Model","Serial Number","Station","Physical Location","Unit","Status","Last Update"};
        for(int i=0;i<labels.size();++i) { identityGrid->addWidget(text(labels[i],"smallMuted"),i+1,0); identityValues << text("—","smallMuted"); identityGrid->addWidget(identityValues.last(),i+1,1); }
        identityValues[7]->setProperty("role","equipmentStatus"); identityValues[7]->setAlignment(Qt::AlignCenter); identityValues[7]->setFixedSize(124,28);
        identityGrid->setColumnStretch(1,1);

        auto *recommendations=card("detail"); recommendations->setObjectName("equipmentRecommendationsCard");
        auto *recommendationLayout=new QVBoxLayout(recommendations); recommendationLayout->setContentsMargins(12,10,12,10); recommendationLayout->setSpacing(8);
        recommendationLayout->addWidget(text("Operational Insights","detailTitle"));
        const QStringList titles{"Update freshness","Recommended action"};
        const QStringList glyphs{"calendar","bulb"};
        const QColor colors[]{QColor("#0d47a1"),QColor("#ffaf00")};
        for(int i=0;i<titles.size();++i) {
            auto *row=new QHBoxLayout; row->setSpacing(10); row->addWidget(new Glyph(glyphs[i],colors[i],30,true),0,Qt::AlignTop);
            auto *copyRow=new QVBoxLayout; copyRow->setSpacing(1); copyRow->addWidget(text(titles[i],"detailTitle"));
            recommendationValues << text("—","recommendationCopy"); recommendationValues.last()->setWordWrap(true); copyRow->addWidget(recommendationValues.last());
            row->addLayout(copyRow,1); recommendationLayout->addLayout(row);
        }
        cardsGrid->setColumnStretch(0,1); cardsGrid->setColumnStretch(1,1);
        cardsGrid->addWidget(identity,0,0); cardsGrid->addWidget(recommendations,0,1);
    }
    void setEquipment(const EquipmentRecord *value) { selected=value?*value:EquipmentRecord{}; hasSelection=value; updatePanel(); }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QFrame::resizeEvent(event);
        const int columns=width()<900?1:2;
        QWidget *identity=findChild<QWidget *>("equipmentIdentityCard");
        QWidget *recommendations=findChild<QWidget *>("equipmentRecommendationsCard");
        if(identity) cardsGrid->removeWidget(identity);
        if(recommendations) cardsGrid->removeWidget(recommendations);
        if(identity) cardsGrid->addWidget(identity,0,0);
        if(recommendations) cardsGrid->addWidget(recommendations,0,columns==1?0:1);
        cardsGrid->setColumnStretch(0,1); cardsGrid->setColumnStretch(1,columns==1?0:1);
    }
private:
    void updatePanel() {
        if(!hasSelection) {
            if(statusPulse) statusPulse->stop();
            subtitle->setText("Select an equipment item to view its recorded details."); state->setText("No selection"); updated->clear();
            state->setProperty("tone","neutral"); identityValues[7]->setProperty("tone","neutral");
            state->style()->unpolish(state); state->style()->polish(state);
            identityValues[7]->style()->unpolish(identityValues[7]); identityValues[7]->style()->polish(identityValues[7]);
            photoPreview->setPixmap(QPixmap()); photoPreview->setText("Select an equipment item");
            photoCaption->setText("Select an item to view its saved photo or equipment icon.");
            for(QLabel *value:identityValues) value->setText("—");
            for(QLabel *value:recommendationValues) value->setText("—");
            return;
        }
        const EquipmentAssessment assessment=assessEquipment(selected);
        const QColor color=selected.status=="Online"?QColor("#16a34a"):selected.status=="Offline"?QColor("#ef4444"):QColor("#d97706");
        const QString tone=selected.status=="Online"?"online":selected.status=="Offline"?"offline":"maintenance";
        state->setText(selected.status); state->setProperty("tone",tone); identityValues[7]->setProperty("tone",tone);
        state->style()->unpolish(state); state->style()->polish(state);
        identityValues[7]->style()->unpolish(identityValues[7]); identityValues[7]->style()->polish(identityValues[7]);
        if(auto *glow=qobject_cast<QGraphicsDropShadowEffect *>(state->graphicsEffect())) glow->setColor(QColor(color.red(),color.green(),color.blue(),42));
        if(statusPulse && statusPulse->state()!=QAbstractAnimation::Running) statusPulse->start();
        const QDateTime updateDate=QDateTime::fromString(selected.updated,Qt::ISODate);
        const QString updateText=updateDate.isValid()?updateDate.toString("dd MMM yyyy HH:mm"):QStringLiteral("Not recorded");
        subtitle->setText(QString("Equipment %1").arg(selected.id));
        updated->setText(assessment.freshness);
        photoCaption->setText(selected.photoBase64.isEmpty()
            ? QStringLiteral("No photo saved; showing the equipment type icon.")
            : QStringLiteral("Photo saved with this equipment record."));
        photoPreview->setPixmap(equipmentPhoto(selected.photoBase64,selected.type,QSize(120,72)));
        const QStringList identity{selected.id,selected.type,selected.model,selected.serialNumber,
            selected.station.simplified().isEmpty()?QStringLiteral("Reserve / unassigned"):selected.station.simplified(),
            selected.location.isEmpty()?QStringLiteral("Not specified"):selected.location,
            selected.unit,selected.status,updateText};
        for(int i=0;i<identity.size();++i) identityValues[i]->setText(identity[i]);
        recommendationValues[0]->setText(assessment.freshness);
        recommendationValues[1]->setText(assessment.recommendation);
    }
    QLabel *subtitle=nullptr,*state=nullptr,*updated=nullptr,*photoPreview=nullptr,*photoCaption=nullptr;
    QPropertyAnimation *statusPulse=nullptr;
    QList<QLabel *> identityValues,recommendationValues;
    QGridLayout *cardsGrid=nullptr;
    EquipmentRecord selected;
    bool hasSelection=false;
};
struct ReplacementCandidate {
    EquipmentRecord record;
};

QVector<ReplacementCandidate> replacementCandidatesFor(const EquipmentRecord &equipment)
{
    QVector<ReplacementCandidate> available;
    for(const auto &candidate:compatibleReserveReplacements(equipmentRecords,equipment))
        available.append({candidate});
    return available;
}

void showReplacementDialog(QWidget *parent, const EquipmentRecord &equipment,
                           const std::function<bool(const ReplacementCandidate &)> &applyReplacement)
{
    QDialog dialog(parent); dialog.setObjectName("replacementEquipmentDialog");
    dialog.setWindowTitle("Find replacement — " + equipment.id); dialog.setMinimumSize(760, 520); dialog.resize(860, 600);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet(
        "QDialog#replacementEquipmentDialog{background:#f7fbff;color:#091b40;}"
        "QFrame#replacementSourceCard,QFrame#replacementCandidateCard{background:white;border:1px solid #d7e5f7;border-radius:12px;}"
        "QListWidget#replacementList{background:white;border:1px solid #d7e5f7;border-radius:10px;padding:5px;color:#17366f;}"
        "QListWidget#replacementList::item{min-height:56px;padding:9px;border-bottom:1px solid #edf3fb;}"
        "QListWidget#replacementList::item:selected{background:#e9f4ff;color:#091b40;}"
        "QPushButton#confirmReplacementButton{background:#0785ff;color:white;border:none;border-radius:8px;padding:8px 16px;font-weight:600;}"
        "QPushButton#confirmReplacementButton:hover{background:#006fe0;}"
    );
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,22,24,22); layout->setSpacing(14);
    layout->addWidget(text("Find a compatible replacement", "dialogTitle"));
    layout->addWidget(text("Compare available reserve equipment before choosing a replacement for the selected item.", "smallMuted"));

    auto *source=card("detail"); source->setObjectName("replacementSourceCard"); auto *sourceGrid=new QGridLayout(source); sourceGrid->setContentsMargins(16,12,16,12); sourceGrid->setHorizontalSpacing(18); sourceGrid->setVerticalSpacing(4);
    sourceGrid->addWidget(new Glyph(typeIcons.value(types.indexOf(equipment.type),"equipment"),blue,42,true),0,0,2,1);
    sourceGrid->addWidget(text("Equipment to replace", "detailTitle"),0,1);
    sourceGrid->addWidget(text(QString("%1 · %2 · %3").arg(equipment.id,equipment.type,equipment.model),"smallMuted"),1,1);
    sourceGrid->addWidget(text("Station", "smallMuted"),0,2); sourceGrid->addWidget(text(equipment.station.simplified(),"body"),1,2);
    sourceGrid->addWidget(text("Status", "smallMuted"),0,3); auto *sourceStatus=text(equipment.status,"body"); sourceStatus->setStyleSheet(QString("color:%1;font-weight:600;").arg(equipment.status=="Offline"?"#e5484d":"#c47700")); sourceGrid->addWidget(sourceStatus,1,3);
    layout->addWidget(source);

    const QVector<ReplacementCandidate> candidates=replacementCandidatesFor(equipment);
    if(candidates.isEmpty()) {
        auto *empty=card("detail"); auto *emptyLayout=new QVBoxLayout(empty); emptyLayout->setContentsMargins(20,18,20,18);
        emptyLayout->addWidget(text("No compatible replacement available in reserve", "detailTitle"));
        const QString reason="No Online reserve equipment matches the same type, model, and unit. Update the inventory when a compatible item becomes available.";
        emptyLayout->addWidget(text(reason+" The responsible person can update the equipment specifications or prepare a procurement request.", "smallMuted"));
        layout->addWidget(empty,1);
        auto *close=button("Close",""); layout->addWidget(close,0,Qt::AlignRight); QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);
        dialog.exec(); return;
    }

    layout->addWidget(text(QString("%1 compatible replacement%2 available in reserve").arg(candidates.size()).arg(candidates.size()==1?"":"s"),"detailTitle"));
    auto *split=new QHBoxLayout; split->setSpacing(14);
    auto *list=new QListWidget; list->setObjectName("replacementList"); list->setMinimumWidth(330);
    for(const auto &candidate:candidates) {
        const auto &r=candidate.record;
        auto *item=new QListWidgetItem(r.id+"  ·  "+r.model+"\n"+r.station.simplified()+"  ·  Available in reserve");
        item->setData(Qt::UserRole,r.id); list->addItem(item);
    }
    split->addWidget(list,1);
    auto *candidateCard=card("detail"); candidateCard->setObjectName("replacementCandidateCard"); auto *candidateLayout=new QVBoxLayout(candidateCard); candidateLayout->setContentsMargins(18,16,18,16); candidateLayout->setSpacing(8);
    candidateLayout->addWidget(text("Compatibility comparison", "detailTitle"));
    auto *candidateId=text("", "sectionTitle"); candidateLayout->addWidget(candidateId);
    auto *candidateModel=text("", "smallMuted"); candidateLayout->addWidget(candidateModel);
    auto *reserve=text("", "body"); candidateLayout->addWidget(reserve);
    auto *unit=text("", "body"); candidateLayout->addWidget(unit);
    auto *reason=text("", "smallMuted"); reason->setWordWrap(true); candidateLayout->addWidget(reason); candidateLayout->addStretch();
    split->addWidget(candidateCard,1); layout->addLayout(split,1);

    auto updateCandidate=[&](int row) {
        if(row<0 || row>=candidates.size()) return;
        const auto &candidate=candidates[row].record;
        candidateId->setText(candidate.id+" — compatible"); candidateModel->setText(candidate.model);
        reserve->setText("Reserve / unassigned · Online");
        unit->setText("Type: "+candidate.type+" · Unit: "+candidate.unit);
        reason->setText("Why it matches: same type, model and unit; the reserve item is Online.");
    };
    QObject::connect(list,&QListWidget::currentRowChanged,&dialog,updateCandidate); list->setCurrentRow(0);
    auto *actions=new QHBoxLayout; actions->addStretch(); auto *cancel=button("Cancel",""); auto *confirm=button("Choose replacement","replace",true); confirm->setObjectName("confirmReplacementButton"); actions->addWidget(cancel); actions->addWidget(confirm); layout->addLayout(actions);
    QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    QObject::connect(confirm,&QPushButton::clicked,&dialog,[&]{
        const int row=list->currentRow(); if(row<0) return;
        const auto &candidate=candidates[row].record;
        ReplacementCandidate selectedCandidate{candidate};
        if(!applyReplacement(selectedCandidate)) return;
        QMessageBox confirmation(&dialog); confirmation.setWindowTitle("Replacement deployed");
        confirmation.setText(candidate.id+" has been deployed as a replacement.");
        confirmation.setInformativeText(equipment.id+" is retained and marked Maintenance. The existing reserve record "+candidate.id+" was assigned to "+equipment.station.simplified()+".");
        confirmation.exec(); dialog.accept();
    });
    dialog.exec();
}
class ReportCalendar final : public QCalendarWidget {
protected:
    void paintCell(QPainter *painter,const QRect &rect,QDate day) const override {
        painter->save(); painter->setRenderHint(QPainter::Antialiasing);
        const bool selected=day==selectedDate();
        const bool today=day==QDate::currentDate();
        const bool inMonth=day.month()==monthShown() && day.year()==yearShown();
        const QRectF cell=QRectF(rect).adjusted(3,3,-3,-3);
        painter->fillRect(rect,AppTheme::brush(*painter, Qt::white));
        painter->setPen(AppTheme::pen(*painter, today && !selected?QPen(blue,1.3):QPen(Qt::NoPen)));
        painter->setBrush(AppTheme::brush(*painter, selected?blue:today?QColor("#eef6ff"):QColor(Qt::white)));
        painter->drawRoundedRect(cell,8,8);
        QFont font("Segoe UI"); font.setPixelSize(14); font.setBold(selected || today); painter->setFont(font);
        painter->setPen(AppTheme::pen(*painter, selected?Qt::white:!inMonth?QColor("#a1aec0"):day.dayOfWeek()>5?QColor("#62738c"):navy));
        painter->drawText(rect,Qt::AlignCenter,QString::number(day.day()));
        painter->restore();
    }
};

class ReportDateEdit final : public QDateEdit {
public:
    explicit ReportDateEdit(QWidget *parent=nullptr):QDateEdit(QDate::currentDate(),parent) {
        setLocale(QLocale(QLocale::English,QLocale::UnitedStates));
        setDisplayFormat("ddd, dd MMM yyyy");
        setDateRange(QDate(1900,1,1),QDate(2100,12,31));
        setButtonSymbols(QAbstractSpinBox::NoButtons);
        setCursor(Qt::PointingHandCursor);
        popup=new QFrame(this,Qt::Popup);
        popup->setObjectName("reportCalendarPopup");
        popup->setStyleSheet("QFrame#reportCalendarPopup { background:white; border:1px solid #c9def7; border-radius:12px; } QCalendarWidget QAbstractItemView { background:white; selection-background-color:#0785ff; selection-color:white; }");
        auto *box=new QVBoxLayout(popup); box->setContentsMargins(20,18,20,18); box->setSpacing(14);
        auto *title=new QLabel("Choose report date"); title->setProperty("role","sectionTitle"); box->addWidget(title);
        auto *hint=new QLabel("Choose a month and year, then select a day."); hint->setProperty("role","smallMuted"); box->addWidget(hint);
        auto *navigation=new QHBoxLayout;
        auto *previous=new QToolButton; previous->setIcon(icon("down",muted)); previous->setText("‹"); previous->setToolButtonStyle(Qt::ToolButtonTextOnly); previous->setAccessibleName("Previous month");
        month=new Selector; month->setObjectName("reportCalendarMonth");
        for(int i=1;i<=12;++i) month->addItem(QLocale(QLocale::English).monthName(i));
        year=new QSpinBox; year->setObjectName("reportCalendarYear"); year->setRange(1900,2100); year->setFixedWidth(94); year->setAlignment(Qt::AlignCenter);
        auto *next=new QToolButton; next->setText("›"); next->setAccessibleName("Next month");
        previous->setFixedWidth(34); next->setFixedWidth(34);
        previous->setStyleSheet("padding:0;font-size:22px;"); next->setStyleSheet("padding:0;font-size:22px;");
        for(QWidget *w:QList<QWidget *>{previous,month,year,next}) { w->setMinimumHeight(40); w->setCursor(Qt::PointingHandCursor); }
        navigation->addWidget(previous); navigation->addWidget(month,1); navigation->addWidget(year); navigation->addWidget(next); box->addLayout(navigation);
        calendar=new ReportCalendar; calendar->setObjectName("reportCalendar"); calendar->setNavigationBarVisible(false);
        calendar->setLocale(QLocale(QLocale::English)); calendar->setFirstDayOfWeek(Qt::Monday);
        calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        QTextCharFormat weekdayFormat; weekdayFormat.setForeground(navy); weekdayFormat.setBackground(QColor("#eef6ff"));
        QTextCharFormat weekendFormat=weekdayFormat; weekendFormat.setForeground(QColor("#c83c4d"));
        for(Qt::DayOfWeek day:{Qt::Monday,Qt::Tuesday,Qt::Wednesday,Qt::Thursday,Qt::Friday})
            calendar->setWeekdayTextFormat(day,weekdayFormat);
        calendar->setWeekdayTextFormat(Qt::Saturday,weekendFormat);
        calendar->setWeekdayTextFormat(Qt::Sunday,weekendFormat);
        calendar->setDateRange(minimumDate(),maximumDate()); calendar->setFixedSize(350,280); box->addWidget(calendar);
        auto *footer=new QHBoxLayout;
        auto *today=new QPushButton("Today"); today->setObjectName("reportCalendarToday"); today->setMinimumHeight(38); today->setCursor(Qt::PointingHandCursor); today->setProperty("primary",true);
        auto *todayLabel=new QLabel(QDate::currentDate().toString("dd MMM yyyy")); todayLabel->setProperty("role","smallMuted");
        footer->addWidget(todayLabel); footer->addStretch(); footer->addWidget(today); box->addLayout(footer);
        QObject::connect(month,&QComboBox::currentIndexChanged,popup,[this](int value){calendar->setCurrentPage(year->value(),value+1);});
        QObject::connect(year,&QSpinBox::valueChanged,popup,[this](int value){calendar->setCurrentPage(value,month->currentIndex()+1);});
        QObject::connect(calendar,&QCalendarWidget::currentPageChanged,popup,[this](int y,int m){QSignalBlocker a(month),b(year);month->setCurrentIndex(m-1);year->setValue(y);});
        QObject::connect(previous,&QToolButton::clicked,calendar,&QCalendarWidget::showPreviousMonth);
        QObject::connect(next,&QToolButton::clicked,calendar,&QCalendarWidget::showNextMonth);
        QObject::connect(calendar,&QCalendarWidget::clicked,this,[this](const QDate &value){setDate(value);popup->hide();setFocus();});
        QObject::connect(calendar,&QCalendarWidget::activated,this,[this](const QDate &value){setDate(value);popup->hide();});
        QObject::connect(today,&QPushButton::clicked,this,[this]{setDate(QDate::currentDate());popup->hide();});
    }
    void openCalendar() {
        calendar->setSelectedDate(date()); calendar->setCurrentPage(date().year(),date().month());
        {QSignalBlocker a(month),b(year);month->setCurrentIndex(date().month()-1);year->setValue(date().year());}
        popup->adjustSize();
        const QRect available=screen()->availableGeometry();
        QPoint position=mapToGlobal(QPoint(width()-popup->width(),height()+6));
        position.setX(qBound(available.left(),position.x(),qMax(available.left(),available.right()-popup->width())));
        if(position.y()+popup->height()>available.bottom()) position.setY(mapToGlobal(QPoint(0,-popup->height()-6)).y());
        popup->move(position); popup->show(); calendar->setFocus();
    }
protected:
    void mousePressEvent(QMouseEvent *event) override { if(event->button()==Qt::LeftButton) {openCalendar();event->accept();} else QDateEdit::mousePressEvent(event); }
    void keyPressEvent(QKeyEvent *event) override { if(event->key()==Qt::Key_F4 || (event->key()==Qt::Key_Down && event->modifiers().testFlag(Qt::AltModifier))) {openCalendar();event->accept();} else QDateEdit::keyPressEvent(event); }
private:
    QFrame *popup; QCalendarWidget *calendar; QComboBox *month; QSpinBox *year;
};

class EquipmentPage final : public QWidget {
    EquipmentTable *table=nullptr;
    QLineEdit *query=nullptr;
    QComboBox *typeFilter=nullptr,*stationFilter=nullptr,*statusFilter=nullptr;
    QPushButton *editButton=nullptr,*deleteButton=nullptr,*replacementButton=nullptr;
    QLabel *resultLabel=nullptr,*feedback=nullptr;
    EquipmentDetailsPanel *detailsPanel=nullptr;
    QDateEdit *reportDate=nullptr;
    QList<QWidget *> metrics;
    QVBoxLayout *body=nullptr;
public:
    explicit EquipmentPage(QWidget *parent):QWidget(parent) {
        setObjectName("equipmentPage");
        QString loadError;
        const bool loaded=inventory.load({},&loadError);
        auto *outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
        auto *top=card("topbar"); auto *bar=new QHBoxLayout(top); bar->setContentsMargins(28,12,24,12); bar->setSpacing(16);
        auto *globalSearch=search("Find equipment, models or serial numbers...");
        globalSearch->setObjectName("globalEquipmentSearch"); globalSearch->setClearButtonEnabled(true);
        bar->addWidget(globalSearch,1); bar->addSpacing(18);
        bar->addWidget(new Glyph("calendar",navy,24));
        auto *dateEditor=new ReportDateEdit;
        reportDate=dateEditor;
        reportDate->setObjectName("reportDatePicker"); reportDate->setFixedHeight(38);
        reportDate->setToolTip("Choose the report date. This date appears in the PDF inventory report.");
        bar->addWidget(reportDate);
        auto *dateArrow=new QToolButton; dateArrow->setObjectName("reportDateArrow"); dateArrow->setIcon(icon("down",muted)); dateArrow->setFixedSize(32,38); dateArrow->setCursor(Qt::PointingHandCursor); dateArrow->setToolTip("Choose day, month and year"); dateArrow->setAccessibleName("Open report calendar");
        bar->addWidget(dateArrow);
        QObject::connect(dateArrow,&QToolButton::clicked,dateEditor,&ReportDateEdit::openCalendar);
        auto *divider=new QFrame; divider->setProperty("role","divider"); divider->setFixedSize(1,35); bar->addWidget(divider);
        bar->addWidget(new Glyph("user",muted,42,true));
        auto *user=new QVBoxLayout; user->setSpacing(1); user->addWidget(text("Alex Martin","detailTitle")); user->addWidget(text("Operations Manager","smallMuted")); bar->addLayout(user);
        bar->addWidget(new Glyph("down",navy,18));
        top->hide();
        outer->addWidget(top);
        auto *scroll=new QScrollArea; scroll->setObjectName("equipmentScroll"); scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
        auto *content=new QWidget; content->setObjectName("equipmentContent");
        body=new QVBoxLayout(content); body->setContentsMargins(24,14,24,22); body->setSpacing(12);
        auto *pageHeader=new QGridLayout; pageHeader->setContentsMargins(0,0,0,0); pageHeader->setHorizontalSpacing(14); pageHeader->setVerticalSpacing(2);
        pageHeader->addWidget(text("Equipment","pageTitle"),0,0);
        auto *subtitle=text("Track devices, status and station assignments.","subtitle");
        subtitle->setWordWrap(true); pageHeader->addWidget(subtitle,1,0);
        auto *actions=new QHBoxLayout; actions->setSpacing(8);
        auto *exportButton=button("Export PDF","export"); exportButton->setObjectName("exportEquipmentButton");
        exportButton->setToolTip("Choose either the complete inventory or selected equipment when exporting a PDF.");
        auto *statisticsButton=button("Statistics","chart"); statisticsButton->setObjectName("equipmentStatisticsButton");
        statisticsButton->setToolTip("View equipment distribution by status, type and station.");
        replacementButton=button("Find Replacement","replace"); replacementButton->setObjectName("findEquipmentReplacementButton");
        replacementButton->setToolTip("Compare compatible equipment available in reserve for the selected item.");
        replacementButton->setEnabled(false);
        editButton=button("Edit","edit"); editButton->setObjectName("editSelectedEquipmentButton");
        deleteButton=button("Delete","delete"); deleteButton->setObjectName("deleteSelectedEquipmentButton");
        auto *addButton=button("Add Equipment","plus",true); addButton->setObjectName("addEquipmentButton");
        actions->addWidget(exportButton); actions->addWidget(statisticsButton); actions->addWidget(replacementButton); actions->addWidget(editButton); actions->addWidget(deleteButton); actions->addWidget(addButton);
        pageHeader->addLayout(actions,0,1,2,1,Qt::AlignRight|Qt::AlignVCenter); pageHeader->setColumnStretch(0,1);
        body->addLayout(pageHeader);
        auto *cards=new QHBoxLayout; cards->setSpacing(10);
        metrics={metricCard("Registered Equipment","0","in inventory","All registered","equipment",blue,"blue"),
                 metricCard("Online Equipment","0","/ 0 total","Operational","wifi",QColor("#00b863"),"green"),
                 metricCard("Offline Equipment","0","/ 0 total","Needs attention","offline",QColor("#ff3859"),"red"),
                 metricCard("In Maintenance","0","/ 0 total","Service in progress","gear",QColor("#7948ff"),"purple")};
        for(auto *m:metrics) cards->addWidget(m,1);
        body->addLayout(cards);
        query=search("Search ID, type, model, station or serial number...");
        query->setObjectName("equipmentSearch"); query->setClearButtonEnabled(true); body->addWidget(query);
        auto *filters=new QHBoxLayout; filters->setSpacing(10);
        typeFilter=selector("All Types"); typeFilter->setObjectName("typeFilter"); typeFilter->addItems(types);
        stationFilter=selector("All Stations"); stationFilter->setObjectName("stationFilter");
        QStringList uniqueStations;
        for(const auto &record:equipmentRecords) {
            const QString station=record.station.simplified();
            if(!station.isEmpty()&&!uniqueStations.contains(station)) uniqueStations.append(station);
        }
        if(std::any_of(equipmentRecords.cbegin(),equipmentRecords.cend(),[](const EquipmentRecord &r){return r.station.trimmed().isEmpty();}))
            stationFilter->addItem("Reserve / unassigned",QStringLiteral("__RESERVE__"));
        for(const auto &station:uniqueStations) stationFilter->addItem(station,station);
        statusFilter=selector("All Statuses"); statusFilter->setObjectName("statusFilter"); statusFilter->addItems({"Online","Offline","Maintenance"});
        auto *clear=button("Clear filters",""); clear->setObjectName("clearEquipmentFilters"); clear->setProperty("link",true);
        filters->addWidget(typeFilter,1); filters->addWidget(stationFilter,1); filters->addWidget(statusFilter,1); filters->addWidget(clear);
        body->addLayout(filters);
        resultLabel=text("","smallMuted"); resultLabel->setObjectName("equipmentResultCount"); body->addWidget(resultLabel);
        table=new EquipmentTable; body->addWidget(table);
        const bool oracleEquipment = QSqlDatabase::database().isOpen();
        feedback=text(loaded
            ? (oracleEquipment ? "Oracle inventory: equipment changes are saved in the EQUIPEMENT table."
                               : "Demo data: changes remain available until the application closes.")
            : "Equipment data could not be loaded: "+loadError,"smallMuted"); feedback->setObjectName("equipmentFeedback");
        feedback->setWordWrap(true); body->addWidget(feedback);
        detailsPanel=new EquipmentDetailsPanel; body->addWidget(detailsPanel); body->addStretch(); scroll->setWidget(content);
        table->changed=[this](const QString &message){feedback->setText(message); refreshStationFilter(); refreshMetrics(); applyFilters();};
        table->selectionChanged=[this]{updateSelection();};
        auto *debounce=new QTimer(this); debounce->setSingleShot(true); debounce->setInterval(150);
        QObject::connect(debounce,&QTimer::timeout,this,[this]{applyFilters();});
        QObject::connect(query,&QLineEdit::textChanged,this,[globalSearch,debounce](const QString &value){
            QSignalBlocker block(globalSearch); globalSearch->setText(value); debounce->start();
        });
        QObject::connect(globalSearch,&QLineEdit::textChanged,query,&QLineEdit::setText);
        QObject::connect(reportDate,&QDateEdit::dateChanged,this,[this](const QDate &date){
            feedback->setText("Report date set to "+date.toString("dd MMM yyyy")+".");
        });
        for(auto *filter:{typeFilter,stationFilter,statusFilter})
            QObject::connect(filter,&QComboBox::currentIndexChanged,this,[this]{applyFilters();});
        QObject::connect(clear,&QPushButton::clicked,this,[this]{query->clear();typeFilter->setCurrentIndex(0);stationFilter->setCurrentIndex(0);statusFilter->setCurrentIndex(0);applyFilters();});
        QObject::connect(addButton,&QPushButton::clicked,this,[this]{
            EquipmentRecord record;
            showAddEquipmentDialog(this,&record,false,[this](const EquipmentRecord &added){
                auto candidate=equipmentRecords; candidate.append(added);
                return table->commit(candidate,added.id+" added and saved.",added.id);
            });
        });
        QObject::connect(editButton,&QPushButton::clicked,this,[this]{table->editEquipment(table->selectedId());});
        QObject::connect(deleteButton,&QPushButton::clicked,this,[this]{table->deleteEquipment(table->selectedId());});
        QObject::connect(replacementButton,&QPushButton::clicked,this,[this]{
            const QString id=table->selectedId();
            for(const auto &record:equipmentRecords) if(record.id==id) {
                showReplacementDialog(this,record,[this,record](const ReplacementCandidate &candidate){
                    auto changedRecords=equipmentRecords;
                    int oldIndex=-1;
                    for(int i=0;i<changedRecords.size();++i) if(changedRecords[i].id==record.id) { oldIndex=i; break; }
                    if(oldIndex<0) return false;
                    const QString timestamp=QDateTime::currentDateTime().toString(Qt::ISODate);
                    changedRecords[oldIndex].status="Maintenance";
                    changedRecords[oldIndex].updated=timestamp;
                    int reserveIndex=-1;
                    for(int i=0;i<changedRecords.size();++i) if(changedRecords[i].id==candidate.record.id) {reserveIndex=i;break;}
                    if(reserveIndex<0)return false;
                    changedRecords[reserveIndex].station=record.station;
                    changedRecords[reserveIndex].status="Online";
                    changedRecords[reserveIndex].updated=timestamp;
                    return table->commit(changedRecords,QString("%1 from reserve assigned to %2; %3 moved to Maintenance.").arg(candidate.record.id,record.station.simplified(),record.id),candidate.record.id);
                });
                return;
            }
        });
        QObject::connect(statisticsButton,&QPushButton::clicked,this,[this]{showEquipmentStatistics(this,equipmentRecords);});
        QObject::connect(exportButton,&QPushButton::clicked,this,[this]{
            const PdfExportScope selectedScope=choosePdfExportScope(this);
            if(selectedScope==PdfExportScope::Cancelled) return;
            const bool exportSelection=selectedScope==PdfExportScope::Selected;
            const QVector<EquipmentRecord> records=exportSelection?chooseEquipmentForPdf(this,equipmentRecords):equipmentRecords;
            if(exportSelection && records.isEmpty()) return;
            const QString scope=exportSelection?QString("%1 selected equipment record%2").arg(records.size()).arg(records.size()==1?"":"s"):
                QString("complete inventory (%1 records)").arg(records.size());
            QString path=QFileDialog::getSaveFileName(this,exportSelection?"Export selected equipment":"Export complete equipment inventory",
                (exportSelection?"SkySense_Selected_Equipment_":"SkySense_Equipment_")+reportDate->date().toString("yyyy-MM-dd")+".pdf","PDF document (*.pdf)");
            if(path.isEmpty()) return;
            if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
            if(QFileInfo::exists(path) && QMessageBox::question(this,"Replace existing PDF", "A PDF already exists at this location. Replace it?",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes) return;
            QString error;
            if(!exportEquipmentPdf(path,records,reportDate->date(),&error,exportSelection)) { QMessageBox::critical(this,"Export failed",error); return; }
            feedback->setText(QString("PDF exported: %1 — report date %2.").arg(scope).arg(reportDate->date().toString("dd MMM yyyy")));
            QMessageBox result(this); result.setWindowTitle("Inventory exported"); result.setTextFormat(Qt::PlainText);
            result.setText(QString("Equipment report exported: %1.").arg(scope)); result.setInformativeText(path);
            auto *open=result.addButton("Open PDF",QMessageBox::ActionRole); result.addButton(QMessageBox::Close); result.exec();
            if(result.clickedButton()==open && !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
                QMessageBox::information(this,"PDF saved","No PDF viewer could be opened. The file is saved at:\n"+path);
        });
        if(!loaded) {
            addButton->setEnabled(false); exportButton->setEnabled(false);
            feedback->setText("Inventory unavailable — "+loadError);
#ifndef SKYSENSE_UI_TESTS
            QTimer::singleShot(0,this,[this,loadError]{QMessageBox::critical(this,"Inventory could not be loaded",loadError);});
#endif
        }
        refreshMetrics(); applyFilters(); adapt();
    }
protected:
    void resizeEvent(QResizeEvent *e) override { QWidget::resizeEvent(e); adapt(); }
private:
    void applyFilters() {
        table->query=query->text();
        table->typeFilter=typeFilter->currentIndex()>0?typeFilter->currentText():QString();
        table->stationFilter=stationFilter->currentIndex()>0?stationFilter->currentData().toString():QString();
        table->statusFilter=statusFilter->currentIndex()>0?statusFilter->currentText():QString();
        table->refresh();
        resultLabel->setText(table->rowCount()?QString("Showing %1 of %2 equipment items • Click a column heading to sort").arg(table->rowCount()).arg(equipmentRecords.size()):
            equipmentRecords.isEmpty()?"Your inventory is empty. Add equipment to get started.":"No equipment matches these filters. Clear filters or change your search.");
    }
    void refreshStationFilter() {
        const QString current=stationFilter->currentData().toString();
        QStringList stationNames;
        for(const auto &record:equipmentRecords) {
            const QString name=record.station.simplified();
            if(!name.isEmpty() && !stationNames.contains(name,Qt::CaseInsensitive)) stationNames.append(name);
        }
        const bool hasReserve=std::any_of(equipmentRecords.cbegin(),equipmentRecords.cend(),[](const EquipmentRecord &r){return r.station.trimmed().isEmpty();});
        std::sort(stationNames.begin(),stationNames.end(),[](const QString &a,const QString &b){
            return QString::compare(a,b,Qt::CaseInsensitive)<0;
        });
        const QSignalBlocker blocker(stationFilter);
        stationFilter->clear(); stationFilter->addItem("All Stations",QString());
        if(hasReserve) stationFilter->addItem("Reserve / unassigned",QStringLiteral("__RESERVE__"));
        for(const QString &name:stationNames) stationFilter->addItem(name,name);
        const int selected=stationFilter->findData(current);
        stationFilter->setCurrentIndex(selected>=0?selected:0);
    }
    void updateSelection() {
        const QString id=table->selectedId();
        editButton->setEnabled(!id.isEmpty()); deleteButton->setEnabled(!id.isEmpty()); replacementButton->setEnabled(false);
        detailsPanel->setEquipment(nullptr);
        for(const auto &r:equipmentRecords) if(r.id==id) {
            detailsPanel->setEquipment(&r);
            replacementButton->setEnabled(!r.station.trimmed().isEmpty());
            break;
        }
    }
    void refreshMetrics() {
        int counts[4]{int(equipmentRecords.size()),0,0,0};
        for(const auto &r:equipmentRecords) {if(r.status=="Online")++counts[1];else if(r.status=="Offline")++counts[2];else ++counts[3];}
        for(int i=0;i<4;++i) {
            metrics[i]->findChild<QLabel *>("metricValue")->setText(QString::number(counts[i]));
            metrics[i]->findChild<QLabel *>("metricTotal")->setText(i?QString("/ %1 total").arg(counts[0]):"in inventory");
            auto *meterWidget=metrics[i]->findChild<QWidget *>("metricMeter");
            auto *meter=dynamic_cast<MiniBars *>(meterWidget);
            if(!meter) continue;
            const qreal target=i==0?(counts[0]>0?1.0:0.0):(counts[0]>0?qreal(counts[i])/counts[0]:0.0);
            if(auto *share=metrics[i]->findChild<QLabel *>("metricShare")) share->setText(i==0?QStringLiteral("%1%").arg(counts[0]>0?100:0):QStringLiteral("%1%").arg(qRound(target*100)));
            auto *animation=new QVariantAnimation(meter); animation->setDuration(380); animation->setStartValue(meter->progress()); animation->setEndValue(target); animation->setEasingCurve(QEasingCurve::OutCubic);
            QObject::connect(animation,&QVariantAnimation::valueChanged,meter,[meter](const QVariant &value){meter->setProgress(value.toReal());});
            QObject::connect(animation,&QVariantAnimation::finished,animation,&QObject::deleteLater); animation->start();
            meter->setToolTip(i==0?QString("%1 item(s) in the inventory").arg(counts[i]):QString("%1 of %2 equipment items · %3%").arg(counts[i]).arg(counts[0]).arg(counts[0]?qRound(100.0*counts[i]/counts[0]):0));
        }
    }
    void adapt() {
        if(!body) return;
        const bool narrow=width()<1000;
        body->setContentsMargins(narrow?14:24,14,narrow?14:24,22);
        for(auto *m:metrics) {
            auto *row=qobject_cast<QHBoxLayout *>(m->layout()); row->setContentsMargins(8,14,8,14); row->setSpacing(6);
            for(auto *w:m->findChildren<QWidget *>(QString(),Qt::FindDirectChildrenOnly)) {
                if(auto *glyph=dynamic_cast<Glyph *>(w)) glyph->setVisible(!narrow);
                if(auto *bars=dynamic_cast<MiniBars *>(w)) bars->setVisible(!narrow);
            }
        }
    }
};

}
void setupEquipmentUi(QWidget *root)
{
    root->setObjectName("equipmentRoot"); auto *layout=new QHBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0); layout->addWidget(new EquipmentPage(root),1);
    QFile stylesheet(":/styles/equipment.qss"); if(stylesheet.open(QIODevice::ReadOnly)) root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}




