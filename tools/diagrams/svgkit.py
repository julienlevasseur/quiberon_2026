"""Small SVG drawing kit for the project's documentation pictures.

Every function returns an SVG fragment (a string). Components that have pins
also return a dict {pin name: (x, y)} so wires can be routed to them.
Plain Python 3, no dependency.
"""
import math

FONT = "Helvetica, Arial, sans-serif"
INK = "#222222"
DIM = "#555555"

WIRE = {  # colour, outline
    "red": ("#d62828", "#7a1010"),
    "black": ("#2a2a2a", "#000000"),
    "white": ("#f2f2f2", "#8a8a8a"),
    "green": ("#2a9d3a", "#145a1e"),
    "blue": ("#2f6fd6", "#163c80"),
    "yellow": ("#f2c014", "#8a6c00"),
    "orange": ("#f07f13", "#8a4500"),
    "purple": ("#8e44ad", "#4a1f5c"),
    "grey": ("#9aa0a6", "#5f6368"),
    "brown": ("#8b5a2b", "#4a2e12"),
}

# Resistor colour code
BAND = {0: "#111111", 1: "#7a4a1d", 2: "#d62828", 3: "#f07f13", 4: "#f2d21b",
        5: "#2a9d3a", 6: "#2f6fd6", 7: "#8e44ad", 8: "#8a8a8a", 9: "#ffffff"}
GOLD = "#c9a33a"


def esc(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def doc(w, h, body, title=None):
    head = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" '
            f'font-family="{FONT}">'
            f'<rect width="{w}" height="{h}" rx="12" fill="#ffffff"/>')
    if title:
        head += text(w / 2, 30, title, 18, "middle", "bold")
    return head + "".join(body) + "</svg>\n"


def text(x, y, s, size=13, anchor="start", weight="normal", fill=INK, italic=False, rotate=None, family=None):
    extra = ' font-style="italic"' if italic else ""
    if rotate is not None:
        extra += f' transform="rotate({rotate} {x} {y})"'
    if family:
        extra += f' font-family="{family}"'
    return (f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" '
            f'font-weight="{weight}" fill="{fill}"{extra}>{esc(s)}</text>')


def lines(x, y, rows, size=12, anchor="start", weight="normal", fill=INK, italic=False, gap=None):
    gap = gap or size * 1.35
    return "".join(text(x, y + i * gap, r, size, anchor, weight, fill, italic) for i, r in enumerate(rows))


def halo(x, y, s, size=10, anchor="start", weight="normal", fill=INK):
    """Text with a white outline, readable on top of wires."""
    return (f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" font-weight="{weight}" '
            f'fill="{fill}" stroke="#ffffff" stroke-width="3.5" paint-order="stroke">{esc(s)}</text>')


def dfplayer_labels(pins):
    """Pin names of a dfplayer(), to draw after the wires."""
    out = []
    for nm in DFP_PINS_L:
        x, y = pins[nm]
        out.append(halo(x - 10, y + 4, nm, 10, "end"))
    for nm in DFP_PINS_R:
        key = "GND_R" if nm == "GND " else nm
        x, y = pins[key]
        out.append(halo(x + 10, y + 4, nm.strip(), 10, "start"))
    return "".join(out)


def mono(x, y, s, size=12, anchor="start", fill=INK, weight="normal"):
    return text(x, y, s, size, anchor, weight, fill, family="ui-monospace, Menlo, Consolas, monospace")


def rect(x, y, w, h, fill="#ffffff", stroke="#888888", sw=1.5, rx=4, extra=""):
    return f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{extra}/>'


def line(x1, y1, x2, y2, stroke=INK, sw=1.5, dash=None, extra=""):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{stroke}" stroke-width="{sw}"{d}{extra}/>'


def poly(points, stroke=INK, sw=1.5, fill="none", dash=None, join="round"):
    d = " ".join(f"{x:.1f},{y:.1f}" for x, y in points)
    da = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'<polyline points="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" '
            f'stroke-linejoin="{join}" stroke-linecap="round"{da}/>')


def arrow(x1, y1, x2, y2, stroke=INK, sw=1.8, head=9):
    a = math.atan2(y2 - y1, x2 - x1)
    hx1 = x2 - head * math.cos(a - 0.4)
    hy1 = y2 - head * math.sin(a - 0.4)
    hx2 = x2 - head * math.cos(a + 0.4)
    hy2 = y2 - head * math.sin(a + 0.4)
    return (line(x1, y1, x2 - 0.6 * head * math.cos(a), y2 - 0.6 * head * math.sin(a), stroke, sw) +
            f'<path d="M{x2:.1f},{y2:.1f} L{hx1:.1f},{hy1:.1f} L{hx2:.1f},{hy2:.1f} z" fill="{stroke}"/>')


