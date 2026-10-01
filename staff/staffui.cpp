#include "staffui.h"
#include <QtWidgets>
#include <algorithm>

namespace {
const QColor ink("#080d34"), blue("#006cff"), muted("#526faa"), line("#e0edfd");
constexpr int W = 1672, H = 941;

QFont uiFont(int size, bool bold = false) {
    QFont result("Segoe UI"); result.setPixelSize(size);
    result.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    return result;
}
const QPixmap &reference() { static QPixmap image(":/assets/reference.png"); return image; }

// Only decorative artwork and portraits are sampled from the supplied reference.
// All text, cards, table cells, badges, icons and controls are genuine Qt elements.
QPixmap portrait(int index) {
    static QHash<int, QPixmap> cache;
    if (!cache.contains(index)) {
        QRect source;
        if (index == 4) source = QRect(326, 671, 62, 62);
        else if (index == 0) source = QRect(1413, 13, 47, 47);
        else source = QRect(517, 374 + index * 34, 31, 31);
        cache.insert(index, reference().copy(source));
    }
    return cache.value(index);
}

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
class Avatar final : public QWidget {
public:
    int index;
    Avatar(QWidget *parent, int index, QRect rect): QWidget(parent), index(index) { setGeometry(rect); }
protected:
    void paintEvent(QPaintEvent *) override { QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setRenderHint(QPainter::SmoothPixmapTransform); QPainterPath clip; clip.addEllipse(QRectF(rect())); p.setClipPath(clip); p.drawPixmap(rect(), portrait(index)); }
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

struct Member {
    QString id, name, email, role, department, status, login, phone, joined;
    int avatar = 0;
    QString note{};
};
QList<Member> demoMembers() {
    return {
        {"STF-2025-012","Alex Martin","alex.martin@skysense.com","Operations Manager","Operations","Active","Apr 24, 2025 09:42","+1 (555) 234-5671","Jan 14, 2022",0},
        {"STF-2025-011","Sophia Chen","sophia.chen@skysense.com","Station Technician","Field Operations","Active","Apr 24, 2025 08:15","+1 (555) 234-5672","Mar 03, 2023",1},
        {"STF-2025-010","James Wilson","james.wilson@skysense.com","Maintenance Lead","Maintenance","Active","Apr 23, 2025 17:28","+1 (555) 234-5673","Sep 18, 2023",2},
        {"STF-2025-009","Emma Thompson","emma.thompson@skysense.com","Data Analyst","Operations","Active","Apr 24, 2025 07:55","+1 (555) 234-5674","Feb 12, 2021",3},
        {"STF-2025-008","Daniel Kim","daniel.kim@skysense.com","Technical Specialist","Engineering","Active","Apr 24, 2025 10:12","+1 (555) 234-5678","Jan 15, 2024",4},
        {"STF-2025-007","Olivia Davis","olivia.davis@skysense.com","Administrator","IT","Active","Apr 23, 2025 16:03","+1 (555) 234-5676","Jun 27, 2022",5},
        {"STF-2025-006","Ryan Patel","ryan.patel@skysense.com","Support Specialist","Operations","Inactive","Apr 20, 2025 11:44","+1 (555) 234-5677","Nov 08, 2024",6},
        {"STF-2025-005","Isabella Garcia","isabella.garcia@skysense.com","Field Technician","Field Operations","Active","Apr 24, 2025 06:31","+1 (555) 234-5679","Oct 08, 2023",7},
        {"STF-2025-004","Noah Brown","noah.brown@skysense.com","Station Technician","Field Operations","Active","Apr 24, 2025 08:00","+1 (555) 234-5680","Feb 08, 2024",2},
        {"STF-2025-003","Mia Johnson","mia.johnson@skysense.com","Data Analyst","Operations","Active","Apr 24, 2025 08:20","+1 (555) 234-5681","Mar 12, 2024",3},
        {"STF-2025-002","Ethan Lee","ethan.lee@skysense.com","Technical Specialist","Engineering","Active","Apr 24, 2025 08:30","+1 (555) 234-5682","Apr 10, 2024",0},
        {"STF-2025-001","Ava Taylor","ava.taylor@skysense.com","Field Technician","Field Operations","Active","Apr 24, 2025 09:00","+1 (555) 234-5683","May 04, 2024",5}
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
    explicit StaffHeader(QWidget *parent):QHeaderView(Qt::Horizontal,parent) { setSectionsClickable(true); setFixedHeight(35); }
protected:
    void paintSection(QPainter *p, const QRect &r, int section) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->fillRect(r,QColor("#f3f8ff")); p->setPen(QColor("#e9f1fc")); p->drawLine(r.bottomLeft(),r.bottomRight());
        if(section) p->drawLine(r.topLeft()+QPoint(0,5),r.bottomLeft()-QPoint(0,5));
        if(section==0) { rounded(*p,QRectF(r.left()+14,r.top()+10,16,16),checked?blue:QColor(Qt::white),checked?blue:QColor("#a3b1c6"),3); if(checked) paintIcon(*p,"check",QRectF(r.left()+14,r.top()+10,16,16),Qt::white); }
        else { p->setFont(uiFont(12)); p->setPen(QColor("#173873")); const QString title=model()->headerData(section,Qt::Horizontal).toString(); p->drawText(r.adjusted(16,0,-5,0),Qt::AlignVCenter,title);
            if(section==6) { const int x=r.left()+62, y=r.center().y(); p->setPen(QPen(blue,1.8)); p->drawLine(x,y-5,x,y+5); p->drawLine(x-3,y+2,x,y+5); p->drawLine(x,y+5,x+3,y+2); }
        } p->restore();
    }
};
enum { AvatarRole=Qt::UserRole+1 };
class StaffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); p->setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect r=option.rect; const bool selected=option.state & QStyle::State_Selected; const int c=index.column();
        p->fillRect(r,selected?QColor("#e0f1ff"):QColor("#ffffff")); p->setPen(QColor("#eaf1fc")); p->drawLine(r.bottomLeft(),r.bottomRight());
        p->setFont(uiFont(12)); p->setPen(QColor("#274989"));
        if(c==0) { const bool checked=index.data(Qt::CheckStateRole).toInt()==Qt::Checked; const QRectF box(r.left()+14,r.center().y()-8,16,16); rounded(*p,box,checked?blue:QColor(Qt::white),checked?blue:QColor("#a6b3c5"),3); if(checked)paintIcon(*p,"check",box,Qt::white); }
        else if(c==2) { const QRectF a(r.left()+17,r.top()+2,29,29); p->save(); QPainterPath clip; clip.addEllipse(a); p->setClipPath(clip); p->drawPixmap(a,portrait(index.data(AvatarRole).toInt()),QRectF(portrait(index.data(AvatarRole).toInt()).rect())); p->restore(); p->setPen(ink); const QRect copy=r.adjusted(63,0,-4,0);p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width())); }
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
    QList<Member> members=demoMembers();
    QList<int> filtered;
    QSet<QString> checked{"STF-2025-008"};
    QHash<QString,QList<bool>> permissions;
    QString selectedId="STF-2025-008";
    int page=0, tab=0;
    QLineEdit *search{};
    Selector *roleFilter{},*statusFilter{},*listRoles{};
    QTableWidget *table{};
    StaffHeader *header{};
    QLabel *pagination{},*name{},*role{},*description{},*status{},*email{},*phone{},*joined{};
    Avatar *avatar{};
    QPushButton *previous{},*next{},*editStaffButton{};
    QLabel *emptyList{};
    QList<QPushButton*> tabs;
    QList<QLabel*> infoValues;
    QList<QCheckBox*> access;
    QWidget *overview{}, *tabContent{};
    QLabel *tabText{};
    QStringList events{"Logged in to system","Updated station configuration","Completed maintenance task","Added equipment record","Updated profile information"};
    QStringList eventDates{"Apr 24, 2025 10:12","Apr 24, 2025 09:45","Apr 23, 2025 16:20","Apr 22, 2025 14:11","Apr 20, 2025 11:08"};
