"""Hardware pictures of the French guide (docs/guide/img/)."""
from svgkit import (dfplayer_labels, halo, DIM, INK, Axes, amplifier, arduino_uno, arrow, badge, battery, bike, blade_fuse,
                    breadboard, callout, dfplayer, doc, dot, generator, led_strip, line, lines, mono,
                    poly, potentiometer, push_button, rect, resistor, ring, sd_card, sdot,
                    seven_seg_display, speaker, text, wire)


def card(x, y, w, h, title, colour="#f7f9fb"):
    return rect(x, y, w, h, colour, "#cfd6dd", 1.5, 10) + text(x + 12, y + 22, title, 13, "start", "bold")


# ------------------------------------------------------------ overview

def vue_ensemble():
    W, H = 1240, 820
    p = []
    # cards
    p.append(card(30, 60, 250, 190, "Afficheur des calories"))
    p.append(seven_seg_display(70, 100, "42", 80))
    p.append(text(155, 238, "matériel encore à choisir", 11, "middle", "normal", DIM, True))

    p.append(card(30, 280, 250, 170, "Bouton de départ"))
    sv, _ = push_button(155, 355)
    p.append(sv)

    p.append(card(330, 150, 400, 330, "Arduino Uno : le cerveau"))
    sv, pins = arduino_uno(380, 200, highlight=("D2", "D3", "D5", "D6", "A1"), show_title=False)
    p.append(sv)

    p.append(card(780, 50, 430, 230, "Son"))
    sv, dp = dfplayer(810, 110, 0.8, labels=False)
    p.append(sv)
    p.append(text(856, 230, "lecteur MP3", 11, "middle", "bold"))
    p.append(text(856, 245, "+ carte SD", 11, "middle", "normal", DIM))
    p.append(amplifier(930, 120, 140, 60))
    p.append(text(1000, 200, "ampli", 11, "middle", "bold"))
    p.append(speaker(1110, 95, 70))
    p.append(text(1145, 200, "haut-parleur", 11, "middle", "bold"))
    p.append(wire([(890, 150), (930, 150)], "purple", 3))
    p.append(wire([(1070, 150), (1110, 150)], "black", 3))

    p.append(card(780, 310, 430, 230, "Guirlandes du sapin (2 × 300 LED)"))
    p.append(led_strip(820, 1180, 380, None, True, False))
    p.append(led_strip(820, 1180, 470, None, True, False))
    p.append(text(1000, 515, "même image sur les deux guirlandes", 11, "middle", "normal", DIM, True))

    p.append(card(30, 480, 450, 310, "Vélo + génératrice"))
    p.append(bike(70, 660, 0.85))
    sv, _ = generator(300, 620)
    p.append(sv)
    p.append(text(360, 730, "génératrice", 11, "middle", "bold"))
    p.append(text(360, 746, "tension ∝ vitesse", 11, "middle", "normal", DIM))
    p.append(line(255, 650, 300, 650, "#555", 3))

    p.append(card(530, 560, 230, 230, "Adaptateur de signal"))
    p.append(rect(570, 610, 150, 90, "#2d6a3e", "#1b4026", 1.5, 4))
    p.append(resistor(615, 640, (5, 6, 3), length=46))
    p.append(resistor(615, 675, (1, 0, 3), length=46))
    p.append(text(645, 725, "ramène la tension", 11, "middle", "normal", DIM))
    p.append(text(645, 740, "entre 0 et 5 V", 11, "middle", "normal", DIM))

    p.append(card(800, 570, 410, 220, "Énergie"))
    sv, bp = battery(830, 640, 170, 110)
    p.append(sv)
    p.append(blade_fuse(1060, 670, "F1"))
    p.append(blade_fuse(1110, 670, "F2"))
    p.append(text(1085, 730, "un fusible", 11, "middle", "normal", DIM))
    p.append(text(1085, 745, "par guirlande", 11, "middle", "normal", DIM))

    # links (schematic level: one line per signal)
    def link(points, colour, label=None, at=None, anchor="start"):
        s = wire(points, colour, 3)
        if label:
            lx, ly = at
            s += text(lx, ly, label, 11, anchor, "bold", INK)
        return s

    p.append(link([(280, 160), (330, 160)], "grey", "?", (305, 152), "middle"))
    p.append(link([(280, 365), (330, 365)], "yellow", "D2", (305, 357), "middle"))
    p.append(link([(730, 230), (755, 230), (755, 160), (780, 160)], "blue", "D5 → RX", (690, 222), "start"))
    p.append(link([(730, 260), (765, 260), (765, 190), (780, 190)], "orange", "BUSY → D3", (650, 275), "start"))
    p.append(link([(730, 420), (780, 420)], "green", "D6", (755, 412), "middle"))
    p.append(link([(645, 560), (645, 480)], "brown", "A1", (655, 525), "start"))
    p.append(link([(480, 650), (530, 650)], "brown", "0…30 V", (505, 640), "middle"))
    p.append(link([(1000, 570), (1000, 540)], "red", "12 V", (1010, 560), "start"))
    p.append(link([(800, 680), (770, 680), (770, 295), (1040, 295), (1040, 180)], "red", None))
    p.append(text(1050, 290, "12 V → ampli", 10, "start", "bold", INK))
    p.append(link([(800, 700), (745, 700), (745, 470), (730, 470)], "red", "12 V → Arduino", (690, 545), "start"))
    p.append(text(W / 2, 30, "Vue d'ensemble : qui parle à qui", 18, "middle", "bold"))
    return doc(W, H, p)