def wire(points, colour, width=4):
    fill, edge = WIRE[colour]
    d = " ".join(f"{x:.1f},{y:.1f}" for x, y in points)
    return (f'<polyline points="{d}" fill="none" stroke="{edge}" stroke-width="{width + 2.5}" '
            f'stroke-linejoin="round" stroke-linecap="round"/>'
            f'<polyline points="{d}" fill="none" stroke="{fill}" stroke-width="{width}" '
            f'stroke-linejoin="round" stroke-linecap="round"/>')


def dot(x, y, r=4.5):
    return f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="#c9c9c9" stroke="#666" stroke-width="1.2"/>'


def sdot(x, y, r=3.2):
    """Schematic junction dot."""
    return f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{INK}"/>'


def ring(x, y, r=8, colour="#ffd60a"):
    return f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="none" stroke="{colour}" stroke-width="2.5"/>'


def callout(x, y, w, rows, title=None, fill="#f7f7f7", stroke="#d5d5d5", size=12):
    h = 14 + (len(rows) + (1 if title else 0)) * size * 1.4
    out = [rect(x, y, w, h, fill, stroke, 1.2, 8)]
    yy = y + 6 + size * 1.25
    if title:
        out.append(text(x + 12, yy, title, size + 1, "start", "bold"))
        yy += size * 1.4
    for r in rows:
        out.append(text(x + 12, yy, r, size))
        yy += size * 1.4
    return "".join(out)


def badge(x, y, n, fill="#1f6feb"):
    return (f'<circle cx="{x}" cy="{y}" r="11" fill="{fill}"/>' + text(x, y + 4.5, n, 12, "middle", "bold", "#fff"))


# --------------------------------------------------------------- components

def resistor(x, y, digits=(3, 3, 1), vertical=False, length=56, label=None):
    """Through-hole resistor body, centred on (x, y). digits: 2 digits + multiplier."""
    out = []
    if vertical:
        out.append(f'<rect x="{x - 9}" y="{y - length / 2}" width="18" height="{length}" rx="8" fill="#d9c08f" stroke="#8a7346" stroke-width="1.5"/>')
        pos = (-length / 2 + 10, -length / 2 + 19, -length / 2 + 28, length / 2 - 12)
        for p, col in zip(pos, [BAND[d] for d in digits] + [GOLD]):
            out.append(f'<rect x="{x - 9}" y="{y + p:.1f}" width="18" height="5" fill="{col}"/>')
        if label:
            out.append(text(x + 16, y + 4, label, 12, "start", "bold"))
    else:
        out.append(f'<rect x="{x - length / 2}" y="{y - 9}" width="{length}" height="18" rx="8" fill="#d9c08f" stroke="#8a7346" stroke-width="1.5"/>')
        pos = (-length / 2 + 10, -length / 2 + 19, -length / 2 + 28, length / 2 - 12)
        for p, col in zip(pos, [BAND[d] for d in digits] + [GOLD]):
            out.append(f'<rect x="{x + p:.1f}" y="{y - 9}" width="5" height="18" fill="{col}"/>')
        if label:
            out.append(text(x, y - 16, label, 12, "middle", "bold"))
    return "".join(out)


UNO_W, UNO_H = 300, 233


