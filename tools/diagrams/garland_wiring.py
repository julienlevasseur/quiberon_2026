#!/usr/bin/env python3
"""Draw the garland wiring picture used by docs/wiring.md and docs/wiring.fr.md.

Writes docs/img/garland-wiring.svg (English) and docs/img/garland-wiring.fr.svg
(French). Edit the drawing here, then run:  python3 tools/diagrams/garland_wiring.py
"""
from pathlib import Path

W, H = 1140, 780

TEXT = {
    "en": {
        "title": "Garland wiring: both garlands on pin D6",
        "uno": "Arduino Uno",
        "pin6": "pin 6",
        "gnd": "GND",
        "res": "330 Ω resistor (orange, orange, brown)",
        "res_near": "one per garland, close to the Arduino",
        "g1": "Garland 1 · WS2815 · 300 LEDs · 5 m",
        "g2": "Garland 2 · WS2815 · 300 LEDs · 5 m",
        "dir": "data direction",
        "jst": "JST input connector",
        "bi": "BI tied to GND",
        "inj": "power also injected at the far end",
        "batt1": "12 V battery",
        "batt2": "LiFePO4 12.8 V",
        "fuses": "one fuse per garland",
        "gndbar": "common GND",
        "f1": "F1 garland 1",
        "f2": "F2 garland 2",
        "leg": "Wire colours",
        "l_red": "+12 V",
        "l_blk": "GND (strip side: white)",
        "l_grn": "data DI",
        "l_blu": "backup data BI",
        "gnd_note": "Arduino GND to the common GND",
    },
    "fr": {
        "title": "Raccordement des guirlandes : les deux sur la broche D6",
        "uno": "Arduino Uno",
        "pin6": "broche 6",
        "gnd": "GND",
        "res": "résistance 330 Ω (orange, orange, marron)",
        "res_near": "une par guirlande, près de l'Arduino",
        "g1": "Guirlande 1 · WS2815 · 300 LED · 5 m",
        "g2": "Guirlande 2 · WS2815 · 300 LED · 5 m",
        "dir": "sens des données",
        "jst": "connecteur d'entrée JST",
        "bi": "BI relié au GND",
        "inj": "alimentation aussi injectée à l'autre bout",
        "batt1": "Batterie 12 V",
        "batt2": "LiFePO4 12,8 V",
        "fuses": "un fusible par guirlande",
        "gndbar": "GND commun",
        "f1": "F1 guirlande 1",
        "f2": "F2 guirlande 2",
        "leg": "Couleur des fils",
        "l_red": "+12 V",
        "l_blk": "GND (côté guirlande : blanc)",
        "l_grn": "données DI",
        "l_blu": "données de secours BI",
        "gnd_note": "GND de l'Arduino vers le GND commun",
    },
}

WIRE = {  # colour, outline
    "red": ("#d62828", "#7a1010"),
    "black": ("#2a2a2a", "#000000"),
    "white": ("#f2f2f2", "#8a8a8a"),
    "green": ("#2a9d3a", "#145a1e"),
    "blue": ("#2f6fd6", "#163c80"),
}

C1, C2 = 165, 395           # garland centre lines
STRIP_X0, STRIP_X1 = 650, 1050
JST_X = 598                 # left edge of the JST housing pair


def wire(points, colour, width=4):
    fill, edge = WIRE[colour]
    d = " ".join(f"{x},{y}" for x, y in points)
    return (f'<polyline points="{d}" fill="none" stroke="{edge}" stroke-width="{width + 2.5}" '
            f'stroke-linejoin="round" stroke-linecap="round"/>'
            f'<polyline points="{d}" fill="none" stroke="{fill}" stroke-width="{width}" '
            f'stroke-linejoin="round" stroke-linecap="round"/>')


def dot(x, y):
    return f'<circle cx="{x}" cy="{y}" r="4.5" fill="#c9c9c9" stroke="#666" stroke-width="1.2"/>'


def text(x, y, s, size=13, anchor="start", weight="normal", fill="#222", italic=False):
    style = ' font-style="italic"' if italic else ""
    return (f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" '
            f'font-weight="{weight}" fill="{fill}"{style}>{s}</text>')