# ------------------------------------------------------------ timeline of a game

def deroulement():
    W, H = 1200, 430
    p = [text(W / 2, 30, "Déroulement d'une partie", 18, "middle", "bold")]
    x0, x1 = 120, 1170
    segs = [("REPOS", 0, 110, "#e8eef5"), ("COMPTE À REBOURS", 110, 270, "#fde2c8"),
            ("COURSE : 3 minutes", 270, 740, "#d7f0dc"), ("FINAL : 30 s", 740, 900, "#efe0f7"),
            ("REPOS", 900, 1000, "#e8eef5")]

    def X(u):
        return x0 + (x1 - x0) * u / 1000

    rows = [("Phase", 60), ("Garlande", 120), ("Son", 190), ("Afficheur", 260), ("Cycliste", 330)]
    for name, y in rows:
        p.append(text(x0 - 15, y + 30, name if name != "Garlande" else "Guirlande", 13, "end", "bold"))
    for label, a, b, col in segs:
        p.append(rect(X(a), 60, X(b) - X(a), 50, col, "#aab4bf", 1, 4))
        p.append(text((X(a) + X(b)) / 2, 90, label, 12, "middle", "bold"))
    # garland row
    g = [(0, 110, "comètes lentes", "#fff5f5"), (110, 270, "remplissage en 5 paliers", "#fff3e6"),
         (270, 740, "effet qui grandit avec la vitesse", "#eefaf0"), (740, 900, "feu d'artifice", "#f7efff"),
         (900, 1000, "comètes lentes", "#fff5f5")]
    for a, b, lab, col in g:
        p.append(rect(X(a), 120, X(b) - X(a), 50, col, "#ccd3da", 1, 4))
        p.append(text((X(a) + X(b)) / 2, 150, lab, 11, "middle"))
    s = [(0, 110, "silence"), (110, 270, "piste 1 : « 5-4-3-2-1-0 »"), (270, 740, "piste 2 : musique, en boucle"),
         (740, 815, "piste 3 : bravo"), (815, 900, "piste 4"), (900, 1000, "silence")]
    for a, b, lab in s:
        p.append(rect(X(a), 190, X(b) - X(a), 50, "#f4f6f8", "#ccd3da", 1, 4))
        p.append(text((X(a) + X(b)) / 2, 220, lab, 11, "middle"))
    d = [(0, 110, "vide"), (110, 270, "0"), (270, 740, "les kcal montent en direct"),
         (740, 900, "figé"), (900, 1000, "dernier score")]
    for a, b, lab in d:
        p.append(rect(X(a), 260, X(b) - X(a), 50, "#1a0000", "#000", 1, 4))
        p.append(text((X(a) + X(b)) / 2, 290, lab, 11, "middle", "bold", "#ff6b5e"))
    p.append(rect(X(110) - 34, 330, 68, 50, "#ffecec", "#d62828", 1.5, 6))
    p.append(text(X(110), 352, "appuie", 11, "middle", "bold", "#a11"))
    p.append(text(X(110), 367, "sur départ", 11, "middle", "bold", "#a11"))
    p.append(rect(X(270), 330, X(740) - X(270), 50, "#f4f6f8", "#ccd3da", 1, 4))
    p.append(text((X(270) + X(740)) / 2, 360, "pédale : plus il va vite, plus le sapin s'illumine", 11, "middle"))
    p.append(text(W - 30, 425, "(durées pas à l'échelle)", 10, "end", "normal", DIM, True))
    for u, lab in ((110, "t = 0"), (270, "≈ 7 s (fin de la piste 1)"), (740, "+ 3 min"), (900, "+ 30 s")):
        p.append(line(X(u), 55, X(u), 390, "#888", 1, "4 3"))
        p.append(text(X(u), 410, lab, 11, "middle", "normal", DIM))
    return doc(W, H, p)


# ------------------------------------------------------------ Arduino pins

def arduino_broches():
    W, H = 1100, 560
    p = [text(W / 2, 30, "Les broches de l'Arduino utilisées par le projet", 18, "middle", "bold")]
    sv, pins = arduino_uno(400, 170, highlight=("D2", "D3", "D4", "D5", "D6", "A1", "5V", "GND_P", "GND_D", "VIN"),
                           show_title=False)
    p.append(sv)
    tags = [
        ("D6", 720, 80, "D6 → données des 2 guirlandes (via 330 Ω)", "green"),
        ("D5", 720, 105, "D5 → RX du lecteur MP3 (via pont 1 kΩ / 2,2 kΩ)", "blue"),
        ("D4", 720, 130, "D4 : réservé (TX du MP3, non utilisé)", "grey"),
        ("D3", 720, 155, "D3 ← BUSY du lecteur MP3", "orange"),
        ("D2", 720, 180, "D2 ← bouton de départ (autre borne au GND)", "yellow"),
    ]
    for pin, tx, ty, lab, col in tags:
        px, py = pins[pin]
        p.append(wire([(px, py), (px, ty - 4 if ty < py else py), (tx - 10, ty - 4)], col, 2.5))
        p.append(text(tx, ty, lab, 12))
    gx, gy = pins["GND_D"]
    p.append(wire([(gx, gy), (gx, 70), (300, 70)], "black", 2.5))
    p.append(text(295, 74, "GND commun", 12, "end"))
    bottoms = [("A1", 720, 480, "A1 ← génératrice, via l'adaptateur (0 à 5 V)", "brown"),
               ("VIN", 720, 505, "VIN / jack ← 12 V de la batterie (7 à 12 V conseillé)", "red"),
               ("5V", 720, 530, "5V → alimentation du lecteur MP3", "red"),
               ("GND_P", 720, 455, "GND → masse commune", "black")]
    for pin, tx, ty, lab, col in bottoms:
        px, py = pins[pin]
        p.append(wire([(px, py), (px, ty - 4), (tx - 10, ty - 4)], col, 2.5))
        p.append(text(tx, ty, lab, 12))
    p.append(callout(30, 120, 330, ["USB : téléversement du programme,",
                                    "journal et console de test",
                                    "(115200 bauds)."], "Port USB"))
    p.append(arrow(360, 160, 395, 215, DIM))
    p.append(callout(30, 360, 330, ["Libres pour l'afficheur :",
                                    "D7, D8, D9, D10, D11, D12, D13,",
                                    "A0, A2, A3, A4 (SDA), A5 (SCL).",
                                    "D0/D1 = USB : ne pas les utiliser."], "Broches libres"))
    return doc(W, H, p)


