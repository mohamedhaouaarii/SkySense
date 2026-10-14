#include "staffui.h"
#include "staffdata.h"
#include "stafftraining.h"
#include <QtWidgets>
#include <algorithm>
#include <limits>

namespace {
using namespace Staff;
const QColor ink("#080d34"), blue("#006cff"), muted("#526faa"), line("#e0edfd");
constexpr int W = 1672, H = 941;

QFont uiFont(int size, bool bold = false) {
    QFont result("Segoe UI"); result.setPixelSize(size);
    result.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    return result;
}
const QPixmap &reference() { static QPixmap image(":/assets/reference.png"); return image; }

void drawSymbol(QPainter &p, const QString &name, QColor color) {
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    if (name == "people" || name == "addperson" || name == "inactive") {
        p.setPen(Qt::NoPen); p.setBrush(color); p.drawEllipse(QRectF(5, 2, 9, 9));
        path.moveTo(1, 23); path.lineTo(1, 19); path.cubicTo(1, 11, 18, 11, 18, 19); path.lineTo(18, 23); path.closeSubpath(); p.drawPath(path);
        if (name == "people") { p.drawEllipse(QRectF(15, 3, 7, 8)); p.drawRoundedRect(QRectF(18, 13, 6, 9), 3, 3); }
        else { p.setPen(QPen(color, 2.8, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(19, 13, 25, 13); if (name == "addperson") p.drawLine(22, 10, 22, 16);
            else { p.drawLine(20, 10, 24, 16); p.drawLine(24, 10, 20, 16); }
        }
    } else if (name == "clock") {
        p.drawEllipse(QRectF(2, 2, 20, 20)); p.drawLine(12, 6, 12, 12); p.drawLine(12, 12, 17, 15);
    } else if (name == "pin") {
        p.setBrush(color); p.setPen(Qt::NoPen); path.moveTo(12, 24); path.cubicTo(8, 18, 3, 13, 3, 9); path.cubicTo(3, -2, 21, -2, 21, 9); path.cubicTo(21, 13, 16, 19, 12, 24); p.drawPath(path);
        p.setBrush(QColor("#08223a")); p.drawEllipse(QRectF(8, 5, 8, 8));
    } else if (name == "equipment") {
        p.setBrush(color); p.setPen(Qt::NoPen);
        for (int y : {3, 15}) { p.drawRoundedRect(QRectF(1, y, 22, 8), 3, 3); p.setBrush(QColor("#08223a")); p.drawEllipse(QRectF(4, y+2, 4, 4)); p.drawRect(QRectF(18, y+3, 2, 2)); p.setBrush(color); }
    } else if (name == "tool") {
        path.moveTo(16, 2); path.cubicTo(8, -1, 7, 6, 9, 10); path.lineTo(1, 19); path.lineTo(5, 23); path.lineTo(14, 14); path.cubicTo(23, 17, 26, 6, 21, 3); path.lineTo(17, 8); path.lineTo(13, 6); path.closeSubpath(); p.drawPath(path);
    } else if (name == "grid") {
        p.setBrush(color); p.setPen(Qt::NoPen); for (int x : {2, 14}) for (int y : {2, 14}) p.drawRoundedRect(QRectF(x, y, 9, 9), 2, 2);
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
        if (name == "clipboard") { p.setBrush(Qt::white); p.drawRoundedRect(QRectF(9, 1, 7, 5), 1, 1); }
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
    label->setStyleSheet(QString("font-family:'Segoe UI';font-size:%1px;font-weight:%2;color:%3;").arg(size).arg(bold ? 600 : 400).arg(color.name()));
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
class AccessCheckBox final : public QCheckBox {
public:
    using QCheckBox::QCheckBox;
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        const QRectF box(1,2,15,15);p.setBrush(isChecked()?blue:QColor(Qt::white));p.setPen(QPen(isChecked()?blue:QColor("#a6b3c5"),1.3));p.drawRoundedRect(box,3,3);
        if(isChecked())paintIcon(p,"check",box,Qt::white);
        if(hasFocus()){p.setPen(QPen(QColor("#86b9ff"),1,Qt::DashLine));p.setBrush(Qt::NoBrush);p.drawRoundedRect(QRectF(0,1,18,18),3,3);}
    }
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
    p.setPen(QPen(stroke, 1)); p.setBrush(fill); p.drawRoundedRect(rect, radius, radius);
}
void badge(QPainter &p, QRectF rect, const QString &value, QColor foreground, QColor background, int size = 12) {
    p.setPen(Qt::NoPen); p.setBrush(background); p.drawRoundedRect(rect, rect.height()/2, rect.height()/2);
    p.setPen(foreground); p.setFont(uiFont(size));
    p.drawText(rect, Qt::AlignCenter, QFontMetrics(p.font()).elidedText(value, Qt::ElideRight, int(rect.width()) - 12));
}

QList<Member> demoMembers() {
    return {
        {"STF-2025-012","Alex Martin","alex.martin@skysense.com","Operations Manager","Operations","Active","Apr 24, 2025 09:42","+1 (555) 234-5671","Jan 14, 2022"},
        {"STF-2025-011","Sophia Chen","sophia.chen@skysense.com","Station Technician","Field Operations","Active","Apr 24, 2025 08:15","+1 (555) 234-5672","Mar 03, 2023"},
        {"STF-2025-010","James Wilson","james.wilson@skysense.com","Maintenance Lead","Maintenance","Active","Apr 23, 2025 17:28","+1 (555) 234-5673","Sep 18, 2023"},
        {"STF-2025-009","Emma Thompson","emma.thompson@skysense.com","Data Analyst","Operations","Active","Apr 24, 2025 07:55","+1 (555) 234-5674","Feb 12, 2021"},
        {"STF-2025-008","Daniel Kim","daniel.kim@skysense.com","Technical Specialist","Engineering","Active","Apr 24, 2025 10:12","+1 (555) 234-5678","Jan 15, 2024"},
        {"STF-2025-007","Olivia Davis","olivia.davis@skysense.com","Administrator","IT","Active","Apr 23, 2025 16:03","+1 (555) 234-5676","Jun 27, 2022"},
        {"STF-2025-006","Ryan Patel","ryan.patel@skysense.com","Support Specialist","Operations","Inactive","Apr 20, 2025 11:44","+1 (555) 234-5677","Nov 08, 2024"},
        {"STF-2025-005","Isabella Garcia","isabella.garcia@skysense.com","Field Technician","Field Operations","Active","Apr 24, 2025 06:31","+1 (555) 234-5679","Oct 08, 2023"},
        {"STF-2025-004","Noah Brown","noah.brown@skysense.com","Station Technician","Field Operations","Active","Apr 24, 2025 08:00","+1 (555) 234-5680","Feb 08, 2024"},
        {"STF-2025-003","Mia Johnson","mia.johnson@skysense.com","Data Analyst","Operations","Active","Apr 24, 2025 08:20","+1 (555) 234-5681","Mar 12, 2024"},
        {"STF-2025-002","Ethan Lee","ethan.lee@skysense.com","Technical Specialist","Engineering","Active","Apr 24, 2025 08:30","+1 (555) 234-5682","Apr 10, 2024"},
        {"STF-2025-001","Ava Taylor","ava.taylor@skysense.com","Field Technician","Field Operations","Active","Apr 24, 2025 09:00","+1 (555) 234-5683","May 04, 2024"}
    };
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
    int sortColumn = 1;
    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    explicit StaffHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) { setSectionsClickable(true); setFixedHeight(35); }
protected:
    void paintSection(QPainter *p, const QRect &r, int section) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->fillRect(r,QColor("#f3f8ff")); p->setPen(QColor("#e9f1fc")); p->drawLine(r.bottomLeft(),r.bottomRight());
        if(section) p->drawLine(r.topLeft()+QPoint(0,5),r.bottomLeft()-QPoint(0,5));
        if(section==0) { rounded(*p,QRectF(r.left()+14,r.top()+10,16,16),checked?blue:QColor(Qt::white),checked?blue:QColor("#a3b1c6"),3); if(checked) paintIcon(*p,"check",QRectF(r.left()+14,r.top()+10,16,16),Qt::white); }
        else { p->setFont(uiFont(12)); p->setPen(QColor("#173873")); const QString title=model()->headerData(section,Qt::Horizontal).toString(); p->drawText(r.adjusted(16,0,-5,0),Qt::AlignVCenter,title);
            if(section==sortColumn) { const int x=r.right()-12, y=r.center().y(); const int d=sortOrder==Qt::AscendingOrder?-1:1; p->setPen(QPen(blue,1.8)); p->drawLine(x,y-5,x,y+5); p->drawLine(x-3,y+2*d,x,y+5*d); p->drawLine(x,y+5*d,x+3,y+2*d); }
        } p->restore();
    }
};
class StaffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); p->setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect r=option.rect; const bool selected=option.state & QStyle::State_Selected; const int c=index.column();
        p->fillRect(r,selected?QColor("#e0f1ff"):QColor("#ffffff")); p->setPen(QColor("#eaf1fc")); p->drawLine(r.bottomLeft(),r.bottomRight());
        p->setFont(uiFont(12)); p->setPen(QColor("#274989"));
        if(c==0) { const bool checked=index.data(Qt::CheckStateRole).toInt()==Qt::Checked; const QRectF box(r.left()+14,r.center().y()-8,16,16); rounded(*p,box,checked?blue:QColor(Qt::white),checked?blue:QColor("#a6b3c5"),3); if(checked)paintIcon(*p,"check",box,Qt::white); }
        else if(c==2) { p->setPen(ink); const QRect copy=r.adjusted(16,0,-4,0);p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width())); }
        else if(c==4) { const auto colors=roleColors(index.data().toString()); badge(*p,QRectF(r.left()+15,r.center().y()-12,137,25),index.data().toString(),colors.first,colors.second); }
        else if(c==6) { bool active=index.data().toString()=="Active"; badge(*p,QRectF(r.left()+15,r.center().y()-12,94,25),index.data().toString(),QColor(active?"#008e3f":"#db001a"),QColor(active?"#d9fbeb":"#ffe1e4")); }
        else if(c==8) { p->setPen(Qt::NoPen); p->setBrush(QColor("#254385")); for(int x:{-5,0,5})p->drawEllipse(QPointF(r.center().x()+x,r.center().y()),1.3,1.3); }
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
    Store storage;
    State data;
    QList<Member> &members=data.members;
    QHash<QString,QList<bool>> &permissions=data.permissions;
    bool writable=true;
    QList<int> filtered;
    QSet<QString> checked{"STF-2025-008"};
    QString selectedId="STF-2025-008";
    int page=0, tab=0;
    QLineEdit *search{};
    Selector *roleFilter{},*statusFilter{},*listRoles{};
    QTableWidget *table{};
    StaffHeader *header{};
    QLabel *pagination{},*name{},*role{},*description{},*status{},*email{},*phone{},*joined{};
    QPushButton *previous{},*next{},*editStaffButton{},*addStaffButton{},*deleteStaffButton{},*accessButton{},*smallEditButton{},*trainingButton{};
    QLabel *emptyList{};
    QList<QPushButton*> tabs;
    QList<QLabel*> infoValues;
    QList<QCheckBox*> access;
    QWidget *overview{}, *tabContent{};
    QTextBrowser *tabBrowser{};
    QList<QLabel*> recentTexts, recentDates;
    QLabel *storageStatus{};