def arduino(t):
    x, y, w, h = 40, 70, 310, 215
    out = [
        f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="10" fill="#0e7c86" stroke="#08545b" stroke-width="2"/>',
        # USB-B and barrel jack
        f'<rect x="22" y="100" width="62" height="48" rx="3" fill="#c7cbd0" stroke="#7d848c" stroke-width="2"/>',
        f'<rect x="30" y="110" width="46" height="28" rx="2" fill="#e8ebee" stroke="#9aa1a8"/>',
        f'<rect x="24" y="215" width="54" height="44" rx="4" fill="#1c1c1c" stroke="#000"/>',
        f'<circle cx="51" cy="237" r="9" fill="#444"/>',
        # ATmega
        f'<rect x="165" y="205" width="150" height="34" rx="3" fill="#1d1d1d"/>',
        f'<circle cx="173" cy="222" r="3" fill="#3a3a3a"/>',
    ]
    for i in range(14):
        px = 172 + i * 10.5
        out.append(f'<rect x="{px}" y="200" width="4" height="5" fill="#bfbfbf"/>')
        out.append(f'<rect x="{px}" y="239" width="4" height="5" fill="#bfbfbf"/>')
    for hx, hy in ((60, 84), (330, 84), (115, 272), (330, 272)):
        out.append(f'<circle cx="{hx}" cy="{hy}" r="6" fill="#f4f4f4" stroke="#08545b"/>')
    out.append(text(240, 175, "UNO", 30, "middle", "bold", "#ffffff"))
    out.append(text(240, 148, "ARDUINO", 13, "middle", "bold", "#d8f3f5"))
    # digital header (top edge)
    out.append('<rect x="117" y="74" width="122" height="15" fill="#151515"/>')
    out.append('<rect x="243" y="74" width="98" height="15" fill="#151515"/>')
    labels1 = ["SCL", "SDA", "AREF", "GND", "13", "12", "11", "10", "9", "8"]
    labels2 = ["7", "~6", "~5", "4", "~3", "2", "TX1", "RX0"]
    for i, lab in enumerate(labels1):
        out.append(_pin(123 + 12 * i, lab))
    for i, lab in enumerate(labels2):
        out.append(_pin(249 + 12 * i, lab))
    out.append(text(290, 132, "DIGITAL (PWM ~)", 8, "middle", "normal", "#d8f3f5"))
    out.append(text(195, y + h + 22, t["uno"], 14, "middle", "bold"))
    return "".join(out)


def _pin(x, lab):
    s = f'<rect x="{x - 3}" y="78" width="6" height="7" fill="#b08d2c"/>'
    s += (f'<text x="{x}" y="95" font-size="6.5" fill="#ffffff" text-anchor="end" '
          f'transform="rotate(-90 {x} 95)" dy="2.5">{lab}</text>')
    return s


def resistor(x, y):
    """Through-hole resistor centred on wire height y, body from x to x+56."""
    out = [f'<rect x="{x}" y="{y - 9}" width="56" height="18" rx="8" fill="#d9c08f" stroke="#8a7346" stroke-width="1.5"/>']
    for bx, col in ((x + 10, "#f07f13"), (x + 19, "#f07f13"), (x + 28, "#7a4a1d"), (x + 44, "#c9a33a")):
        out.append(f'<rect x="{bx}" y="{y - 9}" width="5" height="18" fill="{col}"/>')
    return "".join(out)


def strip(c, label, t):
    y0 = c - 15
    out = [
        # silicone sleeve, then black PCB
        f'<rect x="{STRIP_X0 - 4}" y="{y0 - 5}" width="{STRIP_X1 - STRIP_X0 + 8}" height="40" rx="9" '
        f'fill="#e9eef2" fill-opacity="0.9" stroke="#b4bcc4" stroke-width="1.5"/>',
        f'<rect x="{STRIP_X0}" y="{y0}" width="{STRIP_X1 - STRIP_X0}" height="30" rx="3" fill="#1b1b1b"/>',
    ]
    gap0, gap1 = 832, 868
    x = STRIP_X0 + 18
    while x < STRIP_X1 - 14:
        if not (gap0 - 14 < x < gap1 + 4):
            out.append(f'<rect x="{x - 8}" y="{c - 8}" width="16" height="16" rx="1.5" fill="#f7f7f2" stroke="#9a9a9a"/>')
            out.append(f'<circle cx="{x}" cy="{c}" r="5" fill="#fbe7a1" stroke="#d9c37a"/>')
            out.append(f'<rect x="{x + 13}" y="{c + 6}" width="6" height="4" fill="#3d3d3d"/>')
            out.append(f'<path d="M{x + 13},{c - 9} l6,3 l-6,3 z" fill="#8f8f8f"/>')
        x += 34
    # cut in the drawing: the strip is 5 m long
    out.append(f'<rect x="{gap0}" y="{y0 - 8}" width="{gap1 - gap0}" height="46" fill="#ffffff"/>')
    for gx in (gap0, gap1):
        out.append(f'<path d="M{gx},{y0 - 6} l-5,10 l5,10 l-5,10 l5,10" fill="none" stroke="#999" stroke-width="1.5"/>')
    out.append(text((gap0 + gap1) / 2, c + 5, "≈", 18, "middle", "normal", "#777"))
    out.append(text(STRIP_X0, y0 - 14, label, 14, "start", "bold"))
    out.append(text(STRIP_X1, y0 - 14, f'{t["dir"]} →', 12, "end", "normal", "#555", True))
    return "".join(out)


