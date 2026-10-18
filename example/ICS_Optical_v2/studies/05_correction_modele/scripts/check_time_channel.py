"""Check V3 — what the time channel brings to each model, on the ICS photons with >= 2 hits of the test runs.

Decision changes (argmax among the active cells) when the measured timestamps are replaced by
  ideal : the GATE deposit times (no noise; true gaps between crystals are a few ps)
  zero  : no time information at all.

Usage: /usr/bin/python3 scripts/check_time_channel.py      -> logs/check_time_channel.json
"""

import json

import numpy as np

import s05_common as c
from evaluate import load_models
from events import first_crystal, model_input, signals, windows


def decisions(model, x: np.ndarray, active: np.ndarray) -> np.ndarray:
    return np.where(active, c.qcommon.predict_logits(model, x), -np.inf).argmax(axis=1)


def main():
    models = load_models()
    out = {}
    for T in c.THRESHOLDS:
        acc = {name: {"ideal": 0, "zero": 0} for name in models}
        n = 0
        for pair in c.test_pairs():
            _, _, prep, sig = signals(pair)
            w = windows(prep, sig["ics"], T, first_crystal(prep))
            sel = np.flatnonzero(w["is_ics"] & (w["n_hits"] >= 2))
            sub = {k: w[k][sel] for k in ("E", "t_real", "t_true", "active")}
            x_real, x_ideal = model_input(sub, "real"), model_input(sub, "ideal")
            x_zero = x_real.copy()
            x_zero[:, 1] = 0.0
            n += sel.size
            for name, model in models.items():
                ref = decisions(model, x_real, sub["active"])
                acc[name]["ideal"] += int((decisions(model, x_ideal, sub["active"]) != ref).sum())
                acc[name]["zero"] += int((decisions(model, x_zero, sub["active"]) != ref).sum())
        out[int(T)] = dict(photons=n, **{name: {k: dict(changes=v, pct=100 * v / n) for k, v in a.items()}
                                         for name, a in acc.items()})
        print(T, json.dumps(out[int(T)]), flush=True)
    (c.LOGS_DIR / "check_time_channel.json").write_text(json.dumps(out, indent=1))


if __name__ == "__main__":
    main()