# ------------------------------------------------------------ MP3 module

def lecteur_mp3():
    W, H = 1180, 600
    p = [text(W / 2, 30, "Lecteur MP3 (MP3-TF-16P / DFPlayer Mini) : brochage et câblage", 18, "middle", "bold")]
    sv, ap = arduino_uno(40, 190, highlight=("D3", "D5", "5V", "GND_P"))
    p.append(sv)
    sv, dp = dfplayer(600, 200, 1.6, labels=False)
    p.append(sv)
    # power
    x5, y5 = ap["5V"]
    xg, yg = ap["GND_P"]
    p.append(wire([(x5, y5), (x5, 500), (470, 500), (470, dp["VCC"][1]), dp["VCC"]], "red", 3))
    p.append(wire([(xg, yg), (xg, 520), (490, 520), (490, dp["GND"][1]), dp["GND"]], "black", 3))
    # RX through a 1 kOhm / 2.2 kOhm divider (5 V -> 3.4 V)
    x, y = ap["D5"]
    p.append(wire([(x, y), (x, 120), (420, 120), (420, dp["RX"][1]), dp["RX"]], "blue", 3))
    p.append(resistor(420, 175, (1, 0, 2), vertical=True, length=50, label="1 kΩ"))
    p.append(wire([(440, dp["RX"][1]), (440, 520)], "black", 3))
    p.append(resistor(440, 300, (2, 2, 2), vertical=True, length=50))
    p.append(halo(430, 304, "2,2 kΩ", 12, "end", "bold"))
    p.append(sdot(440, dp["RX"][1], 4))
    p.append(sdot(440, 520, 4))
    # BUSY back to D3
    x, y = ap["D3"]
    bx, by = dp["BUSY"]
    p.append(wire([(x, y), (x, 100), (820, 100), (820, by), (bx, by)], "orange", 3))
    # line out to the amplifier
    p.append(amplifier(900, 380, 170, 70, "ampli audio (12 V)"))
    p.append(speaker(1090, 360, 70))
    for nm, col, yy in (("DAC_R", "purple", 405), ("DAC_L", "grey", 420)):
        px, py = dp[nm]
        p.append(wire([(px, py), (560 if nm == "DAC_R" else 550, py), (560 if nm == "DAC_R" else 550, 560 if nm == "DAC_R" else 553),
                       (860, 560 if nm == "DAC_R" else 553), (860, yy), (900, yy)], col, 2.5))
    gx, gy = dp["GND"]
    p.append(wire([(gx, gy), (540, gy), (540, 575), (880, 575), (880, 435), (900, 435)], "black", 2.5))
    p.append(wire([(1070, 415), (1090, 415)], "black", 3))
    p.append(text(985, 503, "entrée ligne : DAC_R, DAC_L, GND", 11, "middle", "normal", DIM, True))
    p.append(sd_card(1000, 150, "/mp3/0001…0004"))
    p.append(arrow(990, 180, 800, 205, DIM))
    p.append(callout(840, 250, 320, ["BUSY = 0 V pendant la lecture,",
                                     "5 V au repos : l'Arduino sait ainsi",
                                     "quand une piste se termine."], "Pourquoi BUSY ?"))
    p.append(callout(590, 400, 270, ["Le module est en logique 3,3 V :",
                                     "le pont 1 kΩ / 2,2 kΩ ramène le 5 V",
                                     "de D5 à 5 × 2,2 / 3,2 ≈ 3,4 V sur RX."], "Pourquoi deux résistances ?", size=11))
    p.append(dfplayer_labels(dp))
    return doc(W, H, p)


# ------------------------------------------------------------ start button