def arduino_uno(x, y, highlight=(), title="Arduino Uno", show_title=True):
    """Top view of an Arduino Uno R3, USB on the left. Returns (svg, pins)."""
    w, h = UNO_W, UNO_H
    pins = {}
    out = [
        rect(x, y, w, h, "#0e7c86", "#08545b", 2, 10),
        rect(x - 18, y + 28, 64, 48, "#c7cbd0", "#7d848c", 2, 3),
        rect(x - 10, y + 38, 48, 28, "#e8ebee", "#9aa1a8", 1, 2),
        rect(x - 14, y + 160, 54, 44, "#1c1c1c", "#000", 1, 4),
        f'<circle cx="{x + 13}" cy="{y + 182}" r="9" fill="#444"/>',
        rect(x + 120, y + 140, 150, 34, "#1d1d1d", "#1d1d1d", 1, 3),
        f'<circle cx="{x + 128}" cy="{y + 157}" r="3" fill="#3a3a3a"/>',
        text(x + 195, y + 162, "ATmega328P", 9, "middle", "normal", "#9a9a9a"),
        # reset button and crystal
        rect(x + 58, y + 18, 18, 18, "#d0d0d0", "#777", 1, 2),
        f'<circle cx="{x + 67}" cy="{y + 27}" r="5" fill="#c62020"/>',
        rect(x + 90, y + 112, 22, 10, "#c0c4c8", "#888", 1, 4),
        # on-board LED "L" on pin 13
        rect(x + 140, y + 50, 8, 5, "#f7d046", "#b38f00", 0.8, 1),
        text(x + 152, y + 56, "L", 8, "start", "bold", "#d8f3f5"),
    ]
    for i in range(14):
        px = x + 127 + i * 10.5
        out.append(rect(px, y + 135, 4, 5, "#bfbfbf", "#bfbfbf", 0, 0))
        out.append(rect(px, y + 174, 4, 5, "#bfbfbf", "#bfbfbf", 0, 0))
    for hx, hy in ((x + 15, y + 14), (x + 285, y + 14), (x + 75, y + 220), (x + 285, y + 220)):
        out.append(f'<circle cx="{hx}" cy="{hy}" r="6" fill="#f4f4f4" stroke="#08545b"/>')
    out.append(text(x + 195, y + 112, "UNO", 30, "middle", "bold", "#ffffff"))
    out.append(text(x + 195, y + 84, "ARDUINO", 13, "middle", "bold", "#d8f3f5"))

    def header(x0, y0, names, label_below):
        n = len(names)
        out.append(rect(x0 - 6, y0 - 7, n * 11 + 1, 14, "#151515", "#151515", 0, 1))
        for i, nm in enumerate(names):
            px = x0 + i * 11
            out.append(rect(px - 3, y0 - 3, 6, 6, "#b08d2c", "#b08d2c", 0, 0))
            if label_below:
                out.append(f'<text x="{px}" y="{y0 + 10}" font-size="6.5" fill="#ffffff" text-anchor="end" '
                           f'transform="rotate(-90 {px} {y0 + 10})" dy="2.3">{esc(nm)}</text>')
            else:
                out.append(f'<text x="{px}" y="{y0 - 10}" font-size="6.5" fill="#ffffff" text-anchor="start" '
                           f'transform="rotate(-90 {px} {y0 - 10})" dy="2.3">{esc(nm)}</text>')
            pins[nm] = (px, y0)

    header(x + 88, y + 10, ["SCL", "SDA", "AREF", "GND", "13", "12", "~11", "~10", "~9", "8"], True)
    header(x + 200, y + 10, ["7", "~6", "~5", "4", "~3", "2", "TX→1", "RX←0"], True)
    pins["GND_D"] = pins.pop("GND")
    header(x + 95, y + h - 10, ["NC", "IOREF", "RESET", "3V3", "5V", "GND", "GND ", "VIN"], False)
    pins["GND_P"] = pins.pop("GND")
    pins["GND_P2"] = pins.pop("GND ")
    header(x + 200, y + h - 10, ["A0", "A1", "A2", "A3", "A4", "A5"], False)
    out.append(text(x + 230, y + 46, "DIGITAL (PWM ~)", 8, "middle", "normal", "#d8f3f5"))
    out.append(text(x + 160, y + h - 40, "POWER", 8, "middle", "normal", "#d8f3f5"))
    out.append(text(x + 228, y + h - 40, "ANALOG IN", 8, "middle", "normal", "#d8f3f5"))
    # friendly aliases
    alias = {"D2": "2", "D3": "~3", "D4": "4", "D5": "~5", "D6": "~6", "D7": "7", "D8": "8",
             "D9": "~9", "D10": "~10", "D11": "~11", "D12": "12", "D13": "13", "D0": "RX←0", "D1": "TX→1"}
    for k, v in alias.items():
        pins[k] = pins[v]
    for name in highlight:
        px, py = pins[name]
        out.append(ring(px, py, 7))
    if show_title:
        out.append(text(x + w / 2, y + h + 24, title, 14, "middle", "bold"))
    return "".join(out), pins


