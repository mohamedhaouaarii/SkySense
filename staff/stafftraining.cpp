#include "stafftraining.h"
#include <algorithm>

namespace Staff {
QString trainingKey(const QString &value) { return value.simplified().toCaseFolded(); }

RoleRequirements defaultRoleRequirements() {
    // Editable starter templates, not externally certified occupational standards.
    const SkillLevels field{{"Sensor installation",2},{"Sensor calibration",2},{"Fault diagnosis",2}};
    return {
        {"Station Technician",field}, {"Field Technician",field},
        {"Technical Specialist",{{"Sensor calibration",3},{"Fault diagnosis",3},{"Station configuration",2}}},
        {"Maintenance Lead",{{"Preventive maintenance",3},{"Fault diagnosis",3},{"Team coordination",2}}},
        {"Data Analyst",{{"Data quality control",2},{"Statistical analysis",3},{"Reporting",2}}},
        {"Operations Manager",{{"Team coordination",3},{"Intervention planning",3},{"Reporting",2}}},
        {"Administrator",{{"Access management",3},{"Data protection",2},{"System administration",3}}},
        {"Support Specialist",{{"Fault diagnosis",2},{"User support",2},{"Incident documentation",2}}}
    };
}

QString proficiencyLabel(int level) {
    switch(level) {
    case 0: return "0 - Not acquired";
    case 1: return "1 - Foundation";
    case 2: return "2 - Independent";
    case 3: return "3 - Advanced";
    default: return "Not assessed";
    }
}

QString requirementRoleKey(const State &state, const QString &role) {
    for (auto it=state.roleRequirements.cbegin();it!=state.roleRequirements.cend();++it)
        if (trainingKey(it.key())==trainingKey(role)) return it.key();
    return role.trimmed();
}

SkillLevels requirementsForRole(const State &state, const QString &role) {
    return state.roleRequirements.value(requirementRoleKey(state,role));
}

int assessedLevel(const Member &member, const QString &skill) {
    for (auto it=member.skills.cbegin();it!=member.skills.cend();++it)
        if (trainingKey(it.key())==trainingKey(skill)) return it.value();
    return -1;
}

TrainingReport trainingReport(const State &state, const Member &member) {
    TrainingReport result;
    const auto requirements=requirementsForRole(state,member.role);
    result.required=requirements.size();
    for (auto it=requirements.cbegin();it!=requirements.cend();++it) {
        const int level=assessedLevel(member,it.key());
        if (level<0) { result.unassessed.append(it.key()); continue; }
        ++result.assessed;
        if (level>=it.value()) ++result.met;
        else result.needs.append({it.key(),level,it.value(),it.value()-level});
    }
    // Explainable priority: greatest confirmed proficiency gap first, then skill name.
    std::sort(result.needs.begin(),result.needs.end(),[](const TrainingNeed &a,const TrainingNeed &b){
        if (a.gap!=b.gap) return a.gap>b.gap;
        return a.skill.compare(b.skill,Qt::CaseInsensitive)<0;
    });
    return result;
}
}
