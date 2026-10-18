"""Tables and figures from results/metrics.json (scripts/evaluate.py).

Recovered share of the ICS loss for a metric M and a corrected variant v:  (M_v − M_ics) / (M_ideal − M_ics)
(1 = back to the detector without ICS, 0 = no gain), averaged over the 8 CT spectra of each detector.

Usage: /usr/bin/python3 scripts/aggregate.py   -> results/summary.json, figures/*.png
"""

import json

import matplotlib
import numpy as np

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

import s05_common as c  # noqa: E402

NAMES = {"ideal": "sans ICS (idéal)", "ics": "brut (avec ICS)", "grouped": "regroupé, cristal le + énergétique",
         "oracle": "correction parfaite (oracle)", "fp32": "small_v2 tel quel", "retrained": "small_v2 réentraîné"}
COLORS = {"ideal": "k", "ics": "tab:red", "grouped": "tab:orange", "oracle": "tab:green", "fp32": "tab:purple",
          "retrained": "tab:blue"}
CORRECTED = ("grouped", "fp32", "retrained", "oracle")
METRICS = {"mtf_half_nyq": "MTF à fN/2", "mtf_nyq": "MTF à fN", "dqe0": "DQE(0)", "cnr_rel_pw_1mm": "CNR disque 1 mm",
           "cnr_rel_pw_2mm": "CNR disque 2 mm"}
REF_SPECTRUM = "120kVp_water20"


def recovered(v: dict, key: str) -> float:
    gap = v["ideal"][key] - v["ics"][key]
    return {name: (v[name][key] - v["ics"][key]) / gap for name in CORRECTED} if abs(gap) > 1e-9 else None


def main():
    data = json.loads((c.RESULTS_DIR / "metrics.json").read_text())
    runs = data["runs"]
    summary = {"ml_ics_pool": data["ml_ics_pool"], "reference_runs": {}, "recovered_mean_over_spectra": {}}
    for size in (1.2, 2.0):
        for T in (5, 20):
            sel = [r for r in runs if r["crystal_size_mm"] == size]
            key = f"{size:.1f}mm_T{T}"
            rec = {m: {name: float(np.mean([recovered(r["thresholds"][str(T)]["variants"], m)[name] for r in sel]))
                       for name in CORRECTED} for m in METRICS}
            v = [r["thresholds"][str(T)]["variants"] for r in sel]
            rec["extra_counts_removed"] = {name: float(np.mean([1 - x[name]["extra_counts_pct"] / x["ics"]["extra_counts_pct"]
                                                                for x in v])) for name in CORRECTED}
            rec["misplaced_removed"] = {name: float(np.mean([1 - x[name]["misplaced_pct"] / x["ics"]["misplaced_pct"]
                                                             for x in v])) for name in CORRECTED}
            summary["recovered_mean_over_spectra"][key] = rec
            ref = next(r for r in sel if r["spectrum"] == REF_SPECTRUM)["thresholds"][str(T)]
            summary["reference_runs"][key] = {
                name: {k: ref["variants"][name].get(k) for k in ("extra_counts_pct", "misplaced_pct", "mtf_half_nyq",
                                                                  "mtf_nyq", "dqe0", "snr_rel", "cnr_rel_pw_1mm",
                                                                  "cnr_rel_pw_2mm", "cnr_rel_pw_5mm",
                                                                  "low_energy_counts_pct", "eff")}
                | {"timing_fwhm_ps": ref["variants"][name]["timing"].get("fwhm_ps"),
                   "timing_fwtm_ps": ref["variants"][name]["timing"].get("fwtm_ps"),
                   "image_cnr_1mm_rel": ref["image_sim"][name]["cnr_1mm"]["rel_ideal"],
                   "image_cnr_1mm_err": ref["image_sim"][name]["cnr_1mm"]["err"] / ref["image_sim"]["ideal"]["cnr_1mm"]["value"],
                   "image_cnr_2mm_rel": ref["image_sim"][name]["cnr_2mm"]["rel_ideal"]}
                for name in NAMES} | {"ics_pool": ref["ics_pool"]}
    (c.RESULTS_DIR / "summary.json").write_text(json.dumps(summary, indent=1, default=float))
    print(json.dumps(summary["recovered_mean_over_spectra"], indent=1))
    c.FIGURES_DIR.mkdir(exist_ok=True)
    figure_mtf(runs)
    figure_recovered(summary)
    figure_images(runs)
    figure_spectra(runs)