DFP_PINS_L = ["VCC", "RX", "TX", "DAC_R", "DAC_L", "SPK_1", "GND", "SPK_2"]
DFP_PINS_R = ["BUSY", "USB-", "USB+", "ADKEY_2", "ADKEY_1", "IO_2", "GND ", "IO_1"]


def dfplayer(x, y, scale=1.0, labels=True):
    """MP3-TF-16P / DFPlayer Mini, top view, SD slot on top. Returns (svg, pins)."""
    s = scale
    w, h = 116 * s, 124 * s
    pins = {}
    out = [rect(x, y, w, h, "#18324f", "#0c1a2b", 2, 4),
           rect(x + 22 * s, y - 12 * s, 72 * s, 50 * s, "#c9ced3", "#6f777e", 1.5, 3),
           rect(x + 30 * s, y - 12 * s, 56 * s, 8 * s, "#e3e6e9", "#9aa1a8", 1, 1),
           text(x + 58 * s, y + 24 * s, "micro SD", 9 * s, "middle", "bold", "#3a4148"),
           rect(x + 36 * s, y + 62 * s, 44 * s, 30 * s, "#111", "#111", 1, 2),
           text(x + 58 * s, y + 110 * s, "MP3-TF-16P", 9 * s, "middle", "bold", "#cfe0f5")]
    for i, nm in enumerate(DFP_PINS_L):
        py = y + (14 + i * 13.5) * s
        out.append(f'<circle cx="{x + 8 * s:.1f}" cy="{py:.1f}" r="{3.6 * s:.1f}" fill="#d8b04a" stroke="#7a5f16"/>')
        pins[nm] = (x + 8 * s, py)
        if labels:
            out.append(text(x - 6 * s, py + 4, nm, 10, "end", "normal", INK))
    for i, nm in enumerate(DFP_PINS_R):
        py = y + (14 + i * 13.5) * s
        out.append(f'<circle cx="{x + w - 8 * s:.1f}" cy="{py:.1f}" r="{3.6 * s:.1f}" fill="#d8b04a" stroke="#7a5f16"/>')
        pins[nm] = (x + w - 8 * s, py)
        if labels:
            out.append(text(x + w + 6 * s, py + 4, nm.strip(), 10, "start", "normal", INK))
    pins["GND_R"] = pins.pop("GND ")
    return "".join(out), pins


def led_strip(x0, x1, c, label=None, cut=True, pads=True, pitch=34, sleeve=True):
    """WS2815 strip, data flowing to the right, centre line c. LED 0 is at x0."""
    y0 = c - 15
    out = []
    if sleeve:
        out.append(f'<rect x="{x0 - 4}" y="{y0 - 5}" width="{x1 - x0 + 8}" height="40" rx="9" '
                   f'fill="#e9eef2" fill-opacity="0.9" stroke="#b4bcc4" stroke-width="1.5"/>')
    out.append(rect(x0, y0, x1 - x0, 30, "#1b1b1b", "#1b1b1b", 0, 3))
    gap0, gap1 = (x0 + x1) / 2 - 18, (x0 + x1) / 2 + 18
    x = x0 + 18
    while x < x1 - 14:
        if not (cut and gap0 - 14 < x < gap1 + 4):
            out.append(rect(x - 8, c - 8, 16, 16, "#f7f7f2", "#9a9a9a", 1, 1.5))
            out.append(f'<circle cx="{x}" cy="{c}" r="5" fill="#fbe7a1" stroke="#d9c37a"/>')
            out.append(rect(x + 13, c + 6, 6, 4, "#3d3d3d", "#3d3d3d", 0, 0))
            out.append(f'<path d="M{x + 13},{c - 9} l6,3 l-6,3 z" fill="#8f8f8f"/>')
        x += pitch
    if cut:
        out.append(f'<rect x="{gap0}" y="{y0 - 8}" width="{gap1 - gap0}" height="46" fill="#ffffff"/>')
        for gx in (gap0, gap1):
            out.append(f'<path d="M{gx},{y0 - 6} l-5,10 l5,10 l-5,10 l5,10" fill="none" stroke="#999" stroke-width="1.5"/>')
        out.append(text((gap0 + gap1) / 2, c + 5, "≈", 18, "middle", "normal", "#777"))
    if label:
        out.append(text(x0, y0 - 14, label, 13, "start", "bold"))
    return "".join(out)


