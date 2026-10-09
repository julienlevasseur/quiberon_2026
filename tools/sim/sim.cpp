// Visual simulator: plays scripted games through the real firmware (qb::App
// on the fake::Rig bench) and writes one self-contained HTML replay per
// scenario, plus an index page and a text summary on stdout.
//
//   sim <out_dir>          (make sim -> build/sim/)
//
// Recording: 20 fps of simulated time. A sample is taken right after the first
// rendered frame at or past each 50 ms tick, so the pixels are a real frame.
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

#include "core/config.h"
#include "core/scoreboard.h"
#include "fakes/sim_rig.h"

using fake::Rig;
using namespace qb;

namespace {

const uint32_t SAMPLE_MS = 50;  // 20 fps
const uint32_t LEAD_IN_MS = 3000;
const uint32_t TAIL_MS = 5000;
const char* const PHASE_NAMES[4] = {"IDLE", "COUNTDOWN", "RIDE", "FINALE"};

// Effort of the cyclist as a function of the time spent in RIDE.
typedef std::function<uint8_t(uint32_t rideMs)> Effort;

struct Scenario {
    const char* name;
    const char* description;
    bool play;        // false: nobody presses start
    uint32_t idleMs;  // only when !play
    Effort effort;
};

struct Sample {
    uint32_t t;
    uint8_t phase, level, track;
    int16_t display;  // -1 blank
    uint16_t kcal;
    uint32_t remaining;
};

struct Recording {
    std::vector<Sample> samples;
    std::vector<uint8_t> rgb;  // 900 bytes per sample
    std::vector<uint32_t> phaseStart;  // time of each phase entry
    std::vector<uint8_t> phaseOf;
    uint32_t endMs = 0;
};

// Plays the scenario on `r`; the rig keeps the sound log and display history.
Recording record(const Scenario& sc, Rig& r) {
    Recording rec;
    uint32_t nextSample = 0;
    rec.phaseStart.push_back(0);
    rec.phaseOf.push_back(PHASE_IDLE);
    r.onFrame = [&](const Rig& rig) {
        if (static_cast<int32_t>(rig.now - nextSample) < 0) return;
        while (static_cast<int32_t>(rig.now - nextSample) >= 0) nextSample += SAMPLE_MS;
        const Game& g = rig.app.game();
        Sample s;
        s.t = rig.now;
        s.phase = g.phase();
        s.level = g.speedLevel();
        s.track = static_cast<uint8_t>(rig.mp3.audible());
        s.display = static_cast<int16_t>(rig.display.current() < 0 ? -1 : rig.display.current());
        s.kcal = g.kcal();
        s.remaining = g.remainingMs();
        rec.samples.push_back(s);
        for (const Rgb& p : rig.pixels) {
            rec.rgb.push_back(p.r);
            rec.rgb.push_back(p.g);
            rec.rgb.push_back(p.b);
        }
    };
    // One loop step, noting phase changes and feeding the ride effort.
    auto step = [&] {
        if (r.phase() == PHASE_RIDE) r.pedal = sc.effort(r.app.game().elapsedMs());
        else r.pedal = 0;  // nobody pedals outside the ride
        r.step();
        if (r.phase() != rec.phaseOf.back()) {
            rec.phaseOf.push_back(r.phase());
            rec.phaseStart.push_back(r.now);
        }
    };
    auto runFor = [&](uint32_t ms) {
        const uint32_t end = r.now + ms;
        while (static_cast<int32_t>(end - r.now) > 0) step();
    };

    if (!sc.play) {
        runFor(sc.idleMs);
    } else {
        runFor(LEAD_IN_MS);
        r.button = true;  // a 150 ms press, as a visitor would
        runFor(150);
        r.button = false;
        // Play until the game is back to IDLE (bounded, in case of a bug).
        const uint32_t start = r.now;
        while ((r.phase() != PHASE_IDLE || r.now - start < 1000) && r.now - start < 400000) step();
        runFor(TAIL_MS);
    }
    rec.endMs = r.now;
    r.onFrame = nullptr;  // the lambda captures locals of this function
    return rec;
}

std::string base64(const std::vector<uint8_t>& in) {
    static const char* A = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((in.size() + 2) / 3 * 4);
    size_t i = 0;
    for (; i + 2 < in.size(); i += 3) {
        const uint32_t v = (in[i] << 16) | (in[i + 1] << 8) | in[i + 2];
        out += A[v >> 18];
        out += A[(v >> 12) & 63];
        out += A[(v >> 6) & 63];
        out += A[v & 63];
    }
    if (i < in.size()) {
        uint32_t v = in[i] << 16;
        if (i + 1 < in.size()) v |= in[i + 1] << 8;
        out += A[v >> 18];
        out += A[(v >> 12) & 63];
        out += i + 1 < in.size() ? A[(v >> 6) & 63] : '=';
        out += '=';
    }
    return out;
}

std::string jsonString(const std::string& s) {
    std::string o = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') o += '\\';
        if (c == '<') { o += "\\u003c"; continue; }  // never close the <script>
        o += c;
    }
    return o + "\"";
}

