#ifndef STAFFTRAINING_H
#define STAFFTRAINING_H

#include "staffdata.h"
#include <QStringList>
class QWidget;

namespace Staff {
struct TrainingNeed {
    QString skill;
    int current = -1;
    int required = 1;
    int gap = 0;
};

struct TrainingReport {
    int required = 0;
    int assessed = 0;
    int met = 0;
    QStringList unassessed;
    QList<TrainingNeed> needs;
};

QString trainingKey(const QString &value);
QString proficiencyLabel(int level);
QString requirementRoleKey(const State &state, const QString &role);
SkillLevels requirementsForRole(const State &state, const QString &role);
int assessedLevel(const Member &member, const QString &skill);
TrainingReport trainingReport(const State &state, const Member &member);

// Changes stay in a draft until Save. The caller persists and audits atomically.
bool editTraining(State &state, const QString &memberId, QString &details, QWidget *parent);
}
#endif