def push_button(x, y, r=26):
    """Big mushroom push button, top view, centred on (x, y). Returns (svg, pins)."""
    out = [f'<circle cx="{x}" cy="{y}" r="{r + 10}" fill="#b9bec4" stroke="#6c737a" stroke-width="2"/>',
           f'<circle cx="{x}" cy="{y}" r="{r}" fill="#d62828" stroke="#7a1010" stroke-width="2"/>',
           f'<circle cx="{x - r / 3}" cy="{y - r / 3}" r="{r / 3}" fill="#ff6b6b" fill-opacity="0.6"/>']
    pins = {"1": (x - 14, y + r + 22), "2": (x + 14, y + r + 22)}
    for px, py in pins.values():
        out.append(rect(px - 4, py - 14, 8, 14, "#d8b04a", "#7a5f16", 1, 1))
    return "".join(out), pins


def battery(x, y, w=200, h=130, l1="Batterie 12 V", l2="LiFePO4 12,8 V"):
    out = [rect(x, y, w, h, "#2b2b2b", "#111", 2, 8),
           rect(x, y + 35, w, 50, "#f28c28", "#f28c28", 0, 0),
           text(x + w / 2, y + 57, l1, 15, "middle", "bold", "#1b1b1b"),
           text(x + w / 2, y + 77, l2, 12, "middle", "normal", "#1b1b1b"),
           rect(x + 28, y - 16, 26, 18, "#c62020", "#7a1010", 1, 3),
           rect(x + w - 54, y - 16, 26, 18, "#222", "#000", 1, 3),
           text(x + 41, y + 22, "+", 18, "middle", "bold", "#ffffff"),
           text(x + w - 41, y + 22, "−", 18, "middle", "bold", "#ffffff")]
    return "".join(out), {"+": (x + 41, y - 14), "-": (x + w - 41, y - 14)}


def blade_fuse(x, y, label=None):
    out = [rect(x - 12, y - 17, 24, 34, "#e7b04a", "#9b6d16", 1.5, 3),
           rect(x - 6, y + 17, 4, 10, "#c0c0c0", "#c0c0c0", 0, 0),
           rect(x + 2, y + 17, 4, 10, "#c0c0c0", "#c0c0c0", 0, 0)]
    if label:
        out.append(text(x, y - 24, label, 11, "middle", "bold"))
    return "".join(out)


def speaker(x, y, size=90, label=None):
    s = size
    out = [rect(x, y, s, s * 1.15, "#e5e5e5", "#8a8a8a", 2, 8),
           f'<circle cx="{x + s / 2}" cy="{y + s * 0.62}" r="{s * 0.36}" fill="#2a2a2a" stroke="#000"/>',
           f'<circle cx="{x + s / 2}" cy="{y + s * 0.62}" r="{s * 0.14}" fill="#555"/>',
           f'<circle cx="{x + s / 2}" cy="{y + s * 0.17}" r="{s * 0.09}" fill="#2a2a2a"/>']
    if label:
        out.append(text(x + s / 2, y + s * 1.15 + 18, label, 12, "middle", "bold"))
    return "".join(out)


def amplifier(x, y, w=170, h=70, label=None):
    out = [rect(x, y, w, h, "#2b2b2b", "#000", 2, 6),
           rect(x + 10, y + 12, 50, 18, "#4fa3ff", "#1a5fa8", 1, 2),
           f'<circle cx="{x + w - 30}" cy="{y + h / 2}" r="16" fill="#c0c0c0" stroke="#555"/>',
           f'<circle cx="{x + w - 72}" cy="{y + h / 2}" r="11" fill="#c0c0c0" stroke="#555"/>',
           text(x + 12, y + h - 12, "AMP", 11, "start", "bold", "#dddddd")]
    if label:
        out.append(text(x + w / 2, y + h + 18, label, 12, "middle", "bold"))
    return "".join(out)


def generator(x, y, label=None):
    """DC generator (motor body seen from the side)."""
    out = [rect(x, y, 120, 70, "#c9ced3", "#6f777e", 2, 14),
           rect(x + 120, y + 18, 16, 34, "#9aa1a8", "#6f777e", 1.5, 3),
           rect(x + 136, y + 31, 30, 8, "#7d848c", "#555", 1, 2),
           rect(x - 10, y + 12, 12, 46, "#2b2b2b", "#000", 1, 3)]
    for i in range(5):
        out.append(line(x + 20 + i * 18, y + 6, x + 20 + i * 18, y + 64, "#aab0b6", 1.2))
    if label:
        out.append(text(x + 60, y + 92, label, 12, "middle", "bold"))
    return "".join(out), {"+": (x - 4, y + 22), "-": (x - 4, y + 48)}


