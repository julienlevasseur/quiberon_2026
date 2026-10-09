"""Software pictures of the French guide (docs/guide/img/).

figures(data) receives the JSON printed by firmware_data.cpp, i.e. values
computed by the real firmware code.
"""
from svgkit import DIM, INK, Axes, arrow, callout, doc, line, lines, mono, poly, rect, text


def box(x, y, w, h, title, rows=(), fill="#ffffff", stroke="#9aa5b1", size=11, bold_title=True):
    out = [rect(x, y, w, h, fill, stroke, 1.5, 8), text(x + w / 2, y + 20, title, 12.5, "middle", "bold" if bold_title else "normal")]
    for i, r in enumerate(rows):
        out.append(text(x + w / 2, y + 38 + i * 15, r, size, "middle", "normal", DIM))
    return "".join(out)


# ------------------------------------------------------------ architecture

def architecture():
    W, H = 1200, 760
    p = [text(W / 2, 30, "Architecture du programme : la logique au centre, le matériel sur les bords", 18, "middle", "bold")]
    # core
    p.append(rect(200, 170, 800, 330, "#eef6ff", "#5b8fd6", 2, 14))
    p.append(text(600, 195, "src/core/ : la logique du jeu (C++11 pur, aucune dépendance Arduino)", 14, "middle", "bold", "#1d4f91"))
    mods = [
        (220, 215, "App", ["assemble tout,", "1 appel par boucle"]),
        (410, 215, "Game", ["les 4 phases", "et les temps"]),
        (600, 215, "Lights", ["les effets", "lumineux"]),
        (790, 215, "Audio", ["quelle piste,", "quand"]),
        (220, 310, "Button", ["anti-rebond"]),
        (410, 310, "SpeedSensor", ["ADC → vitesse", "filtrée 0..255"]),
        (600, 310, "Calories", ["vitesse → W", "→ kcal"]),
        (790, 310, "DFPlayer", ["trames de", "10 octets"]),
        (220, 405, "Scoreboard", ["que montrer", "sur l'afficheur"]),
        (410, 405, "Console", ["commandes", "de banc d'essai"]),
        (600, 405, "config.h", ["tous les", "réglages"]),
    ]
    for x, y, t, r in mods:
        p.append(box(x, y, 180, 80, t, r))
    p.append(rect(790, 405, 180, 80, "#fff8e1", "#d9a400", 1.5, 8))
    p.append(text(880, 425, "hal.h", 12.5, "middle", "bold"))
    p.append(text(880, 443, "les seules « portes »", 11, "middle", "normal", DIM))
    p.append(text(880, 458, "vers le matériel :", 11, "middle", "normal", DIM))
    p.append(text(880, 473, "Inputs, Rgb, ByteSink, NumberDisplay", 8.5, "middle", "normal", DIM))
    # sketch on top
    p.append(rect(330, 60, 540, 80, "#f3f0ff", "#7c5cd6", 2, 12))
    p.append(text(600, 85, "quiberon_bike.ino : le sketch Arduino (70 lignes)", 14, "middle", "bold", "#4b2fa3"))
    p.append(text(600, 107, "setup() : prépare les broches   ·   loop() : lit les entrées → app.step() → FastLED.show()", 11, "middle", "normal", DIM))
    p.append(text(600, 124, "ne contient aucune règle du jeu", 11, "middle", "normal", DIM, True))
    p.append(arrow(600, 140, 600, 168, "#7c5cd6", 2))
    # bottom: two worlds
    p.append(rect(40, 560, 520, 170, "#eaf7ee", "#3a9d5d", 2, 12))
    p.append(text(300, 585, "Sur l'Arduino : src/hal/ + bibliothèque FastLED", 14, "middle", "bold", "#1e6b3a"))
    p.append(lines(60, 612, ["readInputs() : lit millis(), A1 (8 mesures), D2 et D3",
                             "TxSerial : port série « émission seule » sur D5 (9600 bauds)",
                             "SerialDisplay : affiche les kcal sur l'USB (en attendant l'afficheur)",
                             "FastLED : envoie les 900 octets de couleurs sur D6"], 12))
    p.append(text(300, 715, "make firmware  →  make upload", 12, "middle", "bold", "#1e6b3a", family="ui-monospace, Menlo, monospace"))
    p.append(rect(640, 560, 520, 170, "#fff3e8", "#d9822b", 2, 12))
    p.append(text(900, 585, "Sur le PC : test/ (faux matériel)", 14, "middle", "bold", "#9a4f0c"))
    p.append(lines(660, 612, ["Rig : un banc simulé (cycliste, bouton, temps qui passe)",
                              "Mp3Sim : décode les trames, simule la ligne BUSY",
                              "RecordingDisplay : retient tout ce qui a été affiché",
                              "102 tests + simulateur visuel (pages HTML)"], 12))
    p.append(text(900, 715, "make test  ·  make sim", 12, "middle", "bold", "#9a4f0c", family="ui-monospace, Menlo, monospace"))
    p.append(arrow(400, 500, 300, 558, "#3a9d5d", 2))
    p.append(arrow(800, 500, 900, 558, "#d9822b", 2))
    p.append(text(600, 535, "le même code de src/core/ est compilé des deux côtés", 12, "middle", "bold", INK, True))
    return doc(W, H, p)


