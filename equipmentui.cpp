#include "equipmentui.h"
#include "equipmentdata.h"
#include "databaseconnection.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QSignalBlocker>
#include <QScreen>
#include <QCloseEvent>
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QSpinBox>
#include <QLocale>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QImageReader>
#include <QStandardPaths>
#include <QBuffer>
#include <QDialog>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
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
    } else if (name == "database") {
        for(int y : {4,10,16}) p.drawArc(QRectF(4,y-3,16,7),0,180*16);
        p.drawLine(4,4,4,19); p.drawLine(20,4,20,19); p.drawArc(QRectF(4,16,16,7),180*16,180*16);
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
QLineEdit *search(const QString &placeholder)
{
    auto *edit=new QLineEdit; edit->setPlaceholderText(placeholder); edit->setMinimumWidth(160); edit->setFixedHeight(40);
    edit->addAction(icon("search",muted),QLineEdit::LeadingPosition); return edit;
}

void showOracleConnectionDialog(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setObjectName("oracleConnectionDialog");
    dialog.setWindowTitle("Oracle database connection");
    dialog.setMinimumSize(570, 440);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet(
        "QDialog#oracleConnectionDialog{background:#f7fbff;color:#091b40;}"
        "QFrame#oracleConnectionInfo{background:#ffffff;border:1px solid #d7e5f7;border-radius:12px;}"
        "QLineEdit{background:#ffffff;border:1px solid #cfe0f6;border-radius:7px;padding:0 10px;min-height:34px;color:#17366f;}"
        "QLineEdit:focus{border:2px solid #0785ff;}"
        "QLabel#oracleConnectionStatus{padding:10px 12px;border-radius:8px;background:#edf4ff;color:#31598f;}"
        "QPushButton#testOracleConnectionButton{background:#0785ff;color:#ffffff;border:none;border-radius:8px;padding:8px 16px;font-weight:600;}"
        "QPushButton#testOracleConnectionButton:hover{background:#006fe0;}"
    );
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(28, 25, 28, 24);
    layout->setSpacing(12);
    layout->addWidget(text("Oracle database connection", "dialogTitle"));
    auto *intro = text("Enter your Oracle XE account parameters, then test the connection from this Equipment module.", "subtitle");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *info = card("detail");
    info->setObjectName("oracleConnectionInfo");
    auto *infoLayout = new QHBoxLayout(info);
    infoLayout->setContentsMargins(14, 12, 14, 12);
    auto *infoIcon = new QLabel;
    infoIcon->setPixmap(icon("database", blue).pixmap(34, 34));
    infoLayout->addWidget(infoIcon);
    auto *infoText = text("Detected setup: Oracle XE listener is expected at localhost:1521/XE. Your password is used only for this test and is not saved.", "smallMuted");
    infoText->setWordWrap(true);
    infoLayout->addWidget(infoText, 1);
    layout->addWidget(info);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(9);
    auto makeInput = [] (const QString &value, const QString &placeholder = QString()) {
        auto *input = new QLineEdit(value);
        input->setPlaceholderText(placeholder);
        return input;
    };
    auto *driver = makeInput("Oracle dans OraDB21Home1");
    auto *host = makeInput("localhost");
    auto *port = makeInput("1521");
    auto *service = makeInput("XE");
    auto *user = makeInput({}, "Enter your Oracle username");
    auto *password = makeInput({}, "Enter your Oracle password");
    password->setEchoMode(QLineEdit::Password);
    driver->setObjectName("oracleDriverInput"); host->setObjectName("oracleHostInput"); port->setObjectName("oraclePortInput");
    service->setObjectName("oracleServiceInput"); user->setObjectName("oracleUsernameInput"); password->setObjectName("oraclePasswordInput");
    form->addRow("ODBC driver", driver);
    form->addRow("Host", host);
    form->addRow("Port", port);
    form->addRow("Service / SID", service);
    form->addRow("Username", user);
    form->addRow("Password", password);
    layout->addLayout(form);

    auto *status = text("Enter your personal Oracle account, then choose Test Connection.", "smallMuted");
    status->setObjectName("oracleConnectionStatus");
    layout->addWidget(status);
    auto *actions = new QHBoxLayout;
    actions->addStretch();
    auto *close = button("Close", "");
    auto *test = button("Test Connection", "database", true);
    test->setObjectName("testOracleConnectionButton");
    actions->addWidget(close);
    actions->addWidget(test);
    layout->addLayout(actions);
    QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(test, &QPushButton::clicked, &dialog, [=] {
        const OracleConnectionParameters parameters{driver->text(), host->text(), port->text(), service->text(), user->text(), password->text()};
        const OracleConnectionResult result = DatabaseConnection::testOracleConnection(parameters);
        status->setText(result.message);
        status->setStyleSheet(result.connected
            ? "padding:10px 12px;border-radius:8px;background:#dcfce7;color:#087f43;font-weight:600;"
            : "padding:10px 12px;border-radius:8px;background:#fff1f2;color:#bc2440;font-weight:600;");
    });
    dialog.exec();
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
    auto *change=text(trend,"trend"); change->setProperty("tone",tone); copy->addWidget(change); row->addLayout(copy,1); row->addWidget(new MiniBars(accent),0,Qt::AlignVCenter); return f;
}
const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
const QStringList typeIcons{"thermometer","anemometer","drop","sun","gauge","camera"};
const QStringList stations{"ALP-03\nAlpine Ridge","COA-01\nCoastal Bay","FOR-12\nPine Forest","VAL-07\nGreen Valley","ALP-03\nAlpine Ridge","DES-05\nDesert View"};
const QStringList units{QString::fromUtf8("—"),"m/s","mm",QString::fromUtf8("W/m²"),"hPa",QString::fromUtf8("—")};
const QStringList times{"10:38","10:36","10:20","10:41","10:15","10:37"};
const QStringList statuses{"Online","Online","Offline","Online","Maintenance","Online"};
// A separate local demo file keeps the original inventory data intact while this
// version opens with a complete set of records for the Equipment presentation.
EquipmentStore inventory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/equipment/demo_inventory_v2.json");
const QVector<EquipmentRecord> &equipmentRecords=inventory.records();
const QVector<EquipmentRecord> sampleRecords{
    {"EQP-001",types[0],stations[0],units[0],"2025-04-24T10:38:00",statuses[0],"Vaisala WXT536","W5536-78421"},
    {"EQP-002",types[1],stations[1],units[1],"2025-04-24T10:36:00",statuses[1],"WindSonic 2","WS-20481"},
    {"EQP-003",types[2],stations[2],units[2],"2025-04-24T10:20:00",statuses[2],"RG3-M","RG-12003"},
    {"EQP-004",types[3],stations[3],units[3],"2025-04-24T10:41:00",statuses[3],"CMP11","CMP-09114"},
    {"EQP-005",types[4],stations[4],units[4],"2025-04-24T10:15:00",statuses[4],"PTB330","PTB-55210"},
    {"EQP-006",types[5],stations[5],units[5],"2025-04-24T10:37:00",statuses[5],"SkyCam S2","SC-38201"},
    {"EQP-007",types[0],"NOR-11\nNorthern Peak",units[0],"2026-10-10T08:15:00","Offline","Vaisala WXT536","W5536-78422"},
    {"EQP-008",types[1],"PLA-08\nGreat Plains",units[1],"2026-10-10T09:05:00","Maintenance","WindSonic 2","WS-20482"},
    {"EQP-009",types[2],stations[1],units[2],"2026-10-10T09:42:00","Online","RG3-M","RG-12004"},
    {"EQP-010",types[3],stations[5],units[3],"2026-10-09T16:20:00","Maintenance","CMP11","CMP-09115"},
    {"EQP-011",types[4],stations[0],units[4],"2026-10-10T10:18:00","Online","PTB330","PTB-55211"},
    {"EQP-012",types[5],stations[3],units[5],"2026-10-10T10:44:00","Offline","SkyCam S2","SC-38202"},
    {"EQP-013",types[0],"LAK-04\nLake Valley",units[0],"2026-10-09T14:32:00","Online","Lufft WS600","LF-60501"},
    {"EQP-014",types[1],stations[2],units[1],"2026-10-10T07:55:00","Offline","Gill MaxiMet","GMX-88420"},
    {"EQP-015",types[2],"NOR-11\nNorthern Peak",units[2],"2026-10-09T12:08:00","Maintenance","RG3-M","RG-12005"},
    {"EQP-016",types[3],"PLA-08\nGreat Plains",units[3],"2026-10-10T11:03:00","Online","CMP11","CMP-09116"}
};

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
    QStringList uniqueStations=stations; uniqueStations.removeDuplicates();
    for(const QString &entry:uniqueStations)
        station->addItem(entry.section('\n',0,0)+" — "+entry.section('\n',1,1),entry);
    auto *unit=new QLineEdit(unitForType(type->currentText()));
    unit->setReadOnly(true);
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
        const int stationIndex=station->findData(record->station);
        if(stationIndex>=0) station->setCurrentIndex(stationIndex);
        status->setCurrentText(record->status);
        model->setText(record->model);
        serial->setText(record->serialNumber);
    }

    for(auto *editor:{model,serial}) QObject::connect(editor,&QLineEdit::textEdited,&dialog,[&dialog]{dialog.dirty=true;});
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
    for(auto *editor:{model,serial}) QObject::connect(editor,&QLineEdit::textChanged,validation,&QLabel::clear);
    QObject::connect(saveButton,&QPushButton::clicked,&dialog,[&dialog,&photoData,model,serial,originalId,validation,type,station,unit,status,record,editing,generatedId,save]{
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
        candidate.type=type->currentText(); candidate.station=station->currentData().toString();
        candidate.unit=unit->text(); candidate.updated=QDateTime::currentDateTime().toString(Qt::ISODate);
        candidate.status=status->currentText(); candidate.model=model->text().trimmed();
        candidate.serialNumber=serialValue;
        candidate.photoBase64=photoData;
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
        p->fillRect(r,o.state.testFlag(QStyle::State_Selected)?QColor("#e5f4ff"):Qt::white); p->setPen(QColor("#e5eef9")); p->drawLine(r.bottomLeft(),r.bottomRight());
        QFont font("Segoe UI"); font.setPixelSize(13); p->setFont(font); p->setPen(navy);
        EquipmentRecord data;
        const QString id=index.siblingAtColumn(1).data().toString();
        for(const auto &record:equipmentRecords) if(record.id==id) { data=record; break; }
        const bool selected=o.state.testFlag(QStyle::State_Selected);
        if(col==0) {
            if(selected) p->fillRect(QRect(r.left(),r.top(),3,r.height()),blue);
            QRectF box(r.center().x()-8,r.center().y()-8,16,16); p->setPen(QPen(selected?blue:QColor("#8ca7d3"),1.3)); p->setBrush(selected?blue:Qt::white); p->drawRoundedRect(box,3,3);
            if(selected) { p->translate(box.topLeft()); p->scale(16.0/24,16.0/24); symbol(*p,"check",Qt::white); }
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
            p->setPen(muted); p->drawText(r.adjusted(30,0,0,0),Qt::AlignVCenter,QDateTime::fromString(data.updated,Qt::ISODate).toString("dd MMM yyyy HH:mm"));
        } else if(col==6) {
            const bool offline=data.status=="Offline", maintenance=data.status=="Maintenance";
            QRectF badge(r.center().x()-53,r.center().y()-14,106,28); p->setPen(Qt::NoPen); p->setBrush(offline?QColor("#fee2e2"):maintenance?QColor("#fef3c7"):QColor("#dcfce7")); p->drawRoundedRect(badge,14,14);
            p->setPen(offline?QColor("#ef4444"):maintenance?QColor("#d97706"):QColor("#16a34a")); p->drawText(badge,Qt::AlignCenter,data.status);
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
        setHorizontalHeaderLabels({"","ID","Type","Station","Unit","Last Modified","Status","Actions"});
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
                const QStringList fields{"",r.id,r.type,r.station,r.unit,r.updated,r.status,""};
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
    QString title;
    QVector<QPair<QString,int>> values;
    QColor accent;
public:
    EquipmentBarChart(QString chartTitle, QVector<QPair<QString,int>> chartValues, QColor color, QWidget *parent=nullptr)
        : QWidget(parent), title(std::move(chartTitle)), values(std::move(chartValues)), accent(color) {
        setMinimumHeight(205); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.fillRect(rect(), Qt::white);
        QFont titleFont("Segoe UI",10,QFont::DemiBold); p.setFont(titleFont); p.setPen(navy);
        p.drawText(QRectF(14,12,width()-28,22),Qt::AlignLeft|Qt::AlignVCenter,title);
        const QRectF plot(38,48,width()-54,height()-82);
        const int maximum=qMax(1, std::max_element(values.cbegin(),values.cend(),[](const auto &a,const auto &b){ return a.second<b.second; })->second);
        QFont small("Segoe UI"); small.setPixelSize(10); p.setFont(small);
        for(int i=0;i<4;++i) { const qreal y=plot.bottom()-plot.height()*i/3; p.setPen(QColor("#e7effa")); p.drawLine(plot.left(),y,plot.right(),y); p.setPen(muted); p.drawText(QRectF(0,y-8,32,16),Qt::AlignRight,QString::number(qRound(maximum*i/3.0))); }
        const qreal slot=plot.width()/qMax(1,values.size()); const qreal bar=qMin(42.0,slot*.54);
        for(int i=0;i<values.size();++i) {
            const qreal h=plot.height()*values[i].second/maximum; const QRectF r(plot.left()+i*slot+(slot-bar)/2,plot.bottom()-h,bar,h);
            QLinearGradient fill(r.topLeft(),r.bottomLeft()); fill.setColorAt(0,accent); QColor pale=accent; pale.setAlpha(110); fill.setColorAt(1,pale);
            p.setPen(Qt::NoPen); p.setBrush(fill); p.drawRoundedRect(r,5,5);
            p.setPen(navy); p.drawText(QRectF(r.left()-12,r.top()-20,r.width()+24,17),Qt::AlignCenter,QString::number(values[i].second));
            p.setPen(muted); QString label=values[i].first; if(label.size()>13) label=label.left(12)+QString::fromUtf8("…");
            p.drawText(QRectF(plot.left()+i*slot,plot.bottom()+7,slot,25),Qt::AlignHCenter|Qt::TextWordWrap,label);
        }
    }
};

void showEquipmentStatistics(QWidget *parent, const QVector<EquipmentRecord> &records)
{
    QMap<QString,int> byStatus, byType, byStation;
    for(const auto &record:records) { ++byStatus[record.status]; ++byType[record.type]; ++byStation[record.station.section('\n',0,0)]; }
    const auto pairs=[](const QMap<QString,int> &map) { QVector<QPair<QString,int>> result; for(auto it=map.cbegin();it!=map.cend();++it) result.append({it.key(),it.value()}); return result; };
    QDialog dialog(parent); dialog.setObjectName("equipmentStatisticsDialog"); dialog.setWindowTitle("Equipment statistics"); dialog.setMinimumSize(760,570); dialog.resize(900,600);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(26,24,26,22); layout->setSpacing(8);
    layout->addWidget(text("Equipment statistics","dialogTitle"));
    auto *intro=text("A visual overview of the inventory to identify equipment requiring attention and monitor distribution across stations.","subtitle"); intro->setWordWrap(true); layout->addWidget(intro);
    auto *grid=new QGridLayout; grid->setHorizontalSpacing(14); grid->setVerticalSpacing(14);
    grid->addWidget(new EquipmentBarChart("Equipment by status",pairs(byStatus),QColor("#0785ff")),0,0);
    grid->addWidget(new EquipmentBarChart("Equipment by type",pairs(byType),QColor("#00a86b")),0,1);
    grid->addWidget(new EquipmentBarChart("Equipment by station",pairs(byStation),QColor("#7948ff")),1,0,1,2);
    layout->addLayout(grid,1);
    auto *close=button("Close",""); close->setObjectName("closeEquipmentStatisticsButton"); layout->addWidget(close,0,Qt::AlignRight);
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); dialog.exec();
}


class EquipmentTrend final : public QWidget {
public:
    explicit EquipmentTrend(QWidget *parent=nullptr):QWidget(parent) {
        setMinimumSize(235,142);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }
    void setEquipment(const EquipmentRecord &value, const QString &range) {
        record=value; period=range; update();
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
        const QRectF plot(32,20,width()-40,height()-45);
        QFont font("Segoe UI"); font.setPixelSize(10); painter.setFont(font);
        for(int i=0;i<5;++i) {
            const qreal y=plot.bottom()-plot.height()*i/4;
            painter.setPen(QColor("#e8f0fa")); painter.drawLine(QPointF(plot.left(),y),QPointF(plot.right(),y));
            painter.setPen(muted); painter.drawText(QRectF(0,y-7,28,14),Qt::AlignRight,QString::number(i*10)+QString::fromUtf8("°C"));
        }
        const int offset=qAbs(qHash(record.id))%10;
        const int points=period=="Last 7 Days"?7:period=="Last 30 Days"?10:12;
        QPainterPath line; QVector<QPointF> coordinates;
        for(int i=0;i<points;++i) {
            const qreal value=12+(offset+i*3)%14+(record.status=="Offline"?2:0);
            const QPointF point(plot.left()+plot.width()*i/qMax(1,points-1),plot.bottom()-plot.height()*value/40);
            coordinates << point; if(i==0) line.moveTo(point); else line.lineTo(point);
        }
        QPainterPath fill=line; fill.lineTo(plot.bottomRight()); fill.lineTo(plot.bottomLeft()); fill.closeSubpath();
        QLinearGradient gradient(0,plot.top(),0,plot.bottom()); gradient.setColorAt(0,QColor(0,145,255,75)); gradient.setColorAt(1,QColor(0,145,255,6));
        painter.fillPath(fill,gradient); painter.setPen(QPen(blue,1.7)); painter.drawPath(line); painter.setBrush(blue);
        for(const QPointF &point:coordinates) painter.drawEllipse(point,1.5,1.5);
        if(!coordinates.isEmpty()) {
            const QPointF selected=coordinates.last(); painter.setPen(QPen(blue,1,Qt::DashLine)); painter.drawLine(selected,QPointF(selected.x(),plot.bottom()));
            painter.setPen(Qt::white); painter.drawEllipse(selected,4.5,4.5);
            const QString reading=QString::number(12+(offset+(points-1)*3)%14)+QString::fromUtf8("°C");
            QRectF tip(qBound(plot.left(),selected.x()-38,plot.right()-80),qMax(0.0,selected.y()-51),80,39);
            painter.setPen(Qt::NoPen); painter.setBrush(QColor("#28384e")); painter.drawRoundedRect(tip,4,4);
            painter.setPen(Qt::white); painter.drawText(tip.adjusted(7,4,-3,-18),Qt::AlignLeft,reading);
            font.setPixelSize(9); painter.setFont(font); painter.drawText(tip.adjusted(7,21,-3,-2),Qt::AlignLeft,period);
        }
    }
private:
    EquipmentRecord record;
    QString period{"Last 24 Hours"};
};

class EquipmentDetailsPanel final : public QFrame {
public:
    explicit EquipmentDetailsPanel(QWidget *parent=nullptr):QFrame(parent) {
        setObjectName("equipmentHealth"); setProperty("role","card");
        auto *box=new QVBoxLayout(this); box->setContentsMargins(14,12,14,12); box->setSpacing(10);
        auto *head=new QHBoxLayout; head->setSpacing(12);
        head->addWidget(new Glyph("thermometer",blue,50,true));
        auto *copy=new QVBoxLayout; copy->setSpacing(1); copy->addWidget(text("Equipment Health","sectionTitle"));
        subtitle=text("Select an equipment item to view its recorded details.","smallMuted"); copy->addWidget(subtitle); head->addLayout(copy,1);
        state=text("No selection","smallMuted"); state->setObjectName("equipmentHealthStatus"); state->setAlignment(Qt::AlignCenter); state->setFixedSize(110,28); head->addWidget(state);
        updated=text("","smallMuted"); head->addWidget(updated); box->addLayout(head);

        cardsGrid=new QGridLayout; cardsGrid->setSpacing(10); box->addLayout(cardsGrid);
        auto makeCard=[&](const QString &name) { auto *cardFrame=card("detail"); cardFrame->setObjectName(name); cardsGrid->addWidget(cardFrame); return cardFrame; };
        auto *identity=makeCard("equipmentIdentityCard"); auto *identityGrid=new QGridLayout(identity); identityGrid->setContentsMargins(12,10,12,10); identityGrid->setHorizontalSpacing(12); identityGrid->setVerticalSpacing(4);
        const QStringList labels{"Equipment ID","Type","Model","Serial Number","Station","Unit","Last Modification"};
        for(int i=0;i<labels.size();++i) { identityGrid->addWidget(text(labels[i],"smallMuted"),i,0); identityValues << text("—","smallMuted"); identityGrid->addWidget(identityValues.last(),i,1); }
        identityGrid->setColumnStretch(1,1);

        auto *readingCard=makeCard("equipmentReadingsCard"); auto *readingLayout=new QVBoxLayout(readingCard); readingLayout->setContentsMargins(12,8,12,8); readingLayout->setSpacing(4);
        auto *readingHead=new QHBoxLayout; readingHead->addWidget(text("Latest Readings","detailTitle"),1); readingsPeriod=selector("Last 24 Hours",true); readingsPeriod->addItems({"Last 7 Days","Last 30 Days"}); readingHead->addWidget(readingsPeriod); readingLayout->addLayout(readingHead);
        const QStringList readingLabels{"Temperature","Humidity","Wind Speed","Wind Direction","Pressure","Precipitation"};
        const QStringList glyphs{"thermometer","drop","wind","anemometer","gauge","rain"};
        for(int i=0;i<readingLabels.size();++i) { auto *row=new QHBoxLayout; row->setSpacing(8); row->addWidget(new Glyph(glyphs[i],blue,20)); row->addWidget(text(readingLabels[i],"smallMuted"),1); readingValues << text("—","readingValue"); readingValues.last()->setAlignment(Qt::AlignRight); row->addWidget(readingValues.last()); readingLayout->addLayout(row); }

        auto *trendCard=makeCard("equipmentTrendCard"); auto *trendLayout=new QVBoxLayout(trendCard); trendLayout->setContentsMargins(12,8,12,8); trendLayout->setSpacing(4);
        auto *trendHead=new QHBoxLayout; trendHead->addWidget(text("Temperature Trend","detailTitle"),1); trendPeriod=selector("Last 24 Hours",true); trendPeriod->addItems({"Last 7 Days","Last 30 Days"}); trendHead->addWidget(trendPeriod); trendLayout->addLayout(trendHead); trend=new EquipmentTrend; trendLayout->addWidget(trend,1);

        auto *recommendationCard=makeCard("equipmentRecommendationsCard"); auto *recommendationLayout=new QVBoxLayout(recommendationCard); recommendationLayout->setContentsMargins(12,8,12,8); recommendationLayout->setSpacing(7); recommendationLayout->addWidget(text("Status & Recommendations","detailTitle"));
        const QStringList titles{"Equipment Status","Calibration","Recommendations"}; const QStringList recGlyphs{"check","gear","bulb"}; const QColor colors[]{QColor("#00b864"),QColor("#0d47a1"),QColor("#ffaf00")};
        for(int i=0;i<titles.size();++i) { auto *row=new QHBoxLayout; row->setSpacing(10); row->addWidget(new Glyph(recGlyphs[i],colors[i],32,true),0,Qt::AlignTop); auto *copyRow=new QVBoxLayout; copyRow->setSpacing(1); copyRow->addWidget(text(titles[i],"detailTitle")); recommendationValues << text("—","recommendationCopy"); recommendationValues.last()->setWordWrap(true); copyRow->addWidget(recommendationValues.last()); row->addLayout(copyRow,1); recommendationLayout->addLayout(row); }
        cardsGrid->setColumnStretch(0,1); cardsGrid->setColumnStretch(1,1); cardsGrid->setColumnStretch(2,1); cardsGrid->setColumnStretch(3,1);
        QObject::connect(readingsPeriod,&QComboBox::currentTextChanged,this,[this](const QString &value){ if(trendPeriod->currentText()!=value) trendPeriod->setCurrentText(value); updatePanel(); });
        QObject::connect(trendPeriod,&QComboBox::currentTextChanged,this,[this](const QString &value){ if(readingsPeriod->currentText()!=value) readingsPeriod->setCurrentText(value); updatePanel(); });
    }
    void setEquipment(const EquipmentRecord *value) { selected=value?*value:EquipmentRecord{}; hasSelection=value; updatePanel(); }
protected:
    void resizeEvent(QResizeEvent *event) override { QFrame::resizeEvent(event); updateGrid(width()<1120?2:4); }
private:
    void updateGrid(int columns) {
        QList<QWidget *> cards{findChild<QWidget *>("equipmentIdentityCard"),findChild<QWidget *>("equipmentReadingsCard"),findChild<QWidget *>("equipmentTrendCard"),findChild<QWidget *>("equipmentRecommendationsCard")};
        for(QWidget *cardWidget:cards) if(cardWidget) cardsGrid->removeWidget(cardWidget);
        for(int i=0;i<cards.size();++i) if(cards[i]) cardsGrid->addWidget(cards[i],i/columns,i%columns);
        for(int i=0;i<4;++i) cardsGrid->setColumnStretch(i,i<columns?1:0);
    }
    void updatePanel() {
        if(!hasSelection) {
            subtitle->setText("Select an equipment item to view its recorded details."); state->setText("No selection"); updated->clear();
            for(QLabel *label:identityValues) label->setText("—");
            for(QLabel *label:readingValues) label->setText("—");
            for(QLabel *label:recommendationValues) label->setText("—");
            trend->setEquipment({},trendPeriod->currentText());
            return;
        }
        subtitle->setText(QString("Recorded status and latest available readings for %1.").arg(selected.id));
        const QColor color=selected.status=="Online"?QColor("#16a34a"):selected.status=="Offline"?QColor("#ef4444"):QColor("#d97706");
        state->setText(selected.status); state->setStyleSheet(QString("border-radius:14px;background:%1;color:%2;").arg(selected.status=="Online"?"#dcfce7":selected.status=="Offline"?"#fee2e2":"#fef3c7",color.name()));
        updated->setText("Last modified "+QDateTime::fromString(selected.updated,Qt::ISODate).toString("dd MMM yyyy HH:mm"));
        const QStringList identity{selected.id,selected.type,selected.model,selected.serialNumber,selected.station.simplified(),selected.unit,QDateTime::fromString(selected.updated,Qt::ISODate).toString("dd MMM yyyy HH:mm")};
        for(int i=0;i<identity.size();++i) identityValues[i]->setText(identity[i]);
        const int base=11+qAbs(qHash(selected.id))%12;
        const QStringList readings{QString::number(base)+QString::fromUtf8(" °C"),QString::number(55+base)+" %",QString::number(4+base%9)+" m/s",QString::number(180+base*4)+QString::fromUtf8("° (SW)"),QString::number(1005+base)+" hPa",QString::number(base%4)+".0 mm"};
        for(int i=0;i<readings.size();++i) readingValues[i]->setText(readings[i]);
        recommendationValues[0]->setText(selected.status=="Online"?"Normal operation\nThis equipment is recorded as available.":selected.status=="Offline"?"Requires verification\nCheck reception or the device connection.":"Maintenance recorded\nKeep the intervention status under review.");
        recommendationValues[1]->setText("No calibration schedule is stored for this equipment.");
        recommendationValues[2]->setText(selected.status=="Online"?"No immediate action required.":"Review this equipment before the next report.");
        trend->setEquipment(selected,trendPeriod->currentText());
    }
    QLabel *subtitle=nullptr,*state=nullptr,*updated=nullptr; QList<QLabel *> identityValues,readingValues,recommendationValues; QComboBox *readingsPeriod=nullptr,*trendPeriod=nullptr; EquipmentTrend *trend=nullptr; QGridLayout *cardsGrid=nullptr; EquipmentRecord selected; bool hasSelection=false;
};

struct ReplacementCandidate {
    QString id, model, location, measureRange, connection, reason;
};

QVector<ReplacementCandidate> replacementCandidatesFor(const EquipmentRecord &equipment)
{
    const QMap<QString, QVector<ReplacementCandidate>> candidates{
        {"Weather Station", {{"RSV-101", "Vaisala WXT536", "Central reserve", "Multi-sensor weather data", "RS-485", "Same equipment type and compatible multi-sensor connection."}}},
        {"Anemometer", {{"RSV-203", "WindSonic 2", "North warehouse", "0–60 m/s", "RS-485", "Same type, same wind-speed range and compatible connection."}, {"RSV-204", "Gill MaxiMet", "Central reserve", "0–60 m/s", "RS-485", "Compatible wind-speed range and RS-485 connection."}}},
        {"Rain Gauge", {{"RSV-305", "RG3-M", "Central reserve", "0–500 mm/h", "Pulse output", "Same rain-gauge type, compatible measurement range and output."}}},
        {"Pyranometer", {{"RSV-407", "CMP11", "South warehouse", "0–4000 W/m²", "Analog mV", "Same pyranometer type and compatible irradiance range."}}},
        {"Barometer", {{"RSV-509", "PTB330", "Central reserve", "500–1100 hPa", "RS-485", "Same barometer type, pressure range and connection."}}}
    };
    QVector<ReplacementCandidate> available;
    for(const auto &candidate:candidates.value(equipment.type)) {
        bool alreadyDeployed=false;
        for(const auto &record:equipmentRecords) {
            if(record.serialNumber.compare(candidate.id,Qt::CaseInsensitive)==0) { alreadyDeployed=true; break; }
        }
        if(!alreadyDeployed) available.append(candidate);
    }
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
        emptyLayout->addWidget(text("No available reserve item matches the selected equipment type. The responsible person can keep this equipment under review or prepare a procurement request.", "smallMuted"));
        layout->addWidget(empty,1);
        auto *close=button("Close",""); layout->addWidget(close,0,Qt::AlignRight); QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);
        dialog.exec(); return;
    }

    layout->addWidget(text(QString("%1 compatible replacement%2 available in reserve").arg(candidates.size()).arg(candidates.size()==1?"":"s"),"detailTitle"));
    auto *split=new QHBoxLayout; split->setSpacing(14);
    auto *list=new QListWidget; list->setObjectName("replacementList"); list->setMinimumWidth(330);
    for(const auto &candidate:candidates) {
        auto *item=new QListWidgetItem(candidate.id+"  ·  "+candidate.model+"\n"+candidate.location+"  ·  Available in reserve");
        item->setData(Qt::UserRole,candidate.id); list->addItem(item);
    }
    split->addWidget(list,1);
    auto *candidateCard=card("detail"); candidateCard->setObjectName("replacementCandidateCard"); auto *candidateLayout=new QVBoxLayout(candidateCard); candidateLayout->setContentsMargins(18,16,18,16); candidateLayout->setSpacing(8);
    candidateLayout->addWidget(text("Compatibility comparison", "detailTitle"));
    auto *candidateId=text("", "sectionTitle"); candidateLayout->addWidget(candidateId);
    auto *candidateModel=text("", "smallMuted"); candidateLayout->addWidget(candidateModel);
    auto *reserve=text("", "body"); candidateLayout->addWidget(reserve);
    auto *range=text("", "body"); candidateLayout->addWidget(range);
    auto *connection=text("", "body"); candidateLayout->addWidget(connection);
    auto *reason=text("", "smallMuted"); reason->setWordWrap(true); candidateLayout->addWidget(reason); candidateLayout->addStretch();
    split->addWidget(candidateCard,1); layout->addLayout(split,1);

    auto updateCandidate=[&](int row) {
        if(row<0 || row>=candidates.size()) return;
        const auto &candidate=candidates[row];
        candidateId->setText(candidate.id+" — compatible"); candidateModel->setText(candidate.model);
        reserve->setText("Reserve location: "+candidate.location+" · Available now");
        range->setText("Measurement range: "+candidate.measureRange); connection->setText("Connection: "+candidate.connection);
        reason->setText("Why it matches: "+candidate.reason);
    };
    QObject::connect(list,&QListWidget::currentRowChanged,&dialog,updateCandidate); list->setCurrentRow(0);
    auto *actions=new QHBoxLayout; actions->addStretch(); auto *cancel=button("Cancel",""); auto *confirm=button("Choose replacement","replace",true); confirm->setObjectName("confirmReplacementButton"); actions->addWidget(cancel); actions->addWidget(confirm); layout->addLayout(actions);
    QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    QObject::connect(confirm,&QPushButton::clicked,&dialog,[&]{
        const int row=list->currentRow(); if(row<0) return;
        if(!applyReplacement(candidates[row])) return;
        QMessageBox confirmation(&dialog); confirmation.setWindowTitle("Replacement deployed");
        confirmation.setText(candidates[row].id+" has been deployed as a replacement.");
        confirmation.setInformativeText(equipment.id+" is kept in the inventory with Maintenance status. A new Online equipment record was added for the replacement at "+equipment.station.simplified()+".");
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
        painter->fillRect(rect,Qt::white);
        painter->setPen(today && !selected?QPen(blue,1.3):QPen(Qt::NoPen));
        painter->setBrush(selected?blue:today?QColor("#eef6ff"):QColor(Qt::white));
        painter->drawRoundedRect(cell,8,8);
        QFont font("Segoe UI"); font.setPixelSize(14); font.setBold(selected || today); painter->setFont(font);
        painter->setPen(selected?Qt::white:!inMonth?QColor("#a1aec0"):day.dayOfWeek()>5?QColor("#62738c"):navy);
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
        const bool loaded=inventory.load(sampleRecords,&loadError);
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
        outer->addWidget(top);
        auto *scroll=new QScrollArea; scroll->setObjectName("equipmentScroll"); scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
        auto *content=new QWidget; content->setObjectName("equipmentContent");
        body=new QVBoxLayout(content); body->setContentsMargins(24,14,24,22); body->setSpacing(12);
        auto *pageHeader=new QGridLayout; pageHeader->setContentsMargins(0,0,0,0); pageHeader->setHorizontalSpacing(14); pageHeader->setVerticalSpacing(2);
        pageHeader->addWidget(text("Equipment","pageTitle"),0,0);
        auto *subtitle=text("Monitor weather equipment health, connectivity, calibration, and latest readings.","subtitle");
        subtitle->setWordWrap(true); pageHeader->addWidget(subtitle,1,0);
        auto *actions=new QHBoxLayout; actions->setSpacing(8);
        auto *databaseButton=button("Oracle Connection","database"); databaseButton->setObjectName("oracleConnectionButton");
        databaseButton->setToolTip("Enter your Oracle account parameters and test the database connection.");
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
        actions->addWidget(databaseButton); actions->addWidget(exportButton); actions->addWidget(statisticsButton); actions->addWidget(replacementButton); actions->addWidget(editButton); actions->addWidget(deleteButton); actions->addWidget(addButton);
        pageHeader->addLayout(actions,0,1,2,1,Qt::AlignRight|Qt::AlignVCenter); pageHeader->setColumnStretch(0,1);
        body->addLayout(pageHeader);
        auto *cards=new QHBoxLayout; cards->setSpacing(10);
        metrics={metricCard("Registered Equipment","0","in inventory","Total inventory","equipment",blue,"blue"),
                 metricCard("Online Equipment","0","/ 0 total","Recorded status","wifi",QColor("#00b863"),"green"),
                 metricCard("Offline Equipment","0","/ 0 total","Needs attention","offline",QColor("#ff3859"),"red"),
                 metricCard("In Maintenance","0","/ 0 total","Under maintenance","gear",QColor("#7948ff"),"purple")};
        for(auto *m:metrics) cards->addWidget(m,1);
        body->addLayout(cards);
        query=search("Search ID, type, model, station or serial number...");
        query->setObjectName("equipmentSearch"); query->setClearButtonEnabled(true); body->addWidget(query);
        auto *filters=new QHBoxLayout; filters->setSpacing(10);
        typeFilter=selector("All Types"); typeFilter->setObjectName("typeFilter"); typeFilter->addItems(types);
        stationFilter=selector("All Stations"); stationFilter->setObjectName("stationFilter");
        QStringList uniqueStations=stations; uniqueStations.removeDuplicates();
        for(const auto &record:equipmentRecords) if(!uniqueStations.contains(record.station)) uniqueStations.append(record.station);
        for(const auto &station:uniqueStations) stationFilter->addItem(station.simplified(),station);
        statusFilter=selector("All Statuses"); statusFilter->setObjectName("statusFilter"); statusFilter->addItems({"Online","Offline","Maintenance"});
        auto *clear=button("Clear filters",""); clear->setObjectName("clearEquipmentFilters"); clear->setProperty("link",true);
        filters->addWidget(typeFilter,1); filters->addWidget(stationFilter,1); filters->addWidget(statusFilter,1); filters->addWidget(clear);
        body->addLayout(filters);
        resultLabel=text("","smallMuted"); resultLabel->setObjectName("equipmentResultCount"); body->addWidget(resultLabel);
        table=new EquipmentTable; body->addWidget(table);
        feedback=text("Changes are saved locally on this computer.","smallMuted"); feedback->setObjectName("equipmentFeedback");
        feedback->setWordWrap(true); feedback->setToolTip(inventory.path()); body->addWidget(feedback);
        detailsPanel=new EquipmentDetailsPanel; body->addWidget(detailsPanel); body->addStretch(); scroll->setWidget(content);
        table->changed=[this](const QString &message){feedback->setText(message); refreshMetrics(); applyFilters();};
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
        QObject::connect(databaseButton,&QPushButton::clicked,this,[this]{showOracleConnectionDialog(this);});
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
                    const EquipmentRecord replacement{inventory.nextId(),record.type,record.station,record.unit,timestamp,"Online",candidate.model,candidate.id};
                    changedRecords.append(replacement);
                    return table->commit(changedRecords,QString("%1 deployed at %2; %3 moved to Maintenance.").arg(candidate.id,record.station.simplified(),record.id),replacement.id);
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
            if(records.isEmpty()) return;
            const QString scope=exportSelection?QString("%1 selected equipment record%2").arg(records.size()).arg(records.size()==1?"":"s"):
                QString("complete inventory (%1 records)").arg(records.size());
            QString path=QFileDialog::getSaveFileName(this,exportSelection?"Export selected equipment":"Export complete equipment inventory",
                (exportSelection?"SkySense_Selected_Equipment_":"SkySense_Equipment_")+reportDate->date().toString("yyyy-MM-dd")+".pdf","PDF document (*.pdf)");
            if(path.isEmpty()) return;
            if(!path.endsWith(".pdf",Qt::CaseInsensitive)) {
                path+=".pdf";
                if(QFileInfo::exists(path) && QMessageBox::question(this,"Replace existing PDF", "A PDF already exists at this location. Replace it?",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes) return;
            }
            QString error;
            if(!exportEquipmentPdf(path,records,reportDate->date(),&error)) { QMessageBox::critical(this,"Export failed",error); return; }
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
            QTimer::singleShot(0,this,[this,loadError]{QMessageBox::critical(this,"Inventory could not be loaded",loadError+"\n\n"+inventory.path());});
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
    void updateSelection() {
        const QString id=table->selectedId();
        editButton->setEnabled(!id.isEmpty()); deleteButton->setEnabled(!id.isEmpty()); replacementButton->setEnabled(!id.isEmpty());
        detailsPanel->setEquipment(nullptr);
        for(const auto &r:equipmentRecords) if(r.id==id) {
            detailsPanel->setEquipment(&r);
            break;
        }
    }
    void refreshMetrics() {
        int counts[4]{int(equipmentRecords.size()),0,0,0};
        for(const auto &r:equipmentRecords) {if(r.status=="Online")++counts[1];else if(r.status=="Offline")++counts[2];else ++counts[3];}
        for(int i=0;i<4;++i) {
            metrics[i]->findChild<QLabel *>("metricValue")->setText(QString::number(counts[i]));
            metrics[i]->findChild<QLabel *>("metricTotal")->setText(i?QString("/ %1 total").arg(counts[0]):"in inventory");
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




