#include "stafftraining.h"
#include <QtWidgets>
#include <algorithm>

namespace Staff {
namespace {
// The surrounding Staff page uses custom-painted arrows. Dialogs use an accessible
// native combo box with an explicit chevron, including for keyboard navigation.
class LevelSelector final : public QComboBox {
public:
    explicit LevelSelector(QWidget *parent):QComboBox(parent) {}
protected:
    void paintEvent(QPaintEvent *event) override {
        QComboBox::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#526faa"),1.6));
        const int x=width()-15,y=height()/2;
        painter.drawLine(x-4,y-2,x,y+2);painter.drawLine(x,y+2,x+4,y-2);
    }
};

QLabel *label(const QString &value, QWidget *parent, const QString &style={}) {
    auto *result=new QLabel(value,parent);
    result->setTextFormat(Qt::PlainText);result->setWordWrap(true);
    if(!style.isEmpty())result->setStyleSheet(style);
    return result;
}

void configureTable(QTableWidget *table, const QStringList &headers) {
    table->setColumnCount(headers.size());table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->hide();table->verticalHeader()->setDefaultSectionSize(43);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setWordWrap(false);table->setShowGrid(false);
    table->setStyleSheet("QTableWidget {background:white;border:1px solid #dce8fa;border-radius:6px;}"
                        "QHeaderView::section {background:#edf5ff;color:#24498c;padding:8px;border:0;}"
                        "QTableWidget::item {padding:6px;border-bottom:1px solid #edf3fc;}");
}

bool editRequirements(SkillLevels &requirements,const QString &role,QWidget *parent) {
    QDialog dialog(parent);dialog.setObjectName("roleRequirementsEditor");
    dialog.setWindowTitle("Role requirements");dialog.resize(740,490);
    auto *layout=new QVBoxLayout(&dialog);layout->setContentsMargins(22,20,22,20);layout->setSpacing(12);
    layout->addWidget(label("Requirements - "+role,&dialog,"font-size:20px;font-weight:600;"));
    layout->addWidget(label("These requirements apply to every member with this role. Review the starter templates and set the proficiency expected by your team.",&dialog));
    auto *table=new QTableWidget(&dialog);table->setObjectName("roleRequirementsTable");
    configureTable(table,{"Required skill","Minimum level"});
    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);table->setColumnWidth(1,200);
    layout->addWidget(table,1);
    auto addRow=[&](const QString &skill,int level){
        const int row=table->rowCount();table->insertRow(row);
        auto *name=new QLineEdit(skill,table);name->setMaxLength(80);name->setPlaceholderText("e.g. Sensor calibration");
        name->setAccessibleName("Required skill");table->setCellWidget(row,0,name);
        auto *choice=new LevelSelector(table);choice->setAccessibleName("Minimum proficiency");
        for(int i=1;i<=3;++i)choice->addItem(proficiencyLabel(i),i);
        choice->setCurrentIndex(choice->findData(level));table->setCellWidget(row,1,choice);
        table->selectRow(row);
    };
    for(auto it=requirements.cbegin();it!=requirements.cend();++it)addRow(it.key(),it.value());
    auto *rowButtons=new QHBoxLayout;
    auto *add=new QPushButton("+ Add requirement",&dialog);add->setObjectName("addRoleRequirement");
    auto *remove=new QPushButton("Remove selected",&dialog);remove->setObjectName("removeRoleRequirement");
    rowButtons->addWidget(add);rowButtons->addWidget(remove);rowButtons->addStretch();layout->addLayout(rowButtons);
    QObject::connect(add,&QPushButton::clicked,&dialog,[&]{if(table->rowCount()<100)addRow({},2);});
    QObject::connect(remove,&QPushButton::clicked,&dialog,[&]{if(table->currentRow()>=0)table->removeRow(table->currentRow());});
    auto *error=label({},&dialog,"color:#c3263a;");error->setObjectName("roleRequirementsError");layout->addWidget(error);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);layout->addWidget(buttons);
    buttons->button(QDialogButtonBox::Save)->setText("Apply to draft");
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    SkillLevels edited;
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{
        edited.clear();QSet<QString> keys;
        for(int row=0;row<table->rowCount();++row){
            const QString skill=qobject_cast<QLineEdit*>(table->cellWidget(row,0))->text().simplified();
            if(skill.isEmpty() || keys.contains(trainingKey(skill))){error->setText("Enter a unique skill name for each row (names are case-insensitive).");return;}
            keys.insert(trainingKey(skill));
            edited.insert(skill,qobject_cast<QComboBox*>(table->cellWidget(row,1))->currentData().toInt());
        }
        dialog.accept();
    });
    if(dialog.exec()!=QDialog::Accepted)return false;
    requirements=edited;return true;
}
}

