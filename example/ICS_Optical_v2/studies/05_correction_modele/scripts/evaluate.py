"""Test: the 16 CT-spectrum runs, thresholds 5 and 20 keV, all variants of PLAN.md on the SAME photoelectrons.

Variants (counts = list of (event, crystal offset, energy, timestamp)):
  ideal      study variant (no ICS)                      ics        study variant (one count per crystal >= T)
  grouped    one count per photon on its highest-energy crystal (grouping without any model: reference)
  oracle     one count per photon on the visible window cell with the earliest deposit (best possible choice)
  fp32       small_v2 as is (checkpoints_pikd_small/best.pt)
  retrained  small_v2 retrained on the mono-energy runs (scripts/train.py)
Models get the measured timestamps (small_v2 as is ignores the time channel: check_time_channel.py).
Corrected variants: 1 hit → that hit; >= 2 hits → argmax of the model among the active cells; energy of the count =
sum of the window hits; hits outside the window stay separate counts.

Per run and threshold: counting (extra counts, misplaced = count not on the irradiated crystal), presampled MTF,
DQE(0), SNR / CNR relative to ideal (image_metrics, as performance.py), count spectrum, timestamps (timing_summary).
Whole ICS pool (per detector and threshold): AUPRC, precision, recall, F1, AccTop1 (common.ics_metrics).
Low-contrast disk images (image_sim, common random numbers) for the 120 kVp + 20 cm water runs.

Usage: /usr/bin/python3 scripts/evaluate.py      -> results/metrics.json, data/image_examples.npz
"""

import json
import time

import numpy as np
import torch

import s05_common as c
from events import first_crystal, model_input, one_hot, signals, windows
from pi_kd.models.student import StudentNet  # noqa: E402

im, isim = c.image_metrics, c.image_sim
MODELS = ("fp32", "retrained")
VARIANTS = ("ideal", "ics", "grouped", "oracle") + MODELS
CENTRE_CELL = c.HALF * c.WINDOW + c.HALF          # the window is centred on the highest-energy hit
IMAGE_SIM_SPECTRUM = "120kVp_water20"
E_BINS = np.arange(0.0, 162.0, 2.0)
LOW_E_KEV = 30.0


def load_models() -> dict:
    fp32 = c.qcommon.load_fp32()
    ckpt = torch.load(c.RETRAINED_CKPT, map_location="cpu", weights_only=False)
    retrained = StudentNet(in_ch=3, widths=tuple(ckpt["cfg"]["student"]["widths"]))
    retrained.load_state_dict(ckpt["student"])
    retrained = retrained.to(c.qcommon.DEVICE).eval()
    return {"fp32": fp32, "retrained": retrained}


def chosen_cells(w: dict, logits_ics: np.ndarray) -> np.ndarray:
    """Window cell of every photon: its only active cell, or the model's argmax among its active cells."""
    cell = w["active"].argmax(axis=1)
    ics = np.flatnonzero(w["is_ics"])
    multi = w["n_hits"][ics] >= 2
    masked = np.where(w["active"][ics[multi]], logits_ics[multi], -np.inf)
    cell[ics[multi]] = masked.argmax(axis=1)
    return cell


