#include "staffdata.h"
#include "stafftraining.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <limits>

namespace Staff {
QString operatorName() {
    QString user = qEnvironmentVariable("USERNAME");
    if (user.isEmpty()) user = qEnvironmentVariable("USER");
    return user.isEmpty() ? QStringLiteral("Local operator") : user;
}

QString defaultStoragePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/staff.json";
}

Statistics statistics(const State &state) {
    Statistics result;
    result.total = state.members.size();
    for (const auto &member : state.members) {
        if (member.status == "Active") ++result.active;
        else ++result.inactive;
        ++result.departments[member.department];
        ++result.roles[member.role];
    }
    return result;
}

bool effectiveAccess(const State &state, const QString &id, int module) {
    if (module < 0 || module >= 4) return false;
    for (const auto &member : state.members) {
        if (member.id == id)
            return member.status == "Active" && state.permissions.value(id, {false, false, false, false}).value(module);
    }
    return false;
}

QString validateMember(const State &state, const Member &member, const QString &exceptId) {
    static const QRegularExpression emailPattern(QStringLiteral(R"(^[^\s@]+@[^\s@.]+(?:\.[^\s@.]+)+$)"));
    if (member.name.trimmed().isEmpty() || member.name.size() > 100)
        return "Enter the staff member's full name (maximum 100 characters).";
    if (member.email.size() > 254 || !emailPattern.match(member.email.trimmed()).hasMatch())
        return "Enter a valid email address, for example name@skysense.com.";
    if (member.role.trimmed().isEmpty() || member.department.trimmed().isEmpty())
        return "Role and department are required.";
    if (member.role.size() > 80 || member.department.size() > 80 || member.phone.size() > 40 || member.note.size() > 1000)
        return "A field exceeds its maximum length.";
    if (member.status != "Active" && member.status != "Inactive") return "Invalid staff status.";
    for (const auto &other : state.members)
        if (other.id != exceptId && other.email.compare(member.email.trimmed(), Qt::CaseInsensitive) == 0)
            return "This email address already belongs to another staff member.";
    return {};
}