// The per-sample fields as parallel arrays: compact and quick to index in JS.
std::string dataJson(const Scenario& sc, const Recording& rec, const Rig& sound) {
    std::string j = "{";
    j += "\"name\":" + jsonString(sc.name) + ",\"description\":" + jsonString(sc.description);
    j += ",\"sampleMs\":" + std::to_string(SAMPLE_MS);
    j += ",\"leds\":" + std::to_string(cfg::NUM_LEDS);
    j += ",\"maxBrightness\":" + std::to_string(cfg::MAX_BRIGHTNESS);
    j += ",\"endMs\":" + std::to_string(rec.endMs);
    auto arr = [&](const char* key, const std::function<long(const Sample&)>& f) {
        j += ",\"" + std::string(key) + "\":[";
        for (size_t i = 0; i < rec.samples.size(); ++i) {
            if (i) j += ',';
            j += std::to_string(f(rec.samples[i]));
        }
        j += "]";
    };
    arr("t", [](const Sample& s) { return long(s.t); });
    arr("phase", [](const Sample& s) { return long(s.phase); });
    arr("display", [](const Sample& s) { return long(s.display); });
    arr("level", [](const Sample& s) { return long(s.level); });
    arr("kcal", [](const Sample& s) { return long(s.kcal); });
    arr("track", [](const Sample& s) { return long(s.track); });
    arr("remaining", [](const Sample& s) { return long(s.remaining); });
    j += ",\"phases\":[";
    for (size_t i = 0; i < rec.phaseStart.size(); ++i) {
        if (i) j += ',';
        j += "[" + std::to_string(rec.phaseStart[i]) + "," + std::to_string(rec.phaseOf[i]) + "]";
    }
    j += "],\"events\":[";
    for (size_t i = 0; i < sound.mp3.log.size(); ++i) {
        if (i) j += ',';
        j += "[" + std::to_string(sound.mp3.logMs[i]) + "," + jsonString(sound.mp3.log[i]) + "]";
    }
    j += "]}";
    return j;
}

