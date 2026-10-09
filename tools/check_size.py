"""Fail the firmware build when it does not fit the Uno comfortably.

Reads the JSON that `arduino-cli compile --format json` prints on stdin.
Static RAM must leave room for the stack (FastLED show(), interrupts, the
call chain of one loop), hence a budget well below the Uno's 2048 bytes:
512 bytes are kept for the stack. The real stack use is measured on the bench:
the firmware prints "stack headroom" (never-touched bytes) at each phase
change, see docs/bench-test.md.
"""
import json
import sys

RAM_BUDGET = 1536        # bytes of static RAM (.data + .bss), out of 2048
FLASH_BUDGET_PCT = 95    # percent of the 32256 bytes available to sketches

report = json.load(sys.stdin)
result = report.get("builder_result", report)
if not report.get("success", result.get("success", False)):
    print(report.get("compiler_err", "compile failed"))
    sys.exit(1)

sections = {s["name"]: s for s in result["executable_sections_size"]}
flash, ram = sections["text"], sections["data"]
flash_pct = 100.0 * flash["size"] / flash["max_size"]
print(f"flash: {flash['size']} / {flash['max_size']} bytes ({flash_pct:.1f}%), budget {FLASH_BUDGET_PCT}%")
print(f"ram:   {ram['size']} / {ram['max_size']} bytes, budget {RAM_BUDGET}")

failed = False
if ram["size"] > RAM_BUDGET:
    print(f"FAIL: static RAM {ram['size']} B exceeds the {RAM_BUDGET} B budget")
    failed = True
if flash_pct > FLASH_BUDGET_PCT:
    print(f"FAIL: flash {flash_pct:.1f}% exceeds the {FLASH_BUDGET_PCT}% budget")
    failed = True
sys.exit(1 if failed else 0)