def seven_seg_display(x, y, digits="42", h=90, label=None):
    """Big 2-digit 7-segment display, lit segments for `digits`."""
    seg_map = {"0": "abcdef", "1": "bc", "2": "abged", "3": "abgcd", "4": "fgbc", "5": "afgcd",
               "6": "afgedc", "7": "abc", "8": "abcdefg", "9": "abcdfg", " ": ""}
    dw = h * 0.55
    out = [rect(x, y, len(digits) * (dw + 18) + 18, h + 24, "#151515", "#000", 2, 6)]
    t = h * 0.09
    for k, ch in enumerate(digits):
        ox = x + 18 + k * (dw + 18)
        oy = y + 12
        segs = {"a": (ox + t, oy, dw - 2 * t, t), "g": (ox + t, oy + h / 2 - t / 2, dw - 2 * t, t),
                "d": (ox + t, oy + h - t, dw - 2 * t, t), "f": (ox, oy + t, t, h / 2 - 1.5 * t),
                "b": (ox + dw - t, oy + t, t, h / 2 - 1.5 * t), "e": (ox, oy + h / 2 + t / 2, t, h / 2 - 1.5 * t),
                "c": (ox + dw - t, oy + h / 2 + t / 2, t, h / 2 - 1.5 * t)}
        for name, (sx, sy, sw_, sh) in segs.items():
            on = name in seg_map.get(ch, "")
            out.append(rect(sx, sy, sw_, sh, "#ff3b30" if on else "#3a1010", "none", 0, 2))
    if label:
        out.append(text(x + (len(digits) * (dw + 18) + 18) / 2, y + h + 44, label, 12, "middle", "bold"))
    return "".join(out)


def potentiometer(x, y, label=None):
    """Rotary potentiometer seen from the front, 3 pins at the bottom."""
    out = [f'<circle cx="{x}" cy="{y}" r="26" fill="#2f6fd6" stroke="#163c80" stroke-width="2"/>',
           f'<circle cx="{x}" cy="{y}" r="11" fill="#c0c0c0" stroke="#666"/>',
           line(x, y, x + 7, y - 8, "#444", 3)]
    pins = {}
    for i, nm in enumerate(("1", "C", "3")):
        px = x - 16 + i * 16
        out.append(rect(px - 3, y + 24, 6, 18, "#c0c0c0", "#777", 1, 1))
        pins[nm] = (px, y + 42)
    if label:
        out.append(text(x, y - 34, label, 12, "middle", "bold"))
    return "".join(out), pins


def breadboard(x, y, cols=30, label=None):
    """Half breadboard: two 5-hole halves and the +/- rails. Returns (svg, hole(col, row))."""
    pitch = 12
    w = cols * pitch + 30
    h = 196
    out = [rect(x, y, w, h, "#f4f1ea", "#c8c2b4", 1.5, 6),
           line(x + 10, y + 12, x + w - 10, y + 12, "#d62828", 1.2),
           line(x + 10, y + 40, x + w - 10, y + 40, "#1f4fd1", 1.2),
           line(x + 10, y + h - 40, x + w - 10, y + h - 40, "#d62828", 1.2),
           line(x + 10, y + h - 12, x + w - 10, y + h - 12, "#1f4fd1", 1.2),
           rect(x + 8, y + 95, w - 16, 8, "#e6e1d6", "#e6e1d6", 0, 2)]
    rows = {"+t": y + 20, "-t": y + 32, "a": y + 52, "b": y + 62, "c": y + 72, "d": y + 82, "e": y + 92,
            "f": y + 106, "g": y + 116, "h": y + 126, "i": y + 136, "j": y + 146,
            "+b": y + h - 32, "-b": y + h - 20}
    for c in range(cols):
        cx = x + 15 + c * pitch
        for r, ry in rows.items():
            if r in ("+t", "-t", "+b", "-b") and c % 6 == 5:
                continue
            out.append(rect(cx - 2, ry - 2, 4, 4, "#3a3a3a", "#3a3a3a", 0, 0.5))
    if label:
        out.append(text(x + w / 2, y + h + 18, label, 12, "middle", "bold"))

    def hole(col, row):
        return (x + 15 + col * pitch, rows[row])
    return "".join(out), hole


