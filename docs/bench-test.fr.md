# Procédure de test au banc

*English version: [bench-test.md](bench-test.md).*

Pour le collègue qui teste l'installation au banc. Toutes les fonctions du jeu
ont déjà été testées sur PC (`make test`). Cette procédure vérifie ce qu'un PC
ne peut pas vérifier : le vrai câblage, les vrais modules et le vrai cycliste.
Cocher chaque étape, et renvoyer le **rapport** à la fin (section 9).

Référence de câblage : [wiring.fr.md](wiring.fr.md). Carte SD : [sd-card.fr.md](sd-card.fr.md).

Les messages affichés par la console série et les commandes à taper restent en
anglais : les recopier tels quels.

## 0. Matériel nécessaire

- L'Uno avec le firmware, un câble USB et un ordinateur portable.
- Soit `arduino-cli` (`make setup` puis `make upload`), soit l'IDE Arduino
  avec la bibliothèque **FastLED 3.10.6** : ouvrir `firmware/quiberon_bike/quiberon_bike.ino`,
  carte « Arduino Uno », téléverser.
- Un moniteur série à **115200 bauds**, fin de ligne **« Nouvelle ligne »**
  (« Newline », IDE Arduino) ou `make monitor`.
- Un multimètre. Pour les premières étapes sans le vélo : un potentiomètre
  (10 kΩ entre 5 V et GND, curseur sur A1) peut remplacer la génératrice.
- La carte SD préparée comme dans [sd-card.fr.md](sd-card.fr.md).

## 1. Démarrage

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 1.1 | Allumer l'alimentation 12 V, puis brancher l'USB | La console affiche `quiberon_bike ready, free RAM N`, puis `type help …`, puis `display blank`. **Noter N** (attendu ≥ 450) | |
| 1.2 | Taper `help` | La liste des commandes | |
| 1.3 | Attendre 3 s, taper `status` | `phase idle … busy 0 busyok 0` | |
| 1.4 | Regarder les guirlandes | Des comètes rouges/vertes/dorées lentes qui montent depuis le bas, faiblement, identiques sur les **deux** guirlandes | |