// The page. DATA and PIXELS are spliced in by writeHtml().
const char* const PAGE_HEAD = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Bike tree replay</title>
<style>
:root{--bg:#0b0f17;--panel:#141a26;--ink:#e8ecf3;--dim:#8a93a6;--accent:#ffcc4d}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font:14px/1.4 system-ui,sans-serif}
header{padding:12px 16px;display:flex;gap:16px;align-items:baseline;flex-wrap:wrap}
header h1{font-size:18px;margin:0}
header a{color:var(--dim)}
.wrap{display:grid;grid-template-columns:minmax(260px,420px) 1fr;gap:16px;padding:0 16px 16px}
@media (max-width:760px){.wrap{grid-template-columns:1fr}}
.panel{background:var(--panel);border-radius:10px;padding:12px}
#tree{width:100%;aspect-ratio:3/4;display:block;background:#05070c;border-radius:8px}
#strip{width:100%;height:28px;display:block;background:#05070c;border-radius:4px;margin-top:6px}
.stats{display:grid;grid-template-columns:auto 1fr;gap:6px 12px;align-items:center}
.lcd{font:bold 72px/1 ui-monospace,monospace;color:#ff3b30;background:#1a0000;padding:6px 14px;border-radius:6px;letter-spacing:6px;text-shadow:0 0 12px #ff3b30;display:inline-block;min-width:2.6ch;text-align:right}
.lbl{color:var(--dim)}
#phase{font-size:22px;font-weight:600}
.gauge{height:14px;background:#05070c;border-radius:7px;overflow:hidden}
.gauge>div{height:100%;background:linear-gradient(90deg,#2b8cff,#3fd47a,#ffcc4d,#ff3b30)}
#timeline{width:100%;height:46px;display:block;cursor:pointer;margin-top:10px}
.controls{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin-top:8px}
button{background:#232c3d;color:var(--ink);border:1px solid #33405a;border-radius:6px;padding:6px 12px;font:inherit;cursor:pointer}
button.on{background:var(--accent);color:#000}
#log{max-height:180px;overflow:auto;font:12px ui-monospace,monospace;color:var(--dim);margin:0;padding:0;list-style:none}
#log li.past{color:var(--ink)}
.note{color:var(--dim);font-size:12px}
.legend span{display:inline-block;width:10px;height:10px;border-radius:2px;margin:0 4px 0 10px;vertical-align:middle}
</style></head><body>
<header><h1 id="title"></h1><span id="desc" class="note"></span><a href="index.html">all scenarios</a></header>
<div class="wrap">
 <div class="panel">
  <canvas id="tree" width="420" height="560"></canvas>
  <div class="note">Strip view, LED 0 (bottom of the tree) on the left:</div>
  <canvas id="strip" width="900" height="28"></canvas>
 </div>
 <div class="panel">
  <div class="stats">
   <span class="lbl">Calories</span><span><span id="lcd" class="lcd"></span> <span class="lbl">kcal (exact: <span id="kcal"></span>)</span></span>
   <span class="lbl">Phase</span><span><span id="phase"></span> <span id="left" class="lbl"></span></span>
   <span class="lbl">Speed</span><span><div class="gauge"><div id="gauge" style="width:0"></div></div><span id="level" class="lbl"></span></span>
   <span class="lbl">Sound</span><span id="track"></span>
   <span class="lbl">Time</span><span id="clock"></span>
  </div>
  <canvas id="timeline" width="1000" height="46"></canvas>
  <div class="legend note">
   <span style="background:#3b4660"></span>idle<span style="background:#ff9f1c"></span>countdown<span style="background:#2fbf71"></span>ride<span style="background:#c03bff"></span>finale<span style="background:#fff;width:2px"></span>MP3 command
  </div>
  <div class="controls">
   <button id="play">Play</button>
   <button data-speed="1">x1</button><button data-speed="4">x4</button><button data-speed="16">x16</button>
   <span class="note">Space: play/pause. Click or drag the timeline to seek.</span>
  </div>
  <h3>MP3 commands</h3>
  <ul id="log"></ul>
  <p class="note">LED values are the firmware's pre-gamma values, capped at 180. For display they are
  scaled by 255/180 and given a mild gamma (exponent 0.8) so that 180 looks bright and dim pixels stay
  visible; the real garland's look also depends on FastLED's colour correction and the LEDs themselves.
  The garland is drawn wound around a cone, LED 0 at the bottom; dots behind the tree are dimmer.</p>
 </div>
</div>
<script>
)HTML";

const char* const PAGE_TAIL = R"HTML(
const TRACKS = {0:"(silence)",1:"Countdown AttentionAuDepart543210",2:"Ride WakaWakaShakira",3:"Applause BravoApplaudissements",4:"Winner (file to come)"};
const PHASES = ["IDLE","COUNTDOWN","RIDE","FINALE"];
const PCOL = ["#3b4660","#ff9f1c","#2fbf71","#c03bff"];
const N = DATA.leds, F = DATA.t.length;
const pix = (() => { const s = atob(PIXELS), a = new Uint8Array(s.length); for (let i = 0; i < s.length; i++) a[i] = s.charCodeAt(i); return a; })();
// Display mapping: 180 -> 255, mild gamma so dim pixels stay visible.
const LUT = new Uint8Array(256);
for (let v = 0; v < 256; v++) LUT[v] = Math.round(255 * Math.pow(Math.min(1, v / DATA.maxBrightness), 0.8));
const $ = id => document.getElementById(id);
$("title").textContent = "Scenario: " + DATA.name;
$("desc").textContent = DATA.description;
document.title = DATA.name + " replay";

// Tree geometry: a spiral of N dots around a cone, bottom to top.
const tree = $("tree"), tc = tree.getContext("2d");
const TURNS = 9, geo = [];
for (let i = 0; i < N; i++) {
  const h = i / (N - 1), a = TURNS * 2 * Math.PI * h, rad = 0.92 * (1 - h) + 0.03;
  geo.push({x: rad * Math.sin(a), y: h, z: Math.cos(a), i});
}
const order = geo.slice().sort((p, q) => p.z - q.z);  // back to front
function fmt(ms) { const s = Math.floor(ms / 1000); return Math.floor(s / 60) + ":" + String(s % 60).padStart(2, "0"); }
function drawTree(f) {
  const W = tree.width, H = tree.height, o = f * N * 3;
  tc.fillStyle = "#05070c"; tc.fillRect(0, 0, W, H);
  tc.fillStyle = "#3a2410"; tc.fillRect(W / 2 - 12, H - 40, 24, 30);  // trunk
  tc.globalCompositeOperation = "lighter";
  for (const p of order) {
    const k = o + p.i * 3, depth = 0.45 + 0.55 * (p.z + 1) / 2;
    const r = LUT[pix[k]] * depth, g = LUT[pix[k + 1]] * depth, b = LUT[pix[k + 2]] * depth;
    const x = W / 2 + p.x * W * 0.45, y = H - 45 - p.y * (H - 70), s = 2 + 1.6 * depth;
    if (r + g + b < 6) { tc.fillStyle = "rgba(60,70,90,0.25)"; tc.fillRect(x - 1, y - 1, 2, 2); continue; }
    tc.fillStyle = `rgba(${r|0},${g|0},${b|0},0.35)`;
    tc.beginPath(); tc.arc(x, y, s * 2.4, 0, 6.2832); tc.fill();  // glow
    tc.fillStyle = `rgb(${r|0},${g|0},${b|0})`;
    tc.beginPath(); tc.arc(x, y, s, 0, 6.2832); tc.fill();
  }
  tc.globalCompositeOperation = "source-over";
}
const strip = $("strip"), sc = strip.getContext("2d");
function drawStrip(f) {
  const W = strip.width, H = strip.height, o = f * N * 3, w = W / N;
  sc.fillStyle = "#05070c"; sc.fillRect(0, 0, W, H);
  for (let i = 0; i < N; i++) {
    const k = o + i * 3;
    sc.fillStyle = `rgb(${LUT[pix[k]]},${LUT[pix[k + 1]]},${LUT[pix[k + 2]]})`;
    sc.fillRect(i * w, 2, Math.max(1, w - 0.6), H - 4);
  }
}
const tl = $("timeline"), lc = tl.getContext("2d"), T0 = DATA.t[0], T1 = DATA.t[F - 1];
const xOf = t => (t - T0) / (T1 - T0) * tl.width;
function drawTimeline(f) {
  const W = tl.width, H = tl.height;
  lc.clearRect(0, 0, W, H);
  for (let i = 0; i < DATA.phases.length; i++) {
    const a = Math.max(T0, DATA.phases[i][0]), b = i + 1 < DATA.phases.length ? DATA.phases[i + 1][0] : T1;
    lc.fillStyle = PCOL[DATA.phases[i][1]]; lc.fillRect(xOf(a), 10, Math.max(1, xOf(b) - xOf(a)), H - 20);
  }
  lc.fillStyle = "#fff";
  for (const e of DATA.events) { const x = xOf(Math.max(T0, e[0])); lc.fillRect(x - 1, 0, 2, 12); }
  const x = xOf(DATA.t[f]);
  lc.fillStyle = "#ffcc4d"; lc.fillRect(x - 1.5, 0, 3, H);
}
const log = $("log");
for (const e of DATA.events) { const li = document.createElement("li"); li.textContent = fmt(e[0]) + "." + String(e[0] % 1000).padStart(3, "0") + "  " + e[1]; log.appendChild(li); }
function show(f) {
  const ph = DATA.phase[f], d = DATA.display[f];
  drawTree(f); drawStrip(f); drawTimeline(f);
  $("lcd").textContent = d < 0 ? "" : String(d).padStart(2, " ");
  $("kcal").textContent = DATA.kcal[f];
  $("phase").textContent = PHASES[ph]; $("phase").style.color = PCOL[ph] === "#3b4660" ? "#8a93a6" : PCOL[ph];
  $("left").textContent = ph === 0 ? "" : ph === 1 ? "until the track ends (timeout in " + fmt(DATA.remaining[f] + 999) + ")" : fmt(DATA.remaining[f] + 999) + " left";
  $("gauge").style.width = (DATA.level[f] / 255 * 100) + "%";
  $("level").textContent = "level " + DATA.level[f] + " / 255";
  $("track").textContent = TRACKS[DATA.track[f]] || ("track " + DATA.track[f]);
  $("clock").textContent = fmt(DATA.t[f]) + " / " + fmt(T1);
  const now = DATA.t[f];
  for (let i = 0; i < DATA.events.length; i++) log.children[i].className = DATA.events[i][0] <= now ? "past" : "";
}
// Playback follows the simulated clock: samples are 40 or 60 ms apart (frames
// are rendered every 20 ms), so stepping by index would jitter.
let frame = 0, playing = false, speed = 1, simT = DATA.t[0], last = 0;
function frameAt(t) { let lo = 0, hi = F - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (DATA.t[m] <= t) lo = m; else hi = m - 1; } return lo; }
function setPlaying(p) { playing = p; $("play").textContent = p ? "Pause" : "Play"; $("play").classList.toggle("on", p); if (p && frame >= F - 1) { frame = 0; simT = T0; } last = performance.now(); }
function setSpeed(s) { speed = s; document.querySelectorAll("[data-speed]").forEach(b => b.classList.toggle("on", +b.dataset.speed === s)); }
function tick(ts) {
  if (playing) {
    simT = Math.min(T1, simT + (ts - last) * speed); last = ts;
    const f = frameAt(simT);
    if (f !== frame) { frame = f; show(frame); }
    if (simT >= T1) setPlaying(false);
  }
  requestAnimationFrame(tick);
}
$("play").onclick = () => setPlaying(!playing);
document.querySelectorAll("[data-speed]").forEach(b => b.onclick = () => setSpeed(+b.dataset.speed));
document.addEventListener("keydown", e => { if (e.code === "Space") { e.preventDefault(); setPlaying(!playing); } });
function seek(e) {
  const r = tl.getBoundingClientRect(), t = T0 + (e.clientX - r.left) / r.width * (T1 - T0);
  simT = Math.max(T0, Math.min(T1, t)); frame = frameAt(simT); show(frame);
}
let dragging = false;
tl.addEventListener("pointerdown", e => { dragging = true; tl.setPointerCapture(e.pointerId); seek(e); });
tl.addEventListener("pointermove", e => { if (dragging) seek(e); });
tl.addEventListener("pointerup", () => { dragging = false; });
setSpeed(1); show(0); requestAnimationFrame(tick);
</script></body></html>
)HTML";

bool writeFile(const std::string& path, const std::string& content) {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(content.data(), 1, content.size(), f) == content.size();
    return std::fclose(f) == 0 && ok;
}

void printSummary(const Scenario& sc, const Recording& rec, const Rig& sound, size_t htmlBytes) {
    std::printf("== %s: %s\n", sc.name, sc.description);
    std::printf("   %zu frames over %.1f s, HTML %.2f MB\n", rec.samples.size(), rec.endMs / 1000.0,
                htmlBytes / 1048576.0);
    std::printf("   phases:");
    for (size_t i = 0; i < rec.phaseStart.size(); ++i) {
        const uint32_t end = i + 1 < rec.phaseStart.size() ? rec.phaseStart[i + 1] : rec.endMs;
        std::printf(" %s %.2fs%s", PHASE_NAMES[rec.phaseOf[i]], (end - rec.phaseStart[i]) / 1000.0,
                    i + 1 < rec.phaseStart.size() ? " ->" : "");
    }
    const int shown = sound.display.current();
    std::printf("\n   final kcal %u, display %s\n", static_cast<unsigned>(sound.app.game().kcal()),
                shown < 0 ? "blank" : std::to_string(shown).c_str());
    std::printf("   sound:");
    for (size_t i = 0; i < sound.mp3.log.size(); ++i)
        std::printf(" %.2fs %s%s", sound.mp3.logMs[i] / 1000.0, sound.mp3.log[i].c_str(),
                    i + 1 < sound.mp3.log.size() ? "," : "");
    std::printf("\n");
}

}  // namespace

int main(int argc, char** argv) {
    const std::string out = argc > 1 ? argv[1] : "build/sim";
    const uint32_t ride = cfg::RIDE_MS;
    const std::vector<Scenario> scenarios = {
        {"steady", "effort 128 for the whole ride", true, 0, [](uint32_t) { return uint8_t(128); }},
        {"sprint-then-stop", "effort 240 for 60 s, then the rider stops", true, 0,
         [](uint32_t t) { return uint8_t(t < 60000 ? 240 : 0); }},
        {"slow", "effort 40 for the whole ride", true, 0, [](uint32_t) { return uint8_t(40); }},
        {"ramp", "effort rises from 0 to 255 over the ride", true, 0,
         [ride](uint32_t t) { return uint8_t(t >= ride ? 255 : uint64_t(t) * 255 / ride); }},
        {"idle", "60 s with nobody: the idle garland", false, 60000, [](uint32_t) { return uint8_t(0); }},
    };

    std::string index = "<!doctype html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>Bike tree replays</title><style>body{margin:0;padding:16px;background:#0b0f17;"
        "color:#e8ecf3;font:15px/1.5 system-ui,sans-serif}a{color:#ffcc4d}li{margin:6px 0}"
        ".d{color:#8a93a6}</style></head><body><h1>Bike tree: simulated games</h1>"
        "<p class=\"d\">Each page replays the real firmware logic (qb::App) on the simulated bench "
        "(test/fakes/sim_rig.h) at 20 frames per second. Generated by <code>make sim</code>.</p><ul>\n";
    int failures = 0;
    for (const Scenario& sc : scenarios) {
        Rig r;
        const Recording rec = record(sc, r);
        const std::string html = std::string(PAGE_HEAD) + "const DATA = " + dataJson(sc, rec, r) +
                                 ";\nconst PIXELS = \"" + base64(rec.rgb) + "\";\n" + PAGE_TAIL;
        const std::string file = std::string(sc.name) + ".html";
        if (!writeFile(out + "/" + file, html)) {
            std::fprintf(stderr, "sim: cannot write %s/%s\n", out.c_str(), file.c_str());
            ++failures;
            continue;
        }
        printSummary(sc, rec, r, html.size());
        index += "<li><a href=\"" + file + "\">" + sc.name + "</a> <span class=\"d\">" + sc.description +
                 "</span></li>\n";
    }
    index += "</ul></body></html>\n";
    if (!writeFile(out + "/index.html", index)) {
        std::fprintf(stderr, "sim: cannot write %s/index.html\n", out.c_str());
        ++failures;
    }
    return failures ? 1 : 0;
}