def sd_card(x, y, label=None):
    out = [f'<path d="M{x},{y} h44 l12,12 v54 h-56 z" fill="#2b2b2b" stroke="#000" stroke-width="1.5"/>',
           rect(x + 6, y + 40, 44, 20, "#c62020", "#c62020", 0, 1),
           text(x + 28, y + 54, "SD", 11, "middle", "bold", "#fff")]
    for i in range(6):
        out.append(rect(x + 6 + i * 7, y + 4, 4, 9, "#d8b04a", "#d8b04a", 0, 0))
    if label:
        out.append(text(x + 28, y + 84, label, 11, "middle", "normal", DIM))
    return "".join(out)


def bike(x, y, s=1.0, label=None):
    """Simple side view of a bike on a home trainer."""
    out = []
    r = 46 * s
    for cx in (x + 50 * s, x + 200 * s):
        out.append(f'<circle cx="{cx}" cy="{y}" r="{r}" fill="none" stroke="#2b2b2b" stroke-width="{5 * s}"/>')
        out.append(f'<circle cx="{cx}" cy="{y}" r="{5 * s}" fill="#555"/>')
    fr = "#d62828"
    pts = [(x + 50 * s, y), (x + 105 * s, y - 70 * s), (x + 185 * s, y - 70 * s), (x + 125 * s, y), (x + 50 * s, y)]
    out.append(poly(pts, fr, 6 * s))
    out.append(poly([(x + 105 * s, y - 70 * s), (x + 125 * s, y)], fr, 6 * s))
    out.append(poly([(x + 185 * s, y - 70 * s), (x + 200 * s, y)], fr, 6 * s))
    out.append(poly([(x + 98 * s, y - 70 * s), (x + 92 * s, y - 88 * s)], "#333", 5 * s))
    out.append(rect(x + 76 * s, y - 96 * s, 34 * s, 9 * s, "#222", "#000", 1, 4))
    out.append(poly([(x + 185 * s, y - 70 * s), (x + 190 * s, y - 95 * s), (x + 210 * s, y - 98 * s)], "#333", 5 * s))
    out.append(f'<circle cx="{x + 125 * s}" cy="{y}" r="{13 * s}" fill="#bbb" stroke="#555"/>')
    # trainer stand under the rear wheel
    out.append(poly([(x + 20 * s, y + 60 * s), (x + 50 * s, y), (x + 90 * s, y + 60 * s)], "#555", 5 * s))
    if label:
        out.append(text(x + 125 * s, y + 80 * s, label, 12, "middle", "bold"))
    return "".join(out)


# ------------------------------------------------------------------ graphs

class Axes:
    """Maps data coordinates to a plot box and draws axes, ticks and labels."""

    def __init__(self, x, y, w, h, xmin, xmax, ymin, ymax):
        self.x, self.y, self.w, self.h = x, y, w, h
        self.xmin, self.xmax, self.ymin, self.ymax = xmin, xmax, ymin, ymax

    def px(self, v):
        return self.x + (v - self.xmin) * self.w / (self.xmax - self.xmin)

    def py(self, v):
        return self.y + self.h - (v - self.ymin) * self.h / (self.ymax - self.ymin)

    def frame(self, xticks, yticks, xlabel="", ylabel="", grid=True, fmt=str):
        out = []
        for t in yticks:
            if grid:
                out.append(line(self.x, self.py(t), self.x + self.w, self.py(t), "#e6e6e6", 1))
            out.append(text(self.x - 8, self.py(t) + 4, fmt(t), 11, "end", "normal", DIM))
        for t in xticks:
            if grid:
                out.append(line(self.px(t), self.y, self.px(t), self.y + self.h, "#eeeeee", 1))
            out.append(text(self.px(t), self.y + self.h + 18, fmt(t), 11, "middle", "normal", DIM))
        out.append(line(self.x, self.y + self.h, self.x + self.w, self.y + self.h, "#888", 1.5))
        out.append(line(self.x, self.y, self.x, self.y + self.h, "#888", 1.5))
        if xlabel:
            out.append(text(self.x + self.w / 2, self.y + self.h + 40, xlabel, 12, "middle", "normal", INK))
        if ylabel:
            out.append(text(self.x - 48, self.y + self.h / 2, ylabel, 12, "middle", "normal", INK, rotate=-90))
        return "".join(out)

    def series(self, pts, stroke, sw=2.5, dash=None):
        return poly([(self.px(a), self.py(b)) for a, b in pts], stroke, sw, dash=dash, join="round")
