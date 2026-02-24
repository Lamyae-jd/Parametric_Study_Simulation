# diagnose.py
# Run this from inside gate_parametric/ to pinpoint the issue.
# python diagnose.py

import re
import shutil
from pathlib import Path

TEMPLATE_DIR  = Path("templates")
STATIC_MACROS = {"Visualisation.mac", "Verbose.mac"}

print("=== 1. Template directory ===")
print(f"  TEMPLATE_DIR exists       : {TEMPLATE_DIR.exists()}")
print(f"  TEMPLATE_DIR / macro exists: {(TEMPLATE_DIR / 'macro').exists()}")

mac_files = list((TEMPLATE_DIR / "macro").glob("*.mac"))
print(f"  .mac files found          : {[f.name for f in mac_files]}")

print()
print("=== 2. Simulating fill for run_000 ===")

from params import PARAM_GRID, BASE

params = PARAM_GRID[0]
run_id = 0

run_params = dict(params)
run_params["OUTPUT_ROOT_NAME"]  = f"output/run_{run_id:03d}"
run_params["OUTPUT_ASCII_NAME"] = f"output/run_{run_id:03d}_ascii"
run_params["SPAD_EXPORT_PATH"]  = f"runs/run_{run_id:03d}/output/run_{run_id:03d}_spad.root"

for template_file in mac_files:
    content = template_file.read_text()
    for key, value in run_params.items():
        content = content.replace(f"{{{{{key}}}}}", str(value))

    missing = re.findall(r"\{\{[A-Z0-9_]+\}\}", content)

    print(f"  {template_file.name}")
    print(f"    static (copy as-is) : {template_file.name in STATIC_MACROS}")
    print(f"    unfilled placeholders: {missing if missing else 'none'}")

print()
print("=== 3. Test write to a tmp dir ===")
tmp = Path("_diag_test/macro")
tmp.mkdir(parents=True, exist_ok=True)

written = []
for template_file in mac_files:
    dest = tmp / template_file.name
    if template_file.name in STATIC_MACROS:
        shutil.copy(template_file, dest)
    else:
        content = template_file.read_text()
        for key, value in run_params.items():
            content = content.replace(f"{{{{{key}}}}}", str(value))
        dest.write_text(content)
    written.append(dest.name)

print(f"  Files written to _diag_test/macro/: {written}")

import shutil as _shutil
_shutil.rmtree("_diag_test")
print()
print("If files appear above, the logic works. The issue is elsewhere — check section 2 for unfilled placeholders.")