public:
    explicit StaffCanvas(QWidget *parent=nullptr):QWidget(parent) {
        setObjectName("staffCanvas"); setFixedSize(W,H);
        QFile sheet(":/styles/staff.qss"); if(sheet.open(QIODevice::ReadOnly))setStyleSheet(QString::fromUtf8(sheet.readAll()));
        buildSidebar(); buildHeader(); buildList(); buildDetails(); refresh();
    }
private:
    Member *selected() { for(auto &m:members)if(m.id==selectedId)return &m;return nullptr; }
    void buildSidebar() {
        const QStringList names{"Stations","Equipment","Maintenance","Staff","Services"}, icons{"pin","equipment","tool","people","grid"};
        for(int i=0;i<5;++i) {
            auto *b=button(this,"",{},QRect(11,154+i*60,264,50)); b->setProperty("nav",true); b->setProperty("active",i==3);
            new Glyph(b,icons[i],QRect(20,12,27,27),i==3?Qt::white:QColor("#bbd9f6")); text(b,names[i],QRect(75,0,170,50),17,Qt::white);
            connect(b,&QPushButton::clicked,this,[this,i]{if(i!=3)QMessageBox::information(this,"SkySense", "This standalone project contains the Staff module. Connect this navigation button to your "+QStringList{"Stations","Equipment","Maintenance","Staff","Services"}[i]+" page in the main project.");});
        }
        text(this,"System Status",QRect(58,850,200,25),12,QColor("#bdd0e9"));
        text(this,"All Systems Operational",QRect(58,877,222,23),12,Qt::white);
        text(this,"Last Update: Apr 24, 2025 10:42",QRect(58,900,222,23),12,QColor("#abc7e5"));
    }
    void buildHeader() {
        search=new QLineEdit(this); search->setObjectName("staffSearch"); search->setGeometry(315,15,501,40);
        search->setPlaceholderText("Search staff, roles, departments, or email..."); search->setStyleSheet("QLineEdit { padding-left:45px; font-size:14px; background:#f4f8ff; }");
        search->setClearButtonEnabled(true);search->setAccessibleName("Search staff");
        auto searchPalette=search->palette();searchPalette.setColor(QPalette::PlaceholderText,muted);search->setPalette(searchPalette);
        new Glyph(search,"search",QRect(14,9,20,20),QColor("#7187c1"));
        new Glyph(this,"calendar",QRect(1185,24,24,24),QColor("#264788")); text(this,"Thu, Apr 24, 2025",QRect(1223,19,131,32),14,ink,true);
        new Glyph(this,"down",QRect(1350,29,14,14),muted); new Avatar(this,0,QRect(1413,13,47,47));
        text(this,"Alex Martin",QRect(1472,13,164,24),14,ink,true); text(this,"Operations Manager",QRect(1472,35,179,22),13,muted);
        new Glyph(this,"down",QRect(1626,25,18,18),ink);
        text(this,"Staff",QRect(317,70,470,54),42,QColor("#07072d"),true)->setStyleSheet("font-family:'Segoe UI';font-size:42px;font-weight:700;color:#07072d;");
        text(this,"Manage your team, assign roles, and control system access.",QRect(318,122,740,27),19,muted);
        roleFilter=new Selector(this,QRect(1063,89,139,43),{"Role"},"filter"); roleFilter->setObjectName("roleFilter");
        statusFilter=new Selector(this,QRect(1216,89,139,43),{"Status","Active","Inactive"},"filter"); statusFilter->setObjectName("statusFilter");
        auto *exportButton=button(this,"Export PDF","document",QRect(1369,89,135,43));
        exportButton->setObjectName("exportStaffPdf");
        connect(exportButton,&QPushButton::clicked,this,[this]{exportPdf();});
        auto *add=button(this,"Add Staff","plus",QRect(1518,89,132,43),true); add->setObjectName("addStaff"); connect(add,&QPushButton::clicked,this,[this]{editMember(true);});
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
        table->verticalHeader()->hide();table->verticalHeader()->setDefaultSectionSize(34);table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        const int widths[]{50,126,184,222,162,141,141,193,98}; for(int c=0;c<9;++c)table->setColumnWidth(c,widths[c]);
        table->setShowGrid(false);table->setFrameShape(QFrame::NoFrame);table->setSelectionBehavior(QAbstractItemView::SelectRows);table->setSelectionMode(QAbstractItemView::SingleSelection);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->setFocusPolicy(Qt::StrongFocus);table->setItemDelegate(new StaffDelegate(table));table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        emptyList=text(table->viewport(),"No staff match your search or filters.",QRect(200,90,900,45),16,muted);
        emptyList->setObjectName("emptyStaffList");emptyList->setAlignment(Qt::AlignCenter);emptyList->setAttribute(Qt::WA_TransparentForMouseEvents);emptyList->hide();
        connect(table,&QTableWidget::itemSelectionChanged,this,[this]{const auto rows=table->selectionModel()->selectedRows(1);if(!rows.isEmpty()){selectedId=rows.first().data().toString();updateDetails();}});
        connect(table,&QTableWidget::itemChanged,this,[this](QTableWidgetItem *item){if(item->column()!=0)return; const QString id=table->item(item->row(),1)->text();if(item->checkState()==Qt::Checked)checked.insert(id);else checked.remove(id);syncHeader();});
        connect(table,&QTableWidget::cellClicked,this,[this](int row,int column){if(column==8){selectedId=table->item(row,1)->text();showActions();}});
        connect(header,&QHeaderView::sectionClicked,this,[this](int section){if(section!=0)return;const bool set=!header->checked;for(int r=0;r<table->rowCount();++r)table->item(r,0)->setCheckState(set?Qt::Checked:Qt::Unchecked);});
    }
    void buildDetails() {
        avatar=new Avatar(this,4,QRect(326,671,62,62));
        name=text(this,"Daniel Kim",QRect(403,670,450,30),21,ink,true);name->setObjectName("staffDetailName");
        status=text(this,"Active",QRect(515,674,83,24),12,QColor("#009347"));status->setObjectName("staffDetailStatus"); status->setAlignment(Qt::AlignCenter);
        role=text(this,"Technical Specialist",QRect(403,697,520,20),13);
        description=text(this,"Supports station operations, sensor calibration, and system troubleshooting.",QRect(403,716,580,20),12,muted);
        editStaffButton=button(this,"Edit Staff","edit",QRect(990,679,114,35),true); editStaffButton->setObjectName("editStaff"); connect(editStaffButton,&QPushButton::clicked,this,[this]{editMember(false);});
        new Glyph(this,"mail",QRect(1131,687,23,23),QColor("#254a90")); email=text(this,"",QRect(1165,681,162,35),12,muted);
        new Glyph(this,"phone",QRect(1354,686,23,23));phone=text(this,"",QRect(1386,681,125,35),12,muted);
        new Glyph(this,"calendar",QRect(1518,685,26,26),QColor("#254a90")); text(this,"Joined Date",QRect(1557,680,91,19),12,muted);joined=text(this,"",QRect(1557,700,91,19),12,ink,true);
        const QStringList titles{"Overview","Role && Permissions","Assigned Stations (4)","Activity Log (12)","Notes (3)"};const int x[]{330,490,740,980,1220};const int widths[]{157,234,240,240,210};
        for(int i=0;i<5;++i){auto *b=button(this,titles[i],{},QRect(x[i],741,widths[i],31));b->setProperty("tab",true);b->setProperty("active",i==0);tabs<<b;connect(b,&QPushButton::clicked,this,[this,i]{setTab(i);});}
        overview=new QWidget(this);overview->setObjectName("overviewPanel");overview->setGeometry(325,778,1312,162);
        new Glyph(overview,"document",QRect(13,9,23,23));text(overview,"Staff Information",QRect(44,7,245,23),14,ink,true);
        auto *smallEdit=button(overview,"Edit","edit",QRect(369,7,66,26));smallEdit->setProperty("small",true);connect(smallEdit,&QPushButton::clicked,this,[this]{editMember(false);});
        const QStringList labels{"Staff ID","Full Name","Role","Department","Email","Phone"};
        for(int i=0;i<6;++i){text(overview,labels[i],QRect(15,39+i*19,119,21),12,muted);infoValues<<text(overview,"",QRect(140,39+i*19,303,21),12,QColor("#233f7d"));}
        new Glyph(overview,"tool",QRect(475,9,24,24));text(overview,"System Access",QRect(509,7,220,23),14,ink,true);
        auto *editAccess=button(overview,"Edit Access","edit",QRect(763,7,103,26));editAccess->setObjectName("editStaffAccess");editAccess->setProperty("small",true);connect(editAccess,&QPushButton::clicked,this,[this]{editPermissions();});
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};const QStringList descriptions{"View, manage, and monitor weather stations","Manage sensors and equipment inventory","Create and manage maintenance tasks","View operational services and service status"};
        for(int i=0;i<4;++i){auto *box=new AccessCheckBox(overview);box->setAccessibleName(modules[i]);box->setGeometry(480,40+i*27,22,20);box->setChecked(true);access<<box;text(overview,modules[i],QRect(510,40+i*27,95,20),11,muted);text(overview,descriptions[i],QRect(603,40+i*27,267,20),11,muted);connect(box,&QCheckBox::toggled,this,[this,i](bool value){auto values=permissions.value(selectedId,{true,true,true,true});values[i]=value;permissions.insert(selectedId,values);});}
        new Glyph(overview,"clock",QRect(905,9,24,24));text(overview,"Recent Activity",QRect(940,7,235,23),14,ink,true);
        auto *viewAll=button(overview,"View All",{},QRect(1238,7,62,26));viewAll->setProperty("small",true);connect(viewAll,&QPushButton::clicked,this,[this]{setTab(3);});
        for(int i=0;i<5;++i){text(overview,events[i],QRect(961,38+i*23,228,21),11,i<3?ink:muted);text(overview,eventDates[i],QRect(1192,38+i*23,113,21),11,muted);}
        tabContent=new QWidget(this);tabContent->setGeometry(325,778,1312,162);tabContent->hide();
        tabText=text(tabContent,"",QRect(18,8,1265,150),13,muted);tabText->setTextFormat(Qt::RichText);tabText->setAlignment(Qt::AlignTop|Qt::AlignLeft);tabText->setWordWrap(true);
    }
    void populateRoles() {
        QSignalBlocker a(roleFilter),b(listRoles); const QString current=roleFilter->currentText();
        QStringList roles;for(const auto &m:members)if(!roles.contains(m.role))roles<<m.role;
        roleFilter->clear();roleFilter->addItem("Role");roleFilter->addItems(roles);listRoles->clear();listRoles->addItem("All Roles");listRoles->addItems(roles);
        const int index=roleFilter->findText(current);if(index>0){roleFilter->setCurrentIndex(index);listRoles->setCurrentIndex(index);}
    }
    void syncHeader() { bool all=table->rowCount()>0;for(int r=0;r<table->rowCount();++r)all=all && table->item(r,0)->checkState()==Qt::Checked;header->checked=all;header->viewport()->update(); }
    void refresh() {
        if(!table)return;
        filtered.clear();
        const QString query=search->text().trimmed();
        for(int i=0;i<members.size();++i){const auto &m=members[i];if(!(m.id+" "+m.name+" "+m.email+" "+m.role+" "+m.department).contains(query,Qt::CaseInsensitive))continue;if(roleFilter->currentIndex()>0 && m.role!=roleFilter->currentText())continue;if(statusFilter->currentIndex()>0 && m.status!=statusFilter->currentText())continue;filtered<<i;}
        page=qBound(0,page,qMax(0,(int(filtered.size())-1)/8));
        const QSignalBlocker block(table);table->clearSelection();table->setRowCount(0);
        const int end=qMin((page+1)*8,int(filtered.size()));int selectedRow=-1;
        for(int n=page*8;n<end;++n){const auto &m=members[filtered[n]];const int row=table->rowCount();table->insertRow(row);
            const QStringList values{"",m.id,m.name,m.email,m.role,m.department,m.status,m.login,""};
            for(int c=0;c<9;++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);item->setData(AvatarRole,m.avatar);if(c==0){item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(checked.contains(m.id)?Qt::Checked:Qt::Unchecked);}if(c==8)item->setToolTip("Staff actions");table->setItem(row,c,item);}
            if(m.id==selectedId)selectedRow=row;
        }
        if(table->rowCount()){if(selectedRow<0){selectedRow=0;selectedId=table->item(0,1)->text();}table->selectRow(selectedRow);}else selectedId.clear();
        pagination->setText(filtered.isEmpty()?"0 of 0":QString("%1-%2 of %3").arg(page*8+1).arg(end).arg(filtered.size()));
        previous->setEnabled(page>0);next->setEnabled(end<filtered.size());emptyList->setVisible(filtered.isEmpty());syncHeader();updateDetails();update();
    }
    void updateDetails() {
        auto *m=selected();
        editStaffButton->setEnabled(m!=nullptr);status->setVisible(m!=nullptr);for(auto *b:tabs)b->setEnabled(m!=nullptr);
        if(!m){name->setText("No matching staff");name->setToolTip({});role->clear();description->clear();status->clear();email->clear();email->setToolTip({});phone->clear();phone->setToolTip({});joined->clear();avatar->hide();for(auto *label:infoValues)label->clear();overview->hide();tabContent->hide();return;}
        avatar->index=m->avatar;avatar->show();avatar->update();setFittedText(name,m->name);const int badgeX=403+QFontMetrics(name->font()).horizontalAdvance(name->text())+16;status->move(badgeX,674);
        status->setText(m->status);status->setStyleSheet(QString("font-size:12px;border-radius:12px;background:%1;color:%2;").arg(m->status=="Active"?"#d9fae9":"#ffe0e3",m->status=="Active"?"#009146":"#d00019"));
        setFittedText(role,m->role);setFittedText(description,m->id=="STF-2025-008"?"Supports station operations, sensor calibration, and system troubleshooting.":"Team member in "+m->department+" · "+m->role);
        setFittedText(email,m->email);setFittedText(phone,m->phone);joined->setText(m->joined);
        const QStringList values{m->id,m->name,m->role,m->department,m->email,m->phone};for(int i=0;i<6;++i){setFittedText(infoValues[i],values[i]);}
        const auto colors=roleColors(m->role);infoValues[2]->setGeometry(139,81, qMin(292,QFontMetrics(uiFont(11)).horizontalAdvance(m->role)+24),19);infoValues[2]->setStyleSheet(QString("border-radius:10px;background:%1;color:%2;font-size:11px;padding-left:12px;").arg(colors.second.name(),colors.first.name()));
        infoValues[2]->setText(QFontMetrics(uiFont(11)).elidedText(m->role,Qt::ElideRight,infoValues[2]->width()-24));
        const auto permissionsForMember=permissions.value(m->id,{true,true,true,true});for(int i=0;i<4;++i){QSignalBlocker blocker(access[i]);access[i]->setChecked(permissionsForMember[i]);}
        setTab(tab);
    }
    void setTab(int index) {
        tab=index;for(int i=0;i<tabs.size();++i){tabs[i]->setProperty("active",i==index);tabs[i]->style()->unpolish(tabs[i]);tabs[i]->style()->polish(tabs[i]);}
        overview->setVisible(index==0 && selected());tabContent->setVisible(index!=0 && selected());
        const auto *m=selected();if(!m)return;
        if(index==1){QString html="<b>"+m->role.toHtmlEscaped()+" — Role &amp; Permissions</b><br><br>";const QStringList modules{"Stations","Equipment","Maintenance","Services"};const auto rights=permissions.value(m->id,{true,true,true,true});for(int i=0;i<4;++i)html+=modules[i]+": "+(rights[i]?"Access enabled":"Access disabled")+"<br>";tabText->setText(html);}
        if(index==2)tabText->setText("<b>Assigned Stations (4)</b><br><br>ALP-03 — Alpine Ridge<br>COA-01 — Coastal Bay<br>FOR-12 — Pine Forest<br>VAL-07 — Green Valley");
        if(index==3){QString html="<b>Activity Log — "+m->name.toHtmlEscaped()+"</b><br><br>";for(int i=0;i<events.size();++i)html+=eventDates[i]+" &nbsp; "+events[i]+"<br>";tabText->setText(html);}
        if(index==4)tabText->setText("<b>Notes</b><br><br>"+(m->note.isEmpty()?QString("Station operations support.<br>Sensor calibration and field coordination.<br>System troubleshooting."):m->note.toHtmlEscaped()));
        update();
    }
    void editMember(bool add) {
        Member *existing=selected();if(!add && !existing)return;
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
            const QString address=mail->text().trimmed();
            static const QRegularExpression emailPattern(QStringLiteral(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)"));
            if(fullName->text().trimmed().isEmpty()){error->setText("Enter the staff member's full name.");fullName->setFocus();return;}
            if(!emailPattern.match(address).hasMatch()){error->setText("Enter a valid email address, for example name@skysense.com.");mail->setFocus();return;}
            if(memberRole->currentText().trimmed().isEmpty() || department->currentText().trimmed().isEmpty()){error->setText("Role and department are required.");return;}
            for(const auto &member:members)if((add || member.id!=existing->id) && member.email.compare(address,Qt::CaseInsensitive)==0){error->setText("This email address already belongs to another staff member.");mail->setFocus();return;}
            dialog.accept();
        });
        if(dialog.exec()!=QDialog::Accepted)return;
        if(add){Member m;m.id=QString("STF-2025-%1").arg(members.size()+1,3,10,QChar('0'));m.avatar=0;m.login="Never";m.joined=QDate::currentDate().toString("MMM d, yyyy");members.prepend(m);existing=&members[0];}
        existing->name=fullName->text().trimmed();existing->email=mail->text().trimmed();existing->phone=mobile->text().trimmed();existing->role=memberRole->currentText().trimmed();existing->department=department->currentText().trimmed();existing->status=state->currentText();existing->note=note->text();selectedId=existing->id;
        populateRoles();{QSignalBlocker a(search),b(roleFilter),c(listRoles),d(statusFilter);search->clear();roleFilter->setCurrentIndex(0);listRoles->setCurrentIndex(0);statusFilter->setCurrentIndex(0);}page=0;refresh();
    }
    void editPermissions() {
        if(!selected())return;
        QDialog dialog(this);dialog.setObjectName("staffAccessEditor");dialog.setWindowTitle("Edit System Access");auto *layout=new QVBoxLayout(&dialog);QList<QCheckBox*> boxes;const QStringList modules{"Stations","Equipment","Maintenance","Services"};const auto values=permissions.value(selectedId,{true,true,true,true});
        for(int i=0;i<4;++i){auto *box=new QCheckBox(modules[i],&dialog);box->setChecked(values[i]);layout->addWidget(box);boxes<<box;}
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()==QDialog::Accepted){QList<bool> changed;for(auto *box:boxes)changed<<box->isChecked();permissions.insert(selectedId,changed);updateDetails();}
    }
    void showActions() {
        auto *m=selected();if(!m)return;QMenu menu(this);auto *edit=menu.addAction("Edit Staff");auto *toggle=menu.addAction(m->status=="Active"?"Deactivate account":"Activate account");auto *permissionsAction=menu.addAction("Edit Access");auto *choice=menu.exec(QCursor::pos());
        if(choice==edit)editMember(false);else if(choice==toggle){m->status=m->status=="Active"?"Inactive":"Active";refresh();}else if(choice==permissionsAction)editPermissions();
    }
    void exportPdf() {
        QString path=QFileDialog::getSaveFileName(this,"Export Staff PDF","SkySense-Staff.pdf","PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";
        QSaveFile output(path);if(!output.open(QIODevice::WriteOnly)){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        {QPdfWriter writer(&output);writer.setPageSize(QPageSize(QPageSize::A4));writer.setPageOrientation(QPageLayout::Landscape);writer.setResolution(96);writer.setTitle("SkySense — Staff");
        QString html="<h1>SkySense — Staff</h1><p>Staff list · Demo data</p><table border='1' cellspacing='0' cellpadding='7'><tr><th>Staff ID</th><th>Name</th><th>Email</th><th>Role</th><th>Department</th><th>Status</th></tr>";
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
        int active=0;for(const auto &m:members)active+=m.status=="Active";
        const QStringList titles{"Total Staff","Active Staff","Inactive Staff","Pending Invitations"};const QStringList numbers{QString::number(members.size()),QString::number(active),QString::number(members.size()-active),"2"};
        const QStringList suffixes{"/ 20 seats",QString("/ %1 total").arg(members.size()),QString("/ %1 total").arg(members.size()),"/ 20 seats"};const QStringList trends{"+2 this month","+1 this month","No change","-1 from last month"};
        const QColor accents[]{blue,QColor("#00c471"),QColor("#6f89b2"),QColor("#4f12ff")};const QColor fills[]{QColor("#f2f9ff"),QColor("#f2fdfa"),QColor("#fbfdff"),QColor("#f8f6ff")};const QStringList icons{"people","addperson","inactive","clock"};
        const int xs[]{315,651,987,1326}, widths[]{323,323,325,324};
        for(int i=0;i<4;++i){const int x=xs[i];rounded(p,QRectF(x,157,widths[i],112),fills[i]);QColor bubble=accents[i];bubble.setAlpha(21);p.setPen(Qt::NoPen);p.setBrush(bubble);p.drawEllipse(QRectF(x+21,172,55,55));paintIcon(p,icons[i],QRectF(x+34,184,30,30),accents[i]);
            p.setFont(uiFont(14,true));p.setPen(ink);p.drawText(QRect(x+102,171,219,24),Qt::AlignVCenter,titles[i]);auto numberFont=uiFont(30,true);numberFont.setWeight(QFont::Bold);p.setFont(numberFont);p.drawText(QRect(x+102,192,48,38),Qt::AlignVCenter,numbers[i]);
            p.setFont(uiFont(14));p.setPen(muted);p.drawText(QRect(x+144,205,134,25),Qt::AlignVCenter,suffixes[i]);
            p.setPen(i==2?muted:QColor("#00b751"));p.drawText(QRect(x+126,235,190,25),Qt::AlignVCenter,trends[i]);if(i==2){p.setPen(QPen(muted,2));p.drawLine(x+104,247,x+116,247);}else paintIcon(p,i==3?"downtrend":"uptrend",QRectF(x+102,240,17,17),QColor("#00b751"));
            QColor bar=accents[i];bar.setAlpha(65);p.setPen(Qt::NoPen);p.setBrush(bar);for(int b=0;b<3;++b)p.drawRoundedRect(QRectF(x+widths[i]-47+b*11,214-b*10,6,20+b*10),1.5,1.5);
        }
        rounded(p,QRectF(315,281,1335,369),Qt::white);
        p.setPen(QColor("#e7effb"));p.drawLine(1482,294,1482,324);
        rounded(p,QRectF(315,663,1335,278),Qt::white);p.setPen(QColor("#e5eefb"));p.drawLine(316,741,1649,741);p.drawLine(325,772,1430,772);p.drawLine(1118,679,1118,714);p.drawLine(1331,679,1331,714);p.drawLine(1505,679,1505,714);
        if(tab==0 && selected()){rounded(p,QRectF(325,778,449,162),QColor("#fdfeff"));rounded(p,QRectF(785,778,420,162),QColor("#fdfeff"));rounded(p,QRectF(1216,778,421,162),QColor("#fdfeff"));
            p.setPen(line);p.drawLine(335,814,764,814);p.drawLine(795,814,1195,814);p.drawLine(1226,814,1627,814);p.setPen(QPen(QColor("#d8e4f3"),2));p.drawLine(1260,827,1260,918);
            for(int i=0;i<5;++i){p.setPen(Qt::NoPen);p.setBrush(i==0?QColor("#00b654"):i<3?blue:QColor("#bfccdc"));p.drawEllipse(QPointF(1260,827+i*23),7,7);if(i==0)paintIcon(p,"check",QRectF(1254,821,12,12),Qt::white);}
        }else if(selected())rounded(p,QRectF(325,778,1312,162),QColor("#fdfeff"));
    }
};

class StaffView final : public QGraphicsView {
    StaffCanvas *canvas;
public:
    explicit StaffView(QWidget *parent):QGraphicsView(parent) {
        setObjectName("staffView");setFrameShape(QFrame::NoFrame);setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);setAlignment(Qt::AlignCenter);
        setBackgroundBrush(QColor("#f5f9ff"));setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
        auto *scene=new QGraphicsScene(this);setScene(scene);canvas=new StaffCanvas;scene->addWidget(canvas);scene->setSceneRect(0,0,W,H);
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