def corrected_counts(w: dict, cell: np.ndarray) -> dict:
    n = cell.size
    r = np.arange(n)
    return dict(ev=np.r_[w["ev"], w["ev"][w["out_ev"]]],
                px=np.r_[w["cx"] + cell // c.WINDOW - c.HALF, w["out_px"]],
                py=np.r_[w["cy"] + cell % c.WINDOW - c.HALF, w["out_py"]],
                e=np.r_[w["e_window"], w["out_e"]], t=np.r_[w["t_real"][r, cell], w["out_t"]],
                main=np.r_[np.ones(n, bool), np.zeros(w["out_ev"].size, bool)])


def raw_counts(sig: dict, T: float) -> dict:
    """One count per crystal >= T; main = highest-energy crystal of the photon (study's 'misplaced')."""
    m = np.flatnonzero(sig["e"] >= T)
    ev, e = sig["pev"][m], sig["e"][m]
    order = np.lexsort((-e, ev))
    main = np.zeros(m.size, bool)
    main[order[np.flatnonzero(np.r_[True, ev[order][1:] != ev[order][:-1]])]] = True
    return dict(ev=ev, px=sig["px"][m], py=sig["py"][m], e=e, t=sig["t_ns"][m], main=main)


def count_metrics(cnt: dict, n_inc: int, p: float, sx_ev: np.ndarray, sy_ev: np.ndarray) -> dict:
    ev, px, py = cnt["ev"], cnt["px"], cnt["py"]
    central = (px == 0) & (py == 0)
    n_det = int(cnt["main"].sum())
    w1 = np.ones(ev.size)
    ux, uy = px * p - sx_ev[ev], py * p - sy_ev[ev]
    f1, f2, fy0 = im.freqs_1d(p), im.freqs_2d(p), np.array([0.0])
    T1 = im.mean_transfer(ux, uy, w1, n_inc, f1, fy0)
    S1 = im.second_moment_counts(ev, px, py, w1, n_inc, p, f1, fy0)
    T2 = im.mean_transfer(ux, uy, w1, n_inc, f2, f2)
    S2 = im.second_moment_counts(ev, px, py, w1, n_inc, p, f2, f2)
    out = dict(eff=n_det / n_inc, counts=int(ev.size), photons=n_det,
               extra_counts_pct=100 * (ev.size - n_det) / n_det,
               true_count=float(central.sum() / n_inc), false_counts=float((~central).sum() / n_inc),
               misplaced_pct=float(100 * np.mean(~central[cnt["main"]])),
               low_energy_counts_pct=float(100 * np.mean(cnt["e"] < LOW_E_KEV)),
               spectrum=np.histogram(cnt["e"], bins=E_BINS)[0].tolist(),
               timing=c.timing.timing_summary(cnt["t"], cnt["e"]))
    out.update(im.summarise(f1, T1, S1, f2, T2, S2, p))
    out.pop("nps_norm")
    return out


def library(cnt: dict, n_used: int, sx_ev: np.ndarray, sy_ev: np.ndarray) -> dict:
    """Counted crystal offsets of every event 0..n_used-1 in CSR form (as image_sim.library)."""
    order = np.argsort(cnt["ev"], kind="stable")
    count = np.bincount(cnt["ev"], minlength=n_used)[:n_used]
    sx, sy = np.zeros(n_used), np.zeros(n_used)
    k = min(n_used, sx_ev.size)
    sx[:k], sy[:k] = sx_ev[:k], sy_ev[:k]
    return dict(start=np.r_[0, np.cumsum(count)[:-1]], count=count, px=cnt["px"][order], py=cnt["py"][order],
                sx=sx, sy=sy)


def image_study(counts: dict, prep: dict, sx_ev, sy_ev, seed: int, examples: dict, tag: str) -> dict:
    """image_sim.simulate / measure for every variant, same incident photons (common random numbers)."""
    p = prep["pitch"]
    disks = isim.disk_grid(p, np.random.default_rng(seed))
    masks = isim.roi_masks(disks, p)
    out = {}
    for name, cnt in counts.items():
        lib = library(cnt, prep["n_used"], sx_ev, sy_ev)
        acc = {}
        for i in range(isim.N_IMAGES):
            F, Dm = isim.simulate(lib, prep["n_used"], prep["n_inc"], p, disks, np.random.default_rng([seed, i]))
            if i == 0:
                examples[f"{tag}_{name}"] = (F - Dm).astype(np.int32)
            for k, v in isim.measure(F, Dm, disks, masks, np.random.default_rng([seed, i, 7])).items():
                acc.setdefault(k, []).append(v)
        out[name] = {}
        for k, v in acc.items():
            ratio = np.array(v)[:, 0] / np.array(v)[:, 1]
            out[name][k] = dict(value=float(ratio.mean()), err=float(ratio.std(ddof=1) / np.sqrt(ratio.size)))
    for name in out:
        for k in out[name]:
            out[name][k]["rel_ideal"] = out[name][k]["value"] / out["ideal"][k]["value"]
    return out


def evaluate_pair(pair: dict, models: dict, pool: dict, examples: dict) -> dict:
    _, _, prep, sig = signals(pair)
    first = first_crystal(prep)
    n_inc, p = prep["n_inc"], prep["pitch"]
    sx_ev, sy_ev = sig["ics"]["sx_ev"], sig["ics"]["sy_ev"]
    res = dict(label=c.label(pair), gamma_uuid=pair["gamma_uuid"], optical_uuid=pair["optical_uuid"],
               crystal_size_mm=pair["crystal_size_mm"], spectrum=pair["spectrum"], pitch_mm=p, n_incident=n_inc,
               freqs_1d=im.freqs_1d(p).tolist(), thresholds={})
    for T in c.THRESHOLDS:
        w = windows(prep, sig["ics"], T, first)
        assert not np.any(~w["is_ics"] & (w["n_hits"] >= 2))
        ics = np.flatnonzero(w["is_ics"])
        sub = {k: w[k][ics] for k in ("E", "t_real", "t_true", "active")}
        target = one_hot(w["target"][ics], np.ones(ics.size, bool))
        centre = np.full(w["ev"].size, CENTRE_CELL)
        counts = {"ideal": raw_counts(sig["ideal"], T), "ics": raw_counts(sig["ics"], T),
                  "grouped": corrected_counts(w, centre), "oracle": corrected_counts(w, w["target"])}
        key = (pair["crystal_size_mm"], T)
        pool.setdefault(key, {"target": []})["target"].append(target)
        chosen = {"grouped": centre, "oracle": w["target"]}
        x = model_input(sub, "real")
        for name in MODELS:
            logits = c.qcommon.predict_logits(models[name], x)
            pool[key].setdefault(name, []).append(logits.astype(np.float32))
            chosen[name] = chosen_cells(w, logits)
            counts[name] = corrected_counts(w, chosen[name])
        r = {name: count_metrics(cnt, n_inc, p, sx_ev, sy_ev) for name, cnt in counts.items()}
        for name in r:
            r[name].update(im.relative_to(r["ideal"], r[name]))
        # ICS pool, image side: chosen crystal ≠ true first crystal (the floor = first crystal not visible in window)
        fx, fy = w["fx"][ics] - w["cx"][ics] + c.HALF, w["fy"][ics] - w["cy"][ics] + c.HALF
        inside = (fx >= 0) & (fx < c.WINDOW) & (fy >= 0) & (fy < c.WINDOW)
        first_cell = np.where(inside, fx * c.WINDOW + fy, -1)
        visible = inside & w["active"][ics][np.arange(ics.size), np.clip(first_cell, 0, 24)]
        pool_info = dict(ics_photons=int(ics.size), ics_multi_hit=int((w["n_hits"][ics] >= 2).sum()),
                         first_crystal_not_visible_pct=float(100 * np.mean(~visible)),
                         hits_outside_window=int(w["out_ev"].size))
        for name, cell in chosen.items():
            pool_info[f"{name}_wrong_first_crystal_pct"] = float(100 * np.mean(cell[ics] != first_cell))
        res["thresholds"][int(T)] = dict(variants=r, ics_pool=pool_info)
        if pair["spectrum"] == IMAGE_SIM_SPECTRUM:
            res["thresholds"][int(T)]["image_sim"] = image_study(
                counts, prep, sx_ev, sy_ev, seed=int(T), examples=examples, tag=f"{c.label(pair)}_T{int(T)}")
    return res


def main():
    models = load_models()
    pool, examples, results, t0 = {}, {}, [], time.time()
    pairs = c.test_pairs()
    for k, pair in enumerate(pairs, 1):
        results.append(evaluate_pair(pair, models, pool, examples))
        print(f"[{k}/{len(pairs)}] {c.label(pair)} ({time.time() - t0:.0f} s)", flush=True)
    ml = {}
    for (size, T), d in sorted(pool.items()):
        target = np.concatenate(d["target"])
        ml[f"{size:.1f}mm_T{int(T)}"] = {name: c.qcommon.ics_metrics(np.concatenate(d[name]), target)[0]
                                        for name in MODELS} | {"ics_photons": int(target.shape[0])}
        print(f"{size:.1f} mm, {T:g} keV:", json.dumps(ml[f"{size:.1f}mm_T{int(T)}"]), flush=True)
    c.RESULTS_DIR.mkdir(exist_ok=True)
    (c.RESULTS_DIR / "metrics.json").write_text(json.dumps(dict(ml_ics_pool=ml, runs=results), indent=1, default=float))
    np.savez_compressed(c.DATA_DIR / "image_examples.npz", **examples)


if __name__ == "__main__":
    main()
