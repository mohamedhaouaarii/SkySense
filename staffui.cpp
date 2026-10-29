#include "apptheme.h"
#include "staffui.h"
#include "staffdata.h"
#include "staffmodel.h"
#include <QtWidgets>
#include <algorithm>
#include <limits>

namespace {
using namespace Staff;
const QColor ink("#080d34"), blue("#006cff"), muted("#526faa"), line("#e0edfd");
constexpr int W = 1672, H = 941;
constexpr int integratedSidebarWidth = 285;

QFont uiFont(int size, bool bold = false) {
    QFont result("Segoe UI"); result.setPixelSize(size + 1);
    result.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    return result;
}
const QPixmap &reference() { static QPixmap image(":/assets/reference.png"); return image; }

void drawSymbol(QPainter &p, const QString &name, QColor color) {
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(AppTheme::pen(p, QPen(color, 1.9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)));
    p.setBrush(AppTheme::brush(p, Qt::NoBrush));
    QPainterPath path;
    if (name == "people" || name == "addperson" || name == "inactive") {
        p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, color)); p.drawEllipse(QRectF(5, 2, 9, 9));
        path.moveTo(1, 23); path.lineTo(1, 19); path.cubicTo(1, 11, 18, 11, 18, 19); path.lineTo(18, 23); path.closeSubpath(); p.drawPath(path);
        if (name == "people") { p.drawEllipse(QRectF(15, 3, 7, 8)); p.drawRoundedRect(QRectF(18, 13, 6, 9), 3, 3); }
        else { p.setPen(AppTheme::pen(p, QPen(color, 2.8, Qt::SolidLine, Qt::RoundCap)));
            p.drawLine(19, 13, 25, 13); if (name == "addperson") p.drawLine(22, 10, 22, 16);
            else { p.drawLine(20, 10, 24, 16); p.drawLine(24, 10, 20, 16); }
        }
    } else if (name == "clock") {
        p.drawEllipse(QRectF(2, 2, 20, 20)); p.drawLine(12, 6, 12, 12); p.drawLine(12, 12, 17, 15);
    } else if (name == "pin") {
        p.setBrush(AppTheme::brush(p, color)); p.setPen(AppTheme::pen(p, Qt::NoPen)); path.moveTo(12, 24); path.cubicTo(8, 18, 3, 13, 3, 9); path.cubicTo(3, -2, 21, -2, 21, 9); path.cubicTo(21, 13, 16, 19, 12, 24); p.drawPath(path);
        p.setBrush(AppTheme::brush(p, QColor("#08223a"))); p.drawEllipse(QRectF(8, 5, 8, 8));
    } else if (name == "equipment") {
        p.setBrush(AppTheme::brush(p, color)); p.setPen(AppTheme::pen(p, Qt::NoPen));
        for (int y : {3, 15}) { p.drawRoundedRect(QRectF(1, y, 22, 8), 3, 3); p.setBrush(AppTheme::brush(p, QColor("#08223a"))); p.drawEllipse(QRectF(4, y+2, 4, 4)); p.drawRect(QRectF(18, y+3, 2, 2)); p.setBrush(AppTheme::brush(p, color)); }
    } else if (name == "tool") {
        path.moveTo(16, 2); path.cubicTo(8, -1, 7, 6, 9, 10); path.lineTo(1, 19); path.lineTo(5, 23); path.lineTo(14, 14); path.cubicTo(23, 17, 26, 6, 21, 3); path.lineTo(17, 8); path.lineTo(13, 6); path.closeSubpath(); p.drawPath(path);
    } else if (name == "grid") {
        p.setBrush(AppTheme::brush(p, color)); p.setPen(AppTheme::pen(p, Qt::NoPen)); for (int x : {2, 14}) for (int y : {2, 14}) p.drawRoundedRect(QRectF(x, y, 9, 9), 2, 2);
    } else if (name == "search") {
        p.drawEllipse(QRectF(3, 3, 13, 13)); p.drawLine(15, 15, 22, 22);
    } else if (name == "calendar") {
        p.drawRoundedRect(QRectF(3, 4, 18, 18), 1, 1); p.drawLine(3, 9, 21, 9); p.drawLine(7, 1, 7, 6); p.drawLine(17, 1, 17, 6); p.drawPoint(8, 13); p.drawPoint(13, 13); p.drawPoint(8, 18);
    } else if (name == "down") { p.drawLine(7, 10, 12, 15); p.drawLine(12, 15, 17, 10);
    } else if (name == "left" || name == "right") {
        int a = name == "left" ? 15 : 9, b = name == "left" ? 9 : 15; p.drawLine(a, 6, b, 12); p.drawLine(b, 12, a, 18);
    } else if (name == "filter") {
        path.moveTo(2, 3); path.lineTo(22, 3); path.lineTo(15, 12); path.lineTo(15, 21); path.lineTo(10, 19); path.lineTo(10, 12); path.closeSubpath(); p.drawPath(path);
    } else if (name == "document" || name == "clipboard") {
        p.drawRoundedRect(QRectF(5, 3, 15, 20), 1.5, 1.5);
        if (name == "clipboard") { p.setBrush(AppTheme::brush(p, Qt::white)); p.drawRoundedRect(QRectF(9, 1, 7, 5), 1, 1); }
        else { p.drawLine(14, 3, 14, 8); p.drawLine(14, 8, 20, 8); }
        p.drawLine(9, 12, 16, 12); p.drawLine(9, 16, 16, 16);
    } else if (name == "plus") { p.drawLine(12, 4, 12, 20); p.drawLine(4, 12, 20, 12);
    } else if (name == "mail") {
        p.drawRoundedRect(QRectF(2, 5, 20, 15), 1, 1); p.drawLine(2, 6, 12, 13); p.drawLine(12, 13, 22, 6);
    } else if (name == "phone") {
        path.moveTo(4, 2); path.lineTo(9, 7); path.lineTo(6, 10); path.cubicTo(8, 14, 11, 17, 15, 18); path.lineTo(18, 15); path.lineTo(23, 20); path.cubicTo(17, 29, -4, 8, 4, 2); p.fillPath(path, color);
    } else if (name == "edit") {
        path.moveTo(3, 17); path.lineTo(17, 3); path.lineTo(21, 7); path.lineTo(7, 21); path.lineTo(2, 22); path.closeSubpath(); p.drawPath(path); p.drawLine(14, 6, 18, 10);
    } else if (name == "check") { p.drawLine(5, 12, 10, 17); p.drawLine(10, 17, 20, 6);
    } else if (name == "uptrend") { p.drawLine(4, 20, 20, 4); p.drawLine(10, 4, 20, 4); p.drawLine(20, 4, 20, 14);
    } else if (name == "downtrend") { p.drawLine(12, 3, 12, 21); p.drawLine(5, 14, 12, 21); p.drawLine(12, 21, 19, 14); }
}
QIcon icon(const QString &name, QColor color = blue) {
    QPixmap image(48, 48); image.fill(Qt::transparent); QPainter p(&image); p.scale(2, 2); drawSymbol(p, name, color); p.end(); image.setDevicePixelRatio(2); return QIcon(image);
}
void paintIcon(QPainter &p, const QString &name, QRectF target, QColor color = blue) {
    p.save(); p.translate(target.topLeft()); p.scale(target.width()/24, target.height()/24); drawSymbol(p, name, color); p.restore();
}
QLabel *text(QWidget *parent, QString value, QRect rect, int size = 13, QColor color = ink, bool bold = false) {
    auto *label = new QLabel(value, parent); label->setGeometry(rect); label->setTextFormat(Qt::PlainText);
    label->setStyleSheet(QString("font-family:'Segoe UI';font-size:%1px;font-weight:%2;color:%3;").arg(size + 1).arg(bold ? 600 : 400).arg(color.name()));
    return label;
}
void setFittedText(QLabel *label, const QString &value) {
    label->ensurePolished();
    label->setText(QFontMetrics(label->font()).elidedText(value, Qt::ElideRight, label->width()));
    label->setToolTip(value);
}
QPushButton *button(QWidget *parent, QString title, QString glyph, QRect rect, bool primary = false) {
    auto *b = new QPushButton(title, parent); b->setGeometry(rect); b->setProperty("primary", primary);
    if (!glyph.isEmpty()) b->setIcon(icon(glyph, primary ? Qt::white : QColor("#193c82")));
    b->setIconSize(QSize(19,19)); b->setCursor(Qt::PointingHandCursor); return b;
}
class Glyph final : public QWidget {
    QString name; QColor color;
public:
    Glyph(QWidget *parent, QString name, QRect rect, QColor color = blue): QWidget(parent), name(name), color(color) { setGeometry(rect); setAttribute(Qt::WA_TransparentForMouseEvents); }
protected: void paintEvent(QPaintEvent *) override { QPainter p(this); paintIcon(p, name, rect(), color); }
};
class Selector final : public QComboBox {
    QString glyph;
public:
    Selector(QWidget *parent, QRect rect, QStringList options, QString glyph = {}): QComboBox(parent), glyph(glyph) {
        setGeometry(rect); addItems(options); setCursor(Qt::PointingHandCursor);
        if (!glyph.isEmpty()) setStyleSheet("QComboBox { padding-left:48px; color:#08143b; }");
    }
protected:
    void paintEvent(QPaintEvent *event) override { QComboBox::paintEvent(event); QPainter p(this); paintIcon(p,"down",QRectF(width()-30,height()/2-7,14,14),QColor("#325cac")); if(!glyph.isEmpty()) paintIcon(p,glyph,QRectF(18,height()/2-10,20,20),QColor("#234887")); }
};
void rounded(QPainter &p, QRectF rect, QColor fill, QColor stroke = line, qreal radius = 8) {
    p.setPen(AppTheme::pen(p, QPen(stroke, 1))); p.setBrush(AppTheme::brush(p, fill)); p.drawRoundedRect(rect, radius, radius);
}
void badge(QPainter &p, QRectF rect, const QString &value, QColor foreground, QColor background, int size = 12) {
    p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, background)); p.drawRoundedRect(rect, rect.height()/2, rect.height()/2);
    p.setPen(AppTheme::pen(p, foreground)); p.setFont(uiFont(size));
    p.drawText(rect, Qt::AlignCenter, QFontMetrics(p.font()).elidedText(value, Qt::ElideRight, int(rect.width()) - 12));
}