def figure_mtf(runs: list) -> None:
    fig, axes = plt.subplots(2, 2, figsize=(11, 8.5))
    for i, size in enumerate((1.2, 2.0)):
        r = next(r for r in runs if r["crystal_size_mm"] == size and r["spectrum"] == REF_SPECTRUM)
        f = np.array(r["freqs_1d"])
        fn = 0.5 / r["pitch_mm"]
        for j, T in enumerate((5, 20)):
            ax = axes[i, j]
            v = r["thresholds"][str(T)]["variants"]
            for name in NAMES:
                ax.plot(f / fn, v[name]["mtf"], color=COLORS[name], lw=1.6 if name in ("ics", "retrained") else 1.1,
                        ls="--" if name == "ideal" else "-", label=NAMES[name])
            ax.set_xlim(0, 2)
            ax.set_ylim(0, 1.02)
            ax.axvline(1, color="0.7", lw=0.8)
            ax.set_title(f"LGSO {size} mm (pas {r['pitch_mm']} mm), 120 kVp + 20 cm eau, seuil {T} keV", fontsize=9)
            ax.set_xlabel("fréquence / fréquence de Nyquist")
            ax.set_ylabel("MTF présamplée")
            ins = ax.inset_axes([0.08, 0.08, 0.42, 0.38])
            for name in NAMES:
                ins.plot(f / fn, v[name]["mtf"], color=COLORS[name], ls="--" if name == "ideal" else "-", lw=1.1)
            ins.set_xlim(0.8, 1.2)
            lo = min(np.interp(0.8, f / fn, v["ics"]["mtf"]), np.interp(1.2, f / fn, v["ics"]["mtf"]))
            ins.set_ylim(lo - 0.05, np.interp(0.8, f / fn, v["ideal"]["mtf"]) + 0.02)
            ins.tick_params(labelsize=7)
            ins.set_title("zoom autour de fN", fontsize=7)
    axes[0, 1].legend(fontsize=8, loc="upper right")
    plt.tight_layout()
    plt.savefig(c.FIGURES_DIR / "mtf.png", dpi=130)
    plt.close(fig)


def figure_recovered(summary: dict) -> None:
    keys = ["extra_counts_removed", "misplaced_removed"] + list(METRICS)
    labels = ["coups\nen trop", "coups mal\nplacés"] + [METRICS[k].replace(" ", "\n", 1) for k in METRICS]
    floor = -25.0
    fig, axes = plt.subplots(2, 2, figsize=(13, 8.6), sharey=True)
    for ax, (key, rec) in zip(axes.ravel(), summary["recovered_mean_over_spectra"].items()):
        x = np.arange(len(keys))
        for k, name in enumerate(CORRECTED):
            vals = np.array([100 * rec[m][name] for m in keys])
            xs = x + (k - 1.5) * 0.2
            ax.bar(xs, np.maximum(vals, floor), 0.2, color=COLORS[name], label=NAMES[name])
            for xi, v in zip(xs, vals):
                if v < floor:                                  # clipped bar: write its value
                    ax.text(xi, floor + 2, f"{v:.0f}", ha="center", va="bottom", fontsize=6.5, rotation=90)
        ax.axhline(100, color="k", lw=0.8, ls="--")
        ax.axhline(0, color="k", lw=0.6)
        ax.set_xticks(x)
        ax.set_xticklabels(labels, fontsize=8)
        size, T = key.split("_T")
        ax.set_title(f"LGSO {size.replace('mm', ' mm')}, seuil {T} keV — moyenne des 8 spectres CT", fontsize=9)
        ax.set_ylabel("part de la perte ICS récupérée (%)")
        ax.set_ylim(floor, 110)
    handles, names = axes[0, 0].get_legend_handles_labels()
    fig.legend(handles, names, loc="lower center", ncol=4, fontsize=9)
    plt.tight_layout(rect=(0, 0.05, 1, 1))
    plt.savefig(c.FIGURES_DIR / "recovered_share.png", dpi=130)
    plt.close(fig)