def start_end(c, red_x, blk_x, t, show_labels):
    """JST pigtail at the input end, loose power leads at the far end."""
    out = []
    ry, gy, by, wy = c - 9, c - 3, c + 3, c + 9
    # pigtail from the strip pads to the JST pair
    for yy, col in ((ry, "red"), (gy, "green"), (by, "blue"), (wy, "white")):
        out.append(wire([(STRIP_X0 + 4, yy), (JST_X + 30, yy)], col, 3))
    out.append(f'<rect x="{JST_X}" y="{c - 16}" width="16" height="32" rx="3" fill="#f3f3f3" stroke="#8c8c8c" stroke-width="1.5"/>')
    out.append(f'<rect x="{JST_X + 15}" y="{c - 14}" width="16" height="28" rx="3" fill="#ffffff" stroke="#8c8c8c" stroke-width="1.5"/>')
    # field side: data, BI tied to GND, power pair
    out.append(wire([(JST_X, by), (580, by), (580, wy)], "blue", 3))
    out.append(wire([(JST_X, wy), (blk_x, wy), (blk_x, 560 if c == C1 else 575)], "black"))
    out.append(wire([(JST_X, ry), (red_x, ry), (red_x, 525 if c == C1 else 545)], "red"))
    out.append(dot(580, wy))
    if show_labels:
        out.append(f'<line x1="581" y1="{wy + 4}" x2="600" y2="{wy + 34}" stroke="#666" stroke-width="1"/>')
        out.append(text(603, wy + 40, t["bi"], 12, "start", "bold", "#163c80"))
        out.append(text(603, wy + 56, t["jst"], 11, "start", "normal", "#555"))
    # far end: loose red/white power leads
    ex = STRIP_X1
    out.append(wire([(ex - 4, ry), (ex + 22, ry)], "red", 3))
    out.append(wire([(ex - 4, wy), (ex + 22, wy)], "white", 3))
    return "".join(out)


def battery(t):
    out = [
        f'<rect x="40" y="590" width="200" height="130" rx="8" fill="#2b2b2b" stroke="#111" stroke-width="2"/>',
        f'<rect x="40" y="625" width="200" height="50" fill="#f28c28"/>',
        text(140, 647, t["batt1"], 15, "middle", "bold", "#1b1b1b"),
        text(140, 667, t["batt2"], 12, "middle", "normal", "#1b1b1b"),
        # terminals: + left, - right
        f'<rect x="68" y="574" width="26" height="18" rx="3" fill="#c62020" stroke="#7a1010"/>',
        f'<rect x="186" y="574" width="26" height="18" rx="3" fill="#222" stroke="#000"/>',
        text(81, 612, "+", 18, "middle", "bold", "#ffffff"),
        text(199, 612, "−", 18, "middle", "bold", "#ffffff"),
    ]
    return "".join(out)


def fuse_block(t):
    out = [
        f'<rect x="290" y="580" width="200" height="140" rx="6" fill="#d6d9dc" stroke="#8e959b" stroke-width="2"/>',
        # + bus under the fuses
        f'<rect x="300" y="640" width="180" height="10" rx="2" fill="#b87333" stroke="#7b4a1f"/>',
        # GND bar with screws
        f'<rect x="300" y="690" width="180" height="16" rx="2" fill="#3a3a3a"/>',
        text(390, 735, t["gndbar"], 12, "middle", "bold"),
        text(390, 758, t["fuses"], 12, "middle", "normal", "#555", True),
    ]
    for fx, lab in ((325, t["f1"]), (405, t["f2"])):
        out.append(f'<rect x="{fx - 12}" y="598" width="24" height="34" rx="3" fill="#e7b04a" fill-opacity="0.85" stroke="#9b6d16" stroke-width="1.5"/>')
        out.append(f'<rect x="{fx - 6}" y="632" width="4" height="10" fill="#c0c0c0"/>')
        out.append(f'<rect x="{fx + 2}" y="632" width="4" height="10" fill="#c0c0c0"/>')
        out.append(text(fx, 676, lab, 9.5, "middle", "normal", "#333"))
    for sx in range(312, 480, 24):
        out.append(f'<circle cx="{sx}" cy="698" r="4.5" fill="#bdbdbd" stroke="#555"/>')
    return "".join(out)