QDate staffEntryDate(const QString &value) {
    QDate date=QDate::fromString(value,Qt::ISODate);
    if(!date.isValid()) date=QLocale(QLocale::English).toDate(value,"MMM d, yyyy");
    return date;
}
QString displayStaffEntryDate(const QString &value) {
    const QDate date=staffEntryDate(value);
    return date.isValid()?QLocale(QLocale::English).toString(date,"dd MMM yyyy"):QStringLiteral("Not recorded");
}

QPair<QColor,QColor> roleColors(QString role) {
    if(role=="Maintenance Lead") return {QColor("#3810ef"),QColor("#eae4ff")};
    if(role=="Data Analyst") return {QColor("#f08300"),QColor("#fff0d9")};
    if(role=="Administrator") return {QColor("#e10016"),QColor("#ffe0e3")};
    return {QColor("#005fff"),QColor("#ddecff")};
}

class StaffHeader final : public QHeaderView {
public:
    bool checked = false;
    int sortColumn = 2;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    explicit StaffHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) { setSectionsClickable(true); setFixedHeight(35); }
protected:
    void paintSection(QPainter *p, const QRect &r, int section) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->fillRect(r,AppTheme::brush(*p, QColor("#f3f8ff"))); p->setPen(AppTheme::pen(*p, QColor("#e9f1fc"))); p->drawLine(r.bottomLeft(),r.bottomRight());
        if(section) p->drawLine(r.topLeft()+QPoint(0,5),r.bottomLeft()-QPoint(0,5));
        if(section==0) { rounded(*p,QRectF(r.left()+14,r.top()+10,16,16),checked?blue:QColor(Qt::white),checked?blue:QColor("#a3b1c6"),3); if(checked) paintIcon(*p,"check",QRectF(r.left()+14,r.top()+10,16,16),Qt::white); }
        else { p->setFont(uiFont(12)); p->setPen(AppTheme::pen(*p, QColor("#173873"))); const QString title=model()->headerData(section,Qt::Horizontal).toString(); p->drawText(r.adjusted(16,0,-5,0),Qt::AlignVCenter,title);
            if(section==sortColumn) { const int x=r.right()-12, y=r.center().y(); const int d=sortOrder==Qt::AscendingOrder?-1:1; p->setPen(AppTheme::pen(*p, QPen(blue,1.8))); p->drawLine(x,y-5,x,y+5); p->drawLine(x-3,y+2*d,x,y+5*d); p->drawLine(x,y+5*d,x+3,y+2*d); }
        } p->restore();
    }
};
class StaffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); p->setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect r=option.rect; const bool selected=option.state & QStyle::State_Selected; const int c=index.column();
        p->fillRect(r,AppTheme::brush(*p, selected?QColor("#e0f1ff"):QColor("#ffffff"))); p->setPen(AppTheme::pen(*p, QColor("#eaf1fc"))); p->drawLine(r.bottomLeft(),r.bottomRight());
        p->setFont(uiFont(12)); p->setPen(AppTheme::pen(*p, QColor("#274989")));
        if(c==0) { const bool checked=index.data(Qt::CheckStateRole).toInt()==Qt::Checked; const QRectF box(r.left()+14,r.center().y()-8,16,16); rounded(*p,box,checked?blue:QColor(Qt::white),checked?blue:QColor("#a6b3c5"),3); if(checked)paintIcon(*p,"check",box,Qt::white); }
        else if(c==2) { p->setPen(AppTheme::pen(*p, ink)); const QRect copy=r.adjusted(16,0,-4,0);p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width())); }
        else if(c==4) { const auto colors=roleColors(index.data().toString()); badge(*p,QRectF(r.left()+15,r.center().y()-12,137,25),index.data().toString(),colors.first,colors.second); }
        else if(c==6) { bool active=index.data().toString()=="Active"; badge(*p,QRectF(r.left()+15,r.center().y()-12,94,25),index.data().toString(),QColor(active?"#008e3f":"#db001a"),QColor(active?"#d9fbeb":"#ffe1e4")); }
        else if(c==7) { p->setPen(AppTheme::pen(*p, Qt::NoPen)); p->setBrush(AppTheme::brush(*p, QColor("#254385"))); for(int x:{-5,0,5})p->drawEllipse(QPointF(r.center().x()+x,r.center().y()),1.3,1.3); }
        else { const QRect copy=r.adjusted(16,0,-4,0);p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width())); }
        p->restore();
    }
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (index.column() != 0 || !(index.flags() & Qt::ItemIsUserCheckable))
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::MouseButtonDblClick) {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            const QRect hitBox(option.rect.left() + 10, option.rect.center().y() - 12, 24, 24);
            if (mouse->button() != Qt::LeftButton || !hitBox.contains(mouse->position().toPoint())) return false;
            if (event->type() != QEvent::MouseButtonRelease) return true;
        } else if (event->type() == QEvent::KeyPress) {
            const int key = static_cast<QKeyEvent *>(event)->key();
            if (key != Qt::Key_Space && key != Qt::Key_Select) return false;
        } else return false;
        return model->setData(index, index.data(Qt::CheckStateRole).toInt() == Qt::Checked
                             ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
    }
};

