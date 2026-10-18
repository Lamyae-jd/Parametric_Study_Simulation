"""Training / validation samples for the retrained small_v2 (test = CT-spectrum runs, NOT touched here).

From every mono-energy run 20-160 keV of the two detectors (s05_common.train_pairs) and each threshold (5, 20 keV):
events split 85 / 15 % (train / val) at random (seed = run index), then PER_RUN_TRAIN / PER_RUN_VAL detected photons
drawn at random (natural proportion of ICS). Inputs exactly as events.model_input (measured timestamps).
Output: data/train.npz, data/val.npz with x (n, 3, 5, 5) float32, target (n, 25) one-hot (zeros if not ICS),
is_ics (n,), threshold (n,), run (n,).

Usage: /usr/bin/python3 scripts/make_trainset.py
"""

import json
import time

import numpy as np

import s05_common as c
from events import first_crystal, model_input, one_hot, signals, windows

VAL_FRACTION = 0.15
PER_RUN_TRAIN, PER_RUN_VAL = 100_000, 20_000


def samples(k: int, pair: dict) -> dict:
    _, _, prep, sig = signals(pair)
    first = first_crystal(prep)
    rng = np.random.default_rng(k)
    n_ev = first["n_dep"].size
    is_val = np.zeros(n_ev, bool)
    is_val[rng.permutation(n_ev)[:int(VAL_FRACTION * n_ev)]] = True
    out = {"train": [], "val": []}
    for T in c.THRESHOLDS:
        w = windows(prep, sig["ics"], T, first)
        for name, n_keep in (("train", PER_RUN_TRAIN), ("val", PER_RUN_VAL)):
            pool = np.flatnonzero(is_val[w["ev"]] == (name == "val"))
            idx = np.sort(rng.choice(pool, min(n_keep, pool.size), replace=False))
            out[name].append(dict(x=model_input({key: w[key][idx] for key in ("E", "t_real", "t_true", "active")}),
                                  target=one_hot(w["target"][idx], w["is_ics"][idx]), is_ics=w["is_ics"][idx],
                                  threshold=np.full(idx.size, T, np.float32), run=np.full(idx.size, k, np.int16)))
    return out


def main():
    pairs = c.train_pairs()
    parts, t0 = {"train": [], "val": []}, time.time()
    for k, pair in enumerate(pairs):
        s = samples(k, pair)
        for name in parts:
            parts[name] += s[name]
        print(f"[{k + 1}/{len(pairs)}] {c.label(pair)} ({time.time() - t0:.0f} s)", flush=True)
    c.DATA_DIR.mkdir(exist_ok=True)
    summary = {"runs": [dict(index=k, label=c.label(p), gamma_uuid=p["gamma_uuid"], optical_uuid=p["optical_uuid"])
                        for k, p in enumerate(pairs)]}
    for name, chunks in parts.items():
        data = {key: np.concatenate([ch[key] for ch in chunks]) for key in chunks[0]}
        np.savez(c.DATA_DIR / f"{name}.npz", **data)
        summary[name] = dict(samples=int(data["is_ics"].size), ics=int(data["is_ics"].sum()),
                             ics_pct=float(100 * data["is_ics"].mean()))
        print(f"{name}: {data['is_ics'].size:,} photons, ICS {100 * data['is_ics'].mean():.2f} %", flush=True)
    c.LOGS_DIR.mkdir(exist_ok=True)
    (c.LOGS_DIR / "trainset.json").write_text(json.dumps(summary, indent=1))


if __name__ == "__main__":
    main()