def legend(t):
    x, y = 40, 345
    out = [f'<rect x="{x}" y="{y}" width="300" height="140" rx="8" fill="#f7f7f7" stroke="#d5d5d5"/>',
           text(x + 14, y + 24, t["leg"], 13, "start", "bold")]
    for i, (col, key) in enumerate((("red", "l_red"), ("black", "l_blk"), ("green", "l_grn"), ("blue", "l_blu"))):
        yy = y + 50 + i * 24
        out.append(wire([(x + 16, yy), (x + 60, yy)], col))
        out.append(text(x + 72, yy + 4, t[key], 12))
    return "".join(out)


def build(lang):
    t = TEXT[lang]
    p = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" '
         f'font-family="Helvetica, Arial, sans-serif">',
         f'<rect width="{W}" height="{H}" rx="12" fill="#ffffff"/>',
         text(W / 2, 30, t["title"], 18, "middle", "bold")]
    p.append(arduino(t))
    p.append(legend(t))
    p.append(strip(C1, t["g1"], t))
    p.append(strip(C2, t["g2"], t))
    p.append(battery(t))
    p.append(fuse_block(t))

    # power: F1 feeds both ends of garland 1, F2 both ends of garland 2
    p.append(wire([(331, 598), (331, 505), (1100, 505), (1100, C1 - 9), (STRIP_X1 + 22, C1 - 9)], "red"))
    p.append(wire([(411, 598), (411, 540), (1085, 540), (1085, C2 - 9), (STRIP_X1 + 22, C2 - 9)], "red"))
    p.append(wire([(319, 598), (319, 525), (548, 525)], "red"))
    p.append(wire([(399, 598), (399, 545), (566, 545)], "red"))
    # GND from the bar to every end of both garlands
    p.append(wire([(456, 698), (510, 698), (510, 560), (554, 560)], "black"))
    p.append(wire([(480, 698), (530, 698), (530, 575), (572, 575)], "black"))
    p.append(wire([(480, 706), (1115, 706), (1115, C1 + 9), (STRIP_X1 + 22, C1 + 9)], "black"))
    p.append(wire([(480, 712), (1070, 712), (1070, C2 + 9), (STRIP_X1 + 22, C2 + 9)], "black"))
    # battery to the block
    p.append(wire([(81, 576), (81, 550), (270, 550), (270, 645), (300, 645)], "red", 5))
    p.append(wire([(199, 576), (199, 560), (258, 560), (258, 702), (300, 702)], "black", 5))
    # Arduino GND to the common GND bar
    p.append(wire([(159, 80), (159, 50), (14, 50), (14, 742), (282, 742), (282, 710), (312, 710), (312, 702)], "black"))
    p.append(text(150, 736, t["gnd_note"], 11, "middle", "normal", "#555", True))

    p.append(start_end(C1, 548, 554, t, True))
    p.append(start_end(C2, 566, 572, t, True))

    # D6 data: split, then one 330 ohm resistor per garland
    gy1, gy2 = C1 - 3, C2 - 3
    p.append(wire([(261, 80), (261, 46), (420, 46), (420, gy2), (444, gy2)], "green"))
    p.append(wire([(420, gy1), (444, gy1)], "green"))
    p.append(wire([(500, gy1), (JST_X, gy1)], "green"))
    p.append(wire([(500, gy2), (JST_X, gy2)], "green"))
    p.append(dot(420, gy1))
    p.append(resistor(444, gy1))
    p.append(resistor(444, gy2))
    p.append(text(472, gy1 - 18, "330 Ω", 12, "middle", "bold"))
    p.append(text(472, gy2 - 18, "330 Ω", 12, "middle", "bold"))
    p.append(text(STRIP_X0, 272, t["res"], 11.5, "start", "normal", "#333"))
    p.append(text(STRIP_X0, 288, t["res_near"], 11.5, "start", "normal", "#555", True))

    # highlight the two Arduino pins in use
    p.append('<circle cx="261" cy="81" r="8" fill="none" stroke="#ffd60a" stroke-width="2.5"/>')
    p.append('<circle cx="159" cy="81" r="8" fill="none" stroke="#ffd60a" stroke-width="2.5"/>')
    p.append(text(270, 64, t["pin6"], 12, "start", "bold"))
    p.append(text(168, 64, t["gnd"], 12, "start", "bold"))

    p.append(text(STRIP_X1 + 10, C2 + 52, t["inj"], 11.5, "end", "normal", "#555", True))
    p.append("</svg>\n")
    return "".join(p)


def main():
    out_dir = Path(__file__).resolve().parents[2] / "docs" / "img"
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "garland-wiring.svg").write_text(build("en"), encoding="utf-8")
    (out_dir / "garland-wiring.fr.svg").write_text(build("fr"), encoding="utf-8")


if __name__ == "__main__":
    main()
