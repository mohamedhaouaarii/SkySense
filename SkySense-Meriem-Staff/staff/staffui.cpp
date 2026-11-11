#include "apptheme.h"
#include "appsession.h"
#include <QSqlDatabase>
#include "staffui.h"
#include "staffdata.h"
#include "staffdesign.h"
#include "staffstatisticscurve.h"
#include "personnel.h"
#include "staffprofile.h"
#include "staffaccountpanel.h"
#include "planningpersonnel.h"
#include <QSqlRecord>
#include <QSqlError>
#include "stafftraining.h"
#include <QtWidgets>
#include <algorithm>
#include <limits>

namespace {
using namespace Staff;
const QColor ink("#080d34"), blue("#5949bd"), line("#e0edfd");
constexpr int W = 1672, H = 822;
constexpr int integratedSidebarWidth = 288;

QFont uiFont(int size, bool bold = false) {
    QFont result("Segoe UI"); result.setPixelSize(size);
    result.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    return result;
}
void setFittedText(QLabel *label,const QString &value) {
    label->setText(value);
    label->setToolTip(value);
}
void paintCheck(QPainter &p,const QRectF &box) {
    p.save();p.translate(box.topLeft());p.scale(box.width()/24.0,box.height()/24.0);
    p.setPen(QPen(Qt::white,2.2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.drawLine(5,12,10,17);p.drawLine(10,17,20,6);p.restore();
}

void rounded(QPainter &p, QRectF rect, QColor fill, QColor stroke = line, qreal radius = 8) {
    p.setPen(AppTheme::pen(p, QPen(stroke, 1))); p.setBrush(AppTheme::brush(p, fill)); p.drawRoundedRect(rect, radius, radius);
}
void badge(QPainter &p, QRectF rect, const QString &value, QColor foreground, QColor background, int size = 12) {
    p.setPen(AppTheme::pen(p, Qt::NoPen)); p.setBrush(AppTheme::brush(p, background)); p.drawRoundedRect(rect, rect.height()/2, rect.height()/2);
    p.setPen(AppTheme::pen(p, foreground)); p.setFont(uiFont(size));
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
        p->fillRect(r,AppTheme::brush(*p,QColor("#f3f8ff"))); p->setPen(AppTheme::pen(*p,QColor("#e9f1fc"))); p->drawLine(r.bottomLeft(),r.bottomRight());

        if(section==0) { rounded(*p,QRectF(r.left()+6,r.top()+10,16,16),checked?blue:QColor(Qt::white),checked?blue:QColor("#a3b1c6"),3); if(checked) paintCheck(*p,QRectF(r.left()+6,r.top()+10,16,16)); }
        else { p->setFont(uiFont(12)); p->setPen(AppTheme::pen(*p,QColor("#173873"))); const QString title=model()->headerData(section,Qt::Horizontal).toString(); p->drawText(r.adjusted(4,0,-5,0),Qt::AlignVCenter,title);
            if(section==sortColumn) { const int x=r.right()-12, y=r.center().y(); const int d=sortOrder==Qt::AscendingOrder?-1:1; p->setPen(AppTheme::pen(*p,QPen(blue,1.8))); p->drawLine(x,y-5,x,y+5); p->drawLine(x-3,y+2*d,x,y+5*d); p->drawLine(x,y+5*d,x+3,y+2*d); }
        } p->restore();
    }
};
class StaffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        p->save(); p->setRenderHint(QPainter::Antialiasing); p->setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect r=option.rect; const bool selected=option.state & QStyle::State_Selected; const int c=index.column();
        p->fillRect(r,AppTheme::brush(*p,selected?QColor("#ece9ff"):QColor("#ffffff"))); p->setPen(AppTheme::pen(*p,QColor("#eaf1fc"))); p->drawLine(r.bottomLeft(),r.bottomRight());
        p->setFont(uiFont(12)); p->setPen(AppTheme::pen(*p,QColor("#274989")));
        if(c==0) { const bool checked=index.data(Qt::CheckStateRole).toInt()==Qt::Checked; const QRectF box(r.left()+6,r.center().y()-8,16,16); rounded(*p,box,checked?blue:QColor(Qt::white),checked?blue:QColor("#a6b3c5"),3); if(checked)paintCheck(*p,box); }
        else if(c==2) {
            const QRect chip(r.left()+3,r.center().y()-14,29,29);
            rounded(*p,chip,QColor("#e8e4ff"),QColor("#e8e4ff"),9);
            p->setPen(AppTheme::pen(*p,QColor("#453790")));p->setFont(uiFont(11,true));
            p->drawText(chip,Qt::AlignCenter,index.data(Qt::UserRole).toString());
            p->setFont(uiFont(13,true));p->setPen(AppTheme::pen(*p,ink));
            const QRect copy=r.adjusted(38,0,-3,0);
            p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width()));
        }
        else if(c==4) { const auto colors=roleColors(index.data().toString()); const QRectF cell(r.left()+2,r.center().y()-11,r.width()-5,23);badge(*p,cell,QFontMetrics(uiFont(11)).elidedText(index.data().toString(),Qt::ElideRight,int(cell.width())-5),colors.first,colors.second,11); }
        else if(c==6) { bool active=index.data().toString()=="Active"; badge(*p,QRectF(r.left()+3,r.center().y()-11,r.width()-6,23),index.data().toString(),QColor(active?"#008e3f":"#db001a"),QColor(active?"#d9fbeb":"#ffe1e4")); }
        else if(c==8) { p->setPen(Qt::NoPen); p->setBrush(AppTheme::color(QColor("#254385"),AppTheme::Role::Text)); for(int x:{-5,0,5})p->drawEllipse(QPointF(r.center().x()+x,r.center().y()),1.3,1.3); }
        else { const QRect copy=r.adjusted(4,0,-3,0);p->drawText(copy,Qt::AlignVCenter,QFontMetrics(p->font()).elidedText(index.data().toString(),Qt::ElideRight,copy.width())); }
        p->restore();
    }
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (index.column() != 0 || !(index.flags() & Qt::ItemIsUserCheckable))
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::MouseButtonDblClick) {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            const QRect hitBox(option.rect.left() + 2, option.rect.center().y() - 12, 24, 24);
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
    bool oracleMode=false;
    bool oracleLoaded=false;
    QList<int> filtered;
    QSet<QString> checked{"STF-2025-008"};
    QString selectedId="STF-2025-008";
    int page=0, tab=0;
    QLineEdit *search{};
    QComboBox *statusFilter{},*listRoles{};
    QTableWidget *table{};
    QListWidget *directoryCards{};
    QStackedWidget *directoryViews{};
    StaffHeader *header{};
    QLabel *pagination{},*name{},*role{},*status{},*email{},*phone{},*joined{};
    QPushButton *previous{},*next{},*editStaffButton{},*addStaffButton{},*deleteStaffButton{},*accessButton{},*trainingButton{};
    QLabel *emptyList{}, *profileAvatar{}, *weekCaption{}, *selectionCount{}, *accessSummary{};
    StaffDesign::Ring *accessRing{};
    QList<StaffDesign::Metric*> metrics;
    QList<QLabel*> weekDays;
    QVBoxLayout *homeLayout{};
    QHBoxLayout *panels{};
    QVBoxLayout *profileLayout{};
    QFrame *profileCard{};
    QDate reportDay = QDate::currentDate();
    QString weekKey;
    QStringList weekStates;
    QWidget *detailsBody{};
    QStackedLayout *profileStack{};
    QList<QPushButton*> tabs;
    QList<QLabel*> infoValues;
    QList<QCheckBox*> access;
    QWidget *overview{}, *tabContent{};
    StaffAccountPanel *accountPanel{};
    QTextBrowser *tabBrowser{};
    StaffStatisticsCurve *tabCurve{};
    QScrollArea *tabCurveView{};
    QList<QLabel*> recentTexts, recentDates;
    QPushButton *profileAccess{}, *profilePlanning{};
    QWidget *availabilityTools{};
    QDateEdit *availabilityDay{};
    QList<AuditEntry> deletedHistory;
    QLabel *storageStatus{};
    QStackedWidget *staffPages{};
    QPushButton *alerteEffectif{}, *exportStaffButton{}, *detailStats{}, *detailAudit{};
    QTimer *veilleEffectif{};
    QDate premiereAlerte;
    QWidget *staffListPage{};
public:
    explicit StaffCanvas(const QString &path, QWidget *parent = nullptr)
        : QWidget(parent), storage(path)
    {
        setObjectName("staffCanvas");
        setFixedSize(W - integratedSidebarWidth, H);

        QFile sheet(":/styles/staff.qss");
        if (sheet.open(QIODevice::ReadOnly))
            setStyleSheet(QString::fromUtf8(sheet.readAll()));

        const QStringList arguments = QCoreApplication::arguments();
        const int capture = arguments.indexOf("--screenshot");
        oracleMode = !qApp->property("oraclePreview").toBool()
            && !(capture >= 0 && capture + 1 < arguments.size());

        QString error;

        if (oracleMode) {
            writable = false;
            oracleLoaded = chargerPersonnel(error);
        } else {
            if (storage.exists()) {
                writable = storage.load(data, error);
            } else {
                members = demoMembers();
                for (const auto &member : members)
                    permissions.insert(member.id, {true, true, true, true});
                writable = storage.save(data, error);
            }
        }

        buildStaffPages();
        buildHeader();
        buildList();
        buildDetails();
        refresh();

        if (oracleMode) {
            veilleEffectif = new QTimer(this);
            connect(veilleEffectif,&QTimer::timeout,this,[this] {
                if (isVisible() && staffPages->currentWidget()==staffListPage) actualiserSignalEffectif();
            });
            veilleEffectif->start(60000);
            actualiserSignalEffectif();
        }

        if (!error.isEmpty()) {
            QTimer::singleShot(0, this, [this, error] {
                QMessageBox::warning(this, "Loading staff", error);
            });
        }
    }
    void openAccess() { if (oracleMode && AppSession::isAdministrator() && selected()) setTab(5); }
    void sessionChanged() {
        accountPanel->reset();
        tabs[5]->setVisible(oracleMode && AppSession::isAdministrator());
        if (staffPages->currentWidget()!=staffListPage) closeStaffPage(staffPages->currentWidget());
        tab=0;updateDetails();
    }
private:
    void buildStaffPages()
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        staffPages = new QStackedWidget(this);
        staffPages->setObjectName("staffPages");
        layout->addWidget(staffPages);
        staffListPage = new QWidget;
        staffListPage->setObjectName("staffListPage");
        staffPages->addWidget(staffListPage);
        homeLayout = new QVBoxLayout(staffListPage);
        homeLayout->setContentsMargins(18,10,18,16);
        homeLayout->setSpacing(13);

        connect(staffPages,&QStackedWidget::currentChanged,this,[this](int index) {
            if (alerteEffectif) alerteEffectif->setEnabled(index==0);
            for (const QString &action : {QString("staffStatistics"),QString("staffAudit")})
                if (auto *control=findChild<QPushButton *>(action)) control->setEnabled(index==0);
            if (auto *proxy=graphicsProxyWidget())
                if (proxy->scene())
                    for (auto *view:proxy->scene()->views()) view->verticalScrollBar()->setValue(0);
            update();
        });
    }

    QWidget *newStaffPage(const QString &objectName, const QString &title,
                          const QString &description, QVBoxLayout **layout)
    {
        auto *page = new QWidget;
        page->setObjectName(objectName);
        page->setStyleSheet(QString(
            "QWidget#%1 { background:#f6faff; }"
            "QFrame#staffFormCard { background:white; border:1px solid #d6e5f9; border-radius:12px; }"
            "QLineEdit,QComboBox { min-height:36px; }"
            "QPushButton { min-height:36px; padding:0 18px; }").arg(objectName));
        *layout = new QVBoxLayout(page);
        (*layout)->setContentsMargins(28,22,28,26);
        (*layout)->setSpacing(16);
        auto *heading = new QLabel(title,page);
        heading->setStyleSheet("font-size:30px;font-weight:700;color:#080d34;");
        (*layout)->addWidget(heading);
        auto *hint = new QLabel(description,page);
        hint->setWordWrap(true);
        hint->setStyleSheet("color:#526faa;font-size:15px;");
        (*layout)->addWidget(hint);
        staffPages->addWidget(page);
        return page;
    }

    void closeStaffPage(QWidget *page)
    {
        staffPages->setCurrentWidget(staffListPage);
        staffPages->removeWidget(page);
        page->deleteLater();
    }

    bool chargerPersonnel(QString &erreur)
    {
        Personnel personnel;
        QSqlQueryModel *modele = personnel.afficher(this);

        if (!modele) {
            erreur = personnel.derniereErreur();
            return false;
        }

        while (modele->canFetchMore())
            modele->fetchMore();

        if (modele->lastError().isValid()) {
            erreur = modele->lastError().text();
            delete modele;
            return false;
        }

        QList<Member> resultat;
        const QLocale anglais(QLocale::English);

        for (int i = 0; i < modele->rowCount(); ++i) {
            const QSqlRecord ligne = modele->record(i);
            Member membre;

            membre.id = ligne.value("ID_PERSONNEL").toString();
            membre.cin = ligne.value("CIN").toString();
            membre.nom = ligne.value("NOM").toString();
            membre.prenom = ligne.value("PRENOM").toString();
            membre.name = (membre.prenom + " " + membre.nom).trimmed();
            membre.email = ligne.value("EMAIL").toString();
            membre.phone = ligne.value("TELEPHONE").toString();
            membre.role = ligne.value("FONCTION").toString();
            membre.status = ligne.value("STATUT").toString();
            membre.debutIndisponibilite = ligne.value("DATE_DEBUT_INDISPONIBILITE").toDate();
            membre.finIndisponibilite = ligne.value("DATE_FIN_INDISPONIBILITE").toDate();
            const QDate dateEntree = ligne.value("DATE_ENTREE").toDate();
            if (dateEntree.isValid())
                membre.joined = anglais.toString(dateEntree, "MMM d, yyyy");

            resultat.append(membre);
        }

        delete modele;
        if (!loadProfileData(erreur)) return false;
        members = resultat;
        weekKey.clear();
        checked.clear();
        selectedId.clear();
        page = 0;
        return true;
    }


    bool loadProfileData(QString &error) {
        if (!StaffProfile::load(permissions,data.audit,error)) return false;
        data.audit.append(deletedHistory);
        std::stable_sort(data.audit.begin(),data.audit.end(),[](const AuditEntry &a,const AuditEntry &b) {
            return a.timestamp>b.timestamp;
        });
        return true;
    }
    bool reloadProfileData() {
        if (!oracleMode) return true;
        QString error;
        if (loadProfileData(error)) return true;
        QMessageBox::warning(dialogOwner(),"Staff profile",error);
        return false;
    }
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
        auto *member=selected();
        if (!member || rights.size()!=4 || !(oracleMode?oracleLoaded:writable)) return;
        const auto previous=permissions.value(selectedId);
        if (previous==rights) return;
        if (oracleMode) {
            const QString id=selectedId;
            QString error;
            if (!StaffProfile::saveGrants(id,previous,rights,error))
                QMessageBox::warning(dialogOwner(),"Edit access",error);
            if (reloadProfileData()) updateDetails();
            return;
        }
        const auto old=permissions.value(selectedId,{false,false,false,false});
        const State before=data;
        permissions.insert(selectedId,rights);
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};QStringList changes;
        for(int i=0;i<4;++i)if(old[i]!=rights[i])changes<<modules[i]+(rights[i]?": allowed":": denied");
        persist(before,member->id,member->name,"Access changed",changes.join("; "));
    }
    Member *selected() { for(auto &m:members)if(m.id==selectedId)return &m;return nullptr; }

    QStringList functions(bool includeCatalog = true) const
    {
        QStringList names;
        auto add = [&names](const QString &text) {
            const QString value=text.simplified();
            if (!value.isEmpty() && !names.contains(value,Qt::CaseInsensitive))
                names << value;
        };
        for(const auto &m : members) add(m.role);
        if(includeCatalog) {
            for(const char *value : {"Administrator","Data Analyst","Field Technician",
                    "Maintenance Lead","Meteorologist","Operations Manager","Station Technician",
                    "Support Specialist","Technical Specialist","Weather Forecaster"})
                add(QString::fromLatin1(value));
        }
        names.sort(Qt::CaseInsensitive);
        return names;
    }

    void buildHeader()
    {
        storageStatus=StaffDesign::label(staffListPage,oracleMode ? (oracleLoaded?"Oracle connected":"Oracle unavailable")
                                            : "Local preview","muted");
        if(oracleMode && oracleLoaded)
            storageStatus->setText("Oracle / SKYSENSE_MERIEM_STAFF");
        storageStatus->setObjectName("staffConnectionStatus");
        search=new QLineEdit(staffListPage);
        search->setObjectName("staffSearch");
        search->hide(); // Routed from the workspace search field.
        if(oracleMode) {
            // The shared header hosts this button next to Search.
            alerteEffectif=StaffDesign::action(staffListPage,{},"staffAlerteEffectif");
            alerteEffectif->setFixedSize(40,40);
            alerteEffectif->setIconSize({25,25});
            connect(AppTheme::instance(),&AppTheme::changed,alerteEffectif,[this]{
                alerteEffectif->setIcon(StaffDesign::notificationBell(alerteEffectif->property("alertCount").toInt()!=0));
            });
            alerteEffectif->setAccessibleName("Staffing alerts");
            alerteEffectif->hide();
            connect(alerteEffectif,&QPushButton::clicked,this,[this]{ouvrirAlerteEffectif();});
        }
        exportStaffButton=StaffDesign::action(staffListPage,"Export PDF","exportStaffPdf");
        connect(exportStaffButton,&QPushButton::clicked,this,[this]{exportPdf();});
        addStaffButton=StaffDesign::action(staffListPage,"+  Add staff","addStaff");
        addStaffButton->setProperty("primary",true);
        addStaffButton->setEnabled(writable || (oracleMode && oracleLoaded));
        connect(addStaffButton,&QPushButton::clicked,this,[this]{editMember(true);});

        auto *summary=new QHBoxLayout;
        summary->setSpacing(12);
        const QStringList titles{"Total Staff","Active Staff","Inactive Staff","Functions"};
        const QStringList marks{"all","active","inactive","layers"};
        const QList<QColor> colors{QColor("#6350bd"),QColor("#11845f"),QColor("#c34766"),QColor("#a46a1c")};
        for(int i=0;i<4;++i) {
            auto *tile=new StaffDesign::Metric(staffListPage,titles[i],marks[i],colors[i]);
            metrics << tile;summary->addWidget(tile,1);
        }
        homeLayout->addLayout(summary);
        panels=new QHBoxLayout;
        panels->setSpacing(14);
        homeLayout->addLayout(panels,1);
        connect(search,&QLineEdit::textChanged,this,[this]{page=0;refresh();});
    }

    void updateMetrics()
    {
        const auto counts=statistics(data);
        auto fraction=[&](int n){return QString::number(counts.total?100.0*n/counts.total:0.0,'f',1)+"%";};
        QMap<QDate,QList<const Member*>> cohorts;
        for(const auto &member:members){
            QDate day=QLocale(QLocale::English).toDate(member.joined,"MMM d, yyyy");
            if(day.isValid())cohorts[day].append(&member);
        }
        QList<int> totals{0},active{0},inactive{0},roles{0};
        QSet<QString> functionsSeen;
        for(const auto &cohort:cohorts){
            int a=active.last(),i=inactive.last();
            for(const auto *m:cohort){if(m->status=="Active")++a;else ++i;functionsSeen.insert(m->role);}
            totals<<a+i;active<<a;inactive<<i;roles<<functionsSeen.size();
        }
        metrics[0]->setValue(counts.total,{},totals);
        metrics[1]->setValue(counts.active,fraction(counts.active),active);
        metrics[2]->setValue(counts.inactive,fraction(counts.inactive),inactive);
        metrics[3]->setValue(counts.roles.size(),{},roles);
    }

    void buildList()
    {
        auto *panel=StaffDesign::card(staffListPage);
        panel->setObjectName("staffDirectory");
        panels->addWidget(panel,46);
        auto *layout=StaffDesign::interior(panel,13);
        auto *head=new QHBoxLayout;
        head->addWidget(StaffDesign::label(panel,"Staff List","section"));
        head->addStretch();
        auto *listMode=StaffDesign::action(panel,"","staffListMode");
        auto *gridMode=StaffDesign::action(panel,"","staffGridMode");
        auto *modes=new QButtonGroup(panel);
        for(auto *b:{listMode,gridMode}){
            const bool isList=b==listMode;
            b->setCheckable(true);b->setFixedSize(29,28);
            b->setStyleSheet("padding:2px;");
            b->setIcon(style()->standardIcon(isList?QStyle::SP_FileDialogDetailedView:QStyle::SP_FileDialogListView));
            b->setToolTip(isList?"List view":"Card view");b->setAccessibleName(b->toolTip());
            modes->addButton(b);head->addWidget(b);
        }
        listMode->setChecked(true);
        listRoles=new QComboBox(panel);
        listRoles->setObjectName("listRoleFilter"); listRoles->setAccessibleName("Filter by function");
        listRoles->setMaximumWidth(200);
        statusFilter=new QComboBox(panel);
        statusFilter->setObjectName("statusFilter");
        statusFilter->setAccessibleName("Filter by status");
        statusFilter->addItems({"All statuses","Active","Inactive"});
        populateRoles();
        head->addWidget(listRoles);head->addWidget(statusFilter);
        layout->addLayout(head);
        connect(listRoles,&QComboBox::currentIndexChanged,this,[this]{page=0;refresh();});
        connect(statusFilter,&QComboBox::currentIndexChanged,this,[this]{page=0;refresh();});
        table=new QTableWidget(0,9,panel);
        table->setObjectName("staffTable");
        header=new StaffHeader(table);table->setHorizontalHeader(header);
        table->setHorizontalHeaderLabels({"","Staff ID","Full name","Email","Function","CIN","Status","Joined",""});
        for(int column : {3,5,8})table->setColumnHidden(column,true);
        table->verticalHeader()->hide();
        table->verticalHeader()->setDefaultSectionSize(47);
        header->setSectionResizeMode(QHeaderView::Fixed);
        table->setColumnWidth(0,30);table->setColumnWidth(1,97);
        table->setColumnWidth(4,136);table->setColumnWidth(6,72);table->setColumnWidth(7,88);
        header->setSectionResizeMode(2,QHeaderView::Stretch);
        header->setMinimumSectionSize(25);
        table->setShowGrid(false);table->setFrameShape(QFrame::NoFrame);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setItemDelegate(new StaffDelegate(table));
        directoryViews=new QStackedWidget(panel);directoryViews->addWidget(table);
        directoryCards=new QListWidget(directoryViews);directoryCards->setObjectName("staffCards");
        directoryCards->setViewMode(QListView::IconMode);directoryCards->setMovement(QListView::Static);
        directoryCards->setResizeMode(QListView::Adjust);directoryCards->setSpacing(3);
        directoryCards->setFrameShape(QFrame::NoFrame);directoryCards->setWordWrap(false);
        directoryCards->setItemDelegate(new StaffDesign::DirectoryCards(directoryCards));
        directoryViews->addWidget(directoryCards);layout->addWidget(directoryViews,1);
        connect(listMode,&QPushButton::clicked,this,[this]{directoryViews->setCurrentWidget(table);});
        connect(gridMode,&QPushButton::clicked,this,[this]{directoryViews->setCurrentWidget(directoryCards);});
        connect(directoryCards,&QListWidget::currentRowChanged,this,[this](int row){
            if(row>=0 && row<table->rowCount()) table->selectRow(row);
        });
        emptyList=StaffDesign::label(table->viewport(),"No staff match your search or filters.","muted");
        emptyList->setAlignment(Qt::AlignCenter);emptyList->setAttribute(Qt::WA_TransparentForMouseEvents);
        emptyList->hide();
        auto *footer=new QHBoxLayout;
        selectionCount=StaffDesign::label(panel,"","muted");
        footer->addWidget(selectionCount);footer->addStretch();
        previous=StaffDesign::action(panel,"‹","previousPage");
        next=StaffDesign::action(panel,"›","nextPage");
        previous->setAccessibleName("Previous page");next->setAccessibleName("Next page");
        pagination=StaffDesign::label(panel,"","muted");
        footer->addWidget(previous);footer->addWidget(pagination);footer->addWidget(next);
        layout->addLayout(footer);
        auto *commands=new QHBoxLayout;
        commands->addWidget(addStaffButton);commands->addWidget(exportStaffButton);
        commands->addStretch();commands->addWidget(storageStatus);
        layout->addLayout(commands);
        connect(previous,&QPushButton::clicked,this,[this]{if(page>0){--page;refresh();}});
        connect(next,&QPushButton::clicked,this,[this]{if((page+1)*8<filtered.size()){++page;refresh();}});

        connect(table,&QTableWidget::itemSelectionChanged,this,[this]{
            const auto rows=table->selectionModel()->selectedRows(1);
            if(rows.isEmpty())return;
            selectedId=rows.first().data().toString();
            const QSignalBlocker blocker(directoryCards);
            directoryCards->setCurrentRow(rows.first().row());updateDetails();
        });
        connect(table,&QTableWidget::itemChanged,this,[this](QTableWidgetItem *item){if(item->column()!=0)return; const QString id=table->item(item->row(),1)->text();if(item->checkState()==Qt::Checked)checked.insert(id);else checked.remove(id);syncHeader();});
        connect(table,&QTableWidget::cellClicked,this,[this](int row,int column){if(column==8){selectedId=table->item(row,1)->text();showActions();}});
        connect(header,&QHeaderView::sectionClicked,this,[this](int section){
            if(section==0){const bool set=!header->checked;for(int r=0;r<table->rowCount();++r)table->item(r,0)->setCheckState(set?Qt::Checked:Qt::Unchecked);}
            else if(section<=7){header->sortOrder=section==header->sortColumn && header->sortOrder==Qt::AscendingOrder?Qt::DescendingOrder:Qt::AscendingOrder;header->sortColumn=section;page=0;refresh();}
        });
    }
    void buildDetails()
    {
        profileCard=StaffDesign::card(staffListPage);
        profileCard->setObjectName("staffProfile");
        panels->addWidget(profileCard,54);
        profileLayout=StaffDesign::interior(profileCard,14);
        auto *identity=new QHBoxLayout;
        identity->setSpacing(12);
        profileAvatar=StaffDesign::label(profileCard,"","avatar");
        profileAvatar->setObjectName("staffInitials");
        profileAvatar->setFixedSize(70,70);profileAvatar->setAlignment(Qt::AlignCenter);
        identity->addWidget(profileAvatar);
        auto *identityText=new QVBoxLayout;
        auto *headline=new QHBoxLayout;
        name=StaffDesign::label(profileCard,"","profileName");name->setObjectName("staffDetailName");
        name->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        status=StaffDesign::label(profileCard,"","status");status->setObjectName("staffDetailStatus");
        status->setAlignment(Qt::AlignCenter);
        headline->addWidget(name);headline->addWidget(status);headline->addStretch();
        identityText->addLayout(headline);
        role=StaffDesign::label(profileCard,"","function");identityText->addWidget(role);
        identity->addLayout(identityText,1);
        editStaffButton=StaffDesign::action(profileCard,"Edit","editStaff");
        editStaffButton->setProperty("primary",true);
        deleteStaffButton=StaffDesign::action(profileCard,"Delete","deleteStaff");
        deleteStaffButton->setProperty("danger",true);
        identity->addWidget(editStaffButton); identity->addWidget(deleteStaffButton);
        connect(editStaffButton,&QPushButton::clicked,this,[this]{editMember(false);});
        connect(deleteStaffButton,&QPushButton::clicked,this,[this]{deleteMember();});
        profileLayout->addLayout(identity);
        auto *contact=new QHBoxLayout;
        contact->setSpacing(12);
        email=StaffDesign::label(profileCard,"","contact");
        phone=StaffDesign::label(profileCard,"","contact");
        joined=StaffDesign::label(profileCard,"","contact");
        for(auto *label : {email,phone,joined}) {
            label->setMinimumWidth(0);label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        }
        contact->addWidget(email,3);contact->addWidget(phone,2);contact->addWidget(joined,2);
        profileLayout->addLayout(contact);
        auto *tabBar=new QHBoxLayout;tabBar->setSpacing(3);
        const QStringList titles{"Overview","Permissions","Statistics","Activity log",oracleMode?"Availability":"Notes","Access"};
        for(int i=0;i<titles.size();++i){
            auto *b=StaffDesign::action(profileCard,titles[i],"profileTab");
            b->setProperty("tab",true);tabs<<b;tabBar->addWidget(b);
            if (i==5) b->setVisible(oracleMode && AppSession::isAdministrator());
            connect(b,&QPushButton::clicked,this,[this,i]{
                if ((i==0 || i==1 || i==3) && !reloadProfileData()) return;
                tab=i;updateDetails();
            });
        }
        profileLayout->addLayout(tabBar);
        detailsBody=new QWidget(profileCard);
        profileStack=new QStackedLayout(detailsBody);
        profileLayout->addWidget(detailsBody,1);
        overview=new QWidget(detailsBody);overview->setObjectName("overviewPanel");
        profileStack->addWidget(overview);
        auto *grid=new QGridLayout(overview);
        grid->setContentsMargins(0,0,0,0);grid->setSpacing(9);
        grid->setColumnStretch(0,1);grid->setColumnStretch(1,1);
        auto section=[&](QString title,int row,int col) {
            auto *box=StaffDesign::card(overview,"section");
            grid->addWidget(box,row,col);
            auto *content=StaffDesign::interior(box,11);
            content->addWidget(StaffDesign::label(box,title,"subheading"));
            return content;
        };
        auto *info=section("Staff information",0,0);
        auto *fields=new QGridLayout;
        fields->setHorizontalSpacing(9);fields->setVerticalSpacing(3);
        const QStringList labels{"Staff ID","Full name","Function",oracleMode?"CIN":"Department","Email","Phone"};
        for(int i=0;i<labels.size();++i) {
            fields->addWidget(StaffDesign::label(overview,labels[i],"muted"),i,0);
            auto *v=StaffDesign::label(overview,"","value");
            v->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
            fields->addWidget(v,i,1);infoValues<<v;
        }
        fields->setColumnStretch(1,1);info->addLayout(fields);info->addStretch();

        auto *rights=section("System access",0,1);
        auto *rightsRow=new QHBoxLayout;
        accessRing=new StaffDesign::Ring(overview);
        rightsRow->addWidget(accessRing);
        auto *rightsList=new QVBoxLayout;
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};
        for(int i=0;i<modules.size();++i) {
            auto *box=new QCheckBox(modules[i],overview);box->setAccessibleName(modules[i]);
            access<<box;rightsList->addWidget(box);
            connect(box,&QCheckBox::toggled,this,[this,i](bool value) {
                auto values=permissions.value(selectedId,{false,false,false,false});
                values[i]=value;changeAccess(values);
            });
        }
        rightsRow->addLayout(rightsList,1);rights->addLayout(rightsRow);
        accessSummary=StaffDesign::label(overview,"","muted");rights->addWidget(accessSummary);
        accessButton=StaffDesign::action(overview,"Edit permissions","editStaffAccess");
        rights->addWidget(accessButton,0,Qt::AlignLeft);
        connect(accessButton,&QPushButton::clicked,this,[this]{editPermissions();});
        rights->addStretch();

        auto *week=section("Availability",1,0);
        weekCaption=StaffDesign::label(overview,"","muted");week->addWidget(weekCaption);
        auto *days=new QHBoxLayout;days->setSpacing(2);
        for(int i=0;i<7;++i){
            auto *d=StaffDesign::label(overview,"","day");d->setAlignment(Qt::AlignCenter);
            d->setMinimumHeight(39);days->addWidget(d,1);weekDays<<d;
        }
        week->addLayout(days);week->addStretch();
        trainingButton=StaffDesign::action(overview,oracleMode?"Check availability":"Skills and training",
                                          oracleMode?"staffPlanning":"staffTraining");
        week->addWidget(trainingButton,0,Qt::AlignLeft);
        connect(trainingButton,&QPushButton::clicked,this,[this] {
            if(oracleMode) ouvrirPlanningPersonnel(); else showTraining();
        });
        auto *recent=section("Recent activity",1,1);
        for(int i=0;i<3;++i){
            auto *row=new QHBoxLayout;
            auto *item=StaffDesign::label(overview,"","value");item->setMinimumWidth(0);
            item->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
            auto *date=StaffDesign::label(overview,"","muted");
            recentTexts<<item;recentDates<<date;row->addWidget(item,1);row->addWidget(date);
            recent->addLayout(row);
        }
        recent->addStretch();
        auto *viewAll=StaffDesign::action(overview,"View activity","staffViewAudit");
        recent->addWidget(viewAll,0,Qt::AlignLeft);
        connect(viewAll,&QPushButton::clicked,this,[this]{showAudit(selectedId);});

        accountPanel=new StaffAccountPanel(detailsBody);profileStack->addWidget(accountPanel);
        tabContent=new QWidget(detailsBody);profileStack->addWidget(tabContent);
        auto *tabLayout=new QVBoxLayout(tabContent);tabLayout->setContentsMargins(0,0,0,0);
        auto *actions=new QHBoxLayout;
        actions->setContentsMargins(0,4,0,2);
        tabLayout->addLayout(actions);
        profileAccess=StaffDesign::action(tabContent,"Edit permissions","staffTabAccess");
        actions->addWidget(profileAccess);
        connect(profileAccess,&QPushButton::clicked,this,[this]{editPermissions();});
        if (oracleMode) {
            profilePlanning=StaffDesign::action(tabContent,"Manage availability","staffTabPlanning");
            actions->addWidget(profilePlanning);
            connect(profilePlanning,&QPushButton::clicked,this,[this]{ouvrirPlanningPersonnel();});
            availabilityTools=new QWidget(tabContent);
            auto *dayRow=new QHBoxLayout(availabilityTools);
            dayRow->setContentsMargins(0,0,0,0);
            dayRow->addWidget(new QLabel("Check availability on",availabilityTools));
            availabilityDay=new QDateEdit(reportDay,availabilityTools);
            availabilityDay->setObjectName("staffTabDate");
            availabilityDay->setDisplayFormat("dd/MM/yyyy");
            availabilityDay->setCalendarPopup(true);
            dayRow->addWidget(availabilityDay);dayRow->addStretch();
            tabLayout->addWidget(availabilityTools);
            connect(availabilityDay,&QDateEdit::dateChanged,this,[this]{if(tab==4)setTab(4);});
        }
        tabBrowser=new QTextBrowser(tabContent);tabBrowser->setObjectName("staffTabContent");
        tabBrowser->setFrameShape(QFrame::NoFrame);tabLayout->addWidget(tabBrowser,1);
        tabCurve=new StaffStatisticsCurve;tabCurve->setObjectName("staffTabCurve");
        tabCurveView=staffCurveView(tabCurve,tabContent);tabLayout->addWidget(tabCurveView,1);
        detailStats=StaffDesign::action(tabContent,"Open statistics","staffStatistics");
        detailAudit=StaffDesign::action(tabContent,"Open activity log","staffAudit");
        actions->addWidget(detailStats);actions->addWidget(detailAudit);actions->addStretch();
        connect(detailStats,&QPushButton::clicked,this,[this]{showStatistics();});
        connect(detailAudit,&QPushButton::clicked,this,[this]{showAudit(selectedId);});
    }

    void populateRoles()
    {
        QSignalBlocker block(listRoles);
        const QString current=listRoles->currentText();
        listRoles->clear();listRoles->addItem("All functions");listRoles->addItems(functions());
        const int index=listRoles->findText(current,Qt::MatchFixedString);
        listRoles->setCurrentIndex(qMax(0,index));
    }

    void syncHeader() { if(selectionCount)selectionCount->setText(QString("%1 of %2 selected").arg(checked.size()).arg(members.size())); bool all=table->rowCount()>0;for(int r=0;r<table->rowCount();++r)all=all && table->item(r,0)->checkState()==Qt::Checked;header->checked=all;header->viewport()->update(); }
    void refresh(bool reveal=false) {
        if(!table)return;
        updateMetrics();
        filtered.clear();
        const QString query=search->text().trimmed();
        for(int i=0;i<members.size();++i){const auto &m=members[i];if(!(m.id+" "+m.name+" "+m.email+" "+m.role+" "+m.department+" "+m.cin+" "+m.status).contains(query,Qt::CaseInsensitive))continue;if(listRoles->currentIndex()>0 && m.role.compare(listRoles->currentText(),Qt::CaseInsensitive)!=0)continue;if(statusFilter->currentIndex()>0 && m.status!=statusFilter->currentText())continue;filtered<<i;}
        auto key=[this](const Member &m){switch(header->sortColumn){case 1:return m.id;case 2:return m.name;case 3:return m.email;case 4:return m.role;case 5:return oracleMode?m.cin:m.department;case 6:return m.status;case 7:return oracleMode?QLocale(QLocale::English).toDate(m.joined,"MMM d, yyyy").toString(Qt::ISODate):QLocale(QLocale::English).toDateTime(m.login,"MMM d, yyyy HH:mm").toString(Qt::ISODate);default:return m.id;}};
        std::stable_sort(filtered.begin(),filtered.end(),[&](int left,int right) {
            const auto &a = members[left];
            const auto &b = members[right];
            int order = QString::compare(key(a),key(b),Qt::CaseInsensitive);
            if (oracleMode && header->sortColumn == 1) {
                bool aNumerique = false, bNumerique = false;
                const auto numeroA = a.id.toULongLong(&aNumerique);
                const auto numeroB = b.id.toULongLong(&bNumerique);
                if (aNumerique && bNumerique) order = (numeroA > numeroB) - (numeroA < numeroB);
                else if (aNumerique != bNumerique) order = aNumerique ? 1 : -1;
            }
            if (order == 0) order = QString::compare(a.id,b.id);
            return header->sortOrder == Qt::AscendingOrder ? order < 0 : order > 0;
        });
        if(reveal)for(int i=0;i<filtered.size();++i)if(members[filtered[i]].id==selectedId){page=i/8;break;}
        page=qBound(0,page,qMax(0,(int(filtered.size())-1)/8));
        const QSignalBlocker block(table);table->clearSelection();table->setRowCount(0);
        const QSignalBlocker cardBlock(directoryCards);directoryCards->clear();
        const int end=qMin((page+1)*8,int(filtered.size()));int selectedRow=-1;
        for(int n=page*8;n<end;++n){const auto &m=members[filtered[n]];const int row=table->rowCount();table->insertRow(row);
            const QStringList values{"",m.id,m.name,m.email,m.role,oracleMode?m.cin:m.department,m.status,oracleMode?m.joined:m.login,""};
            for(int c=0;c<9;++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);if(c==2)item->setData(Qt::UserRole,StaffDesign::initials(m.prenom,m.nom,m.name));if(c==0){item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(checked.contains(m.id)?Qt::Checked:Qt::Unchecked);}if(c==8)item->setToolTip("Staff actions");table->setItem(row,c,item);}
            if(m.id==selectedId)selectedRow=row;
            auto *entry=new QListWidgetItem(m.name,directoryCards);
            entry->setData(Qt::UserRole,m.id);
            entry->setData(Qt::UserRole+1,StaffDesign::initials(m.prenom,m.nom,m.name));
            entry->setData(Qt::UserRole+2,m.role);entry->setData(Qt::UserRole+3,m.status);
            entry->setToolTip(m.name+" · "+m.role);
        }
        if(table->rowCount()){if(selectedRow<0){selectedRow=0;selectedId=table->item(0,1)->text();}table->selectRow(selectedRow);}else selectedId.clear();
        directoryCards->setCurrentRow(selectedRow);
        emptyList->setGeometry(table->viewport()->rect());
        pagination->setText(filtered.isEmpty()?"0 of 0":QString("%1-%2 of %3").arg(page*8+1).arg(end).arg(filtered.size()));
        previous->setEnabled(page>0);next->setEnabled(end<filtered.size());emptyList->setVisible(filtered.isEmpty());syncHeader();updateDetails();update();
    }
    void updateDetails() {
        auto *m=selected();
        const bool editable = m && (writable || (oracleMode && oracleLoaded));
        editStaffButton->setEnabled(editable);deleteStaffButton->setEnabled(editable);
        trainingButton->setEnabled(editable);accessButton->setEnabled(editable);profileAccess->setEnabled(editable);
        for(int i=0;i<tabs.size();++i)tabs[i]->setEnabled(m || i==2 || i==3);
        if(!m){
            name->setText("No staff selected");profileAvatar->setText("—");
            role->clear();status->hide();email->clear();phone->clear();joined->clear();
            setTab(tab);
            return;
        }
        detailsBody->show();
        profileAvatar->setText(StaffDesign::initials(m->prenom,m->nom,m->name));
        profileAvatar->setAccessibleName("Initials of "+m->name);
        name->setText(m->name);name->setToolTip(m->name);
        role->setText(m->role);
        status->show();status->setText(m->status);
        status->setStyleSheet(m->status=="Active"?"background:#dff5e9;color:#176646;border-radius:9px;padding:3px 8px;"
                                              :"background:#fce5eb;color:#94334f;border-radius:9px;padding:3px 8px;");
        setFittedText(email,m->email);setFittedText(phone,m->phone);
        setFittedText(joined,m->joined.isEmpty()?"Join date unavailable":"Joined "+m->joined);
        const QStringList values{m->id,m->name,m->role,oracleMode?m->cin:m->department,m->email,m->phone};
        for(int i=0;i<infoValues.size();++i)setFittedText(infoValues[i],values[i].isEmpty()?QStringLiteral("—"):values[i]);
        const auto grants=permissions.value(m->id,{false,false,false,false});
        const bool recorded=permissions.contains(m->id);
        int count=0;
        for(int i=0;i<access.size();++i){
            QSignalBlocker block(access[i]);
            access[i]->setEnabled(editable && m->status=="Active");
            access[i]->setChecked(recorded && grants.value(i) && m->status=="Active");
            count+=access[i]->isChecked();
        }
        accessRing->setCount(recorded?count:-1);
        accessSummary->setText(recorded?QString("%1 of 4 modules granted").arg(count):QString("No access permissions recorded"));
        const QDate today=reportDay;
        QDate monday=today.addDays(1-today.dayOfWeek());
        weekCaption->setText(QString("Week of %1").arg(QLocale(QLocale::English).toString(monday,"MMM d, yyyy")));
        const QString key=m->id+"|"+monday.toString(Qt::ISODate);
        if(weekKey!=key){
            weekKey=key;weekStates.clear();
            for(int i=0;i<7;++i){
                const QDate day=monday.addDays(i);
                QString state="Available";
                if(oracleMode){
                    PlanningPersonnel::Bilan bilan;QString error;
                    if(!PlanningPersonnel::analyser(m->id,day,day,bilan,error)) state="Unknown";
                    else if(!bilan.motifs.isEmpty()) state=bilan.motifs.join("; ");
                    else if(!bilan.affectations.isEmpty()) state="Busy";
                }else if(m->status!="Active") state="Inactive";
                weekStates.append(state);
            }
        }
        for(int i=0;i<weekDays.size();++i){
            const QDate day=monday.addDays(i);
            const QString state=weekStates.value(i,"Unknown");
            auto *d=weekDays[i];
            d->setText(QLocale(QLocale::English).toString(day,"ddd\nd"));
            d->setToolTip(QLocale(QLocale::English).toString(day,"MMM d")+" · "+state);
            const bool available=state=="Available";
            d->setStyleSheet(available?(day==today?"color:white;background:#228363;border-radius:6px;":
                            "color:#24604e;background:#e7f4ee;border-radius:6px;"):
                            state=="Busy"?"color:#973553;background:#fbe9ee;border-radius:6px;":
                            "color:#794b16;background:#fff1d9;border-radius:6px;");
        }

        const auto history=memberAudit();
        for(int i=0;i<recentTexts.size();++i){
            setFittedText(recentTexts[i],i<history.size()?history[i].action:(i==0?"No activity recorded":""));
            recentDates[i]->setText(i<history.size()?QLocale(QLocale::English).toString(
                QDateTime::fromString(history[i].timestamp,Qt::ISODateWithMs).toLocalTime(),"MMM d"):"");
            recentTexts[i]->setToolTip(i<history.size()?history[i].details:QString());
        }

        setTab(tab);
    }
    void setTab(int index) {
        if (index==5 && !(oracleMode && AppSession::isAdministrator() && selected())) index=0;
        if (index!=5) accountPanel->reset();
        tab=index;for(int i=0;i<tabs.size();++i){tabs[i]->setProperty("active",i==index);tabs[i]->style()->unpolish(tabs[i]);tabs[i]->style()->polish(tabs[i]);}
        if (index==5) {
            auto *m=selected();detailsBody->show();profileStack->setCurrentWidget(accountPanel);
            accountPanel->selectMember(m->id,m->name,m->email,m->status=="Active");return;
        }
        profileStack->setCurrentWidget(index==0?overview:tabContent);
        detailStats->setVisible(index==2);detailAudit->setVisible(index==3);
        profileAccess->setVisible(index==1);
        if (profilePlanning) {
            profilePlanning->setVisible(index==4);
            availabilityTools->setVisible(index==4);
        }
        const auto *m=selected();
        detailsBody->setVisible(m || index==2 || index==3);
        if(!m && index!=2 && index!=3)return;
        if (index==1) {
            QString html="<h3>Module access</h3>";
            const QStringList modules{"Stations","Equipment","Maintenance","Services"};
            const auto rights=permissions.value(m->id);
            for (int i=0;i<modules.size();++i) {
                const QString state=rights.value(i)?(m->status=="Active"?"Granted":"Disabled while inactive"):"Not granted";
                html+="<p>"+modules[i]+" &nbsp; <b>"+state+"</b></p>";
            }
            if (rights.isEmpty()) html+="<p>Use Edit permissions to record module grants on this profile.</p>";
            if (oracleMode) html+="<p>The department role in Access controls workspace sign-in.</p>";
            tabBrowser->setHtml(html);
        }
        const bool statistiques = index == 2;
        tabCurveView->setVisible(statistiques);
        tabBrowser->setVisible(!statistiques);
        if (statistiques) {
            const auto s = statistics(data);
            const QString heading = oracleMode ? "Staff by function" : "Staff by department";
            tabCurve->setCounts(QString("%1  ·  Total %2  ·  Active %3  ·  Inactive %4")
                .arg(heading).arg(s.total).arg(s.active).arg(s.inactive),
                oracleMode ? s.roles : s.departments);
        }
        if (index==3) {
            QString html="<h3>Activity log"+(m?" — "+m->name.toHtmlEscaped():QString())+"</h3>";
            const auto history=m?memberAudit():data.audit;
            if (history.isEmpty()) {
                if (m && !m->joined.isEmpty()) html+="<p>Joined "+m->joined.toHtmlEscaped()+"</p>";
                html+="<p>No changes recorded yet. Profile edits, permission changes and saved leave appear here.</p>";
            }
            for (const auto &e:history)
                html+="<p><b>"+e.action.toHtmlEscaped()+"</b> · "+e.details.toHtmlEscaped()+"<br>"+
                    QDateTime::fromString(e.timestamp,Qt::ISODateWithMs).toLocalTime().toString("yyyy-MM-dd HH:mm:ss")+
                    " · "+e.actor.toHtmlEscaped()+"</p>";
            tabBrowser->setHtml(html);
        }
        if(index==4) {
            if (oracleMode) {
                const QDate day=availabilityDay->date();
                PlanningPersonnel::Bilan result;QString error;
                if (PlanningPersonnel::analyser(m->id,day,day,result,error)) {
                    const QStringList reasons=result.motifs+result.affectations;
                    QString html="<h3>"+QString(reasons.isEmpty()?"Available":"Unavailable")+" on "+day.toString("dd/MM/yyyy")+"</h3>";
                    for (const QString &reason:reasons) html+="<p>"+reason.toHtmlEscaped()+"</p>";
                    if (!result.absenceDu.isValid()) html+="<p>No leave scheduled. Use Manage availability to record a period.</p>";
                    else if (day<result.absenceDu || day>result.absenceAu)
                        html+="<p>Saved leave: "+result.absenceDu.toString("dd/MM/yyyy")+" – "+result.absenceAu.toString("dd/MM/yyyy")+"</p>";
                    tabBrowser->setHtml(html);
                } else tabBrowser->setPlainText(error);
            }
            else tabBrowser->setPlainText(m->note.isEmpty()?"No note saved. Use Edit Staff to add a note.":m->note);
        }
        update();
    }
    bool actualiserPersonnelOracle(const QString &idAAfficher = QString(), bool revealInDirectory = true)
    {
        QString error;
        oracleLoaded = chargerPersonnel(error);
        addStaffButton->setEnabled(oracleLoaded);

        if (!oracleLoaded) {
            updateDetails();
            storageStatus->setText("Staff list needs to be reloaded");
            QMessageBox::warning(
                this, "Refresh staff",
                "Your change was saved. The staff list could not be reloaded. "
                "Reopen the application "
                "to reload staff.\n\n" + error);
            return false;
        }

        populateRoles();
        if (!idAAfficher.isEmpty() && revealInDirectory) {
            QSignalBlocker a(search), c(listRoles), d(statusFilter);
            search->clear();
            
            listRoles->setCurrentIndex(0);
            statusFilter->setCurrentIndex(0);
        }
        if (!idAAfficher.isEmpty()) selectedId=idAAfficher;
        storageStatus->setText("Oracle / SKYSENSE_MERIEM_STAFF");
        refresh(!idAAfficher.isEmpty());
        actualiserSignalEffectif();
        return true;
    }



    QMap<QString,int> minimumsEffectif() const
    {
        QSettings settings("CoreVision",QCoreApplication::applicationName());
        settings.beginGroup("Staff/Effectifs");
        QMap<QString,int> minimums;
        for (const QString &key : settings.childKeys()) {
            const QVariantMap regle = settings.value(key).toMap();
            const QString fonction = regle.value("fonction").toString();
            const int minimum = regle.value("minimum").toInt();
            if (!fonction.isEmpty() && minimum > 0) minimums.insert(fonction,minimum);
        }
        return minimums;
    }

    void afficherSignalEffectif(const PlanningPersonnel::RapportEffectif &rapport)
    {
        premiereAlerte = rapport.joursAlerte.isEmpty() ? QDate() : rapport.joursAlerte.firstKey();
        int nombre = 0;
        QStringList alertes;
        for (auto it=rapport.joursAlerte.cbegin(); it!=rapport.joursAlerte.cend(); ++it) {
            nombre += it.value().size();
            alertes << it.key().toString("dd/MM/yyyy")+" — "+it.value().join(", ");
        }
        alerteEffectif->setIcon(StaffDesign::notificationBell(nombre>0));
        alerteEffectif->setAccessibleName(nombre?QString("Staffing alerts, %1 functions affected").arg(nombre):"Staffing alerts");
        alerteEffectif->setProperty("alertCount",nombre);

        alerteEffectif->setToolTip(nombre ? alertes.join("\n") : QString("Check staffing by date and function"));
    }

    void actualiserSignalEffectif()
    {
        if (!alerteEffectif) return;
        PlanningPersonnel::RapportEffectif rapport;
        QString erreur;
        if (PlanningPersonnel::lireEffectifs(QDate::currentDate(),minimumsEffectif(),rapport,erreur)) {
            afficherSignalEffectif(rapport);
        } else {
            premiereAlerte = {};
            alerteEffectif->setIcon(StaffDesign::notificationBell(true));
            alerteEffectif->setAccessibleName("Staffing alerts unavailable");
            alerteEffectif->setProperty("alertCount",-1);
            alerteEffectif->setToolTip(erreur);
        }
    }

    void ouvrirAlerteEffectif()
    {
        QVBoxLayout *layout = nullptr;
        auto *pageAlerte = newStaffPage("staffEffectifPage","Staffing alerts",
            "Review coverage for the day and set the minimum number of people needed for each function.",&layout);
        auto *controls = new QFrame(pageAlerte);
        controls->setObjectName("staffFormCard");
        auto *form = new QFormLayout(controls);
        auto *jour = new QDateEdit(premiereAlerte.isValid()?premiereAlerte:QDate::currentDate(),controls);
        jour->setObjectName("effectifJour");
        jour->setCalendarPopup(true);
        jour->setDisplayFormat("dd/MM/yyyy");
        form->addRow("Date",jour);
        auto *joursAlerte = new QComboBox(controls);
        joursAlerte->setObjectName("effectifJoursAlerte");
        form->addRow("Upcoming shortages",joursAlerte);
        auto *fonctions = new QComboBox(controls);
        fonctions->setObjectName("effectifFonction");
        form->addRow("Function",fonctions);
        auto *minimum = new QSpinBox(controls);
        minimum->setObjectName("effectifMinimum");
        minimum->setRange(1,std::numeric_limits<int>::max());
        form->addRow("Minimum required",minimum);
        auto *appliquer = new QPushButton("Save minimum",controls);
        appliquer->setObjectName("effectifAppliquer");
        form->addRow(appliquer);
        layout->addWidget(controls);

        auto *signal = new QLabel(pageAlerte);
        signal->setObjectName("effectifSignal");
        signal->setTextFormat(Qt::PlainText);
        signal->setWordWrap(true);
        signal->setMargin(10);
        layout->addWidget(signal);
        auto *tableau = new QTableWidget(0,6,pageAlerte);
        tableau->setObjectName("effectifTable");
        tableau->setHorizontalHeaderLabels({"Function","Active staff","On leave","Present","Minimum required","Shortfall"});
        tableau->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableau->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
        tableau->verticalHeader()->hide();
        tableau->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableau->setSelectionBehavior(QAbstractItemView::SelectRows);
        layout->addWidget(tableau,1);
        auto *retour = new QPushButton("Back to staff",pageAlerte);
        retour->setObjectName("effectifRetour");
        layout->addWidget(retour,0,Qt::AlignRight);
        connect(retour,&QPushButton::clicked,pageAlerte,[this,pageAlerte]{closeStaffPage(pageAlerte);});
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape),pageAlerte);
        escape->setContext(Qt::WidgetWithChildrenShortcut);
        connect(escape,&QShortcut::activated,retour,&QPushButton::click);

        const auto choisirFonction = [=] {
            const int seuil = fonctions->currentData().toInt();
            minimum->setValue(qMax(1,seuil));
            appliquer->setEnabled(false);
        };
        const auto actualiser = [=](bool garderSaisie) {
            PlanningPersonnel::RapportEffectif rapport;
            QString erreur;
            const QString choix = fonctions->currentText();
            const bool saisieEnCours = garderSaisie && appliquer->isEnabled();
            const int saisie = minimum->value();
            const QSignalBlocker bloc(fonctions);
            fonctions->clear();
            tableau->setRowCount(0);
            const QSignalBlocker blocJours(joursAlerte);
            joursAlerte->clear();
            if (!PlanningPersonnel::lireEffectifs(jour->date(),minimumsEffectif(),rapport,erreur)) {
                signal->setText("Unable to load staffing counts. "+erreur);
                signal->setStyleSheet("color:#922b20;");
                appliquer->setEnabled(false);
                return;
            }
            afficherSignalEffectif(rapport);
            joursAlerte->addItem(rapport.joursAlerte.isEmpty()?"No shortages expected":"Select an alert");
            for (auto it=rapport.joursAlerte.cbegin();it!=rapport.joursAlerte.cend();++it)
                joursAlerte->addItem(it.key().toString("dd/MM/yyyy")+" — "+it.value().join(", "),it.key());
            joursAlerte->setEnabled(!rapport.joursAlerte.isEmpty());
            joursAlerte->setCurrentIndex(qMax(0,joursAlerte->findData(jour->date())));
            QStringList alertes;
            int premierDeficit = -1;
            for (const auto &g : rapport.fonctions) {
                const int ligne = tableau->rowCount();
                tableau->insertRow(ligne);
                fonctions->addItem(g.fonction,g.minimum);
                const QStringList valeurs{g.fonction,QString::number(g.actifs),QString::number(g.absents.size()),
                                          QString::number(g.presents()),QString::number(g.minimum),QString::number(g.manque())};
                for (int c=0;c<valeurs.size();++c) {
                    auto *cellule = new QTableWidgetItem(valeurs[c]);
                    if (c>0) cellule->setTextAlignment(Qt::AlignCenter);
                    if (c==2) cellule->setToolTip(g.absents.join(", "));
                    if (g.manque()>0 && (c==0 || c==5)) {
                        cellule->setBackground(QColor("#fff0d6"));
                        cellule->setForeground(QColor("#832d0a"));
                    }
                    tableau->setItem(ligne,c,cellule);
                }
                if (g.manque()>0) {
                    if (premierDeficit<0) premierDeficit=ligne;
                    alertes << QString("%1 — %2 more needed").arg(g.fonction).arg(g.manque());
                }
            }
            const int ancienIndex = fonctions->findText(choix);
            fonctions->setCurrentIndex(ancienIndex>=0 ? ancienIndex : qMax(0,premierDeficit));
            minimum->setEnabled(fonctions->count()>0);
            choisirFonction();
            if (saisieEnCours && ancienIndex>=0) minimum->setValue(saisie);
            const QString date = jour->date().toString("dd/MM/yyyy");
            if (rapport.fonctions.isEmpty()) signal->setText("No functions to monitor.");
            else if (alertes.isEmpty()) signal->setText("Staffing meets the minimum on "+date+".");
            else signal->setText("Understaffed on "+date+" : "+alertes.join(" ; "));
            signal->setStyleSheet(alertes.isEmpty() ? "font-weight:600;" :
                "color:#832d0a;background:#fff0d6;border:1px solid #d08837;border-radius:6px;font-weight:600;");
        };
        connect(fonctions,&QComboBox::currentIndexChanged,pageAlerte,choisirFonction);
        connect(minimum,&QSpinBox::valueChanged,pageAlerte,[=](int valeur) {
            appliquer->setEnabled(fonctions->currentIndex()>=0 && valeur!=fonctions->currentData().toInt());
        });
        connect(tableau,&QTableWidget::cellClicked,pageAlerte,[=](int ligne,int) {fonctions->setCurrentIndex(ligne);});
        connect(jour,&QDateEdit::dateChanged,pageAlerte,[=]{actualiser(true);});
        connect(joursAlerte,&QComboBox::activated,pageAlerte,[=](int index) {
            const QDate date = joursAlerte->itemData(index).toDate();
            if (date.isValid()) jour->setDate(date);
        });
        connect(appliquer,&QPushButton::clicked,pageAlerte,[=] {
            if (fonctions->currentIndex()<0) return;
            QSettings settings("CoreVision",QCoreApplication::applicationName());
            settings.beginGroup("Staff/Effectifs");
            const QString nom = fonctions->currentText();
            const QString key = QString::fromLatin1(QUrl::toPercentEncoding(nom.simplified().toCaseFolded()));
            settings.setValue(key,QVariantMap{{"fonction",nom},{"minimum",minimum->value()}});
            settings.sync();
            if (settings.status()!=QSettings::NoError) {
                signal->setText("Could not save the minimum on this device.");
                return;
            }
            actualiser(false);
        });
        connect(veilleEffectif,&QTimer::timeout,pageAlerte,[=]{if (pageAlerte->isVisible()) actualiser(true);});
        staffPages->setCurrentWidget(pageAlerte);
        actualiser(false);
    }

    void ouvrirPlanningPersonnel()
    {
        if (!oracleMode || !oracleLoaded || !selected()) return;
        const Member membre = *selected();
        QVBoxLayout *layout = nullptr;
        auto *pagePlanning = newStaffPage("staffPlanningPage", "Availability",
            membre.name + "  ·  " + membre.role, &layout);
        auto *fiche = new QFrame(pagePlanning);
        fiche->setObjectName("staffFormCard");
        auto *form = new QFormLayout(fiche);
        form->setContentsMargins(18,14,18,14);
        auto creerDate = [fiche](const QString &nom, QDate valeur) {
            auto *champ = new QDateEdit(valeur.isValid() ? valeur : QDate::currentDate(),fiche);
            champ->setObjectName(nom);
            champ->setDisplayFormat("dd/MM/yyyy");
            champ->setCalendarPopup(true);
            return champ;
        };
        const QDate chosen=tab==4?availabilityDay->date():reportDay;
        auto *jour = creerDate("planningJour", chosen);
        form->addRow("Check availability on",jour);
        auto *etat = new QLabel(fiche);
        etat->setTextFormat(Qt::PlainText);
        etat->setWordWrap(true);
        etat->setObjectName("planningEtat");
        form->addRow(etat);
        auto *du = creerDate("planningDu",membre.debutIndisponibilite.isValid()?membre.debutIndisponibilite:chosen);
        auto *au = creerDate("planningAu",membre.finIndisponibilite.isValid()?membre.finIndisponibilite:chosen);
        form->addRow("Leave starts",du);
        form->addRow("Leave ends",au);
        auto *enregistrer = new QPushButton(fiche);
        enregistrer->setObjectName("planningEnregistrer");
        form->addRow(enregistrer);
        auto *info = new QLabel(fiche);
        info->setObjectName("planningPeriode");
        info->setTextFormat(Qt::PlainText);
        info->setWordWrap(true);
        form->addRow(info);
        layout->addWidget(fiche);

        layout->addStretch();
        auto *retour = new QPushButton("Back to staff",pagePlanning);
        retour->setObjectName("planningRetour");
        layout->addWidget(retour,0,Qt::AlignRight);
        connect(retour,&QPushButton::clicked,pagePlanning,[this,pagePlanning,jour] {
            if (tab==4) availabilityDay->setDate(jour->date());
            closeStaffPage(pagePlanning);
        });

        const auto verifierJour = [=] {
            PlanningPersonnel::Bilan bilan;
            QString erreur;
            if (!PlanningPersonnel::analyser(membre.id,jour->date(),jour->date(),bilan,erreur)) {
                etat->setText(erreur);
                return;
            }
            QStringList raisons = bilan.motifs;
            raisons.append(bilan.affectations);
            const QString date = jour->date().toString("dd/MM/yyyy");
            etat->setText(raisons.isEmpty() ? "Available on " + date
                         : "Unavailable on " + date + " — " + raisons.join(" · "));
        };
        connect(jour,&QDateEdit::dateChanged,pagePlanning,verifierJour);
        const auto textePeriode = [=](QDate debut, QDate fin) {
            enregistrer->setText(debut.isValid() ? "Replace saved leave" : "Save leave");
            if (!debut.isValid()) return QString("No leave period recorded.");
            return QString("Leave recorded from %1 to %2.")
                .arg(debut.toString("dd/MM/yyyy"),fin.toString("dd/MM/yyyy"));
        };
        const auto verifierPeriode = [=] {
            PlanningPersonnel::Bilan bilan;
            QString erreur;
            if (!PlanningPersonnel::analyser(membre.id,du->date(),au->date(),bilan,erreur)) {
                info->setText(erreur);
                enregistrer->setEnabled(false);
                return;
            }
            QString message = textePeriode(bilan.absenceDu,bilan.absenceAu);
            enregistrer->setEnabled(bilan.absenceDu!=du->date() || bilan.absenceAu!=au->date());
            if (!bilan.affectations.isEmpty())
                message += "\nScheduling conflict for these dates: " + bilan.affectations.join(" · ");
            info->setText(message);
        };
        connect(du,&QDateEdit::dateChanged,pagePlanning,verifierPeriode);
        connect(au,&QDateEdit::dateChanged,pagePlanning,verifierPeriode);
        connect(enregistrer,&QPushButton::clicked,pagePlanning,
            [=, ancienDebut=membre.debutIndisponibilite, ancienneFin=membre.finIndisponibilite]() mutable {
                QString erreur;
                if (!PlanningPersonnel::enregistrerAbsence(membre.id,du->date(),au->date(),
                                                          ancienDebut,ancienneFin,erreur)) {
                    info->setText(erreur);
                    return;
                }

                ancienDebut = du->date();
                ancienneFin = au->date();
                actualiserPersonnelOracle(membre.id,false);
                verifierPeriode();
                verifierJour();
            });
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape),pagePlanning);
        escape->setContext(Qt::WidgetWithChildrenShortcut);
        connect(escape,&QShortcut::activated,retour,&QPushButton::click);
        staffPages->setCurrentWidget(pagePlanning);
        verifierJour();
        verifierPeriode();
    }

    void editPersonnelOracle(bool add)
    {
        if (!oracleMode || !oracleLoaded || (!add && !selected()))
            return;

        Member original;
        if (!add)
            original = *selected();

        QVBoxLayout *pageLayout = nullptr;
        auto *editor = newStaffPage("staffOracleEditor",add?"Add Staff":"Edit Staff",
            add?"Enter the new team member's details. The Staff ID is assigned automatically when you save."
               :"Update this team member's details. Their Staff ID stays the same.",&pageLayout);
        auto *card = new QFrame(editor);
        card->setObjectName("staffFormCard");
        auto *form = new QFormLayout(card);
        form->setContentsMargins(24,20,24,20);
        form->setVerticalSpacing(12);
        form->setHorizontalSpacing(24);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

        auto *identifier = new QLineEdit(original.id, editor);
        identifier->setObjectName("memberId");
        identifier->setMaxLength(80);
        identifier->setReadOnly(true);
        identifier->setPlaceholderText("Assigned automatically on save");
        identifier->setFocusPolicy(Qt::NoFocus);

        auto *cin = new QLineEdit(original.cin, editor);
        auto *nom = new QLineEdit(original.nom, editor);
        auto *prenom = new QLineEdit(original.prenom, editor);
        auto *mail = new QLineEdit(original.email, editor);
        auto *mobile = new QLineEdit(original.phone, editor);
        cin->setObjectName("memberCin"); cin->setMaxLength(20);
        cin->setPlaceholderText("Enter CIN if known");
        nom->setObjectName("memberNom"); nom->setMaxLength(100);
        prenom->setObjectName("memberPrenom"); prenom->setMaxLength(100);
        mail->setObjectName("memberEmail"); mail->setMaxLength(254);
        mobile->setObjectName("memberPhone"); mobile->setMaxLength(40);
        auto *memberRole = new QComboBox(editor);
        memberRole->setObjectName("memberRole"); memberRole->setEditable(true);
        memberRole->lineEdit()->setMaxLength(80);
        memberRole->setInsertPolicy(QComboBox::NoInsert);
        memberRole->setMaxVisibleItems(14);
        memberRole->addItems(functions());
        memberRole->setCurrentIndex(-1);
        memberRole->lineEdit()->setPlaceholderText("Choose or enter a function");

        auto *state = new QComboBox(editor);
        state->setObjectName("memberStatus"); state->addItems({"Active", "Inactive"});
        if (!add) {
            memberRole->setCurrentText(original.role);
            state->setCurrentText(original.status);
        }
        auto makeDate = [editor](const QString &objectName, const QDate &value) {
            auto *field = new QDateEdit(editor);
            field->setObjectName(objectName); field->setCalendarPopup(true);
            field->setDisplayFormat("dd/MM/yyyy");
            field->setMinimumDate(QDate(1752,9,14));
            field->setSpecialValueText("Not specified");
            field->setDate(value.isValid()?value:field->minimumDate());
            return field;
        };
        const QDate dateEntree = add ? QDate::currentDate()
            : QLocale(QLocale::English).toDate(original.joined, "MMM d, yyyy");
        auto *joinedDate = makeDate("memberJoined",dateEntree);
        auto *unavailable = new QCheckBox("Record a leave period",editor);
        unavailable->setObjectName("memberUnavailable");
        unavailable->setChecked(original.debutIndisponibilite.isValid());
        auto *debut = makeDate("memberUnavailableFrom", original.debutIndisponibilite.isValid()
            ? original.debutIndisponibilite : QDate::currentDate());
        auto *fin = makeDate("memberUnavailableTo", original.finIndisponibilite.isValid()
            ? original.finIndisponibilite : QDate::currentDate());
        debut->setEnabled(unavailable->isChecked()); fin->setEnabled(unavailable->isChecked());
        connect(unavailable,&QCheckBox::toggled,debut,&QWidget::setEnabled);
        connect(unavailable,&QCheckBox::toggled,fin,&QWidget::setEnabled);
        form->setVerticalSpacing(6);
        form->addRow("Staff ID",identifier);
        form->addRow("CIN (unique)",cin);
        form->addRow("Last name",nom);
        form->addRow("First name",prenom);
        form->addRow("Email",mail);
        form->addRow("Phone",mobile);
        form->addRow("Function",memberRole);
        form->addRow("Status",state);
        form->addRow("Join date",joinedDate);
        form->addRow(unavailable);
        form->addRow("Leave starts",debut);
        form->addRow("Leave ends",fin);

        pageLayout->addWidget(card);
        auto *error = new QLabel(editor);
        error->setObjectName("staffValidationError");
        error->setWordWrap(true);
        error->setTextFormat(Qt::PlainText);
        error->setStyleSheet("color:#d62038;");
        error->setMinimumHeight(36);
        pageLayout->addWidget(error);

        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, editor);
        buttons->setObjectName("staffEditorButtons");
        buttons->button(QDialogButtonBox::Save)->setProperty("primary",true);
        buttons->button(QDialogButtonBox::Save)->style()->unpolish(buttons->button(QDialogButtonBox::Save));
        buttons->button(QDialogButtonBox::Save)->style()->polish(buttons->button(QDialogButtonBox::Save));
        pageLayout->addWidget(buttons);
        pageLayout->addStretch();
        connect(buttons, &QDialogButtonBox::rejected,editor,[this,editor] { closeStaffPage(editor); });
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape),editor);
        escape->setContext(Qt::WidgetWithChildrenShortcut);
        connect(escape,&QShortcut::activated,editor,[this,editor] { closeStaffPage(editor); });

        connect(buttons, &QDialogButtonBox::accepted, editor, [=] {
            const auto dateValue = [](QDateEdit *field) {
                return field->date()==field->minimumDate()?QDate():field->date();
            };
            Personnel personnel(add?QString():original.id,cin->text(),nom->text(),prenom->text(),
                mail->text(),mobile->text(),memberRole->currentText(),state->currentText(),
                dateValue(joinedDate),unavailable->isChecked()?dateValue(debut):QDate(),
                unavailable->isChecked()?dateValue(fin):QDate());

            bool saved = false;
            if (add)
                saved = personnel.ajouter();
            else
                saved = personnel.modifier();

            if (!saved) {
                error->setText(personnel.derniereErreur());
                return;
            }

            const QString savedId = personnel.getId();

            closeStaffPage(editor);
            if (!actualiserPersonnelOracle(savedId)) return;
            const QDate nouveauDebut = personnel.getDateDebutIndisponibilite();
            const QDate nouvelleFin = personnel.getDateFinIndisponibilite();
            if (nouveauDebut.isValid() && (nouveauDebut != original.debutIndisponibilite
                                         || nouvelleFin != original.finIndisponibilite)) {
                PlanningPersonnel::Bilan bilan;
                QString erreurPlanning;
                if (!PlanningPersonnel::analyser(savedId,nouveauDebut,nouvelleFin,bilan,erreurPlanning)
                    || !bilan.affectations.isEmpty())
                    ouvrirPlanningPersonnel();
            }
        });
        staffPages->setCurrentWidget(editor);
        nom->setFocus();
    }

    void supprimerPersonnelOracle()
    {
        if (!oracleMode || !oracleLoaded || !selected())
            return;

        const Member member = *selected();
        QVBoxLayout *layout = nullptr;
        auto *confirmation = newStaffPage("staffOracleDeletePage","Delete Staff",
            "Review the selected team member before confirming deletion.",&layout);
        auto *message = new QLabel(confirmation);
        message->setTextFormat(Qt::PlainText);
        message->setWordWrap(true);
        message->setText(QString("Delete %1 (%2)?")
                             .arg(member.name, member.id));
        layout->addWidget(message);
        auto *error = new QLabel(confirmation);
        error->setObjectName("staffDeleteError");
        error->setTextFormat(Qt::PlainText);
        error->setWordWrap(true);
        error->setStyleSheet("color:#d62038;");
        layout->addWidget(error);

        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Yes | QDialogButtonBox::No, confirmation);
        buttons->setObjectName("staffDeleteButtons");
        buttons->button(QDialogButtonBox::Yes)->setText("Delete");
        buttons->button(QDialogButtonBox::No)->setText("Cancel");
        buttons->button(QDialogButtonBox::No)->setDefault(true);
        layout->addWidget(buttons);
        layout->addStretch();
        connect(buttons, &QDialogButtonBox::accepted, confirmation,[this,confirmation,error,member] {
            Personnel personnel;
            if (!personnel.supprimer(member.id)) {
                error->setText(personnel.derniereErreur());
                return;
            }
            for (const auto &entry:data.audit)
                if (entry.memberId==member.id) deletedHistory.append(entry);
            deletedHistory.prepend({QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),operatorName(),member.id,member.name,"Staff deleted",member.role});
            closeStaffPage(confirmation);
            actualiserPersonnelOracle();
        });
        connect(buttons, &QDialogButtonBox::rejected,confirmation,[this,confirmation] { closeStaffPage(confirmation); });
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape),confirmation);
        escape->setContext(Qt::WidgetWithChildrenShortcut);
        connect(escape,&QShortcut::activated,confirmation,[this,confirmation] { closeStaffPage(confirmation); });
        staffPages->setCurrentWidget(confirmation);
        buttons->button(QDialogButtonBox::No)->setFocus();
    }

    void editMember(bool add) {
        if (oracleMode) { editPersonnelOracle(add); return; }
        Member *existing=selected();if(!writable || (!add && !existing))return;
        const Member original=add?Member{}:*existing;
        QDialog dialog(this);dialog.setWindowTitle(add?"Add Staff":"Edit Staff");dialog.setMinimumWidth(520);auto *form=new QFormLayout(&dialog);
        dialog.setObjectName("staffEditor");
        auto *fullName=new QLineEdit(add?"":existing->name,&dialog),*mail=new QLineEdit(add?"":existing->email,&dialog),*mobile=new QLineEdit(add?"":existing->phone,&dialog);
        fullName->setObjectName("memberName");mail->setObjectName("memberEmail");mobile->setObjectName("memberPhone");
        fullName->setMaxLength(100);mail->setMaxLength(254);mobile->setMaxLength(40);
        auto *memberRole=new QComboBox(&dialog);memberRole->setEditable(true);memberRole->addItems(functions());if(!add)memberRole->setCurrentText(existing->role);
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
        {QSignalBlocker a(search),c(listRoles),d(statusFilter);search->clear();listRoles->setCurrentIndex(0);statusFilter->setCurrentIndex(0);}
        persist(before,candidate.id,candidate.name,add?"Staff created":"Staff updated",add?"New member; module permissions default to denied.":"Changed fields: "+changes.join(", "),true);
    }
    void editPermissions() {
        if(!(oracleMode?oracleLoaded:writable) || !selected() || !reloadProfileData()) return;
        QDialog dialog(dialogOwner());dialog.setObjectName("staffAccessEditor");dialog.setWindowTitle("Edit access");
        dialog.setMinimumWidth(440);auto *layout=new QVBoxLayout(&dialog);QList<QCheckBox*> boxes;
        const QStringList modules{"Stations","Equipment","Maintenance","Services"};const auto values=permissions.value(selectedId,{false,false,false,false});
        auto *explanation=new QLabel("Choose module grants for "+selected()->name+". Inactive members have no effective access.",&dialog);
        explanation->setWordWrap(true);layout->addWidget(explanation);
        auto *profile=new QComboBox(&dialog);profile->setObjectName("accessProfile");profile->addItems({"Custom","Administrator — all modules","Field technician — stations and equipment","Maintenance — equipment and maintenance","No access"});layout->addWidget(profile);
        for(int i=0;i<4;++i){auto *box=new QCheckBox(modules[i],&dialog);box->setChecked(values[i]);layout->addWidget(box);boxes<<box;}
        connect(profile,&QComboBox::currentIndexChanged,&dialog,[&](int index){const QList<QList<bool>> profiles{{},{true,true,true,true},{true,true,false,false},{false,true,true,false},{false,false,false,false}};if(index>0)for(int i=0;i<4;++i){QSignalBlocker blocker(boxes[i]);boxes[i]->setChecked(profiles[index][i]);}});
        for(auto *box:boxes)connect(box,&QCheckBox::toggled,&dialog,[&]{QSignalBlocker blocker(profile);profile->setCurrentIndex(0);});
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()==QDialog::Accepted){QList<bool> changed;for(auto *box:boxes)changed<<box->isChecked();changeAccess(changed);}
    }
    void deleteMember() {
        if (oracleMode) { supprimerPersonnelOracle(); return; }
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
        if (oracleMode) {
            if (!oracleLoaded || !selected()) return;
            QMenu menu(this);
            menu.setObjectName("staffActions");
            auto *edit = menu.addAction("Edit Staff");
            auto *remove = menu.addAction("Delete Staff");
            auto *choice = menu.exec(QCursor::pos());
            if (choice == edit) editPersonnelOracle(false);
            else if (choice == remove) supprimerPersonnelOracle();
            return;
        }
        auto *m=selected();if(!m || !writable)return;QMenu menu(this);menu.setObjectName("staffActions");auto *edit=menu.addAction("Edit Staff");auto *toggle=menu.addAction(m->status=="Active"?"Deactivate account":"Activate account");auto *permissionsAction=menu.addAction("Edit Access");auto *trainingAction=menu.addAction("Skills & Training");menu.addSeparator();auto *remove=menu.addAction("Delete Staff");auto *choice=menu.exec(QCursor::pos());
        if(choice==edit)editMember(false);else if(choice==toggle)toggleMember();else if(choice==permissionsAction)editPermissions();else if(choice==trainingAction)showTraining();else if(choice==remove)deleteMember();
    }
    QWidget *dialogOwner() { return StaffDesign::dialogOwner(this); }
    void showStatistics() {
        QDialog dialog(dialogOwner());
        dialog.setFont(uiFont(14));
        dialog.setObjectName("statisticsDialog");
        dialog.setWindowTitle("Staff statistics");
        dialog.resize(920,455);
        auto *layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(22,18,22,16);
        layout->setSpacing(12);
        const auto stats = statistics(data);
        auto *summary = new QLabel(QString("Total %1  ·  Active %2  ·  Inactive %3")
            .arg(stats.total).arg(stats.active).arg(stats.inactive),&dialog);
        summary->setObjectName("statisticsSummary");
        summary->setFont(uiFont(18,true));
        layout->addWidget(summary);

        auto *grouping = new QComboBox(&dialog);
        grouping->setObjectName("statisticsGrouping");
        grouping->setAccessibleName("Staff grouping");
        if (!oracleMode) grouping->addItem("By department","department");
        grouping->addItem("By function","role");
        grouping->addItem("By status","status");
        layout->addWidget(grouping,0,Qt::AlignLeft);

        auto *curve = new StaffStatisticsCurve;
        curve->setObjectName("statisticsCurve");
        layout->addWidget(staffCurveView(curve,&dialog),1);
        const auto populate = [&] {
            const QString mode = grouping->currentData().toString();
            if (mode=="status")
                curve->setCounts("Staff by status",{{"Active",stats.active},{"Inactive",stats.inactive}});
            else if (mode=="department")
                curve->setCounts("Staff by department",stats.departments);
            else
                curve->setCounts("Staff by function",stats.roles);
        };
        connect(grouping,&QComboBox::currentIndexChanged,&dialog,populate);
        populate();
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close,&dialog);
        buttons->button(QDialogButtonBox::Close)->setText("Close");
        connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        layout->addWidget(buttons);
        dialog.exec();
    }
    void showAudit(const QString &memberId={}) {
        if (!reloadProfileData()) return;
        QDialog dialog(dialogOwner());dialog.setFont(uiFont(14));dialog.setObjectName("auditDialog");dialog.setWindowTitle("Staff Activity Log");dialog.resize(1100,520);
        auto *layout=new QVBoxLayout(&dialog);
        auto *filters=new QHBoxLayout;
        auto *scope=new QComboBox(&dialog);scope->setObjectName("auditScope");
        scope->addItem("All staff",QString());
        QMap<QString,QString> people;
        for (const auto &m:members) people.insert(m.id,m.name);
        for (const auto &e:data.audit) if (!people.contains(e.memberId)) people.insert(e.memberId,e.memberName);
        for (auto i=people.cbegin();i!=people.cend();++i) scope->addItem(i.value()+" · "+i.key(),i.key());
        scope->setCurrentIndex(qMax(0,scope->findData(memberId)));filters->addWidget(scope);
        auto *query=new QLineEdit(&dialog);query->setObjectName("auditSearch");query->setPlaceholderText("Search operator, action or details...");query->setClearButtonEnabled(true);filters->addWidget(query,1);
        layout->addLayout(filters);
        auto *grid=new QTableWidget(&dialog);grid->setObjectName("auditTable");grid->setColumnCount(6);grid->setHorizontalHeaderLabels({"Date / time","Operator","Staff ID","Name","Action","Details"});grid->verticalHeader()->hide();grid->setEditTriggers(QAbstractItemView::NoEditTriggers);grid->setSelectionBehavior(QAbstractItemView::SelectRows);grid->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);grid->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Stretch);layout->addWidget(grid);
        auto *count=new QLabel(&dialog);count->setObjectName("auditCount");layout->addWidget(count);
        auto populate=[&]{grid->setRowCount(0);for(const auto &entry:data.audit){
            const QString date=QDateTime::fromString(entry.timestamp,Qt::ISODateWithMs).toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
            const QStringList values{date,entry.actor,entry.memberId,entry.memberName,entry.action,entry.details};
            if (!scope->currentData().toString().isEmpty() && entry.memberId!=scope->currentData().toString()) continue;
            if(!values.join(' ').contains(query->text().trimmed(),Qt::CaseInsensitive))continue;
            int row=grid->rowCount();grid->insertRow(row);for(int c=0;c<values.size();++c){auto *item=new QTableWidgetItem(values[c]);item->setToolTip(values[c]);grid->setItem(row,c,item);}
        }count->setText(QString("%1 matching actions / %2 recorded").arg(grid->rowCount()).arg(data.audit.size()));};
        connect(query,&QLineEdit::textChanged,&dialog,populate);
        connect(scope,&QComboBox::currentIndexChanged,&dialog,populate);populate();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
    }
    void exportPdf() {
        QString path=QFileDialog::getSaveFileName(this,"Export Staff PDF","SkySense-Staff.pdf","PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";
        QSaveFile output(path);if(!output.open(QIODevice::WriteOnly)){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        {QPdfWriter writer(&output);writer.setPageSize(QPageSize(QPageSize::A4));writer.setPageOrientation(QPageLayout::Landscape);writer.setResolution(96);writer.setTitle("SkySense — Staff");
            QString html="<h1>SkySense — Staff</h1><p>Generated "+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")+" · "+QString::number(filtered.size())+" matching members</p><table border='1' cellspacing='0' cellpadding='7'><tr><th>Staff ID</th><th>Name</th><th>Email</th><th>Role</th><th>%4</th><th>Status</th></tr>";
            html.replace("%4",oracleMode?"CIN":"Department");
            for(int i:filtered){const auto &m=members[i];html+="<tr>";for(const auto &value:QStringList{m.id,m.name,m.email,m.role,oracleMode?m.cin:m.department,m.status})html+="<td>"+value.toHtmlEscaped()+"</td>";html+="</tr>";}QTextDocument document;document.setHtml(html+"</table>");document.print(&writer);}
        if(!output.commit()){QMessageBox::warning(this,"Export failed",output.errorString());return;}
        QMessageBox::information(this,"Export PDF","Staff report saved successfully.");
    }
protected:
    bool eventFilter(QObject *watched,QEvent *event) override {
        if(event->type()==QEvent::DynamicPropertyChange
            && static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName()=="workspaceReportDate") {
            const auto day=watched->property("workspaceReportDate").toDate();
            if(day.isValid() && day!=reportDay) {
                reportDay=day;
                if(availabilityDay){QSignalBlocker block(availabilityDay);availabilityDay->setDate(day);}
                updateDetails();
            }
        }
        return QWidget::eventFilter(watched,event);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.fillRect(rect(),AppTheme::color(QColor("#f1f1fb"),AppTheme::Role::Surface));
        static const QPixmap sky(":/images/weather_landscape.png");
        QRect top(0,0,width(),165);
        if(!sky.isNull()) {
            const auto scaled=sky.scaled(top.size(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            painter.drawPixmap(top,scaled,QRect(0,(scaled.height()-top.height())/2,scaled.width(),top.height()));
        }
        QLinearGradient veil(0,0,width(),0);
        const bool dark=AppTheme::dark();
        veil.setColorAt(0,dark?QColor(25,26,48,253):QColor(245,244,255,253));
        veil.setColorAt(1,dark?QColor(24,25,48,180):QColor(234,234,255,158));
        painter.fillRect(top,veil);
        static const QPixmap radar(":/assets/staff/header_radar.jpg");
        if(!radar.isNull()){
            QRect tower(width()-112,0,112,145);
            painter.setOpacity(dark?.22:.54);
            painter.drawPixmap(tower,radar,radar.rect());
            painter.setOpacity(1);
        }
        QLinearGradient fade(0,35,0,165);
        QColor base=AppTheme::color(QColor("#f1f1fb"),AppTheme::Role::Surface);
        QColor clear=base;clear.setAlpha(0);
        fade.setColorAt(0,clear);fade.setColorAt(1,base);
        painter.fillRect(top,fade);
    }
};

class StaffView final : public QGraphicsView {
    StaffCanvas *canvas;
public:
    explicit StaffView(QWidget *parent):QGraphicsView(parent) {
        setObjectName("staffView");setFrameShape(QFrame::NoFrame);setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setBackgroundBrush(QColor("#f5f9ff"));setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
        QString path=parent->property("staffStoragePath").toString();if(path.isEmpty())path=qApp->property("staffStoragePath").toString();if(path.isEmpty())path=defaultStoragePath();
        auto *scene=new QGraphicsScene(this);setScene(scene);canvas=new StaffCanvas(path);scene->addWidget(canvas);scene->setSceneRect(0,0,W-integratedSidebarWidth,H);
        canvas->setProperty("referenceWidth",W);
    }
    StaffCanvas *designCanvas() const { return canvas; }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        const int designWidth=qBound(1190,viewport()->width()-6,W-integratedSidebarWidth);
        const qreal scale = qMin<qreal>(1.1,(viewport()->width()-6)/qreal(designWidth));
        const int height=qMax(H,qFloor((viewport()->height()-2)/scale));
        canvas->setFixedSize(designWidth,height);
        if(auto *directory=canvas->findChild<QTableWidget*>("staffTable")){
            directory->setColumnHidden(7,designWidth<1250);
            directory->setColumnWidth(4,designWidth<1250?122:136);
        }
        scene()->setSceneRect(0,0,designWidth,height);
        setTransform(QTransform::fromScale(scale, scale));
        verticalScrollBar()->setValue(verticalScrollBar()->minimum());
    }
};
}

void openStaffAccess(QWidget *root) {
    auto *canvas=static_cast<StaffCanvas*>(root->property("staffCanvas").value<QWidget*>());
    if(canvas) canvas->openAccess();
}

void refreshStaffSession(QWidget *root) {
    auto *canvas=static_cast<StaffCanvas*>(root->property("staffCanvas").value<QWidget*>());
    if (canvas) canvas->sessionChanged();
}

void setupStaffUi(QWidget *root) {
    root->setObjectName("staffRoot");auto *layout=new QVBoxLayout(root);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);auto *view=new StaffView(root);layout->addWidget(view);
    root->setProperty("staffCanvas",QVariant::fromValue(static_cast<QWidget*>(view->designCanvas())));
    root->installEventFilter(view->designCanvas());
}