namespace {
bool decodeLevels(const QJsonValue &value, SkillLevels &levels, int minimum, QString &error) {
    if (!value.isObject() || value.toObject().size()>100) {
        error="Invalid training skills or role requirements."; return false;
    }
    QSet<QString> keys;
    const auto object=value.toObject();
    for (auto it=object.begin();it!=object.end();++it) {
        const QString key=trainingKey(it.key());
        const int level=it.value().toInt(-1);
        if (key.isEmpty() || it.key().size()>80 || keys.contains(key)
            || !it.value().isDouble() || it.value().toDouble()!=level || level<minimum || level>3) {
            error="Training skills must be unique, with valid proficiency levels."; return false;
        }
        keys.insert(key);
        levels.insert(it.key(),level);
    }
    return true;
}

QJsonObject encodeLevels(const SkillLevels &levels) {
    QJsonObject object;
    for (auto it=levels.cbegin();it!=levels.cend();++it) object.insert(it.key(),it.value());
    return object;
}

bool decode(const QByteArray &bytes, State &state, QString &error) {
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(bytes, &parseError);
    const auto object = document.object();
    const int schema=object.value("schema").toInt();
    if (parseError.error != QJsonParseError::NoError || !document.isObject()
        || (schema!=1 && schema!=2) || !object.value("members").isArray()
        || !object.value("permissions").isObject() || !object.value("audit").isArray()) {
        error = "The Staff data file is invalid or uses an unsupported format. It has not been changed.";
        return false;
    }
    State loaded;
    loaded.nextId = object.value("nextId").toInt(0);
    loaded.revision = object.value("revision").toInt(0);
    if (loaded.nextId < 1 || loaded.revision < 1) { error = "Invalid Staff data counters."; return false; }
    QSet<QString> ids;
    for (const auto &value : object.value("members").toArray()) {
        const auto row = value.toObject();
        Member member{row.value("id").toString(), row.value("name").toString(), row.value("email").toString(),
                      row.value("role").toString(), row.value("department").toString(), row.value("status").toString(),
                      row.value("login").toString(), row.value("phone").toString(), row.value("joined").toString(),
                      row.value("note").toString()};
        if (schema==2 && !decodeLevels(row.value("skills"),member.skills,0,error)) return false;
        if (member.id.isEmpty() || ids.contains(member.id) || !validateMember(loaded, member).isEmpty()) {
            error = "Invalid or duplicate Staff record. The data file has not been changed."; return false;
        }
        ids.insert(member.id);
        loaded.members.append(member);
    }
    // Schema 1 files keep all existing records and receive editable starter templates.
    // Their members remain unassessed. Upgrade is written only on the next successful save.
    if (schema==2) {
        if (!object.value("roleRequirements").isObject()) {
            error="Missing training role requirements."; return false;
        }
        loaded.roleRequirements.clear();
        const auto roles=object.value("roleRequirements").toObject();
        QSet<QString> keys;
        for (auto it=roles.begin();it!=roles.end();++it) {
            const QString key=trainingKey(it.key());
            SkillLevels requirements;
            if (key.isEmpty() || it.key().size()>80 || keys.contains(key)) {
                error="Invalid or duplicate role in training requirements."; return false;
            }
            if (!decodeLevels(it.value(),requirements,1,error)) return false;
            keys.insert(key);
            loaded.roleRequirements.insert(it.key(),requirements);
        }
    }
    const auto grants = object.value("permissions").toObject();
    for (auto it = grants.begin(); it != grants.end(); ++it) {
        if (!ids.contains(it.key()) || !it.value().isArray() || it.value().toArray().size() != 4) {
            error = "Invalid Staff permissions."; return false;
        }
        QList<bool> rights;
        for (const auto &value : it.value().toArray()) {
            if (!value.isBool()) { error = "Invalid Staff permission value."; return false; }
            rights.append(value.toBool());
        }
        loaded.permissions.insert(it.key(), rights);
    }
    for (const auto &value : object.value("audit").toArray()) {
        const auto row = value.toObject();
        AuditEntry entry{row.value("timestamp").toString(), row.value("actor").toString(),
                         row.value("memberId").toString(), row.value("memberName").toString(),
                         row.value("action").toString(), row.value("details").toString()};
        if (!QDateTime::fromString(entry.timestamp, Qt::ISODateWithMs).isValid() || entry.actor.isEmpty()
            || entry.action.isEmpty() || entry.memberId.isEmpty()) {
            error = "Invalid Staff activity entry."; return false;
        }
        loaded.audit.append(entry);
    }
    state = loaded;
    return true;
}

QByteArray encode(const State &state, int revision) {
    QJsonArray members, audit;
    QJsonObject permissions;
    for (const auto &m : state.members)
        members.append(QJsonObject{{"id",m.id},{"name",m.name},{"email",m.email},{"role",m.role},
                                  {"department",m.department},{"status",m.status},{"login",m.login},
                                  {"phone",m.phone},{"joined",m.joined},{"note",m.note},
                                  {"skills",encodeLevels(m.skills)}});
    for (auto it = state.permissions.begin(); it != state.permissions.end(); ++it) {
        QJsonArray rights;
        for (bool right : it.value()) rights.append(right);
        permissions.insert(it.key(), rights);
    }
    for (const auto &e : state.audit)
        audit.append(QJsonObject{{"timestamp",e.timestamp},{"actor",e.actor},{"memberId",e.memberId},
                                 {"memberName",e.memberName},{"action",e.action},{"details",e.details}});
    QJsonObject requirements;
    for (auto it=state.roleRequirements.cbegin();it!=state.roleRequirements.cend();++it)
        requirements.insert(it.key(),encodeLevels(it.value()));
    return QJsonDocument(QJsonObject{{"schema",2},{"revision",revision},{"nextId",state.nextId},
                                    {"members",members},{"permissions",permissions},{"audit",audit},
                                    {"roleRequirements",requirements}}).toJson();
}
}

Store::Store(QString path) : filePath(std::move(path)) {}
bool validateState(const State &state,QString &error) {
    State checked;
    return decode(encode(state,qMax(1,state.revision)),checked,error);
}
bool Store::exists() const { return QFile::exists(filePath); }
bool Store::load(State &state, QString &error) const {
    error.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) { error = file.errorString(); return false; }
    return decode(file.readAll(), state, error);
}

bool Store::save(State &state, QString &error) const {
    error.clear();
    if (!QDir().mkpath(QFileInfo(filePath).absolutePath())) { error = "Cannot create the Staff data folder."; return false; }
    QLockFile lock(filePath + ".lock");
    if (!lock.tryLock(0)) { error = "Another instance is saving Staff data. Try again."; return false; }
    State disk;
    if (exists()) {
        if (!load(disk, error)) return false;
        if (disk.revision != state.revision) {
            error = "Staff data changed in another instance. Reopen this window before editing."; return false;
        }
    } else if (state.revision != 0) { error = "The Staff data file was removed. Reopen this window before editing."; return false; }
    if (state.revision == std::numeric_limits<int>::max()) { error = "Staff data revision limit reached."; return false; }
    const int nextRevision = state.revision + 1;
    const QByteArray bytes = encode(state, nextRevision);
    State validated;
    if (!decode(bytes, validated, error)) return false;
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error = file.errorString(); return false;
    }
    state.revision = nextRevision;
    return true;
}
}