class StaffCanvas final : public QWidget {
    State data;
    QList<Member> &members=data.members;
    bool writable=true;
    QList<int> filtered;
    QSet<QString> checked;
    QString selectedId;
    int page=0, tab=0;
    QLineEdit *search{};
    Selector *roleFilter{},*statusFilter{},*listRoles{};
    QTableWidget *table{};
    StaffHeader *header{};
    QLabel *pagination{},*name{},*role{},*description{},*status{},*email{},*phone{},*joined{};
    QPushButton *previous{},*next{},*editStaffButton{},*addStaffButton{},*deleteStaffButton{},*smallEditButton{};
    QLabel *emptyList{};
    QList<QPushButton*> tabs;
    QList<QLabel*> infoValues;
    QWidget *overview{}, *tabContent{};
    QTextBrowser *tabBrowser{};
    QDateEdit *availabilityStart{},*availabilityEnd{},*availabilityDate{};
    QCheckBox *availabilityEnabled{};
    QLabel *availabilityState{},*reviewSummary{};
    QPushButton *reviewButton{};
    QLabel *storageStatus{};
    StaffModel staffModel;   // all SQL lives in staffmodel.cpp
    bool oracleLive=false;
public:
    explicit StaffCanvas(QWidget *parent=nullptr):QWidget(parent) {
        setObjectName("staffCanvas"); setFixedSize(W-integratedSidebarWidth,H);
        QFile sheet(":/styles/staff.qss"); if(sheet.open(QIODevice::ReadOnly))setStyleSheet(QString::fromUtf8(sheet.readAll()));
        QString error;
        {
            data.members={
                {"STF-2025-012","Alex Martin","alex.martin@skysense.com","Operations Manager","Active","+216 20 000 001","2022-01-14",QDate(),QDate()},
                {"STF-2025-011","Sophie Chen","sophie.chen@skysense.com","Station Technician","Active","+216 20 000 002","2023-03-10",QDate(),QDate()},
                {"STF-2025-010","James Wilson","james.wilson@skysense.com","Maintenance Lead","Active","+216 20 000 003","2021-09-05",QDate(),QDate()},
                {"STF-2025-009","Emma Davis","emma.davis@skysense.com","Data Analyst","Active","+216 20 000 004","2024-02-14",QDate(),QDate()},
                {"STF-2025-008","Ryan Kim","ryan.kim@skysense.com","Technical Specialist","Active","+216 20 000 005","2024-01-15",QDate(),QDate()},
                {"STF-2025-007","Olivia Martinez","olivia.martinez@skysense.com","Support Specialist","Inactive","+216 20 000 006","2023-05-20",QDate(),QDate()}
            };
            data.nextId=13;
            writable=true;
            // READ: when Oracle is connected the list comes from SKYSENSE_STAFF
            // (the demo members above are only used with --skip-oracle / no connection).
            if(StaffModel::isConnected()){
                QList<Member> rows; QString dbError;
                if(staffModel.afficher(&rows,&dbError)){
                    data.members=rows; oracleLive=true; int highest=0;
                    for(const Member &m:rows) highest=qMax(highest,m.id.section('-',-1).toInt());
                    data.nextId=highest+1;
                } else qWarning()<<"Staff: could not read SKYSENSE_STAFF:"<<dbError;
            }
        }
        buildHeader(); buildList(); buildDetails();
        // This Staff module runs inside the common workspace shell. Its local
        // search/date/operator strip duplicates that shell's header, so retain
        // the shared search target but hide its embedded visual controls.
        for (const QString &name : {QStringLiteral("staffSearch"), QStringLiteral("staffLocalCalendarIcon"),
                                    QStringLiteral("staffLocalDateLabel"), QStringLiteral("staffLocalDateArrow"),
                                    QStringLiteral("staffOperatorName"), QStringLiteral("staffLocalOperatorRole"),
                                    QStringLiteral("staffLocalProfileArrow")})
            if (auto *child = findChild<QWidget *>(name, Qt::FindDirectChildrenOnly)) child->hide();
        // The shared application shell owns navigation. Move the Staff
        // content into its own local coordinates instead of clipping a full
        // standalone window, which could paint over the shared sidebar.
        for (auto *child : findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
            child->move(child->x()-integratedSidebarWidth, child->y());
        refresh();
        if (storageStatus) storageStatus->setText(oracleLive?"Oracle":"Demo mode");
    }
private:
    void refreshAfterEdit(bool reveal=false) {
        // Demo edits remain in this session only.
        populateRoles();refresh(reveal);
    }
    Member *selected() { for(auto &m:members)if(m.id==selectedId)return &m;return nullptr; }
    void buildSidebar() {
        const QStringList names{"Stations","Equipment","Maintenance","Staff","Services"}, icons{"pin","equipment","tool","people","grid"};
        for(int i=0;i<5;++i) {
            auto *b=button(this,"",{},QRect(11,154+i*60,264,50)); b->setProperty("nav",true); b->setProperty("active",i==3);
            new Glyph(b,icons[i],QRect(20,12,27,27),i==3?Qt::white:QColor("#bbd9f6")); text(b,names[i],QRect(75,0,170,50),17,Qt::white);
            connect(b,&QPushButton::clicked,this,[this,i]{if(i!=3)QMessageBox::information(this,"SkySense", "This standalone project contains the Staff module. Connect this navigation button to your "+QStringList{"Stations","Equipment","Maintenance","Staff","Services"}[i]+" page in the main project.");});
        }
        text(this,"Staff data",QRect(58,850,200,25),12,QColor("#bdd0e9"));
        storageStatus=text(this,writable?"Preview data":"Preview mode",QRect(58,877,222,23),12,Qt::white);
        storageStatus->setToolTip("Personnel data preview status.");
        text(this,"Demo data · session only",QRect(58,900,222,23),12,QColor("#abc7e5"));
    }
    void buildHeader() {
        search=new QLineEdit(this); search->setObjectName("staffSearch"); search->setGeometry(315,15,501,40);
        search->setPlaceholderText("Search by name, email, function, or status..."); search->setStyleSheet("QLineEdit { padding-left:45px; font-size:14px; background:#f4f8ff; }");
        search->setClearButtonEnabled(true);search->setAccessibleName("Search staff");
        auto searchPalette=search->palette();searchPalette.setColor(QPalette::PlaceholderText,muted);search->setPalette(searchPalette);
        new Glyph(search,"search",QRect(14,9,20,20),QColor("#7187c1"));
        auto *stats=button(this,"Statistics","grid",QRect(974,292,138,33));stats->setObjectName("staffStatistics");connect(stats,&QPushButton::clicked,this,[this]{showStatistics();});
        auto *localCalendarIcon=new Glyph(this,"calendar",QRect(1185,24,24,24),QColor("#264788")); localCalendarIcon->setObjectName("staffLocalCalendarIcon");
        auto *localDateLabel=text(this,QLocale(QLocale::English).toString(QDate::currentDate(),"MMM d, yyyy"),QRect(1223,19,131,32),14,ink,true); localDateLabel->setObjectName("staffLocalDateLabel");
        auto *localDateArrow=new Glyph(this,"down",QRect(1350,29,14,14),muted); localDateArrow->setObjectName("staffLocalDateArrow");
        auto *operatorLabel=text(this,"",QRect(1413,13,200,24),14,ink,true);
        operatorLabel->setObjectName("staffOperatorName");
        setFittedText(operatorLabel,operatorName());
        auto *localOperatorRole=text(this,"Local operator",QRect(1413,35,200,22),13,muted); localOperatorRole->setObjectName("staffLocalOperatorRole");
        auto *localProfileArrow=new Glyph(this,"down",QRect(1626,25,18,18),ink); localProfileArrow->setObjectName("staffLocalProfileArrow");
        auto *pageTitle=text(this,"Staff",QRect(317,27,470,48),34,QColor("#080d2d"),true);
        pageTitle->setObjectName("staffPageTitle");
        pageTitle->setStyleSheet("font-family:'Segoe UI';font-size:34px;font-weight:700;color:#080d2d;");
        auto *pageSubtitle=text(this,"Manage employee records, professional roles, and availability.",QRect(318,75,740,25),16,muted);
        pageSubtitle->setObjectName("staffPageSubtitle");
        roleFilter=new Selector(this,QRect(1063,46,139,43),{"Function"},"filter"); roleFilter->setObjectName("roleFilter");
        statusFilter=new Selector(this,QRect(1216,46,139,43),{"Status","Active","Inactive"},"filter"); statusFilter->setObjectName("statusFilter");
        auto *exportButton=button(this,"Export PDF","document",QRect(1369,46,135,43));
        exportButton->setObjectName("exportStaffPdf");
        connect(exportButton,&QPushButton::clicked,this,[this]{exportPdf();});
        addStaffButton=button(this,"Add Staff","plus",QRect(1518,46,132,43),true); addStaffButton->setObjectName("addStaff");addStaffButton->setEnabled(writable);connect(addStaffButton,&QPushButton::clicked,this,[this]{editMember(true);});
        connect(search,&QLineEdit::textChanged,this,[this]{page=0;refresh();});
        connect(roleFilter,&QComboBox::currentIndexChanged,this,[this](int index){QSignalBlocker block(listRoles);listRoles->setCurrentIndex(index);page=0;refresh();});
        connect(statusFilter,&QComboBox::currentIndexChanged,this,[this]{page=0;refresh();});
    }
    void buildList() {
        new Glyph(this,"clipboard",QRect(334,293,26,26));
        text(this,"Staff List",QRect(377,286,300,27),19,ink,true);
        text(this,"View and manage staff records, professional functions, and availability.",QRect(377,310,730,22),13,muted);
        listRoles=new Selector(this,QRect(1334,292,132,33),{"All Functions"}); listRoles->setObjectName("listRoleFilter"); listRoles->setStyleSheet("font-size:12px;");
        populateRoles();
        pagination=text(this,"1-8 of 12",QRect(1498,292,67,33),12,muted);
        previous=button(this,"","left",QRect(1565,292,33,33)); next=button(this,"","right",QRect(1603,292,33,33)); previous->setObjectName("previousPage");next->setObjectName("nextPage");
        previous->setAccessibleName("Previous staff page");previous->setToolTip("Previous page");
        next->setAccessibleName("Next staff page");next->setToolTip("Next page");
        connect(previous,&QPushButton::clicked,this,[this]{if(page>0){--page;refresh();}});
        connect(next,&QPushButton::clicked,this,[this]{if((page+1)*8<filtered.size()){++page;refresh();}});
        connect(listRoles,&QComboBox::currentIndexChanged,this,[this](int index){QSignalBlocker block(roleFilter);roleFilter->setCurrentIndex(index);page=0;refresh();});
        table=new QTableWidget(0,8,this); table->setObjectName("staffTable");table->setGeometry(324,335,1317,307);
        header=new StaffHeader(table);table->setHorizontalHeader(header);
        table->setHorizontalHeaderLabels({"","Staff ID","Nom et prénom","Email","Function","Date d'entrée","Status","Actions"});
        for(int column=1;column<=6;++column)table->horizontalHeaderItem(column)->setToolTip("Click to sort ascending / descending across all pages.");
        table->verticalHeader()->hide();table->verticalHeader()->setDefaultSectionSize(34);table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        const int widths[]{50,126,210,260,210,150,130,181}; for(int c=0;c<8;++c)table->setColumnWidth(c,widths[c]);
        table->setShowGrid(false);table->setFrameShape(QFrame::NoFrame);table->setSelectionBehavior(QAbstractItemView::SelectRows);table->setSelectionMode(QAbstractItemView::SingleSelection);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->setFocusPolicy(Qt::StrongFocus);table->setItemDelegate(new StaffDelegate(table));table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        emptyList=text(table->viewport(),"No staff match your search or filters.",QRect(200,90,900,45),16,muted);
        emptyList->setObjectName("emptyStaffList");emptyList->setAlignment(Qt::AlignCenter);emptyList->setAttribute(Qt::WA_TransparentForMouseEvents);emptyList->hide();
        connect(table,&QTableWidget::itemSelectionChanged,this,[this]{const auto rows=table->selectionModel()->selectedRows(1);if(!rows.isEmpty()){selectedId=rows.first().data().toString();updateDetails();}});
        connect(table,&QTableWidget::itemChanged,this,[this](QTableWidgetItem *item){if(item->column()!=0)return; const QString id=table->item(item->row(),1)->text();if(item->checkState()==Qt::Checked)checked.insert(id);else checked.remove(id);syncHeader();});
        connect(table,&QTableWidget::cellClicked,this,[this](int row,int column){if(column==7){selectedId=table->item(row,1)->text();showActions();}});
        connect(header,&QHeaderView::sectionClicked,this,[this](int section){
            if(section==0){const bool set=!header->checked;for(int r=0;r<table->rowCount();++r)table->item(r,0)->setCheckState(set?Qt::Checked:Qt::Unchecked);}
            else if(section<=6){header->sortOrder=section==header->sortColumn && header->sortOrder==Qt::AscendingOrder?Qt::DescendingOrder:Qt::AscendingOrder;header->sortColumn=section;page=0;refresh();}
        });
    }
    void buildDetails() {
        name=text(this,"Daniel Kim",QRect(335,670,360,30),21,ink,true);name->setObjectName("staffDetailName");
        status=text(this,"Active",QRect(515,674,83,24),12,QColor("#009347"));status->setObjectName("staffDetailStatus");status->setAlignment(Qt::AlignCenter);
        role=text(this,"Technical Specialist",QRect(335,697,450,20),13);
        description=text(this,"Professional role and employee contact details.",QRect(335,716,640,20),12,muted);
        editStaffButton=button(this,"Edit Staff","edit",QRect(990,679,114,35),true);editStaffButton->setObjectName("editStaff");connect(editStaffButton,&QPushButton::clicked,this,[this]{editMember(false);});
        new Glyph(this,"mail",QRect(1131,687,23,23),QColor("#254a90"));email=text(this,"",QRect(1165,681,162,35),12,muted);
        new Glyph(this,"phone",QRect(1354,686,23,23));phone=text(this,"",QRect(1386,681,125,35),12,muted);
        new Glyph(this,"calendar",QRect(1518,685,26,26),QColor("#254a90"));text(this,"Joined Date",QRect(1557,680,91,19),12,muted);joined=text(this,"",QRect(1557,700,91,19),12,ink,true);
        const QStringList titles{"Overview","Statistics"};const int x[]{330,490};const int widths[]{157,240};
        for(int i=0;i<titles.size();++i){auto *b=button(this,titles[i],{},QRect(x[i],741,widths[i],31));b->setProperty("tab",true);b->setProperty("active",i==0);tabs<<b;connect(b,&QPushButton::clicked,this,[this,i]{setTab(i);});}
        deleteStaffButton=button(this,"Delete Staff",{},QRect(1480,744,157,26));deleteStaffButton->setObjectName("deleteStaff");deleteStaffButton->setStyleSheet("color:#c3263a;border-color:#f3c9cf;");connect(deleteStaffButton,&QPushButton::clicked,this,[this]{deleteMember();});
        overview=new QWidget(this);overview->setObjectName("overviewPanel");overview->setGeometry(325,778,1312,162);
        new Glyph(overview,"document",QRect(13,9,23,23));text(overview,"Staff Information",QRect(44,7,245,23),14,ink,true);
        smallEditButton=button(overview,"Edit","edit",QRect(369,7,66,26));smallEditButton->setProperty("small",true);connect(smallEditButton,&QPushButton::clicked,this,[this]{editMember(false);});
        const QStringList labels{"Staff ID","Nom et prénom","Professional Function","Email","Phone"};
        for(int i=0;i<labels.size();++i){text(overview,labels[i],QRect(15,39+i*19,119,21),12,muted);infoValues<<text(overview,"",QRect(140,39+i*19,303,21),12,QColor("#233f7d"));}
        new Glyph(overview,"calendar",QRect(475,9,24,24),QColor("#254a90"));text(overview,"Unavailability period",QRect(509,7,245,23),14,ink,true);text(overview,"Demo dates remain available until the app closes.",QRect(482,120,458,20),9,muted);
        availabilityEnabled=new QCheckBox("One period",overview);availabilityEnabled->setObjectName("staffAvailabilityPeriodEnabled");availabilityEnabled->setGeometry(770,8,130,24);
        text(overview,"Start date",QRect(482,39,105,24),11,muted);availabilityStart=new QDateEdit(QDate::currentDate(),overview);availabilityStart->setObjectName("staffUnavailableFrom");availabilityStart->setCalendarPopup(true);availabilityStart->setDisplayFormat("dd MMM yyyy");availabilityStart->setGeometry(582,37,135,27);
        text(overview,"End date",QRect(728,39,85,24),11,muted);availabilityEnd=new QDateEdit(QDate::currentDate(),overview);availabilityEnd->setObjectName("staffUnavailableTo");availabilityEnd->setCalendarPopup(true);availabilityEnd->setDisplayFormat("dd MMM yyyy");availabilityEnd->setGeometry(805,37,135,27);
        text(overview,"Check date",QRect(482,77,105,24),11,muted);availabilityDate=new QDateEdit(QDate::currentDate(),overview);availabilityDate->setObjectName("staffAvailabilityCheckDate");availabilityDate->setCalendarPopup(true);availabilityDate->setDisplayFormat("dd MMM yyyy");availabilityDate->setGeometry(582,75,135,27);
        availabilityState=text(overview,"Select a staff member",QRect(728,73,211,42),12,muted);availabilityState->setObjectName("staffAvailabilityAtSelectedDate");availabilityState->setWordWrap(true);
        auto *issueGlyph=new Glyph(overview,"clipboard",QRect(905,9,24,24));Q_UNUSED(issueGlyph);text(overview,"Records to review",QRect(940,7,240,23),14,ink,true);
        reviewSummary=text(overview,"Select a member to assess record completeness.",QRect(940,39,350,45),12,muted);reviewSummary->setObjectName("staffReviewSummary");reviewSummary->setWordWrap(true);
        reviewButton=button(overview,"View incomplete / check records",{},QRect(940,96,260,32));reviewButton->setObjectName("staffReviewIncomplete");reviewButton->setProperty("small",true);connect(reviewButton,&QPushButton::clicked,this,[this]{showIncompleteRecords();});
        connect(availabilityEnabled,&QCheckBox::toggled,this,[this](bool enabled){availabilityStart->setEnabled(enabled);availabilityEnd->setEnabled(enabled);if(auto *m=selected()){if(enabled){m->unavailabilityStart=availabilityStart->date();m->unavailabilityEnd=availabilityEnd->date();}else{m->unavailabilityStart={};m->unavailabilityEnd={};}}updateAvailabilityStatus();});
        connect(availabilityStart,&QDateEdit::dateChanged,this,[this](const QDate &d){if(availabilityEnabled->isChecked())if(auto *m=selected()){m->unavailabilityStart=d;if(availabilityEnd->date()<d)availabilityEnd->setDate(d);m->unavailabilityEnd=availabilityEnd->date();updateAvailabilityStatus();}});
        connect(availabilityEnd,&QDateEdit::dateChanged,this,[this](const QDate &d){if(availabilityEnabled->isChecked())if(auto *m=selected()){m->unavailabilityEnd=d;if(availabilityStart->date()>d)availabilityStart->setDate(d);m->unavailabilityStart=availabilityStart->date();updateAvailabilityStatus();}});
        connect(availabilityDate,&QDateEdit::dateChanged,this,[this](const QDate &){updateAvailabilityStatus();});
        tabContent=new QWidget(this);tabContent->setGeometry(325,778,1312,162);tabContent->hide();
        tabBrowser=new QTextBrowser(tabContent);tabBrowser->setObjectName("staffTabContent");tabBrowser->setGeometry(12,5,1288,152);tabBrowser->setFrameShape(QFrame::NoFrame);tabBrowser->setStyleSheet("background:transparent;color:#36598d;");tabBrowser->setOpenExternalLinks(false);
    }
    void populateRoles() {
        QSignalBlocker a(roleFilter),b(listRoles); const QString current=roleFilter->currentText();
        QStringList roles;for(const auto &m:members)if(!roles.contains(m.role))roles<<m.role;
        roleFilter->clear();roleFilter->addItem("Function");roleFilter->addItems(roles);listRoles->clear();listRoles->addItem("All Functions");listRoles->addItems(roles);
        const int index=roleFilter->findText(current);if(index>0){roleFilter->setCurrentIndex(index);listRoles->setCurrentIndex(index);}
    }
    void syncHeader() { bool all=table->rowCount()>0;for(int r=0;r<table->rowCount();++r)all=all && table->item(r,0)->checkState()==Qt::Checked;header->checked=all;header->viewport()->update(); }
    void refresh(bool reveal=false) {
        if(!table)return;
        filtered.clear();
        const QString query=search->text().trimmed();
        for(int i=0;i<members.size();++i){const auto &m=members[i];if(!(m.id+" "+m.name+" "+m.email+" "+m.role+" "+m.status).contains(query,Qt::CaseInsensitive))continue;if(roleFilter->currentIndex()>0 && m.role!=roleFilter->currentText())continue;if(statusFilter->currentIndex()>0 && m.status!=statusFilter->currentText())continue;filtered<<i;}
        auto key=[this](const Member &m){switch(header->sortColumn){case 1:return m.id;case 2:return m.name;case 3:return m.email;case 4:return m.role;case 6:return m.status;default:return m.id;}};
        std::stable_sort(filtered.begin(),filtered.end(),[&](int left,int right){
            int order=0;
            if(header->sortColumn==5){
                const QDate a=staffEntryDate(members[left].joined),b=staffEntryDate(members[right].joined);
                if(a.isValid()!=b.isValid()) order=a.isValid()?-1:1;
                else if(a.isValid()&&a!=b) order=a<b?-1:1;
            } else order=QString::compare(key(members[left]),key(members[right]),Qt::CaseInsensitive);
            if(order==0)order=QString::compare(members[left].id,members[right].id,Qt::CaseInsensitive);
            return header->sortOrder==Qt::AscendingOrder?order<0:order>0;
        });
        if(reveal)for(int i=0;i<filtered.size();++i)if(members[filtered[i]].id==selectedId){page=i/8;break;}
        page=qBound(0,page,qMax(0,(int(filtered.size())-1)/8));
        const QSignalBlocker block(table);table->clearSelection();table->setRowCount(0);
        const int end=qMin((page+1)*8,int(filtered.size()));int selectedRow=-1;
        for(int n=page*8;n<end;++n){const auto &m=members[filtered[n]];const int row=table->rowCount();table->insertRow(row);
            const QStringList values{"",m.id,m.name,m.email,m.role,displayStaffEntryDate(m.joined),m.status,""};
            for(int c=0;c<8;++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);if(c==0){item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(checked.contains(m.id)?Qt::Checked:Qt::Unchecked);}if(c==7)item->setToolTip("Staff actions");table->setItem(row,c,item);}
            if(m.id==selectedId)selectedRow=row;
        }
        if(table->rowCount()){if(selectedRow<0){selectedRow=0;selectedId=table->item(0,1)->text();}table->selectRow(selectedRow);}else selectedId.clear();
        pagination->setText(filtered.isEmpty()?"0 of 0":QString("%1-%2 of %3").arg(page*8+1).arg(end).arg(filtered.size()));
        previous->setEnabled(page>0);next->setEnabled(end<filtered.size());emptyList->setVisible(filtered.isEmpty());syncHeader();updateDetails();update();
    }
    void updateDetails() {
        auto *m=selected();
        editStaffButton->setEnabled(writable && m);deleteStaffButton->setEnabled(writable && m);smallEditButton->setEnabled(writable && m);
        for(auto *b:tabs)b->setEnabled(m!=nullptr);
        if(!m){name->setText("No matching staff");name->setToolTip({});role->clear();description->clear();status->clear();email->clear();email->setToolTip({});phone->clear();phone->setToolTip({});joined->clear();for(auto *label:infoValues)label->clear();overview->hide();tabContent->hide();reviewSummary->setText("Select a member to assess record completeness.");availabilityEnabled->setChecked(false);availabilityEnabled->setEnabled(false);availabilityStart->setEnabled(false);availabilityEnd->setEnabled(false);availabilityDate->setEnabled(false);availabilityState->setText("Select a staff member");return;}
        setFittedText(name,m->name);const int badgeX=name->x()+QFontMetrics(name->font()).horizontalAdvance(name->text())+16;status->move(badgeX,674);
        status->setText(m->status);status->setStyleSheet(QString("font-size:12px;border-radius:12px;background:%1;color:%2;").arg(m->status=="Active"?"#d9fae9":"#ffe0e3",m->status=="Active"?"#009146":"#d00019"));
        setFittedText(role,m->role);setFittedText(description,"Professional function: "+m->role);
        setFittedText(email,m->email);setFittedText(phone,m->phone);joined->setText(displayStaffEntryDate(m->joined));
        const QStringList values{m->id,m->name,m->role,m->email,m->phone};for(int i=0;i<5;++i)setFittedText(infoValues[i],values[i]);
        const auto colors=roleColors(m->role);infoValues[2]->setGeometry(139,81,qMin(292,QFontMetrics(uiFont(11)).horizontalAdvance(m->role)+24),19);infoValues[2]->setStyleSheet(QString("border-radius:10px;background:%1;color:%2;font-size:11px;padding-left:12px;").arg(colors.second.name(),colors.first.name()));infoValues[2]->setText(QFontMetrics(uiFont(11)).elidedText(m->role,Qt::ElideRight,infoValues[2]->width()-24));
        availabilityEnabled->setEnabled(true);availabilityDate->setEnabled(true);availabilityStart->setEnabled(availabilityEnabled->isChecked());availabilityEnd->setEnabled(availabilityEnabled->isChecked());
        const bool hasPeriod=m->unavailabilityStart.isValid()&&m->unavailabilityEnd.isValid();{QSignalBlocker blocker(availabilityEnabled);availabilityEnabled->setChecked(hasPeriod);}
        if(hasPeriod){QSignalBlocker a(availabilityStart),b(availabilityEnd);availabilityStart->setDate(m->unavailabilityStart);availabilityEnd->setDate(m->unavailabilityEnd);}
        else {QSignalBlocker a(availabilityStart),b(availabilityEnd);availabilityStart->setDate(QDate::currentDate());availabilityEnd->setDate(QDate::currentDate());}
        const QStringList missing=incompleteReasons(*m);reviewSummary->setText(missing.isEmpty()?"This staff record has its name, email, function, phone and employment date recorded.":QString("Review needed: %1").arg(missing.join(", ")));
        setTab(tab);
    }
    QStringList incompleteReasons(const Member &m) const {
        QStringList reasons;if(m.name.trimmed().isEmpty())reasons<<"name missing";if(m.email.trimmed().isEmpty())reasons<<"email missing";if(m.role.trimmed().isEmpty())reasons<<"professional function missing";if(m.phone.trimmed().isEmpty())reasons<<"phone missing";if(!staffEntryDate(m.joined).isValid())reasons<<"employment date missing";if(m.status!="Active")reasons<<"inactive status — verify";return reasons;
    }
    void updateAvailabilityStatus() {
        auto *m=selected();if(!m){availabilityState->setText("Select a staff member");return;}
        if(!availabilityEnabled->isChecked()){availabilityState->setText("Availability not recorded for this staff member");availabilityState->setStyleSheet("color:#526faa;font-weight:600;");return;}
        const QDate chosen=availabilityDate->date(),from=availabilityStart->date(),to=availabilityEnd->date();
        const bool unavailable=chosen>=from&&chosen<=to;
        availabilityState->setText(unavailable?"Unavailable on selected date":"Available outside recorded period");
        availabilityState->setStyleSheet(unavailable?"color:#b42318;font-weight:600;":"color:#15803d;font-weight:600;");
    }
    void showIncompleteRecords() {
        QDialog dialog(this);dialog.setWindowTitle("Staff records to review");dialog.resize(760,430);auto *layout=new QVBoxLayout(&dialog);
        auto *caption=new QLabel("Records with missing core information or inactive status are listed for manual verification.",&dialog);caption->setWordWrap(true);layout->addWidget(caption);
        auto *grid=new QTableWidget(&dialog);grid->setObjectName("staffIncompleteRecordsTable");grid->setColumnCount(4);grid->setHorizontalHeaderLabels({"Staff ID","Nom et prénom","Professional role","Review reason"});grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);grid->verticalHeader()->hide();grid->setEditTriggers(QAbstractItemView::NoEditTriggers);layout->addWidget(grid);
        for(const Member &member:members){const QStringList reasons=incompleteReasons(member);if(reasons.isEmpty())continue;const int row=grid->rowCount();grid->insertRow(row);const QStringList values{member.id,member.name,member.role,reasons.join(", ")};for(int c=0;c<4;++c)grid->setItem(row,c,new QTableWidgetItem(values[c]));}
        if(grid->rowCount()==0){grid->setRowCount(1);grid->setSpan(0,0,1,4);grid->setItem(0,0,new QTableWidgetItem("No incomplete or inactive staff records found."));}
        auto *close=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(close);connect(close,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
    }
    void setTab(int index) {
        tab=index;for(int i=0;i<tabs.size();++i){tabs[i]->setProperty("active",i==index);tabs[i]->style()->unpolish(tabs[i]);tabs[i]->style()->polish(tabs[i]);}
        overview->setVisible(index==0 && selected());tabContent->setVisible(index!=0 && selected());const auto *m=selected();if(!m)return;
        if(index==1){const auto stats=statistics(data);QString html=QString("<b>Staff statistics</b><br>Total: %1 &nbsp; Active: %2 &nbsp; Inactive: %3<br><br><b>By function</b><br>").arg(stats.total).arg(stats.active).arg(stats.inactive);for(auto it=stats.functions.begin();it!=stats.functions.end();++it)html+=it.key().toHtmlEscaped()+": "+QString::number(it.value())+"<br>";tabBrowser->setHtml(html);}
        update();
    }
    void editMember(bool add) {
        Member *existing=selected();if(!writable || (!add && !existing))return;
        const Member original=add?Member{}:*existing;
        QDialog dialog(this);dialog.setWindowTitle(add?"Add Staff":"Edit Staff");dialog.setMinimumWidth(520);auto *form=new QFormLayout(&dialog);
        dialog.setObjectName("staffEditor");
        auto *lastName=new QLineEdit(&dialog);
        auto *firstName=new QLineEdit(&dialog);
        auto *mail=new QLineEdit(add?"":existing->email,&dialog),*mobile=new QLineEdit(add?"":existing->phone,&dialog);
        lastName->setObjectName("memberLastName");firstName->setObjectName("memberFirstName");mail->setObjectName("memberEmail");mobile->setObjectName("memberPhone");
        lastName->setMaxLength(100);firstName->setMaxLength(100);mail->setMaxLength(254);mobile->setMaxLength(40);
        auto *entryDate=new QDateEdit(QDate::currentDate(),&dialog);entryDate->setObjectName("memberEntryDate");entryDate->setCalendarPopup(true);entryDate->setDisplayFormat("dd MMM yyyy");entryDate->setSpecialValueText("Not recorded");entryDate->setMinimumDate(QDate(1900,1,1));
        const QDate existingEntry=add?QDate::currentDate():staffEntryDate(existing->joined);entryDate->setDate(existingEntry.isValid()?existingEntry:entryDate->minimumDate());
        auto *memberRole=new QComboBox(&dialog);memberRole->setEditable(true);for(int i=1;i<roleFilter->count();++i)memberRole->addItem(roleFilter->itemText(i));if(!add)memberRole->setCurrentText(existing->role);
        auto *state=new QComboBox(&dialog);state->addItems({"Active","Inactive"});if(!add)state->setCurrentText(existing->status);
        memberRole->setObjectName("memberRole");state->setObjectName("memberStatus");
        memberRole->lineEdit()->setMaxLength(80);
        auto *periodEnabled=new QCheckBox("Record one unavailability period",&dialog);
        periodEnabled->setObjectName("memberUnavailabilityEnabled");
        auto *periodStart=new QDateEdit(QDate::currentDate(),&dialog);
        periodStart->setObjectName("memberUnavailabilityStart");periodStart->setCalendarPopup(true);periodStart->setDisplayFormat("dd MMM yyyy");
        auto *periodEnd=new QDateEdit(QDate::currentDate(),&dialog);
        periodEnd->setObjectName("memberUnavailabilityEnd");periodEnd->setCalendarPopup(true);periodEnd->setDisplayFormat("dd MMM yyyy");
        if(!add&&existing->unavailabilityStart.isValid()&&existing->unavailabilityEnd.isValid()) {
            periodEnabled->setChecked(true);periodStart->setDate(existing->unavailabilityStart);periodEnd->setDate(existing->unavailabilityEnd);
        }
        periodStart->setEnabled(periodEnabled->isChecked());periodEnd->setEnabled(periodEnabled->isChecked());
        connect(periodEnabled,&QCheckBox::toggled,periodStart,&QDateEdit::setEnabled);
        connect(periodEnabled,&QCheckBox::toggled,periodEnd,&QDateEdit::setEnabled);
        auto *periodDates=new QWidget(&dialog);auto *periodDatesLayout=new QHBoxLayout(periodDates);periodDatesLayout->setContentsMargins(0,0,0,0);periodDatesLayout->setSpacing(8);
        periodDatesLayout->addWidget(new QLabel("From",periodDates));periodDatesLayout->addWidget(periodStart,1);
        periodDatesLayout->addWidget(new QLabel("To",periodDates));periodDatesLayout->addWidget(periodEnd,1);
        auto *previewOnly=new QLabel("Demo: unavailability dates remain available until the app closes.",&dialog);previewOnly->setObjectName("memberUnavailabilityPreviewOnly");previewOnly->setWordWrap(true);previewOnly->setStyleSheet("color:#65728a;font-size:11px;");
        auto *periodValidation=new QLabel(&dialog);periodValidation->setObjectName("memberUnavailabilityValidation");periodValidation->setWordWrap(true);periodValidation->setStyleSheet("color:#d62038;font-size:11px;");
        auto *existingNameNote=new QLabel(add?QString():"Current full name kept unchanged until you enter both fields: "+existing->name,&dialog);
        existingNameNote->setObjectName("existingFullNamePreserved");existingNameNote->setWordWrap(true);existingNameNote->setStyleSheet("color:#65728a;font-size:11px;");existingNameNote->setVisible(!add);
        form->addRow(QString(),existingNameNote);form->addRow("Nom",lastName);form->addRow("Prénom",firstName);form->addRow("Email",mail);form->addRow("Phone",mobile);form->addRow("Date d'entrée",entryDate);form->addRow("Function",memberRole);form->addRow("Status",state);
        form->addRow("Availability",periodEnabled);form->addRow("Unavailability dates",periodDates);form->addRow(QString(),periodValidation);form->addRow(QString(),previewOnly);
        auto *error=new QLabel(&dialog);error->setObjectName("staffValidationError");error->setWordWrap(true);error->setStyleSheet("color:#d62038;");form->addRow(error);
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);form->addRow(buttons);
        auto updatePeriodValidation=[periodEnabled,periodStart,periodEnd,periodValidation,buttons]() {
            const bool invalid=periodEnabled->isChecked()&&periodEnd->date()<periodStart->date();
            periodValidation->setText(invalid?"End date must be on or after start date.":QString());
            if(auto *save=buttons->button(QDialogButtonBox::Save)) save->setEnabled(!invalid);
        };
        connect(periodEnabled,&QCheckBox::toggled,updatePeriodValidation);
        connect(periodStart,&QDateEdit::dateChanged,updatePeriodValidation);
        connect(periodEnd,&QDateEdit::dateChanged,updatePeriodValidation);
        updatePeriodValidation();
        connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{
            Member candidate=original;
            const QString familyName=lastName->text().trimmed(),givenName=firstName->text().trimmed();
            const bool namesProvided=!familyName.isEmpty()&&!givenName.isEmpty();
            if(familyName.isEmpty()!=givenName.isEmpty() || (add&&!namesProvided)){error->setText(add?"Enter both Nom and Prénom.":"Enter both Nom and Prénom, or leave both empty to preserve the existing full name.");return;}
            if(namesProvided) candidate.name=givenName+" "+familyName;
            candidate.email=mail->text().trimmed();candidate.role=memberRole->currentText().trimmed();candidate.status=state->currentText();candidate.phone=mobile->text().trimmed();
            candidate.joined=entryDate->date()==entryDate->minimumDate()?QString():entryDate->date().toString(Qt::ISODate);
            candidate.unavailabilityStart=periodEnabled->isChecked()?periodStart->date():QDate();
            candidate.unavailabilityEnd=periodEnabled->isChecked()?periodEnd->date():QDate();
            const QString validation=validateMember(data,candidate,add?QString():original.id);
            if(!validation.isEmpty()){error->setText(validation);return;}
            if(periodEnabled->isChecked()&&periodEnd->date()<periodStart->date()) { error->setText("The unavailability end date must be on or after the start date.");return; }
            dialog.accept();
        });
        if(dialog.exec()!=QDialog::Accepted)return;
        Member candidate=original;
        const QString familyName=lastName->text().trimmed(),givenName=firstName->text().trimmed();
        if(!familyName.isEmpty()&&!givenName.isEmpty()) candidate.name=givenName+" "+familyName;
        candidate.email=mail->text().trimmed();candidate.phone=mobile->text().trimmed();candidate.role=memberRole->currentText().trimmed();candidate.status=state->currentText();
        candidate.joined=entryDate->date()==entryDate->minimumDate()?QString():entryDate->date().toString(Qt::ISODate);
        candidate.unavailabilityStart=periodEnabled->isChecked()?periodStart->date():QDate();
        candidate.unavailabilityEnd=periodEnabled->isChecked()?periodEnd->date():QDate();
        if(!add && candidate.name==original.name && candidate.email==original.email &&
           candidate.phone==original.phone && candidate.role==original.role &&
           candidate.status==original.status && candidate.joined==original.joined &&
           candidate.unavailabilityStart==original.unavailabilityStart && candidate.unavailabilityEnd==original.unavailabilityEnd) {
            updateDetails(); return;
        }
        const State before=data;
        if(add){
            do {
                if(data.nextId==std::numeric_limits<int>::max()){data=before;QMessageBox::warning(this,"Cannot add Staff","Staff identifier limit reached.");return;}
                candidate.id=QString("STF-%1-%2").arg(QDate::currentDate().year()).arg(data.nextId++,3,10,QChar('0'));
            }while(std::any_of(members.cbegin(),members.cend(),[&](const Member &m){return m.id==candidate.id;}));
            QString dbError;
            if(!staffModel.ajouter(candidate,&dbError)){data=before;QMessageBox::warning(this,"Cannot add Staff","Oracle could not save the staff member:\n"+dbError);return;}
            members.prepend(candidate);
        }else{
            QString dbError;
            if(!staffModel.modifier(candidate,&dbError)){data=before;QMessageBox::warning(this,"Cannot edit Staff","Oracle could not update the staff member:\n"+dbError);return;}
            for(int i=0;i<members.size();++i)if(members[i].id==candidate.id){members[i]=candidate;break;}
        }
        selectedId=candidate.id;
        {QSignalBlocker a(search),b(roleFilter),c(listRoles),d(statusFilter);search->clear();roleFilter->setCurrentIndex(0);listRoles->setCurrentIndex(0);statusFilter->setCurrentIndex(0);}
        refreshAfterEdit(true);
    }
    void deleteMember() {
        if(!writable || !selected())return;
        const Member member=*selected();
        QDialog confirmation(this);
        confirmation.setObjectName("deleteStaffConfirmation");
        confirmation.setWindowTitle("Delete Staff");
        confirmation.setModal(true);
        confirmation.setMinimumWidth(470);
        confirmation.setStyleSheet(
            "QDialog#deleteStaffConfirmation{background:#ffffff;color:#10264b;}"
            "QLabel{background:transparent;color:#10264b;}"
            "QPushButton{min-height:38px;min-width:112px;border-radius:8px;padding:0 16px;font-weight:600;}"
            "QPushButton#cancelDeleteStaffButton{background:#f5f8fc;color:#29466f;border:1px solid #cbd8e8;}"
            "QPushButton#confirmDeleteStaffButton{background:#d9364f;color:#ffffff;border:1px solid #c52e45;}"
            "QPushButton#confirmDeleteStaffButton:hover{background:#bd293f;}");
        auto *layout=new QVBoxLayout(&confirmation);
        layout->setContentsMargins(24,22,24,20);
        layout->setSpacing(12);
        auto *title=new QLabel("Delete this staff member?",&confirmation);
        title->setStyleSheet("font-size:18px;font-weight:700;color:#10264b;");
        auto *details=new QLabel(QString("%1 (%2) will be removed from the Staff list.")
                                     .arg(member.name,member.id),&confirmation);
        details->setWordWrap(true);
        details->setStyleSheet("font-size:13px;color:#526782;");
        layout->addWidget(title);
        layout->addWidget(details);
        layout->addSpacing(4);
        auto *buttons=new QHBoxLayout;
        buttons->addStretch();
        auto *cancel=new QPushButton("Cancel",&confirmation);
        cancel->setObjectName("cancelDeleteStaffButton");
        auto *remove=new QPushButton("Delete Staff",&confirmation);
        remove->setObjectName("confirmDeleteStaffButton");
        remove->setDefault(true);
        buttons->addWidget(cancel);
        buttons->addWidget(remove);
        layout->addLayout(buttons);
        connect(cancel,&QPushButton::clicked,&confirmation,&QDialog::reject);
        connect(remove,&QPushButton::clicked,&confirmation,&QDialog::accept);
        if(confirmation.exec()!=QDialog::Accepted)return;
        {QString dbError;if(!staffModel.supprimer(member.id,&dbError)){QMessageBox::warning(this,"Cannot delete Staff","Oracle could not delete the staff member:\n"+dbError);return;}}
        for(int i=0;i<members.size();++i)if(members[i].id==member.id){members.removeAt(i);break;}
        checked.remove(member.id);
        refreshAfterEdit();
    }
    void toggleMember() {
        if(!writable || !selected())return;
        const Member member=*selected();
        const QString nextStatus=member.status=="Active"?"Inactive":"Active";
        {Member changed=member;changed.status=nextStatus;QString dbError;if(!staffModel.modifier(changed,&dbError)){QMessageBox::warning(this,"Cannot change status","Oracle could not update the staff member:\n"+dbError);return;}}
        for(int i=0;i<members.size();++i)if(members[i].id==member.id){members[i].status=nextStatus;break;}
        refreshAfterEdit();
    }
    void showActions() {
        auto *m=selected();if(!m || !writable)return;QMenu menu(this);menu.setObjectName("staffActions");auto *edit=menu.addAction("Edit Staff");auto *toggle=menu.addAction(m->status=="Active"?"Deactivate member":"Activate member");menu.addSeparator();auto *remove=menu.addAction("Delete Staff");auto *choice=menu.exec(QCursor::pos());
        if(choice==edit)editMember(false);else if(choice==toggle)toggleMember();else if(choice==remove)deleteMember();
    }
    void showStatistics() {
        QDialog dialog(this);dialog.setObjectName("statisticsDialog");dialog.setWindowTitle("Staff Statistics");dialog.resize(740,510);
        auto *layout=new QVBoxLayout(&dialog);const auto s=statistics(data);
        auto *summary=new QLabel(QString("Total: %1    Active: %2    Inactive: %3    Functions: %4")
            .arg(s.total).arg(s.active).arg(s.inactive).arg(s.functions.size()),&dialog);
        summary->setObjectName("statisticsSummary");summary->setStyleSheet("font-size:18px;font-weight:600;padding:12px;");layout->addWidget(summary);
        auto *caption=new QLabel("Calculated from all saved staff records. Bars show the share of total staff.",&dialog);caption->setWordWrap(true);layout->addWidget(caption);
        auto *grouping=new QComboBox(&dialog);grouping->setObjectName("statisticsGrouping");grouping->addItems({"By function","By status"});layout->addWidget(grouping);
        auto *grid=new QTableWidget(&dialog);grid->setObjectName("statisticsTable");grid->setColumnCount(3);grid->setHorizontalHeaderLabels({"Group","Members","Share of staff"});grid->verticalHeader()->hide();grid->setEditTriggers(QAbstractItemView::NoEditTriggers);grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);layout->addWidget(grid);
        auto populate=[&]{
            QMap<QString,int> counts=grouping->currentIndex()==0?s.functions:QMap<QString,int>{{"Active",s.active},{"Inactive",s.inactive}};grid->setRowCount(counts.size());int row=0;
            for(auto it=counts.begin();it!=counts.end();++it,++row){grid->setItem(row,0,new QTableWidgetItem(it.key()));grid->setItem(row,1,new QTableWidgetItem(QString::number(it.value())));auto *bar=new QProgressBar(grid);bar->setRange(0,qMax(1,s.total));bar->setValue(it.value());bar->setFormat(QString::number(s.total?100.0*it.value()/s.total:0.0,'f',1)+"%");grid->setCellWidget(row,2,bar);}
        };
        connect(grouping,&QComboBox::currentIndexChanged,&dialog,[&]{populate();});populate();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
    }

    void exportPdf() {
        QString path=QFileDialog::getSaveFileName(this,"Export Staff PDF","SkySense-Staff.pdf","PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";
        QSaveFile output(path);if(!output.open(QIODevice::WriteOnly)){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        {QPdfWriter writer(&output);writer.setPageSize(QPageSize(QPageSize::A4));writer.setPageOrientation(QPageLayout::Landscape);writer.setResolution(96);writer.setTitle("SkySense — Staff");
        QString html="<h1>SkySense — Staff</h1><p>Generated "+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")+" · "+QString::number(filtered.size())+" matching members</p><table border='1' cellspacing='0' cellpadding='7'><tr><th>Staff ID</th><th>Name</th><th>Email</th><th>Phone</th><th>Function</th><th>Status</th><th>Employment date</th></tr>";
        for(int i:filtered){const auto &m=members[i];html+="<tr>";for(const auto &value:QStringList{m.id,m.name,m.email,m.phone,m.role,m.status,displayStaffEntryDate(m.joined)})html+="<td>"+value.toHtmlEscaped()+"</td>";html+="</tr>";}QTextDocument document;document.setHtml(html+"</table>");document.print(&writer);}
        if(!output.commit()){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        QMessageBox::information(this,"Export PDF","Staff report saved successfully.");
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.fillRect(rect(),AppTheme::brush(p, QColor("#f6faff")));
        p.translate(-integratedSidebarWidth,0);
        QLinearGradient body(285,66,1672,941);body.setColorAt(0,QColor("#f4faff"));body.setColorAt(1,Qt::white);p.fillRect(QRect(285,66,1387,875),AppTheme::brush(p, body));
        // Exact decorative artwork; no screenshot text or interactive area is used.
        p.fillRect(QRect(0,0,285,H),AppTheme::brush(p, QColor("#031b2c")));
        p.drawPixmap(QRect(0,444,285,398),reference(),QRect(0,444,285,398));
        p.drawPixmap(QRect(15,42,253,77),reference(),QRect(15,42,253,77));
        p.setPen(AppTheme::pen(p, QColor("#16415a")));p.drawLine(27,842,258,842);p.setPen(AppTheme::pen(p, Qt::NoPen));p.setBrush(AppTheme::brush(p, QColor("#00d195")));p.drawEllipse(QPointF(40,865),7,7);p.drawEllipse(QPointF(40,891),7,7);
        p.fillRect(QRect(285,0,3,H),AppTheme::brush(p, QColor("#d9e5f3")));
        QLinearGradient top(0,0,0,66);top.setColorAt(0,QColor("#f8fbff"));top.setColorAt(1,QColor("#fdfefe"));p.fillRect(QRect(288,0,1384,66),AppTheme::brush(p, top));
        QLinearGradient shadow(0,66,0,77);shadow.setColorAt(0,QColor(161,190,227,30));shadow.setColorAt(1,QColor(161,190,227,0));p.fillRect(QRect(288,66,1384,11),AppTheme::brush(p, shadow));
        p.setPen(AppTheme::pen(p, QColor("#cedff4")));p.drawLine(1387,18,1387,53);
        const auto s=statistics(data);
        const QStringList titles{"Total Staff","Active Staff","Inactive Staff","Functions"};const QStringList numbers{QString::number(s.total),QString::number(s.active),QString::number(s.inactive),QString::number(s.functions.size())};
        const QStringList suffixes{"members",QString("/ %1 total").arg(s.total),QString("/ %1 total").arg(s.total),"functions"};
        const QList<int> amounts{s.total,s.active,s.inactive,int(s.functions.size())};
        const QStringList captions{"All saved staff",QString::number(s.total?100.0*s.active/s.total:0.0,'f',1)+"% of staff",QString::number(s.total?100.0*s.inactive/s.total:0.0,'f',1)+"% of staff",QString::number(s.functions.size())+" distinct functions"};
        const QColor accents[]{blue,QColor("#00c471"),QColor("#6f89b2"),QColor("#4f12ff")};const QColor fills[]{QColor("#f2f9ff"),QColor("#f2fdfa"),QColor("#fbfdff"),QColor("#f8f6ff")};const QStringList icons{"people","addperson","inactive","grid"};
        const int xs[]{315,651,987,1326}, widths[]{323,323,325,324};
        for(int i=0;i<4;++i){const int x=xs[i];rounded(p,QRectF(x,157,widths[i],112),fills[i]);QColor bubble=accents[i];bubble.setAlpha(21);p.setPen(AppTheme::pen(p, Qt::NoPen));p.setBrush(AppTheme::brush(p, bubble));p.drawEllipse(QRectF(x+21,172,55,55));paintIcon(p,icons[i],QRectF(x+34,184,30,30),accents[i]);
            p.setFont(uiFont(14,true));p.setPen(AppTheme::pen(p, ink));p.drawText(QRect(x+102,171,219,24),Qt::AlignVCenter,titles[i]);auto numberFont=uiFont(numbers[i].size()>4?22:30,true);numberFont.setWeight(QFont::Bold);p.setFont(numberFont);const int numberWidth=QFontMetrics(numberFont).horizontalAdvance(numbers[i]);p.drawText(QRect(x+102,192,numberWidth+4,38),Qt::AlignVCenter,numbers[i]);
            p.setFont(uiFont(14));p.setPen(AppTheme::pen(p, muted));p.drawText(QRect(x+110+numberWidth,205,qMax(0,widths[i]-125-numberWidth),25),Qt::AlignVCenter,suffixes[i]);
            p.setFont(uiFont(12));p.drawText(QRect(x+102,232,207,22),Qt::AlignVCenter,captions[i]);
            if(i==1 || i==2){p.setPen(AppTheme::pen(p, Qt::NoPen));p.setBrush(AppTheme::brush(p, QColor("#e4edf8")));p.drawRoundedRect(QRectF(x+102,258,195,4),2,2);
                p.setBrush(AppTheme::brush(p, accents[i]));p.drawRoundedRect(QRectF(x+102,258,195.0*amounts[i]/qMax(1,s.total),4),2,2);}
        }
        rounded(p,QRectF(315,281,1335,369),Qt::white);
        p.setPen(AppTheme::pen(p, QColor("#e7effb")));p.drawLine(1482,294,1482,324);
        rounded(p,QRectF(315,663,1335,278),Qt::white);p.setPen(AppTheme::pen(p, QColor("#e5eefb")));p.drawLine(316,741,1649,741);p.drawLine(325,772,1430,772);p.drawLine(1118,679,1118,714);p.drawLine(1331,679,1331,714);p.drawLine(1505,679,1505,714);
        if(tab==0 && selected()){rounded(p,QRectF(325,778,449,162),QColor("#fdfeff"));rounded(p,QRectF(785,778,420,162),QColor("#fdfeff"));rounded(p,QRectF(1216,778,421,162),QColor("#fdfeff"));
            p.setPen(AppTheme::pen(p, line));p.drawLine(335,814,764,814);p.drawLine(795,814,1195,814);p.drawLine(1226,814,1627,814);
        }else if(selected())rounded(p,QRectF(325,778,1312,162),QColor("#fdfeff"));
    }
};

class StaffView final : public QGraphicsView {
    StaffCanvas *canvas;
public:
    explicit StaffView(QWidget *parent):QGraphicsView(parent) {
        setObjectName("staffView");setFrameShape(QFrame::NoFrame);setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setBackgroundBrush(QColor("#f5f9ff"));setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
        auto *scene=new QGraphicsScene(this);setScene(scene);canvas=new StaffCanvas;auto *canvasItem=scene->addWidget(canvas);canvasItem->setPos(0,0);
        scene->setSceneRect(0,0,W-integratedSidebarWidth,H);
        canvas->setProperty("referenceWidth",W);
    }
    StaffCanvas *designCanvas() const { return canvas; }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        const qreal scale = qMin<qreal>(1.1, (viewport()->width() - 18) / qreal(W-integratedSidebarWidth));
        setTransform(QTransform::fromScale(scale, scale));
        horizontalScrollBar()->setValue(0);
        verticalScrollBar()->setValue(0);
    }
};
}

void setupStaffUi(QWidget *root) {
    root->setObjectName("staffRoot");auto *layout=new QVBoxLayout(root);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);auto *view=new StaffView(root);layout->addWidget(view);
    root->setProperty("staffCanvas",QVariant::fromValue(static_cast<QWidget*>(view->designCanvas())));
}