def bouton():
    W, H = 1160, 470
    p = [text(W / 2, 30, "Bouton de départ : câblage, résistance de rappel interne et rebonds", 18, "middle", "bold")]
    sv, ap = arduino_uno(40, 170, highlight=("D2", "GND_D"))
    p.append(sv)
    sv, bp = push_button(500, 150)
    p.append(sv)
    x2, y2 = ap["D2"]
    xg, yg = ap["GND_D"]
    p.append(wire([(x2, y2), (x2, 245), (486, 245), bp["1"]], "yellow", 3))
    p.append(wire([(xg, yg), (xg, 260), (514, 260), bp["2"]], "black", 3))
    p.append(text(500, 290, "2 fils, longueur libre", 11, "middle", "normal", DIM, True))
    # equivalent schematic
    sx, sy = 640, 80
    p.append(rect(sx, sy, 230, 300, "#fafbfc", "#d5dbe1", 1.2, 8))
    p.append(text(sx + 115, sy + 22, "Ce qu'il y a dans la puce", 12, "middle", "bold"))
    p.append(text(sx + 115, sy + 50, "+5 V", 12, "middle", "bold"))
    p.append(line(sx + 115, sy + 56, sx + 115, sy + 80))
    p.append(rect(sx + 106, sy + 80, 18, 50, "#ffffff", INK, 1.5, 1))
    p.append(text(sx + 132, sy + 110, "≈ 35 kΩ", 11))
    p.append(text(sx + 132, sy + 124, "(pull-up)", 11, "start", "normal", DIM))
    p.append(line(sx + 115, sy + 130, sx + 115, sy + 170))
    p.append(sdot(sx + 115, sy + 160))
    p.append(line(sx + 115, sy + 160, sx + 40, sy + 160))
    p.append(text(sx + 35, sy + 164, "D2", 12, "end", "bold"))
    p.append(line(sx + 115, sy + 170, sx + 115, sy + 190))
    p.append(line(sx + 115, sy + 190, sx + 135, sy + 215))  # open switch
    p.append(line(sx + 115, sy + 222, sx + 115, sy + 250))
    p.append(text(sx + 145, sy + 210, "bouton", 11))
    p.append(line(sx + 100, sy + 250, sx + 130, sy + 250, INK, 2))
    p.append(line(sx + 105, sy + 256, sx + 125, sy + 256, INK, 2))
    p.append(line(sx + 110, sy + 262, sx + 120, sy + 262, INK, 2))
    p.append(text(sx + 115, sy + 285, "relâché : D2 = 5 V, appuyé : D2 = 0 V", 10.5, "middle", "normal", DIM))
    # bounce graph
    gx, gy = 900, 110
    p.append(text(gx + 120, gy - 10, "Ce que voit l'Arduino quand on appuie", 12, "middle", "bold"))
    a = Axes(gx, gy, 240, 140, 0, 60, 0, 5.5)
    p.append(a.frame([0, 30, 60], [0, 5], "temps (ms)", "", fmt=lambda v: str(v)))
    pts = [(0, 5), (8, 5), (8, 0), (9, 0), (9, 5), (10.5, 5), (10.5, 0), (12, 0), (12, 4.8), (12.6, 4.8), (12.6, 0),
           (60, 0)]
    p.append(a.series(pts, "#d62828", 2))
    p.append(line(a.px(12.6), gy + 150, a.px(42.6), gy + 150, "#1f6feb", 2))
    p.append(text(a.px(27), gy + 168, "30 ms stable", 10, "middle", "bold", "#1f6feb"))
    p.append(text(a.px(42.6), gy + 200, "→ 1 seul appui", 11, "middle", "bold", "#1f6feb"))
    p.append(lines(gx - 10, gy + 240, ["Le contact « rebondit » quelques ms :",
                                       "le programme n'accepte un état que s'il",
                                       "dure 30 ms (BUTTON_DEBOUNCE_MS)."], 11, fill=DIM))
    return doc(W, H, p)


# ------------------------------------------------------------ generator input

