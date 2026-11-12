#pragma once
#include <QDate>
#include <QStringList>
#include <QList>
#include <QMap>

namespace PlanningPersonnel {
struct Bilan {
    QDate absenceDu, absenceAu;
    QStringList motifs, affectations;
};

struct EffectifFonction {
    QString fonction;
    int minimum = 1;
    int actifs = 0;
    QStringList absents;
    int presents() const { return actifs - int(absents.size()); }
    int manque() const { return qMax(0, minimum - presents()); }
};
struct RapportEffectif {
    QList<EffectifFonction> fonctions;
    QMap<QDate,QStringList> joursAlerte;
};
// Rapport pour le jour demandé et premier déficit prévu à compter d'aujourd'hui.
bool lireEffectifs(QDate jour, const QMap<QString,int> &minimums, RapportEffectif &rapport,
                   QString &erreur);
bool analyser(const QString &id, QDate debut, QDate fin, Bilan &resultat, QString &erreur);
bool enregistrerAbsence(const QString &id, QDate debut, QDate fin,
                        QDate ancienDebut, QDate ancienneFin, QString &erreur);
}