def figure_images(runs: list) -> None:
    ex = np.load(c.DATA_DIR / "image_examples.npz")
    show = ("ideal", "ics", "fp32", "retrained", "oracle")
    fig, axes = plt.subplots(4, len(show), figsize=(3.1 * len(show), 12.4))
    for i, size in enumerate((1.2, 2.0)):
        r = next(r for r in runs if r["crystal_size_mm"] == size and r["spectrum"] == REF_SPECTRUM)
        tag = f"{c.label({'crystal_size_mm': size, 'spectrum': REF_SPECTRUM})}_T5"
        ideal = ex[f"{tag}_ideal"].astype(float)
        crop = (slice(2, 34), slice(2, 34))                    # 32 x 32 pixels with 1, 2 and 5 mm disks
        vmin, vmax = np.percentile(ideal[crop], [1, 99])
        for j, name in enumerate(show):
            img = ex[f"{tag}_{name}"].astype(float)
            ax = axes[2 * i, j]
            shown = img[crop] / img.mean() * ideal.mean()          # each image on its own mean: same grey scale
            ax.imshow(shown.T, cmap="gray", vmin=vmin, vmax=vmax, origin="lower")
            cnr = r["thresholds"]["5"]["image_sim"][name]["cnr_1mm"]["rel_ideal"]
            ax.set_title(f"{NAMES[name]}\nCNR 1 mm / idéal = {cnr:.3f}", fontsize=8)
            ax.set_xticks([])
            ax.set_yticks([])
            ax = axes[2 * i + 1, j]
            d = img - ideal
            lim = np.percentile(np.abs(ex[f"{tag}_ics"].astype(float) - ideal)[crop], 99)
            ax.imshow(d[crop].T, cmap="bwr", vmin=-lim, vmax=lim, origin="lower")
            ax.set_title(f"− idéal : Σ = {int(d.sum()):+,} coups".replace(",", " "), fontsize=8)
            ax.set_xticks([])
            ax.set_yticks([])
        axes[2 * i, 0].set_ylabel(f"LGSO {size} mm, seuil 5 keV", fontsize=9)
    fig.suptitle("Fantôme à disques (contraste 10 %, 1 000 photons / pixel), 120 kVp + 20 cm eau — mêmes photons "
                 "pour toutes les variantes\nlignes 1 et 3 : chaque image ramenée à sa moyenne ; lignes 2 et 4 : "
                 "coups en plus (rouge) / en moins (bleu) par rapport à l'idéal", fontsize=10)
    plt.tight_layout()
    plt.savefig(c.FIGURES_DIR / "disk_images.png", dpi=120)
    plt.close(fig)


def figure_spectra(runs: list) -> None:
    from evaluate import E_BINS
    fig, axes = plt.subplots(1, 2, figsize=(12, 4.2))
    centres = 0.5 * (E_BINS[1:] + E_BINS[:-1])
    for ax, size in zip(axes, (1.2, 2.0)):
        r = next(r for r in runs if r["crystal_size_mm"] == size and r["spectrum"] == REF_SPECTRUM)
        v = r["thresholds"]["5"]["variants"]
        for name in ("ideal", "ics", "retrained"):
            ax.step(centres, np.array(v[name]["spectrum"]) / v["ideal"]["photons"], where="mid", color=COLORS[name],
                    label=f"{NAMES[name]} — {v[name]['low_energy_counts_pct']:.1f} % des coups < 30 keV")
        ax.set_xlabel("énergie du coup (keV)")
        ax.set_ylabel("coups par photon détecté / 2 keV")
        ax.set_title(f"LGSO {size} mm, 120 kVp + 20 cm eau, seuil 5 keV", fontsize=9)
        ax.legend(fontsize=7.5)
    plt.tight_layout()
    plt.savefig(c.FIGURES_DIR / "spectra.png", dpi=130)
    plt.close(fig)


if __name__ == "__main__":
    main()