def adaptateur_generatrice():
    W, H = 1180, 600
    p = [text(W / 2, 30, "Adaptateur de signal de la génératrice (exemple pour 30 V maximum)", 18, "middle", "bold")]
    # schematic
    sx, sy = 60, 90
    p.append(rect(sx - 20, sy - 20, 620, 330, "#fafbfc", "#d5dbe1", 1.2, 8))
    p.append(text(sx, sy + 2, "Schéma", 13, "start", "bold"))
    # generator symbol
    p.append(f'<circle cx="{sx + 50}" cy="{sy + 150}" r="34" fill="#fff" stroke="{INK}" stroke-width="1.8"/>')
    p.append(text(sx + 50, sy + 157, "G", 20, "middle", "bold"))
    p.append(text(sx + 92, sy + 145, "génératrice", 11, "start"))
    p.append(text(sx + 92, sy + 160, "0 à 30 V", 11, "start", "normal", DIM))
    top, bot = sy + 60, sy + 260
    p.append(poly([(sx + 50, sy + 116), (sx + 50, top), (sx + 160, top)]))
    p.append(poly([(sx + 50, sy + 184), (sx + 50, bot), (sx + 560, bot)]))
    p.append(text(sx + 40, top - 6, "+", 14, "end", "bold"))
    # R1
    p.append(rect(sx + 160, top - 9, 70, 18, "#fff", INK, 1.5, 1))
    p.append(text(sx + 195, top - 16, "R1 = 56 kΩ", 12, "middle", "bold"))
    p.append(line(sx + 230, top, sx + 560, top))
    node = sx + 300
    p.append(sdot(node, top))
    # R2 vertical
    p.append(line(node, top, node, top + 60))
    p.append(rect(node - 9, top + 60, 18, 70, "#fff", INK, 1.5, 1))
    p.append(text(node - 16, top + 100, "R2 = 10 kΩ", 12, "end", "bold"))
    p.append(line(node, top + 130, node, bot))
    p.append(sdot(node, bot))
    # C
    cx = node + 90
    p.append(sdot(cx, top))
    p.append(line(cx, top, cx, top + 88))
    p.append(line(cx - 18, top + 88, cx + 18, top + 88, INK, 2.5))
    p.append(line(cx - 18, top + 98, cx + 18, top + 98, INK, 2.5))
    p.append(line(cx, top + 98, cx, bot))
    p.append(sdot(cx, bot))
    p.append(text(cx + 22, top + 97, "C = 100 nF", 12, "start", "bold"))
    # zener
    zx = cx + 120
    p.append(sdot(zx, top))
    p.append(line(zx, top, zx, top + 80))
    p.append(f'<path d="M{zx - 14},{top + 106} L{zx + 14},{top + 106} L{zx},{top + 82} z" fill="#fff" stroke="{INK}" stroke-width="1.5"/>')
    p.append(poly([(zx - 18, top + 76), (zx - 14, top + 82), (zx + 14, top + 82), (zx + 18, top + 88)], INK, 1.8))
    p.append(line(zx, top + 106, zx, bot))
    p.append(sdot(zx, bot))
    p.append(text(zx + 22, top + 97, "Zener 5,1 V", 12, "start", "bold"))
    p.append(text(zx + 22, top + 112, "(protection)", 11, "start", "normal", DIM))
    p.append(line(sx + 560, top, sx + 580, top))
    p.append(f'<circle cx="{sx + 584}" cy="{top}" r="4" fill="#fff" stroke="{INK}" stroke-width="1.5"/>')
    p.append(text(sx + 594, top + 5, "A1", 13, "start", "bold"))
    p.append(f'<circle cx="{sx + 564}" cy="{bot}" r="4" fill="#fff" stroke="{INK}" stroke-width="1.5"/>')
    p.append(text(sx + 574, bot + 5, "GND", 13, "start", "bold"))
    # formula box
    p.append(callout(720, 70, 430, [
        "V(A1) = V(génératrice) × R2 / (R1 + R2)",
        "      = 30 V × 10 / (56 + 10) = 4,5 V",
        "",
        "1. Mesurer la tension maximale Vmax de",
        "   la génératrice (sprint le plus fort).",
        "2. Choisir R2 = 10 kΩ, puis",
        "   R1 ≥ R2 × (Vmax / 4,5 − 1).",
        "3. Vérifier au multimètre : A1 < 5 V.",
        "",
        "C filtre les parasites du moteur.",
        "La Zener écrête au-delà de 5,1 V",
        "(génératrice en survitesse, pic)."], "Comment choisir les valeurs", size=12))
    # graph V(A1) vs speed
    a = Axes(130, 440, 420, 110, 0, 40, 0, 6)
    p.append(a.frame([0, 10, 20, 30, 40], [0, 4.5, 5.1], "tension de la génératrice (V)", "A1 (V)",
                     fmt=lambda v: str(v).replace(".", ",")))
    p.append(a.series([(0, 0), (30, 4.545), (33.6, 5.1), (40, 5.2)], "#2f6fd6", 2.5))
    p.append(a.series([(0, 4.5), (40, 4.5)], "#2a9d3a", 1.2, "5 4"))
    p.append(text(a.px(40) + 6, a.py(5.15), "écrêtage Zener", 11, "start", "normal", "#2f6fd6"))
    p.append(text(a.px(40) + 6, a.py(4.4) + 8, "ADC_MAX ≈ 4,5 V (920)", 11, "start", "normal", "#2a9d3a"))
    # perfboard view
    bx, by = 860, 430
    p.append(rect(bx, by, 250, 130, "#2d6a3e", "#1b4026", 1.5, 6))
    for i in range(20):
        for j in range(9):
            p.append(f'<circle cx="{bx + 14 + i * 12}" cy="{by + 14 + j * 12.8}" r="2" fill="#c49a3b"/>')
    p.append(resistor(bx + 70, by + 40, (5, 6, 3), length=60, label=None))
    p.append(resistor(bx + 150, by + 75, (1, 0, 3), vertical=True, length=50))
    p.append(rect(bx + 190, by + 58, 22, 14, "#e8c56a", "#9b7d2a", 1, 6))
    p.append(f'<rect x="{bx + 222}" y="{by + 66}" width="20" height="9" rx="4" fill="#d07a2a" stroke="#6b3a10"/>')
    p.append(rect(bx + 236, by + 66, 3, 9, "#111", "#111", 0, 0))
    p.append(text(bx + 125, by + 150, "réalisation sur plaque à pastilles", 11, "middle", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ WS2815 anatomy and protocol

def ws2815():
    W, H = 1180, 600
    p = [text(W / 2, 30, "Le ruban WS2815 : comment il est fait et comment il reçoit les couleurs", 18, "middle", "bold")]
    # close-up of 3 pixels
    x0, y0 = 60, 80
    p.append(rect(x0, y0, 640, 150, "#1b1b1b", "#000", 1.5, 6))
    for k in range(3):
        cx = x0 + 110 + k * 200
        p.append(rect(cx - 30, y0 + 45, 60, 60, "#f7f7f2", "#9a9a9a", 1.5, 3))
        p.append(f'<circle cx="{cx}" cy="{y0 + 75}" r="22" fill="#fbe7a1" stroke="#d9c37a"/>')
        p.append(rect(cx - 6, y0 + 69, 12, 12, "#333", "#333", 0, 1))
        p.append(text(cx, y0 + 130, f"LED {k}", 11, "middle", "bold", "#ddd"))
        p.append(rect(cx + 48, y0 + 60, 18, 10, "#555", "#777", 1, 1))
        p.append(rect(cx + 48, y0 + 82, 18, 10, "#555", "#777", 1, 1))
        if k < 2:
            p.append(line(cx + 90, y0 + 6, cx + 90, y0 + 144, "#d8b04a", 1.5, "6 4"))
            p.append(text(cx + 90, y0 + 20, "✂", 14, "middle", "normal", "#d8b04a"))
    for i, (nm, col) in enumerate((("12V", "#d62828"), ("DI", "#2a9d3a"), ("BI", "#2f6fd6"), ("GND", "#f2f2f2"))):
        yy = y0 + 22 + i * 36
        p.append(rect(x0 + 8, yy - 8, 30, 16, "#c49a3b", "#7a5f16", 1, 2))
        p.append(text(x0 + 46, yy + 4, nm, 11, "start", "bold", col))
    p.append(arrow(x0 + 360, y0 + 160, x0 + 520, y0 + 160, "#2a9d3a", 2.5))
    p.append(text(x0 + 440, y0 + 182, "sens des données (flèches imprimées)", 11, "middle", "bold", "#2a9d3a"))
    p.append(callout(740, 70, 410, [
        "• 12 V : chaque LED consomme ~15 mA en",
        "  blanc maximal (3 couleurs en série).",
        "• DI : entrée des données. Chaque LED garde",
        "  ses 24 bits et passe la suite à la suivante.",
        "• BI : entrée de secours. Si une LED meurt,",
        "  la suivante lit BI et la guirlande continue.",
        "  Sur la 1re LED, BI est relié au GND.",
        "• On peut couper entre deux LED (traits ✂)."], "Les 4 connexions", size=12))
    # protocol timing
    a_y = 330
    p.append(text(60, a_y, "Un bit = 1,25 µs. Le niveau haut court = 0, le niveau haut long = 1 :", 13, "start", "bold"))
    bx = 80
    u = 140  # px per 1.25 us

    def bit(x, one):
        hi = 0.6 if one else 0.24
        return [(x, a_y + 90), (x, a_y + 30), (x + u * hi, a_y + 30), (x + u * hi, a_y + 90), (x + u, a_y + 90)]
    pts = []
    for i, b in enumerate((0, 1, 1, 0)):
        pts += bit(bx + i * u, b)
    p.append(poly(pts, "#2a9d3a", 2.5))
    for i, b in enumerate((0, 1, 1, 0)):
        p.append(text(bx + i * u + u / 2, a_y + 115, f"« {b} »", 13, "middle", "bold"))
    p.append(text(bx + 0.12 * u, a_y + 22, "≈0,3 µs", 10, "middle", "normal", DIM))
    p.append(text(bx + u + 0.3 * u, a_y + 22, "≈0,75 µs", 10, "middle", "normal", DIM))
    p.append(line(bx + 4 * u, a_y + 90, bx + 4 * u + 90, a_y + 90, "#2a9d3a", 2.5))
    p.append(text(bx + 4 * u + 45, a_y + 80, "> 280 µs", 11, "middle", "bold"))
    p.append(text(bx + 4 * u + 45, a_y + 115, "= fin d'image", 11, "middle", "normal", DIM))
    # bytes order
    yb = a_y + 160
    p.append(text(60, yb, "Pour chaque LED : 24 bits, vert puis rouge puis bleu (ordre « GRB »).", 13, "start", "bold"))
    cols = [("G7…G0", "#2a9d3a"), ("R7…R0", "#d62828"), ("B7…B0", "#2f6fd6")]
    for i, (lab, col) in enumerate(cols):
        p.append(rect(80 + i * 170, yb + 15, 165, 34, col, col, 0, 4))
        p.append(text(80 + i * 170 + 82, yb + 37, lab, 13, "middle", "bold", "#fff"))
    p.append(text(600, yb + 37, "× 300 LED = 7200 bits ≈ 9 ms par image", 13, "start", "bold"))
    p.append(text(600, yb + 57, "pendant ces 9 ms l'Arduino ne fait rien d'autre", 11, "start", "normal", DIM, True))
    return doc(W, H, p)


# ------------------------------------------------------------ power

def alimentation():
    W, H = 1180, 560
    p = [text(W / 2, 30, "Alimentation : qui consomme quoi", 18, "middle", "bold")]
    sv, bp = battery(60, 200, 210, 140)
    p.append(sv)
    p.append(rect(340, 120, 190, 300, "#d6d9dc", "#8e959b", 2, 6))
    p.append(text(435, 145, "distribution", 12, "middle", "bold"))
    p.append(blade_fuse(390, 200, "F1"))
    p.append(blade_fuse(480, 200, "F2"))
    p.append(blade_fuse(390, 300, "F3"))
    p.append(blade_fuse(480, 300, "F4"))
    p.append(wire([bp["+"], (101, 170), (330, 170), (330, 250), (340, 250)], "red", 5))
    p.append(wire([bp["-"], (229, 160), (300, 160), (300, 400), (340, 400)], "black", 5))
    p.append(rect(350, 385, 170, 20, "#3a3a3a", "#000", 1, 2))
    p.append(text(435, 399, "GND commun", 11, "middle", "bold", "#fff"))
    loads = [(200, "F1 → guirlande 1", "jusqu'à 4,5 A (blanc max), souvent < 1 A", "green"),
             (240, "F2 → guirlande 2", "idem", "green"),
             (300, "F3 → ampli audio", "selon l'ampli (voir sa notice)", "purple"),
             (340, "F4 → Arduino (jack ou VIN)", "≈ 0,1 A, Arduino + lecteur MP3", "blue")]
    for y, a, b, col in loads:
        p.append(wire([(530, y), (620, y)], "red", 3))
        p.append(text(630, y - 2, a, 13, "start", "bold"))
        p.append(text(630, y + 14, b, 11, "start", "normal", DIM))
    p.append(callout(60, 440, 1060, [
        "Pire cas des guirlandes : 600 LED × 15 mA = 9 A sous 12 V (≈ 110 W). Le programme plafonne la luminosité "
        "(MAX_BRIGHTNESS = 180/255), soit ≈ 6,4 A au maximum.",
        "En jeu réel, les effets allument une partie des LED à mi-intensité : quelques ampères au plus. "
        "Fils : 1,5 mm² pour les guirlandes, fusible au plus près de la batterie."], "Bilan", size=12))
    p.append(callout(800, 60, 350, ["Dans le schéma de raccordement,",
                                    "la génératrice recharge aussi la",
                                    "batterie à travers une diode",
                                    "anti-retour (hors programme)."], "Recharge", size=11))
    return doc(W, H, p)


# ------------------------------------------------------------ bench setup on a breadboard

def banc_essai():
    W, H = 1220, 750
    p = [text(W / 2, 30, "Montage d'essai sur table (sans vélo ni guirlandes complètes)", 18, "middle", "bold")]
    sv, ap = arduino_uno(40, 330, highlight=("D2", "D3", "D5", "D6", "A1", "5V", "GND_P"))
    p.append(sv)
    sv, hole = breadboard(440, 330, 40, None)
    p.append(sv)
    p.append(text(40, 650, "plaque d'essai : les 5 trous d'une même colonne (a-e, f-j) sont reliés,", 11, "start", "normal", DIM, True))
    p.append(text(40, 664, "les lignes rouge et bleue du bas sont les rails + (5 V) et − (GND)", 11, "start", "normal", DIM, True))

    def hx(c):
        return hole(c, "a")[0]

    # Arduino 5 V / GND to the bottom rails
    x5, y5 = ap["5V"]
    xg, yg = ap["GND_P"]
    p.append(wire([(x5, y5), (x5, 590), (420, 590), (420, hole(0, "+b")[1]), hole(0, "+b")], "red", 3))
    p.append(wire([(xg, yg), (xg, 600), (430, 600), (430, hole(0, "-b")[1]), hole(1, "-b")], "black", 3))

    # trimmer potentiometer on the bottom half, legs in row h, cols 4, 6, 8
    tx0, ty0 = hx(4) - 10, hole(4, "f")[1] - 8
    p.append(rect(tx0, ty0, hx(8) - hx(4) + 20, 26, "#2f6fd6", "#163c80", 1.5, 3))
    p.append(f'<circle cx="{hx(6)}" cy="{ty0 + 13}" r="8" fill="#e8e8e8" stroke="#555"/>')
    p.append(line(hx(6) - 5, ty0 + 13, hx(6) + 5, ty0 + 13, "#555", 2))
    for c in (4, 6, 8):
        p.append(line(hx(c), ty0 + 26, hx(c), hole(c, "h")[1], "#9aa0a6", 2.5))
    p.append(wire([hole(4, "j"), (hx(4), hole(4, "+b")[1])], "red", 2.5))
    p.append(wire([hole(8, "j"), (hx(8), hole(8, "-b")[1])], "black", 2.5))
    xa, ya = ap["A1"]
    p.append(wire([hole(6, "j"), (hx(6), 615), (xa, 615), (xa, ya)], "brown", 2.5))
    p.append(halo(hx(6) + 12, 552, "potentiomètre 10 kΩ", 12, "start", "bold"))
    p.append(halo(hx(6) + 12, 566, "= la génératrice", 11, "start", "normal", DIM))

    # push button across the centre gap, cols 14-16
    bx0 = hx(14) - 6
    p.append(rect(bx0, hole(14, "e")[1] - 6, hx(16) - hx(14) + 12, hole(14, "f")[1] - hole(14, "e")[1] + 12, "#333", "#000", 1, 3))
    p.append(f'<circle cx="{hx(15)}" cy="{(hole(14, "e")[1] + hole(14, "f")[1]) / 2}" r="7" fill="#1f1f1f" stroke="#888"/>')
    p.append(wire([hole(16, "j"), (hx(16), hole(16, "-b")[1])], "black", 2.5))
    p.append(halo(hx(15), hole(15, "c")[1] + 4, "bouton", 11, "middle", "bold"))

    # 1 kOhm (cols 22-26) and 330 Ohm (cols 30-34) in row c
    p.append(line(hx(22), hole(22, "c")[1], hx(26), hole(26, "c")[1], "#9aa0a6", 2.5))
    p.append(resistor((hx(22) + hx(26)) / 2, hole(22, "c")[1], (1, 0, 2), length=34))
    p.append(line(hx(30), hole(30, "c")[1], hx(34), hole(34, "c")[1], "#9aa0a6", 2.5))
    p.append(resistor((hx(30) + hx(34)) / 2, hole(30, "c")[1], (3, 3, 1), length=34))
    p.append(halo((hx(22) + hx(26)) / 2, hole(22, "e")[1] + 4, "1 kΩ", 11, "middle", "bold"))
    p.append(halo((hx(30) + hx(34)) / 2, hole(30, "e")[1] + 4, "330 Ω", 11, "middle", "bold"))
    # 2.2 kOhm from the RX column to GND (divider with the 1 kOhm): jumper over the gap, cols 26-30 in row h
    p.append(wire([hole(26, "e"), hole(26, "g")], "blue", 2))
    p.append(line(hx(26), hole(26, "h")[1], hx(30), hole(30, "h")[1], "#9aa0a6", 2.5))
    p.append(resistor((hx(26) + hx(30)) / 2, hole(26, "h")[1], (2, 2, 2), length=34))
    p.append(wire([hole(30, "j"), (hx(30), hole(30, "-b")[1])], "black", 2.5))
    p.append(halo((hx(26) + hx(30)) / 2, hole(26, "j")[1] + 6, "2,2 kΩ", 11, "middle", "bold"))

    # Arduino digital pins to the breadboard
    for pin, level, col, colour in (("D6", 280, 30, "green"), ("D5", 295, 22, "blue"), ("D2", 310, 14, "yellow")):
        x, y = ap[pin]
        p.append(wire([(x, y), (x, level), (hx(col), level), hole(col, "a")], colour, 2.5))

    # MP3 module, off the board, with jumper wires
    sv, dp = dfplayer(1010, 90, 1.05, labels=False)
    p.append(sv)
    p.append(text(1071, 237, "lecteur MP3", 12, "middle", "bold"))
    p.append(wire([hole(26, "a"), (hx(26), 250), (985, 250), (985, dp["RX"][1]), dp["RX"]], "blue", 2.5))
    x, y = ap["D3"]
    p.append(wire([(x, y), (x, 265), (1150, 265), (1150, dp["BUSY"][1]), dp["BUSY"]], "orange", 2.5))
    p.append(wire([dp["VCC"], (995, dp["VCC"][1]), (995, 330), (1000, 330), (1000, 550), (hx(37), 550), hole(37, "+b")], "red", 2.5))
    p.append(wire([dp["GND"], (975, dp["GND"][1]), (975, 240), (970, 240), (970, 535), (hx(38), 535), hole(38, "-b")], "black", 2.5))
    p.append(speaker(1135, 300, 56))
    p.append(wire([dp["SPK_1"], (1003, dp["SPK_1"][1]), (1003, 285), (1120, 285), (1120, 335), (1135, 335)], "grey", 2))
    p.append(wire([dp["SPK_2"], (1008, dp["SPK_2"][1]), (1008, 292), (1115, 292), (1115, 350), (1135, 350)], "black", 2))
    p.append(text(1163, 385, "petit HP ≤ 3 W", 11, "middle", "bold"))

    # short LED strip with its own 12 V supply
    sy = 665
    p.append(led_strip(720, 1040, sy, None, cut=False, pitch=32, sleeve=False))
    p.append(text(880, sy + 32, "10 LED de ruban WS2815", 12, "middle", "bold"))
    p.append(wire([hole(34, "a"), (hx(34), 320), (960, 320), (960, 630), (700, 630), (700, sy - 3), (720, sy - 3)], "green", 2.5))
    p.append(wire([(720, sy + 9), (690, sy + 9), (690, 640), (hx(39), 640), (hx(39), hole(39, "-b")[1])], "black", 2.5))
    p.append(wire([(720, sy + 3), (705, sy + 3), (705, sy + 9)], "blue", 2))
    p.append(rect(1080, 640, 110, 50, "#2b2b2b", "#000", 1.5, 6))
    p.append(text(1135, 662, "alim 12 V", 12, "middle", "bold", "#fff"))
    p.append(text(1135, 678, "1 A suffit", 10, "middle", "normal", "#ccc"))
    p.append(wire([(1080, 655), (1060, 655), (1060, 640), (710, 640), (710, sy - 9), (720, sy - 9)], "red", 2.5))
    p.append(wire([(1080, 675), (1065, 675), (1065, 712), (680, 712), (680, sy + 9), (690, sy + 9)], "black", 2.5))
    p.append(text(680, 735, "le − de l'alim 12 V rejoint le GND de l'Arduino", 11, "start", "normal", DIM, True))

    p.append(callout(40, 60, 400, ["Tout se teste sans le vélo :",
                                   "• le potentiomètre simule la vitesse,",
                                   "• un petit haut-parleur suffit pour le son,",
                                   "• quelques LED suffisent pour les effets.",
                                   "Ne jamais alimenter le ruban par l'Arduino !"], "L'idée", size=12))
    p.append(dfplayer_labels(dp))
    return doc(W, H, p)


FIGURES = {
    "vue-ensemble.svg": vue_ensemble,
    "deroulement-partie.svg": deroulement,
    "arduino-broches.svg": arduino_broches,
    "lecteur-mp3.svg": lecteur_mp3,
    "bouton.svg": bouton,
    "adaptateur-generatrice.svg": adaptateur_generatrice,
    "ws2815.svg": ws2815,
    "alimentation.svg": alimentation,
    "banc-essai.svg": banc_essai,
}