# ------------------------------------------------------------ state machine

def machine_etats():
    W, H = 1100, 520
    p = [text(W / 2, 30, "La machine à états du jeu (classe Game)", 18, "middle", "bold")]
    st = {"IDLE": (190, 270, "#e8eef5", "REPOS", "IDLE"), "COUNTDOWN": (470, 130, "#fde2c8", "COMPTE À REBOURS", "COUNTDOWN"),
          "RIDE": (800, 270, "#d7f0dc", "COURSE", "RIDE"), "FINALE": (470, 410, "#efe0f7", "FINAL", "FINALE")}
    for k, (x, y, col, fr, en) in st.items():
        p.append(f'<ellipse cx="{x}" cy="{y}" rx="110" ry="48" fill="{col}" stroke="#7d8790" stroke-width="2"/>')
        p.append(text(x, y - 2, fr, 14, "middle", "bold"))
        p.append(mono(x, y + 18, "PHASE_" + en, 11, "middle", DIM))
    p.append(arrow(250, 230, 380, 165, INK, 2))
    p.append(callout(40, 120, 270, ["bouton appuyé (après anti-rebond)", "→ kcal remises à 0, piste 1"], "IDLE → COUNTDOWN", size=11))
    p.append(arrow(570, 160, 730, 228, INK, 2))
    p.append(callout(640, 60, 420, ["fin de la piste 1 (BUSY repasse au repos)",
                                    "ou, au plus tard, 10 s (COUNTDOWN_TIMEOUT_MS)",
                                    "→ comptage des kcal, musique en boucle"], "COUNTDOWN → RIDE", size=11))
    p.append(arrow(730, 315, 570, 385, INK, 2))
    p.append(callout(760, 360, 300, ["3 minutes écoulées (RIDE_MS)",
                                     "→ kcal figées, applaudissements"], "RIDE → FINALE", size=11))
    p.append(arrow(380, 380, 250, 315, INK, 2))
    p.append(callout(40, 380, 270, ["30 s écoulées (FINALE_MS)", "→ silence, retour aux comètes"], "FINALE → IDLE", size=11))
    p.append(text(W / 2, 498, "Un appui sur le bouton pendant une partie est ignoré. La console de banc peut forcer une phase (« phase ride »…).",
                  12, "middle", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ one loop iteration

def boucle():
    W, H = 1200, 460
    p = [text(W / 2, 30, "Une boucle loop(), vue au microscope (toutes les ~20 ms une image LED)", 18, "middle", "bold")]
    x0, sc = 60, 48  # px per ms
    y = 90

    def seg(t0, t1, row, col, lab, sub=None):
        out = [rect(x0 + t0 * sc, y + row * 90, (t1 - t0) * sc, 46, col, "#7d8790", 1.2, 4),
               text(x0 + (t0 + t1) / 2 * sc, y + row * 90 + 21, lab, 11.5, "middle", "bold")]
        if sub:
            out.append(text(x0 + (t0 + t1) / 2 * sc, y + row * 90 + 37, sub, 10, "middle", "normal", DIM))
        return "".join(out)
    p.append(text(x0, y - 12, "Boucle typique avec une image LED à envoyer :", 12, "start", "bold"))
    p.append(seg(0, 1.0, 0, "#e6f0ff", "lecture", None))
    p.append(seg(1.0, 1.5, 0, "#fff3d6", "jeu", None))
    p.append(seg(1.5, 3.0, 0, "#fde2c8", "effet LED", "calcul 300 px"))
    p.append(seg(3.0, 12.0, 0, "#d7f0dc", "FastLED.show()", "9 ms, interruptions coupées"))
    p.append(text(x0 + 12.3 * sc, y + 28, "→ recommence", 11, "start", "normal", DIM))
    p.append(text(x0, y + 78, "Boucle où le jeu envoie aussi une commande au lecteur MP3 :", 12, "start", "bold"))
    p.append(seg(0, 1.0, 1, "#e6f0ff", "lecture", None))
    p.append(seg(1.0, 11.4, 1, "#f3e6ff", "trame MP3 : 10 octets à 9600 bauds", "≈ 10,4 ms"))
    p.append(seg(11.4, 12.9, 1, "#fde2c8", "effet LED", None))
    p.append(seg(12.9, 21.9, 1, "#d7f0dc", "FastLED.show()", "9 ms"))
    a = 0
    for t in range(0, 23, 2):
        p.append(line(x0 + t * sc, 290, x0 + t * sc, 296, "#888", 1))
        p.append(text(x0 + t * sc, 310, f"{t} ms", 10, "middle", "normal", DIM))
        a += 1
    p.append(line(x0, 290, x0 + 22 * sc, 290, "#888", 1))
    p.append(callout(60, 330, 1080, [
        "Règle d'or : rien ne bloque. Pas de delay(), pas d'attente active. Chaque module regarde l'heure (millis()) et décide",
        "s'il a quelque chose à faire maintenant. Au pire, une boucle dure ~22 ms : le bouton (anti-rebond 30 ms), la vitesse",
        "et le son restent réactifs. Les tests vérifient qu'une boucle lente donne exactement la même partie qu'une boucle rapide."],
        "Pourquoi c'est important", size=12))
    return doc(W, H, p)


# ------------------------------------------------------------ memory

def memoire():
    W, H = 1100, 350
    p = [text(W / 2, 30, "Où va la mémoire de l'Arduino Uno", 18, "middle", "bold")]
    x0, w = 80, 940
    y = 90
    p.append(text(x0, y - 10, "RAM : 2048 octets en tout (c'est très peu !)", 13, "start", "bold"))
    total = 2048
    parts = [(900, "#ff8a65", "couleurs des 300 LED", "900 o"), (619, "#90caf9", "autres variables", "619 o"),
             (529, "#e0e0e0", "libre pour la pile", "529 o")]
    xx = x0
    for n, col, lab, v in parts:
        ww = w * n / total
        p.append(rect(xx, y, ww, 56, col, "#666", 1.2, 0))
        p.append(text(xx + ww / 2, y + 25, lab, 12, "middle", "bold"))
        p.append(text(xx + ww / 2, y + 43, v, 11, "middle", "normal", INK))
        xx += ww
    lim = x0 + w * 1536 / total
    p.append(line(lim, y - 6, lim, y + 70, "#d62828", 2, "5 4"))
    p.append(text(lim, y + 86, "limite imposée par make firmware : 1536 o", 11, "middle", "bold", "#d62828"))
    y2 = 230
    p.append(text(x0, y2 - 10, "Flash (programme) : 32 256 octets", 13, "start", "bold"))
    fl = 16678
    p.append(rect(x0, y2, w * fl / 32256, 40, "#a5d6a7", "#666", 1.2, 0))
    p.append(rect(x0 + w * fl / 32256, y2, w - w * fl / 32256, 40, "#f1f1f1", "#666", 1.2, 0))
    p.append(text(x0 + w * fl / 32256 / 2, y2 + 25, "programme : 16 678 o (52 %)", 12, "middle", "bold"))
    p.append(text(x0 + w * (fl + (32256 - fl) / 2) / 32256, y2 + 25, "libre", 12, "middle", "normal", DIM))
    p.append(lines(x0, y2 + 80, [
        "Les textes de la console sont gardés en flash (F(\"...\"), PSTR) et non en RAM ; le port série « émission seule »",
        "remplace SoftwareSerial (64 o de tampon économisés) ; les tampons USB sont réduits (Makefile). Mesures du 9/10/2026."], 12, fill=DIM))
    return doc(W, H, p)


# ------------------------------------------------------------ DFPlayer frame

def trame_dfplayer():
    W, H = 1150, 470
    p = [text(W / 2, 30, "Une commande pour le lecteur MP3 : « joue la piste 2 »", 18, "middle", "bold")]
    bytes_ = [("7E", "début", "#e0e0e0"), ("FF", "version", "#e0e0e0"), ("06", "longueur", "#e0e0e0"),
              ("12", "commande", "#ffcc80"), ("00", "pas d'accusé", "#e0e0e0"), ("00", "param. haut", "#a5d6a7"),
              ("02", "param. bas", "#a5d6a7"), ("FE", "somme haut", "#90caf9"), ("E7", "somme bas", "#90caf9"),
              ("EF", "fin", "#e0e0e0")]
    x0 = 75
    for i, (b, lab, col) in enumerate(bytes_):
        x = x0 + i * 100
        p.append(rect(x, 70, 90, 56, col, "#666", 1.2, 6))
        p.append(mono(x + 45, 106, b, 22, "middle", INK, "bold"))
        p.append(text(x + 45, 146, lab, 11, "middle", "normal", DIM))
    p.append(lines(75, 185, [
        "0x12 = « joue le fichier /mp3/NNNN », NNNN = paramètre (ici 0002).",
        "Somme de contrôle = 0 − (FF + 06 + 12 + 00 + 00 + 02) = 0 − 0x119 = 0xFEE7 (sur 16 bits).",
        "Autres commandes utilisées : 0x06 volume, 0x16 stop, 0x19 répétition de la piste en cours."], 12))
    # UART byte
    y = 300
    p.append(text(75, y - 12, "Comment part un octet sur le fil (0x7E, 9600 bauds, 104 µs par bit, bit de poids faible d'abord) :", 12, "start", "bold"))
    bits = [0] + [(0x7E >> k) & 1 for k in range(8)] + [1]
    names = ["start"] + [f"b{k}" for k in range(8)] + ["stop"]
    bw = 90
    pts = [(55, y + 20)]
    for i, b in enumerate(bits):
        yy = y + 20 if b else y + 70
        pts += [(75 + i * bw, pts[-1][1]), (75 + i * bw, yy), (75 + (i + 1) * bw, yy)]
    pts.append((75 + 10 * bw + 30, y + 20))
    p.append(poly(pts, "#2f6fd6", 2.5))
    for i, (b, n) in enumerate(zip(bits, names)):
        p.append(text(75 + i * bw + bw / 2, y + 100, n, 11, "middle", "bold"))
        p.append(text(75 + i * bw + bw / 2, y + 116, str(b), 11, "middle", "normal", DIM))
        p.append(line(75 + i * bw, y + 10, 75 + i * bw, y + 80, "#ddd", 1, "3 3"))
    p.append(text(40, y + 24, "5 V", 10, "end", "normal", DIM))
    p.append(text(40, y + 74, "0 V", 10, "end", "normal", DIM))
    return doc(W, H, p)


# ------------------------------------------------------------ MP3 command sequence

def sequence_son():
    W, H = 1180, 520
    p = [text(W / 2, 30, "Ce que l'Arduino envoie au lecteur MP3 pendant une partie", 18, "middle", "bold")]
    ax, mx = 270, 1060
    p.append(rect(ax - 70, 55, 140, 36, "#0e7c86", "#08545b", 1.5, 6))
    p.append(text(ax, 78, "Arduino (Audio)", 12, "middle", "bold", "#fff"))
    p.append(rect(mx - 70, 55, 140, 36, "#18324f", "#0c1a2b", 1.5, 6))
    p.append(text(mx, 78, "lecteur MP3", 12, "middle", "bold", "#fff"))
    p.append(line(ax, 91, ax, 500, "#999", 1.5, "6 4"))
    p.append(line(mx, 91, mx, 500, "#999", 1.5, "6 4"))
    msgs = [("démarrage + 2 s", "volume 20", None),
            ("appui sur départ", "joue 1 (compte à rebours)", "BUSY : lecture… puis repos → début de la course"),
            ("début de la course", "joue 2 (musique)", None),
            ("BUSY confirme la lecture", "répétition ON", "la musique boucle pendant 3 min"),
            ("fin de la course", "répétition OFF", None),
            ("", "joue 3 (applaudissements)", "BUSY : lecture… puis repos"),
            ("fin des applaudissements (ou 12 s)", "joue 4 (vainqueur)", None),
            ("fin du final", "stop", None)]
    y = 125
    for left, cmd, back in msgs:
        p.append(arrow(ax + 4, y, mx - 4, y, "#2f6fd6", 1.8))
        p.append(text((ax + mx) / 2, y - 6, cmd, 12, "middle", "bold", "#1d4f91"))
        if left:
            p.append(text(ax - 12, y + 4, left, 11, "end", "normal", DIM))
        if back:
            p.append(arrow(mx - 4, y + 20, ax + 4, y + 20, "#d9822b", 1.5))
            p.append(text((ax + mx) / 2, y + 34, back, 11, "middle", "normal", "#9a4f0c", True))
            y += 62
        else:
            y += 44
    p.append(text(W / 2, 510, "Flèches bleues : commandes (fil D5 → RX). Flèches orange : ce que dit la ligne BUSY (→ D3).",
                  11, "middle", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ speed filter

def filtre_vitesse(data):
    W, H = 1180, 440
    p = [text(W / 2, 30, "De la tension de la génératrice à la « vitesse » 0..255 (classe SpeedSensor)", 18, "middle", "bold")]
    a = Axes(110, 80, 400, 250, 0, 1023, 0, 255)
    p.append(a.frame([0, 256, 512, 768, 920, 1023], [0, 64, 128, 192, 255], "valeur ADC lue sur A1 (0 = 0 V, 1023 = 5 V)",
                     "niveau de vitesse", fmt=str))
    p.append(a.series([(0, 0), (20, 0), (920, 255), (1023, 255)], "#2f6fd6", 3))
    p.append(text(a.px(20) + 6, a.py(10), "ADC_REST = 20 (bruit au repos)", 10.5, "start", "normal", DIM))
    p.append(text(a.px(330), a.py(235), "ADC_MAX = 920 (≈ 4,5 V)", 10.5, "start", "normal", DIM))
    p.append(text(310, 400, "1. Conversion linéaire, bornée", 13, "middle", "bold"))
    f = data["filter"]
    b = Axes(680, 80, 440, 250, 0, 3000, 0, 255)
    p.append(b.frame([0, 500, 1000, 1500, 2000, 2500, 3000], [0, 64, 128, 192, 255], "temps (ms)", "niveau", fmt=str))
    p.append(b.series([(t, tgt) for t, tgt, _ in f], "#bbbbbb", 2, "6 4"))
    p.append(b.series([(t, lv) for t, _, lv in f], "#d62828", 3))
    p.append(text(b.px(300), b.py(215), "pédalage brut", 11, "start", "normal", "#888"))
    p.append(text(b.px(560), b.py(120), "niveau filtré", 11, "start", "bold", "#d62828"))
    p.append(text(b.px(1560), b.py(225), "le cycliste s'arrête", 11, "start", "normal", DIM))
    p.append(text(900, 400, "2. Lissage (constante de temps 300 ms)", 13, "middle", "bold"))
    p.append(text(900, 418, "courbe calculée par le vrai code du firmware", 11, "middle", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ calories

def calories(data):
    W, H = 1180, 440
    p = [text(W / 2, 30, "Le modèle de calories (classe Calories)", 18, "middle", "bold")]
    wts = data["watts"]
    a = Axes(110, 80, 420, 250, 0, 255, 0, 340)
    p.append(a.frame([0, 64, 128, 192, 255], [0, 60, 130, 220, 320], "niveau de vitesse", "puissance (W)", fmt=str))
    p.append(a.series(list(enumerate(wts)), "#d9822b", 3))
    for lv, w in zip((0, 64, 128, 192, 255), (0, 60, 130, 220, 320)):
        p.append(f'<circle cx="{a.px(lv):.1f}" cy="{a.py(w):.1f}" r="5" fill="#d9822b" stroke="#fff" stroke-width="1.5"/>')
    p.append(text(320, 400, "1. Table POWER_LEVEL → POWER_WATTS (5 points, à ajuster)", 13, "middle", "bold"))
    k = data["kcal"]
    b = Axes(700, 80, 420, 250, 0, 255, 0, 60)
    p.append(b.frame([0, 64, 128, 192, 255], [0, 10, 20, 30, 40, 50, 60], "niveau constant pendant 3 min", "kcal affichées", fmt=str))
    p.append(b.series([(lv, kc) for lv, kc in k], "#2a9d3a", 3))
    p.append(text(910, 400, "2. Score après 3 minutes de course", 13, "middle", "bold"))
    p.append(text(910, 418, "kcal = énergie (J) / 4184 / 0,24 (rendement du corps humain)", 11, "middle", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ light effects

def effets(data):
    eff = {e["name"]: e["px"] for e in data["effects"]}
    rows = [("idle", "REPOS : comètes rouge, vert, or qui montent (une LED toutes les 40 ms)"),
            ("cd1", "COMPTE À REBOURS, 1re seconde : 1/5 de la guirlande, rouge"),
            ("cd3", "3e seconde : 3/5, orange-jaune"),
            ("cd5", "5e seconde : toute la guirlande, presque verte"),
            ("cdgo", "« GO » : flashs verts"),
            ("ride0", "COURSE, cycliste arrêté (niveau 0) : lueur violette + courte zone bleue"),
            ("ride64", "niveau 64 : la zone monte, bleu-cyan, comètes plus rapides"),
            ("ride128", "niveau 128 : la moitié du sapin, vert"),
            ("ride192", "niveau 192 : jaune-vert, paillettes blanches"),
            ("ride255", "niveau 255 (sprint) : tout le sapin, rouge-orange, paillettes"),
            ("fw1", "FINAL : une fusée monte…"),
            ("fw2", "…explose : le bouquet s'ouvre…"),
            ("fw3", "…étincelles qui s'écartent et s'éteignent"),
            ("fw4", "plusieurs bouquets en même temps")]
    W = 1180
    H = 70 + len(rows) * 52 + 40
    p = [text(W / 2, 30, "Les effets lumineux, calculés par le vrai code (LED 0 à gauche = bas du sapin)", 18, "middle", "bold")]
    x0, pw = 40, 3.6
    y = 60
    for name, lab in rows:
        px = eff[name]
        p.append(text(x0, y + 12, lab, 12, "start", "bold" if name in ("idle", "cd1", "ride0", "fw1") else "normal"))
        p.append(rect(x0 - 2, y + 18, 300 * pw + 4, 18, "#05070c", "#05070c", 0, 3))
        for i in range(300):
            r, g, b = (int(px[6 * i + k * 2: 6 * i + k * 2 + 2], 16) for k in range(3))
            if r + g + b == 0:
                continue
            m = max(r, g, b)
            boost = (m / 255) ** 0.45 * 255 / m  # brightened for a screen, hue kept
            r, g, b = (min(255, int(v * boost)) for v in (r, g, b))
            p.append(f'<rect x="{x0 + i * pw:.1f}" y="{y + 20}" width="{pw:.1f}" height="14" fill="rgb({r},{g},{b})"/>')
        y += 52
    p.append(text(W / 2, H - 18, "Luminosité rehaussée pour l'écran ; sur le sapin, le programme plafonne à 180/255.",
                  11, "middle", "normal", DIM, True))
    return doc(W, H, p)


def figures(data):
    return {
        "architecture-logicielle.svg": architecture,
        "machine-etats.svg": machine_etats,
        "boucle.svg": boucle,
        "memoire.svg": memoire,
        "trame-dfplayer.svg": trame_dfplayer,
        "sequence-son.svg": sequence_son,
        "filtre-vitesse.svg": lambda: filtre_vitesse(data),
        "calories.svg": lambda: calories(data),
        "effets-lumineux.svg": lambda: effets(data),
    }
