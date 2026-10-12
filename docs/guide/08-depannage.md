# 8. Dépannage et glossaire

[← Refaire le projet](07-refaire-le-projet.md) · [Sommaire](README.md)

## Dépannage

| Symptôme | Causes probables | Que faire |
|---|---|---|
| `make firmware` échoue avec `FAIL: static RAM … exceeds` | une modification a ajouté des variables | chercher ce qui a été ajouté ; textes en `F("...")` ; types plus petits ; voir [la mémoire](04-logiciel.md#la-mémoire-la-vraie-contrainte) |
| `make upload` : port introuvable ou permission refusée | mauvais port, ou droits sous Linux | `arduino-cli board list` ; sous Linux, ajouter l'utilisateur au groupe `dialout` puis se reconnecter |
| La console n'affiche rien ou des caractères bizarres | mauvaise vitesse | régler le moniteur sur **115200 bauds** |
| La console ne répond pas aux commandes | pas de fin de ligne envoyée | moniteur de l'IDE : choisir « Nouvelle ligne » |
| Aucune LED ne s'allume | GND non commun ; pas de 12 V ; fil de données sur la sortie du ruban | vérifier la masse commune ; vérifier le sens des flèches ; `leds white` |
| Couleurs fausses (`leds red` donne du vert) | ordre des couleurs | l'ordre est `GRB` dans `FastLED.addLeds<…>()` du sketch ; le changer si votre ruban est différent |
| La première LED scintille | BI de la première LED non relié | relier BI au GND côté entrée |
| La fin de la guirlande est jaune / rouge en `leds white` | chute de tension sur 5 m | injecter le 12 V aux deux bouts, fils plus gros |
| Des LED clignotent au hasard, surtout loin de l'Arduino | fil de données trop long, parasites | raccourcir, vérifier la résistance de 330 Ω, ajouter un tampon 74HCT245 |
| Pas de son | carte SD mal préparée ; fil RX ; volume | `play 1` ; vérifier le dossier `/mp3` et les noms `0001_…` ([carte SD](../sd-card.fr.md)) ; vérifier les résistances de RX (1 kΩ en série, 2,2 kΩ vers GND : ≈ 3,4 V sur RX au repos) |
| Le son met plusieurs secondes à démarrer | fichiers MP3 avec de grosses pochettes | retirer les images des fichiers (tags ID3) |
| Le compte à rebours dure toujours 10 s | BUSY ne fonctionne pas | `status` : `busyok 0` ? vérifier le fil BUSY → D3 ; polarité |
| La musique de course s'arrête | le clone ignore la commande de répétition et BUSY ne marche pas | réparer BUSY : le programme relance alors la musique tout seul |
| `level` reste à 0 en pédalant | adaptateur mal câblé ; `ADC_MAX` trop grand ; génératrice non reliée au GND | `watch` et regarder `adc` ; mesurer la tension sur A1 |
| `level` ne descend pas à 0 à l'arrêt | bruit au-dessus de `ADC_REST` | augmenter `ADC_REST` (valeur au repos + 10) |
| `level` atteint 255 trop facilement | `ADC_MAX` trop petit | refaire l'étalonnage |
| Scores de calories irréalistes | table de puissance à ajuster | modifier `POWER_WATTS` ([chapitre 5](05-interfaces.md#régler-le-jeu--configh)) |
| Le jeu redémarre tout seul (le message `ready` réapparaît) | baisse de tension de l'alimentation, ou pile débordée | vérifier l'alimentation de l'Arduino ; regarder `stack headroom` |
| Une partie démarre toute seule | parasites sur le long fil du bouton | condensateur de 100 nF entre D2 et GND, câble torsadé |

## Glossaire

| Mot | Définition |
|---|---|
| **ADC** | convertisseur analogique-numérique : transforme une tension (0 à 5 V) en nombre (0 à 1023) |
| **Anti-rebond** | traitement qui ne garde qu'un seul changement d'état quand un contact mécanique rebondit |
| **Baud** | nombre de bits transmis par seconde sur une liaison série |
| **BUSY** | broche du lecteur MP3 qui indique s'il est en train de jouer |
| **Console série** | dialogue texte entre le PC et l'Arduino par le câble USB |
| **Diviseur de tension** | deux résistances en série qui donnent une fraction d'une tension |
| **Firmware** | le programme qui tourne dans le microcontrôleur |
| **Flash** | mémoire qui garde le programme hors tension |
| **GND, masse** | la référence 0 V commune à tout le montage |
| **HAL** | *Hardware Abstraction Layer* : la couche de code qui isole la logique du matériel |
| **Interruption** | événement matériel qui suspend un instant le programme pour exécuter une petite fonction |
| **Machine à états** | programme organisé en un petit nombre d'états, avec des règles de passage de l'un à l'autre |
| **`millis()`** | nombre de millisecondes depuis l'allumage de l'Arduino |
| **Non bloquant** | se dit d'un code qui ne s'arrête jamais pour attendre |
| **Pile (stack)** | zone de la RAM où les fonctions rangent leurs variables locales |
| **Pull-up** | résistance qui tire une entrée vers +5 V quand rien ne la force à 0 |
| **RAM** | mémoire de travail, effacée à chaque redémarrage (2048 octets sur l'Uno) |
| **Sketch** | le fichier `.ino` d'un programme Arduino |
| **Somme de contrôle** | nombre calculé à partir d'un message pour détecter les erreurs de transmission |
| **Trame** | un message complet envoyé d'un bloc (ici 10 octets pour le lecteur MP3) |
| **UART** | liaison série asynchrone : un bit de start, 8 bits de données, un bit de stop |
| **Virgule fixe** | façon de représenter des nombres à virgule avec des entiers (on décide que les 8 bits du bas sont la partie après la virgule) |
| **WS2815** | LED RGB avec sa puce de commande intégrée, alimentée en 12 V, avec une ligne de données de secours |
| **Zener (diode)** | diode qui conduit en inverse au-delà d'une tension précise : sert à limiter une tension |