bool editTraining(State &state,const QString &memberId,QString &details,QWidget *parent) {
    State draft=state;
    auto found=std::find_if(draft.members.begin(),draft.members.end(),[&](const Member &m){return m.id==memberId;});
    if(found==draft.members.end())return false;
    Member &member=*found;
    const SkillLevels originalSkills=member.skills;
    QDialog dialog(parent);dialog.setObjectName("staffTrainingDialog");
    dialog.setWindowTitle("SkySense - Skills & Training");dialog.resize(1020,730);dialog.setMinimumSize(860,620);
    auto *outer=new QVBoxLayout(&dialog);outer->setContentsMargins(20,16,20,16);outer->setSpacing(12);
    auto *scroll=new QScrollArea(&dialog);scroll->setObjectName("trainingScrollArea");
    scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);
    auto *content=new QWidget;content->setStyleSheet("background:#f8fbff;");
    auto *layout=new QVBoxLayout(content);layout->setContentsMargins(4,4,12,4);layout->setSpacing(12);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    scroll->setWidget(content);outer->addWidget(scroll,1);
    auto *heading=new QHBoxLayout;
    auto *titles=new QVBoxLayout;
    titles->addWidget(label("Skills & Training",&dialog,"font-size:25px;font-weight:700;color:#080d34;"));
    titles->addWidget(label(member.name+"  |  "+member.role+"  |  "+member.status,&dialog,"color:#526faa;font-size:14px;"));
    heading->addLayout(titles,1);
    auto *requirementsButton=new QPushButton("Edit role requirements",&dialog);requirementsButton->setObjectName("editRoleRequirements");
    heading->addWidget(requirementsButton);layout->addLayout(heading);
    layout->addWidget(label("Assess each skill, compare it with this role's requirements, and review the suggested training priorities.",&dialog));
    auto *notice=label("Starter role templates are editable. No skill level is assumed for a member. Changes are saved only with Save assessment.",&dialog,
                       "background:#edf5ff;color:#355b95;border:1px solid #dce8fa;border-radius:6px;padding:10px;");
    layout->addWidget(notice);
    auto *summary=label({},&dialog,"font-size:15px;font-weight:600;color:#005fcb;padding:5px 0;");summary->setObjectName("trainingSummary");layout->addWidget(summary);
    auto *skills=new QTableWidget(&dialog);skills->setObjectName("trainingSkillsTable");
    configureTable(skills,{"Skill","Required level","Assessed level","Assessment"});
    skills->setMinimumHeight(160);layout->addWidget(skills,3);
    auto *priorities=label("Recommended training",&dialog,"font-size:17px;font-weight:600;");priorities->setObjectName("trainingRecommendationsHeading");layout->addWidget(priorities);
    layout->addWidget(label("High priority: a gap of 2 or 3 levels. Normal priority: a gap of 1. Unassessed skills require evaluation before a training recommendation.",&dialog,"color:#526faa;"));
    auto *needs=new QTableWidget(&dialog);needs->setObjectName("trainingNeedsTable");
    configureTable(needs,{"Priority","Training area","Reason"});
    needs->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);needs->setColumnWidth(0,115);
    needs->setMinimumHeight(115);layout->addWidget(needs,2);
    auto *empty=label({},&dialog,"color:#526faa;");empty->setObjectName("trainingEmptyState");layout->addWidget(empty);
    auto *footer=new QHBoxLayout;
    footer->addWidget(label("Suggestions support the responsible person's decision; no training is booked automatically.",&dialog,"color:#526faa;"),1);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);
    buttons->button(QDialogButtonBox::Save)->setText("Save assessment");buttons->button(QDialogButtonBox::Save)->setProperty("primary",true);
    footer->addWidget(buttons);outer->addLayout(footer);

    auto updateReport=[&]{
        const auto report=trainingReport(draft,member);
        summary->setText(QString("%1 / %2 required skills assessed   |   %3 requirements met   |   %4 training needs   |   %5 to assess")
                         .arg(report.assessed).arg(report.required).arg(report.met).arg(report.needs.size()).arg(report.unassessed.size()));
        needs->setRowCount(report.needs.size());
        for(int row=0;row<report.needs.size();++row){
            const auto &need=report.needs[row];
            const QStringList values{need.gap>=2?"High":"Normal",need.skill,
                QString("Level %1 assessed / %2 required - gap %3").arg(need.current).arg(need.required).arg(need.gap)};
            for(int col=0;col<3;++col){auto *item=new QTableWidgetItem(values[col]);item->setToolTip(values[col]);needs->setItem(row,col,item);}
            needs->item(row,0)->setForeground(QColor(need.gap>=2?"#c3263a":"#a36600"));
        }
        if(!report.required)empty->setText("No requirements configured for this role. Use Edit role requirements before drawing conclusions.");
        else if(!report.unassessed.isEmpty())empty->setText(QString("Assessment still needed for %1 skill(s). These are not treated as missing competencies.").arg(report.unassessed.size()));
        else if(report.needs.isEmpty())empty->setText("All configured requirements are met. No training gap detected.");
        else empty->setText("Priorities are ordered by the largest confirmed gap. Review the recommendations with the member.");
        const auto required=requirementsForRole(draft,member.role);
        for(int row=0;row<skills->rowCount();++row){
            const QString skill=skills->item(row,0)->text();const int level=assessedLevel(member,skill);
            QString result="Additional skill";QColor color("#526faa");
            if(required.contains(skill)){
                if(level<0)result="To assess";
                else if(level>=required.value(skill)){result="Requirement met";color=QColor("#00884b");}
                else{result=QString("Gap: %1 level(s)").arg(required.value(skill)-level);color=QColor("#a36600");}
            }
            auto *item=new QTableWidgetItem(result);item->setForeground(color);skills->setItem(row,3,item);
        }
    };
    auto rebuildSkills=[&]{
        const auto required=requirementsForRole(draft,member.role);
        QStringList names=required.keys();QSet<QString> keys;
        for(const auto &name:names)keys.insert(trainingKey(name));
        for(auto it=member.skills.cbegin();it!=member.skills.cend();++it)
            if(!keys.contains(trainingKey(it.key())))names.append(it.key());
        skills->setRowCount(0);
        for(const auto &skill:names){
            const int row=skills->rowCount();skills->insertRow(row);
            auto *item=new QTableWidgetItem(skill);item->setToolTip(skill);skills->setItem(row,0,item);
            skills->setItem(row,1,new QTableWidgetItem(required.contains(skill)?proficiencyLabel(required.value(skill)):"Not required"));
            auto *level=new LevelSelector(skills);level->setAccessibleName("Assessed level: "+skill);
            for(int n=-1;n<=3;++n)level->addItem(proficiencyLabel(n),n);
            level->setCurrentIndex(level->findData(assessedLevel(member,skill)));skills->setCellWidget(row,2,level);
            QObject::connect(level,&QComboBox::currentIndexChanged,&dialog,[&,level,skill]{
                for(auto it=member.skills.begin();it!=member.skills.end();){
                    if(trainingKey(it.key())==trainingKey(skill))it=member.skills.erase(it);else ++it;
                }
                if(level->currentData().toInt()>=0)member.skills.insert(skill,level->currentData().toInt());
                updateReport();
            });
        }
        updateReport();
    };
    QObject::connect(requirementsButton,&QPushButton::clicked,&dialog,[&]{
        auto requirements=requirementsForRole(draft,member.role);
        if(editRequirements(requirements,member.role,&dialog)){
            draft.roleRequirements.insert(requirementRoleKey(draft,member.role),requirements);rebuildSkills();
        }
    });
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    rebuildSkills();
    if(dialog.exec()!=QDialog::Accepted)return false;
    if(member.skills==originalSkills && draft.roleRequirements==state.roleRequirements)return false;
    QStringList changes;
    if(member.skills!=originalSkills){
        QSet<QString> names;
        for(auto it=member.skills.cbegin();it!=member.skills.cend();++it)names.insert(it.key());
        for(auto it=originalSkills.cbegin();it!=originalSkills.cend();++it)names.insert(it.key());
        QStringList sorted=names.values();sorted.sort(Qt::CaseInsensitive);
        for(const auto &skill:sorted)if(member.skills.value(skill,-1)!=originalSkills.value(skill,-1))
            changes<<skill+": "+proficiencyLabel(originalSkills.value(skill,-1))+" -> "+proficiencyLabel(member.skills.value(skill,-1));
    }
    if(draft.roleRequirements!=state.roleRequirements)
        changes<<"Requirements updated for role "+member.role+" (applies to all members in this role)";
    details=changes.join("; ");state=draft;return true;
}
}
