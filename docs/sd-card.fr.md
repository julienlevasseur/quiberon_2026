# Carte SD du lecteur MP3 (MP3-TF-16P / DFPlayer Mini)

*English version: [sd-card.md](sd-card.md).*

Le lecteur trouve les pistes par leur **numéro**, pas par leur nom. Le firmware
demande « la piste 1 du dossier `mp3` », et le module cherche un fichier dont le
nom **commence par `0001`**. Ce qui suit les quatre chiffres est libre.

## Organisation

Carte formatée en **FAT32** (FAT16 fonctionne aussi), 32 Go maximum.

```
/mp3/0001_AttentionAuDepart543210.mp3   compte à rebours de départ, la course de 3 minutes démarre à la fin
/mp3/0002_WakaWakaShakira.mp3           pendant la course, en boucle
/mp3/0003_BravoApplaudissements.mp3     applaudissements au début du final
/mp3/0004_Vainqueur.mp3                 musique du vainqueur après les applaudissements (fichier à venir)
```

- Le dossier s'appelle `mp3` en minuscules, à la racine de la carte.
- Quatre chiffres, puis n'importe quoi : `0002_WakaWakaShakira.mp3` et `0002.mp3` sont équivalents.
- Les numéros correspondent aux `TRACK_*` de [config.h](../firmware/quiberon_bike/src/core/config.h).
  Pour changer un son, remplacer le fichier en gardant son numéro.
- MP3 en 44,1 kHz ou 48 kHz, 128 à 320 kbit/s. Éviter les gros tags ID3 avec
  pochette : certains modules attendent plusieurs secondes avant de les jouer.
- Le volume est réglé par le firmware (`VOLUME = 20` sur 30), pas par les
  fichiers. Normaliser les quatre fichiers à un niveau sonore proche, pour que
  les applaudissements ne soient pas deux fois plus forts que la musique.

## Bonnes pratiques

- Copier les fichiers sur une carte **fraîchement formatée**. Supprimer puis
  recopier des fichiers peut laisser des entrées de répertoire obsolètes qui
  perturbent certains clones.
- Ne pas laisser d'autres fichiers à la racine (pas de `System Volume Information`
  de Windows ni de `.Trashes` de macOS si possible ; sous macOS, lancer
  `dot_clean /Volumes/<carte>` après la copie).
- Après la copie, vérifier au banc avec la console série : `play 1` … `play 4`
  (voir [bench-test.fr.md](bench-test.fr.md)).
