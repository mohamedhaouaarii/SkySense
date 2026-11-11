#include "planningpersonnel.h"
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <algorithm>

namespace PlanningPersonnel {
namespace {
struct Fiche {
    QString id, nomComplet, fonction, nomInverse, statut;
    QDate entree, absenceDu, absenceAu;
};

QString cle(const QString &texte)
{
    return texte.simplified().toCaseFolded();
}

bool chargerFiches(QList<Fiche> &fiches, QString &erreur)
{
    QSqlQuery q;
    if (!q.exec("SELECT ID_PERSONNEL, PRENOM, NOM, FONCTION, STATUT, DATE_ENTREE, "
                "DATE_DEBUT_INDISPONIBILITE, DATE_FIN_INDISPONIBILITE FROM STAFF_PERSONNEL "
                "ORDER BY NOM, PRENOM, ID_PERSONNEL")) {
        erreur = q.lastError().text();
        return false;
    }
    while (q.next()) {
        const QString prenom = q.value(1).toString();
        const QString nom = q.value(2).toString();
        fiches.append({q.value(0).toString(), prenom+" "+nom, q.value(3).toString(),
                       nom+" "+prenom, q.value(4).toString(), q.value(5).toDate(),
                       q.value(6).toDate(), q.value(7).toDate()});
    }
    if (q.lastError().isValid()) {
        erreur = q.lastError().text();
        return false;
    }
    return true;
}

bool chargerAffectations(const QString &sql, QDate debut, QDate fin,
                         QHash<QString,QStringList> &planning, QString &erreur)
{
    QSqlQuery q;
    if (!q.prepare(sql)) {
        erreur = q.lastError().text();
        return false;
    }
    q.addBindValue(fin);
    q.addBindValue(debut);
    if (!q.exec()) {
        erreur = q.lastError().text();
        return false;
    }
    while (q.next()) {
        const QString personne = cle(q.value(0).toString());
        if (!personne.isEmpty())
            planning[personne].append(q.value(1).toString() + " — " + q.value(2).toString());
    }
    if (q.lastError().isValid()) {
        erreur = q.lastError().text();
        return false;
    }
    return true;
}

QStringList motifs(const Fiche &p, QDate debut, QDate fin)
{
    QStringList raisons;
    if (p.statut != "Active")
        raisons << "Inactive staff member";
    if (p.entree.isValid() && p.entree > debut)
        raisons << "Join date is after the start of this period";
    if (p.absenceDu.isValid() && p.absenceDu <= fin && p.absenceAu >= debut)
        raisons << QString("On leave from %1 to %2")
                   .arg(p.absenceDu.toString("dd/MM/yyyy"),p.absenceAu.toString("dd/MM/yyyy"));
    return raisons;
}
}

bool analyser(const QString &id, QDate debut, QDate fin, Bilan &resultat, QString &erreur)
{
    resultat = {};
    erreur.clear();
    QList<Fiche> fiches;
    if (!chargerFiches(fiches,erreur)) return false;
    const auto choix = std::find_if(fiches.cbegin(),fiches.cend(),
        [&id](const Fiche &p){ return p.id == id; });
    if (choix == fiches.cend()) {
        erreur = "This staff record no longer exists.";
        return false;
    }
    if (!debut.isValid() || !fin.isValid() || fin < debut) {
        erreur = "Choose an end date on or after the start date.";
        return false;
    }

    // Les listes de choix des autres modules ont leurs propres identifiants numériques.
    // L'affectation textuelle réelle contient le nom complet ou l'identifiant Staff.
    QHash<QString,QStringList> planning;
    if (!chargerAffectations(
            "SELECT p.STAFF_KEY, m.CODE, m.TITRE FROM MAINTENANCE m JOIN REALISER r ON r.ID_MAINTENANCE=m.ID_MAINTENANCE "
            "JOIN PERSONNEL p ON p.ID_PERSONNEL=r.ID_PERSONNEL WHERE m.STATUT IN ('Planned','In Progress') "
            "AND TRUNC(m.DATE_PREVUE)<=? AND TRUNC(COALESCE(m.DATE_REALISEE,m.DATE_PREVUE))>=? ORDER BY m.DATE_PREVUE,m.CODE",
            debut,fin,planning,erreur)) return false;
    if (!chargerAffectations(
            "SELECT p.STAFF_KEY, TO_CHAR(s.ID_SERVICE), s.TITRE FROM SERVICE s JOIN AFFECTER a ON a.ID_SERVICE=s.ID_SERVICE "
            "JOIN PERSONNEL p ON p.ID_PERSONNEL=a.ID_PERSONNEL WHERE s.STATUT IN ('Scheduled','In Progress','Overdue') "
            "AND TRUNC(s.DATE_PREVUE)<=? AND TRUNC(s.DATE_PREVUE)>=? ORDER BY s.DATE_PREVUE,s.ID_SERVICE",
            debut,fin,planning,erreur)) return false;

    resultat.absenceDu = choix->absenceDu;
    resultat.absenceAu = choix->absenceAu;
    resultat.motifs = motifs(*choix,debut,fin);
    resultat.affectations = planning.value(cle(choix->id));
    resultat.affectations += planning.value(cle(choix->nomComplet));
    resultat.affectations += planning.value(cle(choix->nomInverse));
    resultat.affectations.removeDuplicates();
    return true;
}

bool lireEffectifs(QDate jour, const QMap<QString,int> &minimums, RapportEffectif &rapport,
                   QString &erreur)
{
    rapport = {};
    erreur.clear();
    if (!jour.isValid()) {
        erreur = "Choose a date to check.";
        return false;
    }
    QList<Fiche> fiches;
    if (!chargerFiches(fiches,erreur)) return false;
    const QDate aujourdhui = QDate::currentDate();
    QMap<QString,EffectifFonction> groupes;
    QMap<QString,QMap<QDate,int>> variations;
    for (auto it = minimums.cbegin(); it != minimums.cend(); ++it) {
        const QString role = cle(it.key());
        if (role.isEmpty()) continue;
        auto &groupe = groupes[role];
        groupe.fonction = it.key().simplified();
        groupe.minimum = qMax(1,it.value());
    }
    for (const auto &fiche : fiches) {
        const QString role = cle(fiche.fonction);
        auto &groupe = groupes[role];
        if (groupe.fonction.isEmpty()) groupe.fonction = fiche.fonction.simplified();
        if (fiche.statut != "Active") continue;
        if (!fiche.entree.isValid() || fiche.entree <= jour) {
            ++groupe.actifs;
            if (fiche.absenceDu.isValid() && fiche.absenceDu <= jour && jour <= fiche.absenceAu)
                groupe.absents << fiche.nomComplet;
        }
        // Une entrée, un début d'absence et le lendemain du retour sont les seuls
        // jours où la couverture peut changer. Additionner les événements du même jour.
        const QDate entree = qMax(aujourdhui,fiche.entree);
        auto &deltas = variations[role];
        ++deltas[entree];
        if (fiche.absenceDu.isValid() && fiche.absenceAu >= entree) {
            --deltas[qMax(entree,fiche.absenceDu)];
            const QDate retour = fiche.absenceAu.addDays(1);
            if (retour.isValid()) ++deltas[retour];
        }
    }
    rapport.fonctions = groupes.values();
    for (auto it = groupes.cbegin(); it != groupes.cend(); ++it) {
        auto &deltas = variations[it.key()];
        deltas[aujourdhui] += 0; // Inclut les fonctions sans aucune présence.
        int presents = 0;
        for (auto d = deltas.cbegin(); d != deltas.cend(); ++d) {
            presents += d.value();
            if (presents >= it->minimum) continue;
            rapport.joursAlerte[d.key()].append(it->fonction);
            break;
        }
    }
    return true;
}

bool enregistrerAbsence(const QString &id, QDate debut, QDate fin,
                        QDate ancienDebut, QDate ancienneFin, QString &erreur)
{
    erreur.clear();
    if (!debut.isValid() || !fin.isValid() || fin < debut) {
        erreur = "Choose an end date on or after the start date.";
        return false;
    }
    // Éviter de remplacer une période modifiée entre-temps par un collègue.
    QString sql = "UPDATE STAFF_PERSONNEL SET DATE_DEBUT_INDISPONIBILITE=?, "
                  "DATE_FIN_INDISPONIBILITE=? WHERE ID_PERSONNEL=? AND ";
    if (ancienDebut.isValid())
        sql += "TRUNC(DATE_DEBUT_INDISPONIBILITE)=? AND TRUNC(DATE_FIN_INDISPONIBILITE)=?";
    else
        sql += "DATE_DEBUT_INDISPONIBILITE IS NULL AND DATE_FIN_INDISPONIBILITE IS NULL";
    QSqlQuery q;
    if (!q.prepare(sql)) {
        erreur = q.lastError().text();
        return false;
    }
    q.addBindValue(debut);
    q.addBindValue(fin);
    q.addBindValue(id);
    if (ancienDebut.isValid()) {
        q.addBindValue(ancienDebut);
        q.addBindValue(ancienneFin);
    }
    if (!q.exec()) {
        erreur = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() != 1) {
        erreur = "This record has changed. Reopen availability before saving.";
        return false;
    }
    return true;
}
}
