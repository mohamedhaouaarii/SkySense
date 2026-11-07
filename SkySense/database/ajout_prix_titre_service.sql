-- SERVICE : titre unique et nouvel attribut PRIX (demande de l'equipe).
-- Script non destructif : ajoute une colonne et deux contraintes, aucune donnee n'est supprimee.

-- Prix du service en TND, facultatif, jamais negatif.
ALTER TABLE SERVICE ADD (PRIX NUMBER(10,2));
ALTER TABLE SERVICE ADD CONSTRAINT CK_SERVICE_PRIX CHECK (PRIX IS NULL OR PRIX >= 0);

-- Deux services ne peuvent pas avoir le meme titre.
ALTER TABLE SERVICE ADD CONSTRAINT UQ_SERVICE_TITRE UNIQUE (TITRE);