public:
    explicit StaffCanvas(const QString &path, QWidget *parent=nullptr):QWidget(parent),storage(path) {
        setObjectName("staffCanvas"); setFixedSize(W,H);
        QFile sheet(":/styles/staff.qss"); if(sheet.open(QIODevice::ReadOnly))setStyleSheet(QString::fromUtf8(sheet.readAll()));
        QString error;
        if(storage.exists()) writable=storage.load(data,error);
        else {
            members=demoMembers();
            for(const auto &member:members)permissions.insert(member.id,{true,true,true,true});
            writable=storage.save(data,error);
        }
        buildSidebar(); buildHeader(); buildList(); buildDetails(); refresh();
        if(!writable)QTimer::singleShot(0,this,[this,error]{QMessageBox::warning(this,"Staff data unavailable",error+"\n\n"+storage.path());});
    }
private:
    QList<AuditEntry> memberAudit() const {
        QList<AuditEntry> result;for(const auto &entry:data.audit)if(entry.memberId==selectedId)result.append(entry);return result;
    }
    bool persist(const State &before, const QString &id, const QString &memberName,
                 const QString &action, const QString &details, bool reveal=false) {
        data.audit.prepend({QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),operatorName(),id,memberName,action,details});
        QString error;
        if(!storage.save(data,error)) {
            data=before;populateRoles();refresh();
            QMessageBox::warning(this,"Changes not saved",error+"\nYour change was not applied.");return false;
        }
        populateRoles();refresh(reveal);return true;
    }
    void changeAccess(const QList<bool> &rights) {
        auto *member=selected();if(!writable || !member || rights.size()!=4)return;
        const auto old=permissions.value(selectedId,{false,false,false,false});if(old==rights)return;
        const State before=data;
        permissions.insert(selectedId,rights);
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};QStringList changes;
        for(int i=0;i<4;++i)if(old[i]!=rights[i])changes<<modules[i]+(rights[i]?": allowed":": denied");
        persist(before,member->id,member->name,"Access changed",changes.join("; "));
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
        storageStatus=text(this,writable?"Saved on this device":"Storage unavailable",QRect(58,877,222,23),12,Qt::white);
        storageStatus->setToolTip(QDir::toNativeSeparators(storage.path()));
        text(this,"Local storage / JSON",QRect(58,900,222,23),12,QColor("#abc7e5"));
    }
    void buildHeader() {
        search=new QLineEdit(this); search->setObjectName("staffSearch"); search->setGeometry(315,15,501,40);
        search->setPlaceholderText("Search staff, roles, departments, or email..."); search->setStyleSheet("QLineEdit { padding-left:45px; font-size:14px; background:#f4f8ff; }");
        search->setClearButtonEnabled(true);search->setAccessibleName("Search staff");
        auto searchPalette=search->palette();searchPalette.setColor(QPalette::PlaceholderText,muted);search->setPalette(searchPalette);
        new Glyph(search,"search",QRect(14,9,20,20),QColor("#7187c1"));
        auto *stats=button(this,"Statistics","grid",QRect(832,15,138,40));stats->setObjectName("staffStatistics");connect(stats,&QPushButton::clicked,this,[this]{showStatistics();});
        auto *audit=button(this,"Activity Log","clock",QRect(982,15,155,40));audit->setObjectName("staffAudit");connect(audit,&QPushButton::clicked,this,[this]{showAudit();});
        new Glyph(this,"calendar",QRect(1185,24,24,24),QColor("#264788")); text(this,QLocale(QLocale::English).toString(QDate::currentDate(),"MMM d, yyyy"),QRect(1223,19,131,32),14,ink,true);
        new Glyph(this,"down",QRect(1350,29,14,14),muted);
        setFittedText(text(this,"",QRect(1413,13,200,24),14,ink,true),operatorName());text(this,"Local operator",QRect(1413,35,200,22),13,muted);
        new Glyph(this,"down",QRect(1626,25,18,18),ink);
        text(this,"Staff",QRect(317,70,470,54),42,QColor("#07072d"),true)->setStyleSheet("font-family:'Segoe UI';font-size:42px;font-weight:700;color:#07072d;");
        text(this,"Manage your team, assign roles, and control system access.",QRect(318,122,740,27),19,muted);
        roleFilter=new Selector(this,QRect(1063,89,139,43),{"Role"},"filter"); roleFilter->setObjectName("roleFilter");
        statusFilter=new Selector(this,QRect(1216,89,139,43),{"Status","Active","Inactive"},"filter"); statusFilter->setObjectName("statusFilter");
        auto *exportButton=button(this,"Export PDF","document",QRect(1369,89,135,43));
        exportButton->setObjectName("exportStaffPdf");
        connect(exportButton,&QPushButton::clicked,this,[this]{exportPdf();});
        addStaffButton=button(this,"Add Staff","plus",QRect(1518,89,132,43),true); addStaffButton->setObjectName("addStaff");addStaffButton->setEnabled(writable);connect(addStaffButton,&QPushButton::clicked,this,[this]{editMember(true);});
        connect(search,&QLineEdit::textChanged,this,[this]{page=0;refresh();});
        connect(roleFilter,&QComboBox::currentIndexChanged,this,[this](int index){QSignalBlocker block(listRoles);listRoles->setCurrentIndex(index);page=0;refresh();});
        connect(statusFilter,&QComboBox::currentIndexChanged,this,[this]{page=0;refresh();});
    }
    void buildList() {
        new Glyph(this,"clipboard",QRect(334,293,26,26));
        text(this,"Staff List",QRect(377,286,300,27),19,ink,true);
        text(this,"View and manage all team members, their roles, and system access.",QRect(377,310,730,22),13,muted);
        listRoles=new Selector(this,QRect(1334,292,132,33),{"All Roles"}); listRoles->setObjectName("listRoleFilter"); listRoles->setStyleSheet("font-size:12px;");
        populateRoles();
        pagination=text(this,"1-8 of 12",QRect(1498,292,67,33),12,muted);
        previous=button(this,"","left",QRect(1565,292,33,33)); next=button(this,"","right",QRect(1603,292,33,33)); previous->setObjectName("previousPage");next->setObjectName("nextPage");
        previous->setAccessibleName("Previous staff page");previous->setToolTip("Previous page");
        next->setAccessibleName("Next staff page");next->setToolTip("Next page");
        connect(previous,&QPushButton::clicked,this,[this]{if(page>0){--page;refresh();}});
        connect(next,&QPushButton::clicked,this,[this]{if((page+1)*8<filtered.size()){++page;refresh();}});
        connect(listRoles,&QComboBox::currentIndexChanged,this,[this](int index){QSignalBlocker block(roleFilter);roleFilter->setCurrentIndex(index);page=0;refresh();});
        table=new QTableWidget(0,9,this); table->setObjectName("staffTable");table->setGeometry(324,335,1317,307);
        header=new StaffHeader(table);table->setHorizontalHeader(header);
        table->setHorizontalHeaderLabels({"","Staff ID","Name","Email","Role","Department","Status","Last Login","Actions"});
        for(int column=1;column<=7;++column)table->horizontalHeaderItem(column)->setToolTip(column==7?"Sort by last login (initial sample data; this module does not authenticate users).":"Click to sort ascending / descending across all pages.");
        table->verticalHeader()->hide();table->verticalHeader()->setDefaultSectionSize(34);table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        const int widths[]{50,126,184,222,162,141,141,193,98}; for(int c=0;c<9;++c)table->setColumnWidth(c,widths[c]);
        table->setShowGrid(false);table->setFrameShape(QFrame::NoFrame);table->setSelectionBehavior(QAbstractItemView::SelectRows);table->setSelectionMode(QAbstractItemView::SingleSelection);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->setFocusPolicy(Qt::StrongFocus);table->setItemDelegate(new StaffDelegate(table));table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        emptyList=text(table->viewport(),"No staff match your search or filters.",QRect(200,90,900,45),16,muted);
        emptyList->setObjectName("emptyStaffList");emptyList->setAlignment(Qt::AlignCenter);emptyList->setAttribute(Qt::WA_TransparentForMouseEvents);emptyList->hide();
        connect(table,&QTableWidget::itemSelectionChanged,this,[this]{const auto rows=table->selectionModel()->selectedRows(1);if(!rows.isEmpty()){selectedId=rows.first().data().toString();updateDetails();}});
        connect(table,&QTableWidget::itemChanged,this,[this](QTableWidgetItem *item){if(item->column()!=0)return; const QString id=table->item(item->row(),1)->text();if(item->checkState()==Qt::Checked)checked.insert(id);else checked.remove(id);syncHeader();});
        connect(table,&QTableWidget::cellClicked,this,[this](int row,int column){if(column==8){selectedId=table->item(row,1)->text();showActions();}});
        connect(header,&QHeaderView::sectionClicked,this,[this](int section){
            if(section==0){const bool set=!header->checked;for(int r=0;r<table->rowCount();++r)table->item(r,0)->setCheckState(set?Qt::Checked:Qt::Unchecked);}
            else if(section<=7){header->sortOrder=section==header->sortColumn && header->sortOrder==Qt::AscendingOrder?Qt::DescendingOrder:Qt::AscendingOrder;header->sortColumn=section;page=0;refresh();}
        });
    }
    void buildDetails() {
        name=text(this,"Daniel Kim",QRect(335,670,360,30),21,ink,true);name->setObjectName("staffDetailName");
        status=text(this,"Active",QRect(515,674,83,24),12,QColor("#009347"));status->setObjectName("staffDetailStatus"); status->setAlignment(Qt::AlignCenter);
        role=text(this,"Technical Specialist",QRect(335,697,450,20),13);
        description=text(this,"Supports station operations, sensor calibration, and system troubleshooting.",QRect(335,716,640,20),12,muted);
        editStaffButton=button(this,"Edit Staff","edit",QRect(990,679,114,35),true); editStaffButton->setObjectName("editStaff"); connect(editStaffButton,&QPushButton::clicked,this,[this]{editMember(false);});
        trainingButton=button(this,"Skills && Training","clipboard",QRect(805,679,175,35));trainingButton->setObjectName("staffTraining");
        trainingButton->setToolTip("Assess skills and identify training needs for the selected member");
        connect(trainingButton,&QPushButton::clicked,this,[this]{showTraining();});
        new Glyph(this,"mail",QRect(1131,687,23,23),QColor("#254a90")); email=text(this,"",QRect(1165,681,162,35),12,muted);
        new Glyph(this,"phone",QRect(1354,686,23,23));phone=text(this,"",QRect(1386,681,125,35),12,muted);
        new Glyph(this,"calendar",QRect(1518,685,26,26),QColor("#254a90")); text(this,"Joined Date",QRect(1557,680,91,19),12,muted);joined=text(this,"",QRect(1557,700,91,19),12,ink,true);
        const QStringList titles{"Overview","Role && Permissions","Statistics","Activity Log","Notes"};const int x[]{330,490,740,980,1220};const int widths[]{157,234,240,240,210};
        for(int i=0;i<5;++i){auto *b=button(this,titles[i],{},QRect(x[i],741,widths[i],31));b->setProperty("tab",true);b->setProperty("active",i==0);tabs<<b;connect(b,&QPushButton::clicked,this,[this,i]{setTab(i);});}
        deleteStaffButton=button(this,"Delete Staff",{},QRect(1480,744,157,26));deleteStaffButton->setObjectName("deleteStaff");deleteStaffButton->setStyleSheet("color:#c3263a;border-color:#f3c9cf;");connect(deleteStaffButton,&QPushButton::clicked,this,[this]{deleteMember();});
        overview=new QWidget(this);overview->setObjectName("overviewPanel");overview->setGeometry(325,778,1312,162);
        new Glyph(overview,"document",QRect(13,9,23,23));text(overview,"Staff Information",QRect(44,7,245,23),14,ink,true);
        smallEditButton=button(overview,"Edit","edit",QRect(369,7,66,26));smallEditButton->setProperty("small",true);connect(smallEditButton,&QPushButton::clicked,this,[this]{editMember(false);});
        const QStringList labels{"Staff ID","Full Name","Role","Department","Email","Phone"};
        for(int i=0;i<6;++i){text(overview,labels[i],QRect(15,39+i*19,119,21),12,muted);infoValues<<text(overview,"",QRect(140,39+i*19,303,21),12,QColor("#233f7d"));}
        new Glyph(overview,"tool",QRect(475,9,24,24));text(overview,"System Access",QRect(509,7,220,23),14,ink,true);
        accessButton=button(overview,"Edit Access","edit",QRect(763,7,103,26));accessButton->setObjectName("editStaffAccess");accessButton->setProperty("small",true);connect(accessButton,&QPushButton::clicked,this,[this]{editPermissions();});
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};const QStringList descriptions{"View, manage, and monitor weather stations","Manage sensors and equipment inventory","Create and manage maintenance tasks","View operational services and service status"};
        for(int i=0;i<4;++i){auto *box=new AccessCheckBox(overview);box->setAccessibleName(modules[i]);box->setGeometry(480,40+i*27,22,20);access<<box;text(overview,modules[i],QRect(510,40+i*27,95,20),11,muted);text(overview,descriptions[i],QRect(603,40+i*27,267,20),11,muted);connect(box,&QCheckBox::toggled,this,[this,i](bool value){auto values=permissions.value(selectedId,{false,false,false,false});values[i]=value;changeAccess(values);});}
        new Glyph(overview,"clock",QRect(905,9,24,24));text(overview,"Recent Activity",QRect(940,7,235,23),14,ink,true);
        auto *viewAll=button(overview,"View All",{},QRect(1238,7,62,26));viewAll->setProperty("small",true);connect(viewAll,&QPushButton::clicked,this,[this]{showAudit(selectedId);});
        for(int i=0;i<5;++i){recentTexts<<text(overview,"",QRect(961,38+i*23,218,21),11,ink);recentDates<<text(overview,"",QRect(1182,38+i*23,123,21),11,muted);}
        tabContent=new QWidget(this);tabContent->setGeometry(325,778,1312,162);tabContent->hide();
        tabBrowser=new QTextBrowser(tabContent);tabBrowser->setObjectName("staffTabContent");tabBrowser->setGeometry(12,5,1288,152);tabBrowser->setFrameShape(QFrame::NoFrame);tabBrowser->setStyleSheet("background:transparent;color:#36598d;");tabBrowser->setOpenExternalLinks(false);
    }
    void populateRoles() {
        QSignalBlocker a(roleFilter),b(listRoles); const QString current=roleFilter->currentText();
        QStringList roles;for(const auto &m:members)if(!roles.contains(m.role))roles<<m.role;
        roleFilter->clear();roleFilter->addItem("Role");roleFilter->addItems(roles);listRoles->clear();listRoles->addItem("All Roles");listRoles->addItems(roles);
        const int index=roleFilter->findText(current);if(index>0){roleFilter->setCurrentIndex(index);listRoles->setCurrentIndex(index);}
    }
    void syncHeader() { bool all=table->rowCount()>0;for(int r=0;r<table->rowCount();++r)all=all && table->item(r,0)->checkState()==Qt::Checked;header->checked=all;header->viewport()->update(); }
    void refresh(bool reveal=false) {
        if(!table)return;
        filtered.clear();
        const QString query=search->text().trimmed();
        for(int i=0;i<members.size();++i){const auto &m=members[i];if(!(m.id+" "+m.name+" "+m.email+" "+m.role+" "+m.department).contains(query,Qt::CaseInsensitive))continue;if(roleFilter->currentIndex()>0 && m.role!=roleFilter->currentText())continue;if(statusFilter->currentIndex()>0 && m.status!=statusFilter->currentText())continue;filtered<<i;}
        auto key=[this](const Member &m){switch(header->sortColumn){case 1:return m.id;case 2:return m.name;case 3:return m.email;case 4:return m.role;case 5:return m.department;case 6:return m.status;case 7:return QLocale(QLocale::English).toDateTime(m.login,"MMM d, yyyy HH:mm").toString(Qt::ISODate);default:return m.id;}};
        std::stable_sort(filtered.begin(),filtered.end(),[&](int left,int right){int order=QString::compare(key(members[left]),key(members[right]),Qt::CaseInsensitive);if(order==0)order=QString::compare(members[left].id,members[right].id);return header->sortOrder==Qt::AscendingOrder?order<0:order>0;});
        if(reveal)for(int i=0;i<filtered.size();++i)if(members[filtered[i]].id==selectedId){page=i/8;break;}
        page=qBound(0,page,qMax(0,(int(filtered.size())-1)/8));
        const QSignalBlocker block(table);table->clearSelection();table->setRowCount(0);
        const int end=qMin((page+1)*8,int(filtered.size()));int selectedRow=-1;
        for(int n=page*8;n<end;++n){const auto &m=members[filtered[n]];const int row=table->rowCount();table->insertRow(row);
            const QStringList values{"",m.id,m.name,m.email,m.role,m.department,m.status,m.login,""};
            for(int c=0;c<9;++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);if(c==0){item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(checked.contains(m.id)?Qt::Checked:Qt::Unchecked);}if(c==8)item->setToolTip("Staff actions");table->setItem(row,c,item);}
            if(m.id==selectedId)selectedRow=row;
        }
        if(table->rowCount()){if(selectedRow<0){selectedRow=0;selectedId=table->item(0,1)->text();}table->selectRow(selectedRow);}else selectedId.clear();
        pagination->setText(filtered.isEmpty()?"0 of 0":QString("%1-%2 of %3").arg(page*8+1).arg(end).arg(filtered.size()));
        previous->setEnabled(page>0);next->setEnabled(end<filtered.size());emptyList->setVisible(filtered.isEmpty());syncHeader();updateDetails();update();
    }
    void updateDetails() {
        auto *m=selected();
        editStaffButton->setEnabled(writable && m);deleteStaffButton->setEnabled(writable && m);smallEditButton->setEnabled(writable && m);accessButton->setEnabled(writable && m);trainingButton->setEnabled(writable && m);status->setVisible(m!=nullptr);for(auto *b:tabs)b->setEnabled(m!=nullptr);
        for(auto *box:access)box->setEnabled(writable && m);
        if(!m){name->setText("No matching staff");name->setToolTip({});role->clear();description->clear();status->clear();email->clear();email->setToolTip({});phone->clear();phone->setToolTip({});joined->clear();for(auto *label:infoValues)label->clear();overview->hide();tabContent->hide();return;}
        setFittedText(name,m->name);const int badgeX=name->x()+QFontMetrics(name->font()).horizontalAdvance(name->text())+16;status->move(badgeX,674);
        status->setText(m->status);status->setStyleSheet(QString("font-size:12px;border-radius:12px;background:%1;color:%2;").arg(m->status=="Active"?"#d9fae9":"#ffe0e3",m->status=="Active"?"#009146":"#d00019"));
        setFittedText(role,m->role);setFittedText(description,m->id=="STF-2025-008"?"Supports station operations, sensor calibration, and system troubleshooting.":"Team member in "+m->department+" · "+m->role);
        setFittedText(email,m->email);setFittedText(phone,m->phone);joined->setText(m->joined);
        const QStringList values{m->id,m->name,m->role,m->department,m->email,m->phone};for(int i=0;i<6;++i){setFittedText(infoValues[i],values[i]);}
        const auto colors=roleColors(m->role);infoValues[2]->setGeometry(139,81, qMin(292,QFontMetrics(uiFont(11)).horizontalAdvance(m->role)+24),19);infoValues[2]->setStyleSheet(QString("border-radius:10px;background:%1;color:%2;font-size:11px;padding-left:12px;").arg(colors.second.name(),colors.first.name()));
        infoValues[2]->setText(QFontMetrics(uiFont(11)).elidedText(m->role,Qt::ElideRight,infoValues[2]->width()-24));
        const auto permissionsForMember=permissions.value(m->id,{false,false,false,false});for(int i=0;i<4;++i){QSignalBlocker blocker(access[i]);access[i]->setChecked(permissionsForMember[i]);access[i]->setToolTip(effectiveAccess(data,m->id,i)?"Effective access: allowed":"Effective access: denied (inactive member or permission not granted)");}
        const auto history=memberAudit();for(int i=0;i<5;++i){setFittedText(recentTexts[i],i<history.size()?history[i].action:(i==0?"No activity recorded yet":""));recentTexts[i]->setToolTip(i<history.size()?history[i].actor+" — "+history[i].details:QString());recentDates[i]->setText(i<history.size()?QDateTime::fromString(history[i].timestamp,Qt::ISODateWithMs).toLocalTime().toString("MMM d, HH:mm"):QString());}
        setTab(tab);
    }
    void setTab(int index) {
        tab=index;for(int i=0;i<tabs.size();++i){tabs[i]->setProperty("active",i==index);tabs[i]->style()->unpolish(tabs[i]);tabs[i]->style()->polish(tabs[i]);}
        overview->setVisible(index==0 && selected());tabContent->setVisible(index!=0 && selected());
        const auto *m=selected();if(!m)return;
        if(index==1){QString html="<b>"+m->role.toHtmlEscaped()+" — Effective permissions</b><br>";const QStringList modules{"Stations","Equipment","Maintenance","Services"};for(int i=0;i<4;++i)html+=modules[i]+": "+(effectiveAccess(data,m->id,i)?"Allowed":"Denied")+"<br>";html+="<br>Inactive members have no effective access. Manage grants using Edit Access.";tabBrowser->setHtml(html);}
        if(index==2){const auto s=statistics(data);QString html=QString("<b>Staff statistics — all members</b><br>Total: %1 &nbsp; Active: %2 &nbsp; Inactive: %3<br><br><b>By department</b><br>").arg(s.total).arg(s.active).arg(s.inactive);for(auto it=s.departments.begin();it!=s.departments.end();++it)html+=it.key().toHtmlEscaped()+": "+QString::number(it.value())+"<br>";tabBrowser->setHtml(html);}
        if(index==3){QString html="<b>Activity Log — "+m->name.toHtmlEscaped()+"</b><br><br>";const auto history=memberAudit();if(history.isEmpty())html+="No activity recorded yet.";for(const auto &e:history)html+=QDateTime::fromString(e.timestamp,Qt::ISODateWithMs).toLocalTime().toString("yyyy-MM-dd HH:mm:ss")+" — "+e.actor.toHtmlEscaped()+" — <b>"+e.action.toHtmlEscaped()+"</b>: "+e.details.toHtmlEscaped()+"<br>";tabBrowser->setHtml(html);}
        if(index==4)tabBrowser->setPlainText(m->note.isEmpty()?"No note saved. Use Edit Staff to add a note.":m->note);
        update();
    }
    void editMember(bool add) {
        Member *existing=selected();if(!writable || (!add && !existing))return;
        const Member original=add?Member{}:*existing;
        QDialog dialog(this);dialog.setWindowTitle(add?"Add Staff":"Edit Staff");dialog.setMinimumWidth(520);auto *form=new QFormLayout(&dialog);
        dialog.setObjectName("staffEditor");
        auto *fullName=new QLineEdit(add?"":existing->name,&dialog),*mail=new QLineEdit(add?"":existing->email,&dialog),*mobile=new QLineEdit(add?"":existing->phone,&dialog);
        fullName->setObjectName("memberName");mail->setObjectName("memberEmail");mobile->setObjectName("memberPhone");
        fullName->setMaxLength(100);mail->setMaxLength(254);mobile->setMaxLength(40);
        auto *memberRole=new QComboBox(&dialog);memberRole->setEditable(true);for(int i=1;i<roleFilter->count();++i)memberRole->addItem(roleFilter->itemText(i));if(!add)memberRole->setCurrentText(existing->role);
        auto *department=new QComboBox(&dialog);department->setEditable(true);department->addItems({"Operations","Field Operations","Maintenance","Engineering","IT"});if(!add)department->setCurrentText(existing->department);
        auto *state=new QComboBox(&dialog);state->addItems({"Active","Inactive"});if(!add)state->setCurrentText(existing->status);
        auto *note=new QLineEdit(add?"":existing->note,&dialog);
        memberRole->setObjectName("memberRole");department->setObjectName("memberDepartment");state->setObjectName("memberStatus");note->setObjectName("memberNotes");
        memberRole->lineEdit()->setMaxLength(80);department->lineEdit()->setMaxLength(80);note->setMaxLength(1000);
        form->addRow("Full name",fullName);form->addRow("Email",mail);form->addRow("Phone",mobile);form->addRow("Role",memberRole);form->addRow("Department",department);form->addRow("Status",state);form->addRow("Notes",note);
        auto *error=new QLabel(&dialog);error->setObjectName("staffValidationError");error->setWordWrap(true);error->setStyleSheet("color:#d62038;");form->addRow(error);
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);form->addRow(buttons);
        connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{
            Member candidate=original;candidate.name=fullName->text().trimmed();candidate.email=mail->text().trimmed();candidate.role=memberRole->currentText().trimmed();candidate.department=department->currentText().trimmed();candidate.status=state->currentText();candidate.phone=mobile->text().trimmed();candidate.note=note->text();
            const QString validation=validateMember(data,candidate,add?QString():original.id);
            if(!validation.isEmpty()){error->setText(validation);return;}
            dialog.accept();
        });
        if(dialog.exec()!=QDialog::Accepted)return;
        Member candidate=original;
        candidate.name=fullName->text().trimmed();candidate.email=mail->text().trimmed();candidate.phone=mobile->text().trimmed();candidate.role=memberRole->currentText().trimmed();candidate.department=department->currentText().trimmed();candidate.status=state->currentText();candidate.note=note->text();
        const QStringList oldValues{original.name,original.email,original.phone,original.role,original.department,original.status,original.note};
        const QStringList newValues{candidate.name,candidate.email,candidate.phone,candidate.role,candidate.department,candidate.status,candidate.note};
        if(!add && oldValues==newValues)return;
        const State before=data;
        if(add){
            do {
                if(data.nextId==std::numeric_limits<int>::max()){data=before;QMessageBox::warning(this,"Cannot add Staff","Staff identifier limit reached.");return;}
                candidate.id=QString("STF-%1-%2").arg(QDate::currentDate().year()).arg(data.nextId++,3,10,QChar('0'));
            }while(std::any_of(members.cbegin(),members.cend(),[&](const Member &m){return m.id==candidate.id;}));
            candidate.login="Never";candidate.joined=QLocale(QLocale::English).toString(QDate::currentDate(),"MMM d, yyyy");
            members.prepend(candidate);permissions.insert(candidate.id,{false,false,false,false});
        }else{for(int i=0;i<members.size();++i)if(members[i].id==candidate.id){members[i]=candidate;break;}}
        selectedId=candidate.id;
        const QStringList fields{"name","email","phone","role","department","status","note"};QStringList changes;
        for(int i=0;i<fields.size();++i)if(oldValues[i]!=newValues[i])changes<<fields[i];
        {QSignalBlocker a(search),b(roleFilter),c(listRoles),d(statusFilter);search->clear();roleFilter->setCurrentIndex(0);listRoles->setCurrentIndex(0);statusFilter->setCurrentIndex(0);}
        persist(before,candidate.id,candidate.name,add?"Staff created":"Staff updated",add?"New member; module permissions default to denied.":"Changed fields: "+changes.join(", "),true);
    }
    void editPermissions() {
        if(!writable || !selected())return;
        QDialog dialog(this);dialog.setObjectName("staffAccessEditor");dialog.setWindowTitle("Edit System Access");dialog.setMinimumWidth(480);auto *layout=new QVBoxLayout(&dialog);QList<QCheckBox*> boxes;const QStringList modules{"Stations","Equipment","Maintenance","Services"};const auto values=permissions.value(selectedId,{false,false,false,false});
        auto *explanation=new QLabel("Choose module grants. Inactive members have no effective access.\nThese settings are saved locally; no user login is provided here.",&dialog);explanation->setWordWrap(true);layout->addWidget(explanation);
        auto *profile=new QComboBox(&dialog);profile->setObjectName("accessProfile");profile->addItems({"Custom","Administrator — all modules","Field technician — stations and equipment","Maintenance — equipment and maintenance","No access"});layout->addWidget(profile);
        for(int i=0;i<4;++i){auto *box=new QCheckBox(modules[i],&dialog);box->setChecked(values[i]);layout->addWidget(box);boxes<<box;}
        connect(profile,&QComboBox::currentIndexChanged,&dialog,[&](int index){const QList<QList<bool>> profiles{{},{true,true,true,true},{true,true,false,false},{false,true,true,false},{false,false,false,false}};if(index>0)for(int i=0;i<4;++i){QSignalBlocker blocker(boxes[i]);boxes[i]->setChecked(profiles[index][i]);}});
        for(auto *box:boxes)connect(box,&QCheckBox::toggled,&dialog,[&]{QSignalBlocker blocker(profile);profile->setCurrentIndex(0);});
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()==QDialog::Accepted){QList<bool> changed;for(auto *box:boxes)changed<<box->isChecked();changeAccess(changed);}
    }
    void deleteMember() {
        if(!writable || !selected())return;
        const Member member=*selected();
        if(QMessageBox::question(this,"Delete Staff",QString("Delete %1 (%2)?\nThis removes the member and their permissions. The activity log is kept.").arg(member.name,member.id),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
        const State before=data;
        for(int i=0;i<members.size();++i)if(members[i].id==member.id){members.removeAt(i);break;}
        permissions.remove(member.id);
        if(persist(before,member.id,member.name,"Staff deleted","Member and module grants removed; history retained."))checked.remove(member.id);
    }
    void toggleMember() {
        if(!writable || !selected())return;
        const Member member=*selected();const State before=data;
        const QString nextStatus=member.status=="Active"?"Inactive":"Active";
        for(int i=0;i<members.size();++i)if(members[i].id==member.id){members[i].status=nextStatus;break;}
        persist(before,member.id,member.name,"Status changed",member.status+" → "+nextStatus);
    }
    void showTraining() {
        if(!writable || !selected())return;
        const Member member=*selected();const State before=data;QString details;
        if(editTraining(data,member.id,details,this))
            persist(before,member.id,member.name,"Training assessment updated",details);
    }
    void showActions() {
        auto *m=selected();if(!m || !writable)return;QMenu menu(this);menu.setObjectName("staffActions");auto *edit=menu.addAction("Edit Staff");auto *toggle=menu.addAction(m->status=="Active"?"Deactivate account":"Activate account");auto *permissionsAction=menu.addAction("Edit Access");auto *trainingAction=menu.addAction("Skills & Training");menu.addSeparator();auto *remove=menu.addAction("Delete Staff");auto *choice=menu.exec(QCursor::pos());
        if(choice==edit)editMember(false);else if(choice==toggle)toggleMember();else if(choice==permissionsAction)editPermissions();else if(choice==trainingAction)showTraining();else if(choice==remove)deleteMember();
    }
    void showStatistics() {
        QDialog dialog(this);dialog.setObjectName("statisticsDialog");dialog.setWindowTitle("Staff Statistics");dialog.resize(740,510);
        auto *layout=new QVBoxLayout(&dialog);const auto s=statistics(data);
        auto *summary=new QLabel(QString("Total: %1    Active: %2    Inactive: %3    Departments: %4")
            .arg(s.total).arg(s.active).arg(s.inactive).arg(s.departments.size()),&dialog);
        summary->setObjectName("statisticsSummary");summary->setStyleSheet("font-size:18px;font-weight:600;padding:12px;");layout->addWidget(summary);
        auto *caption=new QLabel("Calculated from all saved staff records. Bars show the share of total staff.",&dialog);caption->setWordWrap(true);layout->addWidget(caption);
        auto *grouping=new QComboBox(&dialog);grouping->setObjectName("statisticsGrouping");grouping->addItems({"By department","By role"});layout->addWidget(grouping);
        auto *grid=new QTableWidget(&dialog);grid->setObjectName("statisticsTable");grid->setColumnCount(3);grid->setHorizontalHeaderLabels({"Group","Members","Share of staff"});grid->verticalHeader()->hide();grid->setEditTriggers(QAbstractItemView::NoEditTriggers);grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);layout->addWidget(grid);
        auto populate=[&]{
            const auto counts=grouping->currentIndex()==0?s.departments:s.roles;grid->setRowCount(counts.size());int row=0;
            for(auto it=counts.begin();it!=counts.end();++it,++row){grid->setItem(row,0,new QTableWidgetItem(it.key()));grid->setItem(row,1,new QTableWidgetItem(QString::number(it.value())));auto *bar=new QProgressBar(grid);bar->setRange(0,qMax(1,s.total));bar->setValue(it.value());bar->setFormat(QString::number(s.total?100.0*it.value()/s.total:0.0,'f',1)+"%");grid->setCellWidget(row,2,bar);}
        };
        connect(grouping,&QComboBox::currentIndexChanged,&dialog,[&]{populate();});populate();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
    }
    void showAudit(const QString &initialQuery={}) {
        QDialog dialog(this);dialog.setObjectName("auditDialog");dialog.setWindowTitle("Staff Activity Log");dialog.resize(1100,520);
        auto *layout=new QVBoxLayout(&dialog);auto *caption=new QLabel("Saved Staff actions — date, local operator, member and change. Deleted members remain in this history.",&dialog);caption->setWordWrap(true);layout->addWidget(caption);
        auto *query=new QLineEdit(&dialog);query->setObjectName("auditSearch");query->setPlaceholderText("Search member, operator, action or details...");query->setClearButtonEnabled(true);query->setText(initialQuery);layout->addWidget(query);
        auto *grid=new QTableWidget(&dialog);grid->setObjectName("auditTable");grid->setColumnCount(6);grid->setHorizontalHeaderLabels({"Date / time","Operator","Staff ID","Name","Action","Details"});grid->verticalHeader()->hide();grid->setEditTriggers(QAbstractItemView::NoEditTriggers);grid->setSelectionBehavior(QAbstractItemView::SelectRows);grid->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);grid->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Stretch);layout->addWidget(grid);
        auto *count=new QLabel(&dialog);count->setObjectName("auditCount");layout->addWidget(count);
        auto populate=[&]{grid->setRowCount(0);for(const auto &entry:data.audit){
            const QString date=QDateTime::fromString(entry.timestamp,Qt::ISODateWithMs).toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
            const QStringList values{date,entry.actor,entry.memberId,entry.memberName,entry.action,entry.details};
            if(!values.join(' ').contains(query->text().trimmed(),Qt::CaseInsensitive))continue;
            int row=grid->rowCount();grid->insertRow(row);for(int c=0;c<values.size();++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);grid->setItem(row,c,item);}
        }count->setText(QString("%1 matching actions / %2 recorded").arg(grid->rowCount()).arg(data.audit.size()));};
        connect(query,&QLineEdit::textChanged,&dialog,[&]{populate();});populate();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
    }
    void exportPdf() {
        QString path=QFileDialog::getSaveFileName(this,"Export Staff PDF","SkySense-Staff.pdf","PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";
        QSaveFile output(path);if(!output.open(QIODevice::WriteOnly)){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        {QPdfWriter writer(&output);writer.setPageSize(QPageSize(QPageSize::A4));writer.setPageOrientation(QPageLayout::Landscape);writer.setResolution(96);writer.setTitle("SkySense — Staff");
        QString html="<h1>SkySense — Staff</h1><p>Generated "+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")+" · "+QString::number(filtered.size())+" matching members</p><table border='1' cellspacing='0' cellpadding='7'><tr><th>Staff ID</th><th>Name</th><th>Email</th><th>Role</th><th>Department</th><th>Status</th></tr>";
        for(int i:filtered){const auto &m=members[i];html+="<tr>";for(const auto &value:QStringList{m.id,m.name,m.email,m.role,m.department,m.status})html+="<td>"+value.toHtmlEscaped()+"</td>";html+="</tr>";}QTextDocument document;document.setHtml(html+"</table>");document.print(&writer);}
        if(!output.commit()){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        QMessageBox::information(this,"Export PDF","Staff report saved successfully.");
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.fillRect(rect(),QColor("#f6faff"));
        QLinearGradient body(285,66,1672,941);body.setColorAt(0,QColor("#f4faff"));body.setColorAt(1,Qt::white);p.fillRect(QRect(285,66,1387,875),body);
        // Exact decorative artwork; no screenshot text or interactive area is used.
        p.fillRect(QRect(0,0,285,H),QColor("#031b2c"));
        p.drawPixmap(QRect(0,444,285,398),reference(),QRect(0,444,285,398));
        p.drawPixmap(QRect(15,42,253,77),reference(),QRect(15,42,253,77));
        p.setPen(QColor("#16415a"));p.drawLine(27,842,258,842);p.setPen(Qt::NoPen);p.setBrush(QColor("#00d195"));p.drawEllipse(QPointF(40,865),7,7);p.drawEllipse(QPointF(40,891),7,7);
        p.fillRect(QRect(285,0,3,H),QColor("#d9e5f3"));
        QLinearGradient top(0,0,0,66);top.setColorAt(0,QColor("#f8fbff"));top.setColorAt(1,QColor("#fdfefe"));p.fillRect(QRect(288,0,1384,66),top);
        QLinearGradient shadow(0,66,0,77);shadow.setColorAt(0,QColor(161,190,227,30));shadow.setColorAt(1,QColor(161,190,227,0));p.fillRect(QRect(288,66,1384,11),shadow);
        p.setPen(QColor("#cedff4"));p.drawLine(1387,18,1387,53);
        const auto s=statistics(data);
        const QStringList titles{"Total Staff","Active Staff","Inactive Staff","Departments"};const QStringList numbers{QString::number(s.total),QString::number(s.active),QString::number(s.inactive),QString::number(s.departments.size())};
        const QStringList suffixes{"members",QString("/ %1 total").arg(s.total),QString("/ %1 total").arg(s.total),"teams"};
        const QList<int> amounts{s.total,s.active,s.inactive,int(s.departments.size())};
        const QStringList captions{"All saved staff",QString::number(s.total?100.0*s.active/s.total:0.0,'f',1)+"% of staff",QString::number(s.total?100.0*s.inactive/s.total:0.0,'f',1)+"% of staff",QString::number(s.roles.size())+" distinct roles"};
        const QColor accents[]{blue,QColor("#00c471"),QColor("#6f89b2"),QColor("#4f12ff")};const QColor fills[]{QColor("#f2f9ff"),QColor("#f2fdfa"),QColor("#fbfdff"),QColor("#f8f6ff")};const QStringList icons{"people","addperson","inactive","grid"};
        const int xs[]{315,651,987,1326}, widths[]{323,323,325,324};
        for(int i=0;i<4;++i){const int x=xs[i];rounded(p,QRectF(x,157,widths[i],112),fills[i]);QColor bubble=accents[i];bubble.setAlpha(21);p.setPen(Qt::NoPen);p.setBrush(bubble);p.drawEllipse(QRectF(x+21,172,55,55));paintIcon(p,icons[i],QRectF(x+34,184,30,30),accents[i]);
            p.setFont(uiFont(14,true));p.setPen(ink);p.drawText(QRect(x+102,171,219,24),Qt::AlignVCenter,titles[i]);auto numberFont=uiFont(numbers[i].size()>4?22:30,true);numberFont.setWeight(QFont::Bold);p.setFont(numberFont);const int numberWidth=QFontMetrics(numberFont).horizontalAdvance(numbers[i]);p.drawText(QRect(x+102,192,numberWidth+4,38),Qt::AlignVCenter,numbers[i]);
            p.setFont(uiFont(14));p.setPen(muted);p.drawText(QRect(x+110+numberWidth,205,qMax(0,widths[i]-125-numberWidth),25),Qt::AlignVCenter,suffixes[i]);
            p.setFont(uiFont(12));p.drawText(QRect(x+102,232,207,22),Qt::AlignVCenter,captions[i]);
            if(i==1 || i==2){p.setPen(Qt::NoPen);p.setBrush(QColor("#e4edf8"));p.drawRoundedRect(QRectF(x+102,258,195,4),2,2);
                p.setBrush(accents[i]);p.drawRoundedRect(QRectF(x+102,258,195.0*amounts[i]/qMax(1,s.total),4),2,2);}
        }
        rounded(p,QRectF(315,281,1335,369),Qt::white);
        p.setPen(QColor("#e7effb"));p.drawLine(1482,294,1482,324);
        rounded(p,QRectF(315,663,1335,278),Qt::white);p.setPen(QColor("#e5eefb"));p.drawLine(316,741,1649,741);p.drawLine(325,772,1430,772);p.drawLine(1118,679,1118,714);p.drawLine(1331,679,1331,714);p.drawLine(1505,679,1505,714);
        if(tab==0 && selected()){rounded(p,QRectF(325,778,449,162),QColor("#fdfeff"));rounded(p,QRectF(785,778,420,162),QColor("#fdfeff"));rounded(p,QRectF(1216,778,421,162),QColor("#fdfeff"));
            p.setPen(line);p.drawLine(335,814,764,814);p.drawLine(795,814,1195,814);p.drawLine(1226,814,1627,814);
            const int recentCount=qMin(5,int(memberAudit().size()));if(recentCount>1){p.setPen(QPen(QColor("#d8e4f3"),2));p.drawLine(1260,827,1260,827+(recentCount-1)*23);}
            for(int i=0;i<recentCount;++i){p.setPen(Qt::NoPen);p.setBrush(i==0?QColor("#00b654"):blue);p.drawEllipse(QPointF(1260,827+i*23),7,7);if(i==0)paintIcon(p,"check",QRectF(1254,821,12,12),Qt::white);}
        }else if(selected())rounded(p,QRectF(325,778,1312,162),QColor("#fdfeff"));
    }
};

class StaffView final : public QGraphicsView {
    StaffCanvas *canvas;
public:
    explicit StaffView(QWidget *parent):QGraphicsView(parent) {
        setObjectName("staffView");setFrameShape(QFrame::NoFrame);setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setAlignment(Qt::AlignCenter);
        setBackgroundBrush(QColor("#f5f9ff"));setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
        QString path=parent->property("staffStoragePath").toString();if(path.isEmpty())path=qApp->property("staffStoragePath").toString();if(path.isEmpty())path=defaultStoragePath();
        auto *scene=new QGraphicsScene(this);setScene(scene);canvas=new StaffCanvas(path);scene->addWidget(canvas);scene->setSceneRect(0,0,W,H);
        canvas->setProperty("referenceWidth",W);
    }
    StaffCanvas *designCanvas() const { return canvas; }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        const qreal scale = qMin(viewport()->width() / qreal(W),
                                 viewport()->height() / qreal(H));
        setTransform(QTransform::fromScale(scale, scale));
    }
};
}

void setupStaffUi(QWidget *root) {
    root->setObjectName("staffRoot");auto *layout=new QVBoxLayout(root);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);auto *view=new StaffView(root);layout->addWidget(view);
    root->setProperty("staffCanvas",QVariant::fromValue(static_cast<QWidget*>(view->designCanvas())));
}