Les commandes de la console série sont listées dans [l'en-tête de la console](../firmware/quiberon_bike/src/core/console.h).

## 2. Guirlandes

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 2.1 | `leds red` | Les 600 LED en **rouge** (pas en vert : cela voudrait dire que l'ordre des couleurs est faux) | |
| 2.2 | `leds green`, `leds blue` | Tout en vert, puis tout en bleu | |
| 2.3 | `leds white` | Tout en blanc, même teinte de la première à la dernière LED (une extrémité jaune/rouge signifie que l'injection 12 V est trop faible). Mesurer le courant de l'alimentation : **le noter** | |
| 2.4 | `leds off` | Tout éteint, aucune LED qui scintille, surtout la première de chaque guirlande (si elle scintille : BIN à GND, voir wiring.fr.md) | |
| 2.5 | `leds auto` | Retour aux comètes lentes du mode repos | |

## 3. Son

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 3.1 | `play 1` | Compte à rebours « Attention au départ 5-4-3-2-1-0 ». **Si on n'entend rien, s'arrêter là et passer au [3.7](#37-si-play-reste-muet)** : la suite de la section 3 n'a pas de sens sans son | |
| 3.2 | `status` pendant la lecture | `busy 1 busyok 1` | |
| 3.3 | `status` une fois la lecture finie | `busy 0`. **Seulement si le morceau a vraiment été entendu en 3.1** : si busy vaut 1 en silence et 0 pendant la lecture, la polarité de BUSY est inversée, le signaler (une ligne à changer). Sans aucun son, `busy 0` est normal (le module est au repos) et `busy 1` voudrait dire que BUSY n'est pas câblé : ne pas changer la polarité | |
| 3.4 | `play 2`, `play 3`, `play 4` | Waka Waka, puis les applaudissements, puis la musique du vainqueur (si le fichier existe) | |
| 3.5 | `phase idle` | Le son s'arrête | |
| 3.6 | Écouter le niveau à la distance des visiteurs | Volume confortable (réglé dans `config.h`, `VOLUME`, 0 à 30) | |

### 3.7 Si `play` reste muet

BUSY indique si le module joue : BAS (`busy 1`) pendant la lecture, HAUT
(`busy 0`) au repos. Muet avec `busy 0` veut dire que le module n'a jamais
démarré. Trouver le côté en défaut, dans cet ordre :

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 3.7.1 | Module alimenté, Arduino laissé tel quel : toucher un court instant la broche `IO2` (broche 11) du DFPlayer avec GND | Le module joue un morceau tout seul (sans la liaison série), BUSY (broche 16) passe à BAS. **Si oui : le module et la carte vont bien, passer au 3.7.3. Sinon : 3.7.2** | |
| 3.7.2 | Vérifier la carte et l'alimentation : FAT32, 32 Go au plus, dossier `mp3` à la racine avec `0001_….mp3`… ([carte SD](sd-card.fr.md)) ; 5 V sur VCC (broche 1) ; GND du module relié au GND de l'Arduino ; haut-parleur sur `SPK_1` / `SPK_2`. Puis essayer une autre carte, puis un autre module | Le 3.7.1 joue maintenant | |
| 3.7.3 | Vérifier la liaison : D5 → 1 kΩ → **RX (broche 2)** du DFPlayer, pas TX (broche 3) ; 2,2 kΩ de RX vers GND ; multimètre RX–GND au repos | ≈ 3,4 V sur RX ; ensuite `play 1` joue | |
| 3.7.4 | Avec un oscilloscope ou un analyseur logique : sonder RX pendant `play 1` | Une trame de 10 octets à 9600 bauds : `7E FF 06 12 00 00 01 FE E8 EF` | |

Indiquer l'étape qui a échoué (et ce qui a été vu au 3.7.4, si fait).

## 4. Bouton de départ

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 4.1 | Appuyer une fois sur le bouton | `phase COUNTDOWN …`, `display 0`, le compte à rebours est joué, la guirlande se remplit depuis le bas en 5 étapes (rouge → vert) | |
| 4.2 | Appuyer de nouveau pendant le compte à rebours | Rien ne change | |
| 4.3 | Fin du son du compte à rebours | `phase RIDE …` à ce moment-là (± 0,5 s), la musique Waka Waka démarre | |
| 4.4 | `phase idle` | Retour au repos, silence | |

## 5. Étalonnage de la génératrice

À faire avec le vrai vélo et la vraie génératrice. **D'abord, au multimètre,
vérifier que A1 ne dépasse jamais 5 V lors du sprint le plus intense.**
Au-dessus de 5 V l'Arduino peut être endommagé : corriger d'abord
l'adaptateur de signal.

| # | Faire | Attendu / à noter | ✓ |
|---|---|---|---|
| 5.1 | Taper `watch` (affiche l'état deux fois par seconde) | Des lignes avec `adc` et `level` | |
| 5.2 | Vélo à l'arrêt | **Noter la valeur adc** (attendu : proche de 0) | |
| 5.3 | Pédalage normal, détendu | **Noter adc**, et la tension sur A1 | |
| 5.4 | Sprint intense (un adulte en forme) | **Noter adc**, et la tension sur A1 | |
| 5.5 | Taper de nouveau `watch` pour arrêter | | |

Ensuite dans `config.h` : `ADC_REST` = valeur au repos + une petite marge
(environ +10), `ADC_MAX` = la valeur au sprint. Téléverser de nouveau et
vérifier que `level` est proche de 0 au repos et proche de 255 au sprint. Faute
de temps, envoyer simplement les valeurs.

## 6. Effet de course (sans cycliste)

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 6.1 | `phase ride`, puis `speed 0` | Des points violets qui « respirent » lentement, une courte section bleue en bas avec des comètes en mouvement. Pas plus sombre que le repos | |
| 6.2 | `speed 64`, `speed 128`, `speed 200`, `speed 255` | La partie allumée monte dans le sapin, les comètes deviennent plus rapides et plus lumineuses, la couleur passe du bleu au rouge, des paillettes blanches apparaissent au-dessus de 128 ; à 255 tout le sapin est allumé | |
| 6.3 | `speed off`, puis pédaler | La même chose suit le vrai pédalage, en douceur (pas de scintillement à vitesse constante) | |
| 6.4 | `phase finale` | Feu d'artifice : des fusées qui montent, un flash blanc, des étincelles colorées qui retombent en s'écartant et s'éteignent. Applaudissements, puis musique du vainqueur | |
| 6.5 | `phase idle` | Retour au repos | |

## 7. Parties complètes (avec de vrais cyclistes)

Utiliser un chronomètre.

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 7.1 | Appuyer sur départ, pédaler normalement | Compte à rebours, puis la course ; l'affichage augmente pendant le pédalage, s'arrête quand on arrête | |
| 7.2 | Pendant les 3 minutes | La musique ne s'interrompt jamais plus de ~2 s (elle boucle) ; la guirlande suit la vitesse | |
| 7.3 | À 3:00 après la fin du compte à rebours | `phase FINALE`, feu d'artifice, applaudissements puis musique du vainqueur ; **l'affichage se fige** même si on continue à pédaler | |
| 7.4 | 30 s plus tard | `phase IDLE`, silence, comètes du repos ; l'affichage **garde le dernier score** | |
| 7.5 | Chaque ligne `phase …` se termine par `stack headroom N` | **Noter le plus petit N** sur toutes les parties (doit rester au-dessus de 100) | |
| 7.6 | Une deuxième partie juste après | L'affichage revient à 0 au compte à rebours, et tout se rejoue | |
| 7.7 | Calories | 3 minutes détendues ≈ 10 à 20 kcal, 3 minutes intenses ≈ 30 à 50 (l'affichage plafonne à 99). Noter les scores : le modèle de calories devra peut-être être ajusté | |

## 8. Robustesse

| # | Faire | Attendu | ✓ |
|---|---|---|---|
| 8.1 | Débrancher le fil BUSY (D3), jouer une partie | Le jeu fonctionne quand même : le compte à rebours dure 10 s, les applaudissements 12 s avant la musique du vainqueur | |
| 8.2 | Couper l'alimentation, maintenir le bouton, rallumer, maintenir encore 5 s | Aucune partie ne démarre tant que le bouton n'est pas relâché puis appuyé de nouveau | |
| 8.3 | Laisser tourner au repos pendant 1 heure, puis jouer une partie | Tout fonctionne encore | |
| 8.4 | Débrancher l'USB (alimentation par le côté 12 V uniquement, si câblé ainsi) | Fonctionne de la même façon ; la console série ne sert qu'au banc | |

## 9. Rapport à renvoyer

```
RAM libre au démarrage (1.1) :                ____
courant de l'alim, tout en blanc (2.3) :      ____ A
polarité BUSY correcte (3.3) :                oui / non / non vérifiée (pas de son)
lecture muette, étape en échec (3.7) :        ____
volume OK (3.6) :                             oui / non, préféré : ____
adc au repos / normal / sprint :              ____ / ____ / ____
tension A1 au sprint :                        ____ V
musique de course en boucle sans coupure :    oui / non (le clone ignore la répétition ?)
plus petit stack headroom (7.5) :             ____
scores des cyclistes de test (7.7) :          ____
autre chose de bizarre :
```

Le matériel d'affichage n'est pas encore choisi : en attendant, sa valeur est
affichée sur la liaison série (`display N`).
