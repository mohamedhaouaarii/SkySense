#include "staffdata.h"
#include "appsession.h"
#include <QRegularExpression>

namespace Staff {
QString operatorName()
{
    if (AppSession::active()) return AppSession::displayName();
    QString user = qEnvironmentVariable("USERNAME");
    if (user.isEmpty()) user = qEnvironmentVariable("USER");
    return user.isEmpty() ? QStringLiteral("Local operator") : user;
}

Statistics statistics(const State &state)
{
    Statistics result;
    result.total = state.members.size();
    for (const auto &member : state.members) {
        if (member.status == "Active") ++result.active;
        else ++result.inactive;
        ++result.functions[member.role];
    }
    return result;
}

QString validateMember(const State &state, const Member &member, const QString &exceptId)
{
    static const QRegularExpression emailPattern(QStringLiteral(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)"));
    if (member.name.trimmed().isEmpty() || member.name.size() > 100)
        return "Enter the staff member's full name (maximum 100 characters).";
    if (member.email.size() > 254 || !emailPattern.match(member.email.trimmed()).hasMatch())
        return "Enter a valid email address, for example name@skysense.com.";
    if (member.role.trimmed().isEmpty())
        return "Enter the staff member's professional function.";
    if (member.role.size() > 80 || member.phone.size() > 40)
        return "A field exceeds its maximum length.";
    if (member.unavailabilityStart.isValid() != member.unavailabilityEnd.isValid() ||
        (member.unavailabilityStart.isValid() && member.unavailabilityEnd < member.unavailabilityStart))
        return "Enter one valid unavailability period with an end date on or after its start date.";
    if (member.status != "Active" && member.status != "Inactive") return "Invalid staff status.";
    for (const auto &other : state.members)
        if (other.id != exceptId && other.email.compare(member.email.trimmed(), Qt::CaseInsensitive) == 0)
            return "This email address already belongs to another staff member.";
    return {};
}
}
