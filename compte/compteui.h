#ifndef COMPTEUI_H
#define COMPTEUI_H

class QWidget;

// Premier lancement connecte a Oracle : si aucun compte ADMIN n'existe dans PERSONNEL,
// demande a l'utilisateur de creer le compte administrateur. Retourne true si un admin existe.
bool assurerCompteAdmin(QWidget *parent = nullptr);

#endif // COMPTEUI_H
