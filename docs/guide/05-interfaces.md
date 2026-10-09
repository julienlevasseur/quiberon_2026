# 5. Utiliser les interfaces

[← Le programme](04-logiciel.md) · [Sommaire](README.md) · [Chapitre suivant : tests et simulateur →](06-tests.md)

Ce chapitre décrit tout ce qu'on peut manipuler sans modifier la logique du
jeu : les commandes `make`, la console série, les réglages, la carte SD, et
les interfaces du code pour brancher un nouveau matériel (l'afficheur).

## Les commandes `make`

À lancer depuis la racine du dépôt :

| Commande | Ce qu'elle fait |
|---|---|
| `make setup` | une seule fois : installe le support de l'Uno et FastLED (versions figées) |
| `make test` | compile et lance tous les tests sur le PC (aucun matériel nécessaire) |
| `make sim` | joue des parties simulées et produit des pages HTML : ouvrir `build/sim/index.html` |
| `make firmware` | compile pour l'Uno et **échoue** si la RAM ou la flash dépassent le budget |
| `make upload PORT=/dev/ttyACM0` | compile puis téléverse dans l'Arduino |
| `make monitor PORT=/dev/ttyACM0` | ouvre la console série (115200 bauds) |
| `make clean` | efface le dossier `build/` |

Sous Linux, le port est en général `/dev/ttyACM0` (Uno d'origine) ou
`/dev/ttyUSB0` (clones). `arduino-cli board list` affiche les cartes
branchées.

## La console série

Le programme écrit ce qu'il fait sur la liaison USB et accepte des commandes.
On l'ouvre avec `make monitor`, ou avec le moniteur série de l'Arduino IDE
réglé sur **115200 bauds** et fin de ligne **« Nouvelle ligne »**.

### Ce que le programme écrit tout seul

Au démarrage :

```
quiberon_bike ready, free RAM 512
type help for the bench commands
display blank
```

À chaque changement de phase, et chaque fois que l'afficheur change :

```
display 0
phase COUNTDOWN kcal 0 stack headroom 287
phase RIDE kcal 0 stack headroom 287
display 1
display 2
...
phase FINALE kcal 23 stack headroom 287
phase IDLE kcal 23 stack headroom 287
```

`stack headroom` est le nombre d'octets de RAM que la pile n'a jamais
touchés : il doit rester au-dessus de 100 (voir le
[chapitre 4](04-logiciel.md#la-mémoire-la-vraie-contrainte)). Les lignes
`display N` remplacent l'afficheur tant qu'il n'est pas branché. Les nombres
ci-dessus sont des exemples.

### Les commandes

Une commande par ligne, en minuscules ou majuscules. La console répond `ok`,
ou une ligne commençant par `?` en cas d'erreur.

| Commande | Effet | Utile pour |
|---|---|---|
| `help` | liste les commandes | |
| `status` | une ligne d'état (voir ci-dessous) | vérifier une entrée |
| `watch` | `status` deux fois par seconde ; retaper `watch` pour arrêter | étalonner la génératrice |
| `start` | simule un appui sur le bouton | tester sans bouton |
| `phase idle` / `countdown` / `ride` / `finale` | saute directement à une phase | voir un effet sans attendre |
| `speed 0` … `speed 255` | fait comme si le cycliste pédalait à ce niveau | tester l'effet de course sans vélo |
| `speed off` | revient à la vraie génératrice | |
| `play 1` … `play 255` | joue la piste `/mp3/000N` (jusqu'au prochain changement de phase) | vérifier la carte SD et le son |
| `display 42` / `display blank` | écrit directement sur l'afficheur | tester l'afficheur |
| `leds red` / `green` / `blue` / `white` / `off` | toutes les LED d'une couleur (à la luminosité maximale autorisée) | vérifier les guirlandes, mesurer le courant |
| `leds auto` | revient aux effets du jeu | |

La ligne d'état :

```
phase ride t 42 left 137 adc 512 level 142 kcal 9 busy 1 busyok 1
```

| Champ | Signification |
|---|---|
| `phase` | la phase en cours |
| `t` / `left` | secondes écoulées dans la phase / secondes restantes |
| `adc` | mesure brute de A1, 0 à 1023 (0 V à 5 V) |
| `level` | vitesse filtrée, 0 à 255 |
| `kcal` | calories de la partie (non bornées à 99) |
| `busy` | 1 si la broche BUSY dit que le lecteur joue |
| `busyok` | 1 si BUSY a déjà fonctionné au moins une fois depuis l'allumage |
| `[speed injected]`, `[led test]` | rappel qu'une commande `speed` ou `leds` est active |

La [procédure de test au banc](../bench-test.fr.md) utilise ces commandes pour
tout vérifier pas à pas.

## Régler le jeu : `config.h`

Tous les réglages sont dans
[`firmware/quiberon_bike/src/core/config.h`](../../firmware/quiberon_bike/src/core/config.h).
Après une modification : `make test` (les tests vérifient que le jeu reste
cohérent), puis `make upload`.

Les réglages les plus utiles :

| Réglage | Valeur | Effet |
|---|---|---|
| `ADC_REST` | 20 | mesure de A1 vélo à l'arrêt (à mesurer, + une petite marge) |
| `ADC_MAX` | 920 | mesure de A1 au sprint le plus fort (à mesurer) |
| `SPEED_TAU_MS` | 300 | lissage de la vitesse : plus grand = plus doux mais plus lent à réagir |
| `RIDE_MS` | 180000 | durée de la course (3 min) |
| `FINALE_MS` | 30000 | durée du final (30 s) |
| `VOLUME` | 20 | volume du lecteur MP3, 0 à 30 |
| `MAX_BRIGHTNESS` | 180 | luminosité maximale des LED, 0 à 255 (consommation !) |
| `IDLE_BRIGHTNESS` | 60 | luminosité au repos |
| `POWER_WATTS` | 0, 60, 130, 220, 320 | puissance attribuée aux niveaux 0, 64, 128, 192, 255 : règle les calories |
| `IDLE_SHOWS_LAST_SCORE` | true | au repos, garder le dernier score affiché (sinon : éteint) |
| `TRACK_*` | 1 à 4 | numéros des pistes sur la carte SD |
| `NUM_LEDS` | 300 | nombre de LED **par** guirlande |

Les broches (`PIN_*`) sont aussi dans ce fichier. Si vous en changez une,
changez le câblage **et** la documentation.

## La carte SD

Le lecteur trouve les fichiers par leur **numéro** : `/mp3/0001_…mp3` à
`/mp3/0004_…mp3`. Tout est expliqué dans la page [carte SD](../sd-card.fr.md).

| Fichier | Quand |
|---|---|
| `0001_AttentionAuDepart543210.mp3` | compte à rebours : la course commence quand il se termine |
| `0002_WakaWakaShakira.mp3` | pendant la course, en boucle |
| `0003_BravoApplaudissements.mp3` | début du final |
| `0004_Vainqueur.mp3` | après les applaudissements (fichier encore à fournir) |

## Les interfaces du code

Si vous voulez réutiliser ou étendre le programme, voici les points d'entrée.

**`App`** ([`app.h`](../../firmware/quiberon_bike/src/core/app.h)) : tout le jeu.

```cpp
App app(mp3, display, pixels, 300);  // lecteur MP3, afficheur, tableau de pixels
bool nouvelleImage = app.step(in);   // une boucle ; true = afficher les pixels
app.game();                          // accès à la machine à états
app.audio();                         // accès au son
```

**`Game`** ([`game.h`](../../firmware/quiberon_bike/src/core/game.h)) :

```cpp
game.phase();            // PHASE_IDLE, PHASE_COUNTDOWN, PHASE_RIDE ou PHASE_FINALE
game.changed();          // true pendant la boucle où la phase a changé
game.entered(PHASE_RIDE);// true pendant la boucle où on est entré en course
game.elapsedMs();        // temps passé dans la phase
game.remainingMs();      // temps restant
game.speedLevel();       // vitesse filtrée 0..255
game.kcal();             // calories de la partie
game.requestPhase(p);    // forcer une phase (utilisé par la console)
```

**`Audio::playNow(n)`** joue une piste tout de suite (console `play`).

**Les interfaces de `hal.h`** (`Inputs`, `Rgb`, `ByteSink`,
`NumberDisplay`) sont décrites au [chapitre 4](04-logiciel.md#pourquoi-ce-découpage).
Pour ajouter un matériel, on écrit une classe qui hérite de l'interface
correspondante, dans `src/hal/` (côté Arduino) et, si besoin, un faux dans
`test/fakes/` (côté tests).

## Ajouter le vrai afficheur

Le jour où l'afficheur est choisi, il suffit d'écrire une classe qui hérite de
`NumberDisplay` et de la donner à `App` à la place de `SerialDisplay`. Rien
d'autre ne change : `Scoreboard` décide déjà quoi afficher et quand.

Exemple, pour un afficheur à 2 chiffres de 7 segments commandé par deux
registres à décalage chaînés (74HC595, ou TPIC6B595 pour des segments en 12 V),
branchés sur D7 (données), D8 (horloge) et D9 (verrou). **C'est un exemple à
adapter au matériel réel**, pas du code testé :

```cpp
// src/hal/shift_display.h
#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "../core/hal.h"

namespace qb {

// Segments a..g sur les bits 0..6, pour les chiffres 0 à 9 (en flash).
const uint8_t SEGMENTS[10] PROGMEM = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

class ShiftDisplay : public NumberDisplay {
public:
    ShiftDisplay(uint8_t data, uint8_t clock, uint8_t latch)
        : data_(data), clock_(clock), latch_(latch) {}

    void begin() {
        pinMode(data_, OUTPUT);
        pinMode(clock_, OUTPUT);
        pinMode(latch_, OUTPUT);
        blank();
    }
    void showNumber(uint16_t value) override {
        const uint8_t tens = (value / 10) % 10;
        const uint8_t units = value % 10;
        write(value >= 10 ? digit(tens) : 0, digit(units));  // pas de zéro devant
    }
    void blank() override { write(0, 0); }

private:
    static uint8_t digit(uint8_t d) { return pgm_read_byte(&SEGMENTS[d]); }
    void write(uint8_t tens, uint8_t units) {
        digitalWrite(latch_, LOW);
        shiftOut(data_, clock_, MSBFIRST, units);  // le premier octet part au bout de la chaîne
        shiftOut(data_, clock_, MSBFIRST, tens);
        digitalWrite(latch_, HIGH);
    }
    uint8_t data_, clock_, latch_;
};

}  // namespace qb
#endif
```

Puis dans `quiberon_bike.ino` :

```cpp
#include "src/hal/shift_display.h"
// SerialDisplay display;            // l'ancien afficheur provisoire
ShiftDisplay display(7, 8, 9);
...
void setup() {
    ...
    display.begin();
}
```

Règles à respecter pour tout nouvel afficheur :

- **Rapide** : `showNumber()` est appelée depuis la boucle, elle doit durer
  quelques millisecondes au plus (pas de `delay()`).
- **Économe en RAM** : vérifiez avec `make firmware` que le budget tient
  toujours, et au banc que `stack headroom` reste au-dessus de 100.
- **Ajoutez les broches dans `config.h`** et mettez à jour la page
  [câblage](../wiring.fr.md) et le [chapitre 3](03-electronique.md).
