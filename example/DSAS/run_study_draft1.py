# run_study.py
# Orchestrator for Gate parametric study.
# Usage: python run_study.py [--dry-run] [--start-from N]

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

from params import PARAM_GRID

TEMPLATE_DIR = Path("templates")
RUNS_DIR     = Path("runs")
GATE_BIN     = "Gate"
MAIN_MAC     = "main.mac"

# Files that are static (no placeholders) — copied as-is
STATIC_MACROS = {"Visualisation.mac", "Verbose.mac"}


# ---------------------------------------------------------------------------
# Template filling
# ---------------------------------------------------------------------------

def fill_template(template_path: Path, params: dict) -> str:
    content = template_path.read_text()
    for key, value in params.items():
        content = content.replace(f"{{{{{key}}}}}", str(value))
    return content


def check_unfilled(content: str, filename: str):
    missing = re.findall(r"\{\{[A-Z0-9_]+\}\}", content)
    if missing:
        raise ValueError(
            f"Unfilled placeholders in {filename}: {missing}\n"
            f"Add the missing keys to your params dict."
        )


# ---------------------------------------------------------------------------
# Run directory setup
# ---------------------------------------------------------------------------

def setup_run_dir(run_id: int, params: dict) -> Path:
    run_dir = RUNS_DIR / f"run_{run_id:03d}"
    run_dir.mkdir(parents=True, exist_ok=True)
    macro_dir = run_dir / "macro"
    macro_dir.mkdir(exist_ok=True)
    (run_dir / "output").mkdir(exist_ok=True)

    # Inject per-run output paths into params (don't mutate original)
    run_params = dict(params)
    run_params["OUTPUT_ROOT_NAME"]  = f"output/run_{run_id:03d}"
    run_params["OUTPUT_ASCII_NAME"] = f"output/run_{run_id:03d}_ascii"
    run_params["SPAD_EXPORT_PATH"]  = str(run_dir / "output" / f"run_{run_id:03d}_spad.root")

    # Fill and write main.mac
    main_template = TEMPLATE_DIR / MAIN_MAC
    filled_main = fill_template(main_template, run_params)
    check_unfilled(filled_main, MAIN_MAC)
    (run_dir / MAIN_MAC).write_text(filled_main)

    # Fill and write macro/*.mac
    for template_file in (TEMPLATE_DIR / "macro").glob("*.mac"):
        if template_file.name in STATIC_MACROS:
            shutil.copy(template_file, macro_dir / template_file.name)
        else:
            filled = fill_template(template_file, run_params)
            check_unfilled(filled, template_file.name)
            (macro_dir / template_file.name).write_text(filled)

    return run_dir


# ---------------------------------------------------------------------------
# Gate execution
# ---------------------------------------------------------------------------

def run_gate(run_dir: Path) -> bool:
    mac_path = run_dir / MAIN_MAC
    log_path = run_dir / "gate.log"

    with open(log_path, "w") as log_file:
        result = subprocess.run(
            [GATE_BIN, str(mac_path.name)],
            stdout=log_file,
            stderr=subprocess.STDOUT,
            cwd=run_dir,
        )

    return result.returncode == 0


# ---------------------------------------------------------------------------
# Metadata
# ---------------------------------------------------------------------------

def save_metadata(run_dir: Path, run_id: int, params: dict):
    meta = {
        "run_id": run_id,
        "params": params,
        "gate_log": str(run_dir / "gate.log"),
        "output_dir": str(run_dir / "output"),
    }
    (run_dir / "params.json").write_text(json.dumps(meta, indent=2))


# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

def write_study_summary(results: list):
    summary_path = RUNS_DIR / "study_summary.json"
    summary_path.write_text(json.dumps(results, indent=2))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(description="Gate parametric study orchestrator")
    parser.add_argument(
        "--dry-run", action="store_true",
        help="Generate run directories and filled macros without launching Gate"
    )
    parser.add_argument(
        "--start-from", type=int, default=0, metavar="N",
        help="Skip the first N configs (resume after a partial run)"
    )
    return parser.parse_args()


def main():
    args = parse_args()
    RUNS_DIR.mkdir(exist_ok=True)

    total   = len(PARAM_GRID)
    results = []

    for run_id, params in enumerate(PARAM_GRID):

        if run_id < args.start_from:
            continue

        print(f"[{run_id + 1}/{total}] Setting up run_{run_id:03d}...")

        try:
            run_dir = setup_run_dir(run_id, params)
            save_metadata(run_dir, run_id, params)
        except ValueError as exc:
            print(f"  ERROR during setup: {exc}")
            results.append({"run_id": run_id, "status": "SETUP_ERROR", "error": str(exc)})
            continue

        if args.dry_run:
            print(f"  [DRY-RUN] Macros generated at {run_dir}")
            results.append({"run_id": run_id, "status": "DRY_RUN"})
            continue

        print(f"  Running Gate... (log: {run_dir / 'gate.log'})")
        success = run_gate(run_dir)
        status  = "OK" if success else "FAILED"
        print(f"  [{status}]")

        results.append({"run_id": run_id, "status": status, "run_dir": str(run_dir)})

    write_study_summary(results)

    failed = [r for r in results if r["status"] == "FAILED"]
    if failed:
        print(f"\n{len(failed)} run(s) failed:")
        for r in failed:
            print(f"  run_{r['run_id']:03d}  ->  check {RUNS_DIR / f\"run_{r['run_id']:03d}\" / 'gate.log'}")
        sys.exit(1)

    print(f"\nAll {len(results)} runs completed. Summary: {RUNS_DIR / 'study_summary.json'}")


if __name__ == "__main__":
    main()
