"""Shared paths, run lists and imports for study 05 (ICS correction by small_v2).

The study code (analysis/, sim/) is imported read-only, BEFORE the small_v2 helpers (their folder also holds a
`configs` package). small_v2 helpers: quantization study `common` (FP32 checkpoint, ics_metrics), unchanged.
"""

import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
STUDY_DIR = HERE.parent
ROOT = STUDY_DIR.parent.parent                      # ICS_Optical_v2
sys.path.insert(0, str(ROOT / "sim"))
sys.path.insert(0, str(ROOT / "analysis"))

import numpy as np                                                  # noqa: E402

import image_metrics                                                # noqa: E402,F401
import image_sim                                                    # noqa: E402,F401
import performance as perf                                          # noqa: E402
import timing                                                       # noqa: E402,F401
from metrics import load_run, optical_metrics                      # noqa: E402

GNN_DIR = Path("/data/jdil1901/Documents/ics/code/5x5/Code proposition NSS mic/python files/Distillation with GNN")
sys.path.insert(0, str(GNN_DIR / "quantization_fp32_vs_int8" / "scripts"))
import common as qcommon                                            # noqa: E402

RUNS = ROOT / "runs"
DATA_DIR = STUDY_DIR / "data"
LOGS_DIR = STUDY_DIR / "logs"
RESULTS_DIR = STUDY_DIR / "results"
FIGURES_DIR = STUDY_DIR / "figures"
RETRAINED_CKPT = STUDY_DIR / "checkpoints" / "small_v2_retrained_best.pt"

THRESHOLDS = (5.0, 20.0)                 # per-hit trigger thresholds (keV): user's system, study's main threshold
WINDOW, HALF = 5, 2
MAX_TRAIN_ENERGY = 160                   # mono-energy training runs 20-160 keV (CT range; 511 keV excluded)
SEED = 0                                 # light statistics / timestamps (as performance.py)


def _pairs():
    return json.loads((ROOT / "results" / "combined.json").read_text())


def _reference(c) -> bool:
    return (c["material"] == "LGSO" and c["crystal_size_mm"] in (1.2, 2.0) and c["crystal_length_mm"] == 2.5
            and c["optical_reflector"] == "PTFE" and c["optical_finish"] == "polished")


def test_pairs() -> list:
    """The 16 LGSO CT-spectrum runs (1.2 and 2.0 mm) with their reference optical pair."""
    out = {c["gamma_uuid"]: c for c in _pairs() if _reference(c) and c.get("spectrum")
           and ({"G4_spectrum", "G4B_spectrum"} & set(c["sweeps"]))}
    return sorted(out.values(), key=lambda c: (c["crystal_size_mm"], c["spectrum"]))


def train_pairs() -> list:
    """Mono-energy LGSO runs 20-160 keV of the energy sweeps (1.2 and 2.0 mm), reference optical pair."""
    out = {c["gamma_uuid"]: c for c in _pairs() if _reference(c) and not c.get("spectrum")
           and ({"G1_energy", "G1B_energy"} & set(c["sweeps"])) and c["energy_keV"] <= MAX_TRAIN_ENERGY}
    return sorted(out.values(), key=lambda c: (c["crystal_size_mm"], c["energy_keV"]))


def load_pair(pair: dict) -> tuple:
    """Same loading as analysis/build_performance._load_pair (gamma arrays, aux, light kernel, time tables)."""
    gcfg, g = load_run(RUNS / pair["gamma_uuid"])
    aux = dict(np.load(RUNS / pair["gamma_uuid"] / "reduced_aux.npz"))
    if not bool(aux.get("identical", True)):
        g = {**g, **{k[4:]: v for k, v in aux.items() if k.startswith("new_")}}
    ocfg, o = load_run(RUNS / pair["optical_uuid"])
    A = np.asarray(optical_metrics(ocfg, o)["A"])
    tt = dict(np.load(RUNS / pair["optical_uuid"] / "reduced_time.npz"))
    return gcfg, g, aux, A, tt


def label(pair: dict) -> str:
    return f"{pair['crystal_size_mm']:.1f}mm_{pair.get('spectrum') or str(pair['energy_keV']) + 'keV'}"
