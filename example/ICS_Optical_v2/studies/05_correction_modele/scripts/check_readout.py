"""Checks before any model result (PLAN.md):
V1  my `ideal` / `ics` signals give the counting numbers of results/performance.json (same definitions, other
    random draw: agreement within the statistics);
V2  the first crystal (earliest GATE deposit) is the crystal labelled direct / penetration by reduce.py, and every
    event has exactly one such crystal; size of the ICS pool, hits outside the 5x5 window.

Usage: /usr/bin/python3 scripts/check_readout.py            -> logs/check_readout.json
"""

import json
import time

import numpy as np

import s05_common as c
from events import first_crystal, signals, windows


def check(pair: dict, perf_ref: dict) -> dict:
    t0 = time.time()
    gcfg, g, prep, sig = signals(pair)
    first = first_crystal(prep)
    keep = g["ev"] < c.perf.MAX_EVENTS
    prim = (g["E_direct"] + g["E_penetration"])[keep] > 0
    n_prim = np.bincount(prep["rows"]["ev"][prim], minlength=first["n_dep"].size)
    has_rows = first["n_dep"] > 0
    out = dict(label=c.label(pair), n_incident=prep["n_inc"], n_events=int(has_rows.sum()),
               first_is_direct_or_penetration=float(prim[first["first_row"]].mean()),
               events_with_one_primary_crystal=float(np.mean(n_prim[has_rows] == 1)),
               events_with_no_primary_crystal=float(np.mean(n_prim[has_rows] == 0)),
               multi_crystal_events=float(np.mean(first["n_dep"][has_rows] >= 2)), thresholds={})
    for T in c.THRESHOLDS:
        res = {}
        for name in ("ideal", "ics"):
            s = sig[name]
            mine = c.perf._counting(s["pev"], s["px"], s["py"], s["e"], prep["n_inc"], T)
            ref = perf_ref["variants"][name]["counting"][str(int(T))]
            res[name] = {k: dict(mine=mine[k], ref=ref[k]) for k in ("eff", "true_count", "false_counts", "misplaced")}
        w = windows(prep, sig["ics"], T, first)
        res["windows"] = dict(photons=int(w["ev"].size), ics_pool=int(w["is_ics"].sum()),
                              ics_pool_multi_hit=int((w["is_ics"] & (w["n_hits"] >= 2)).sum()),
                              visible_hits=int(w["n_hits"].sum() + w["out_ev"].size),
                              hits_outside_window=int(w["out_ev"].size),
                              non_ics_with_2_hits=int((~w["is_ics"] & (w["n_hits"] >= 2)).sum()))
        out["thresholds"][int(T)] = res
    out["seconds"] = round(time.time() - t0, 1)
    return out


def main():
    perf_all = {(p["gamma_uuid"], p["optical_uuid"]): p for p in json.loads(
        (c.ROOT / "results" / "performance.json").read_text()) if "variants" in p}
    results = []
    for pair in c.test_pairs():
        r = check(pair, perf_all[(pair["gamma_uuid"], pair["optical_uuid"])])
        results.append(r)
        print(json.dumps(r), flush=True)
    c.LOGS_DIR.mkdir(exist_ok=True)
    (c.LOGS_DIR / "check_readout.json").write_text(json.dumps(results, indent=1, default=float))


if __name__ == "__main__":
    main()
