#!/usr/bin/env python3
"""Draw every picture of the French guide into docs/guide/img/.

    python3 tools/diagrams/build_guide.py            # all pictures
    python3 tools/diagrams/build_guide.py bouton     # only the ones whose name contains "bouton"

The software pictures that show LED effects, the speed filter and the calorie
model are drawn from data produced by the real firmware code
(tools/diagrams/firmware_data.cpp, compiled here with g++).
"""
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

import fig_hardware  # noqa: E402
import fig_software  # noqa: E402


def firmware_data():
    out = ROOT / "build" / "diagrams"
    out.mkdir(parents=True, exist_ok=True)
    exe = out / "firmware_data"
    src = ROOT / "firmware" / "quiberon_bike" / "src"
    cmd = ["g++", "-std=c++11", "-O1", "-I", str(src), *map(str, sorted((src / "core").glob("*.cpp"))),
           str(HERE / "firmware_data.cpp"), "-o", str(exe)]
    subprocess.run(cmd, check=True)
    return json.loads(subprocess.run([str(exe)], check=True, capture_output=True, text=True).stdout)


def main():
    only = sys.argv[1] if len(sys.argv) > 1 else ""
    out = ROOT / "docs" / "guide" / "img"
    out.mkdir(parents=True, exist_ok=True)
    data = firmware_data()
    figures = dict(fig_hardware.FIGURES)
    figures.update(fig_software.figures(data))
    for name, make in figures.items():
        if only in name:
            (out / name).write_text(make(), encoding="utf-8")
            print("wrote", out / name)


if __name__ == "__main__":
    main()
