"""Readout of one gamma run (study's `ics` variant) and one 5x5 sample per detected photon (small_v2 format).

signals(pair): calibrated pixel signals of the variants `ideal` and `ics` (performance.prepare / pixel_signals,
  one random stream, seed SEED) + the timestamp of every pixel above the lowest threshold (time of the first
  photoelectron, performance._timestamps with the confined-light time tables, as performance.py does for
  `ideal` / `ics`).
windows(prep, sig, T): one sample per photon with >= 1 hit >= T:
  window 5x5 crystals centred on the highest-energy hit, cell = row·5 + col (row = X offset, col = Y offset);
  x = [E (keV), T − t_min (ns), active]; empty pixels: −t_min (original convention); time channel = measured
  timestamp ("real") or GATE deposit time ("ideal", no noise);
  first crystal = earliest deposit of the photon (GATE time, before the threshold);
  is_ics = deposit in >= 2 crystals; target = visible window cell with the earliest deposit;
  hits >= T outside the window are returned apart (they stay separate counts in the corrected image).
"""

import numpy as np

import s05_common as c

R = c.perf.OPT_RADIUS


def time_tables(tt: dict, material: str, nz: int) -> tuple:
    """Confined-light CDF of the detection time (as performance.performance for ideal / ics)."""
    kin = dict(c.timing.KINETICS[material])
    grid, G = c.timing.photon_time_tables(tt["t_counts"], tt["time_edges_ns"], kin, c.timing.SPTR_FWHM_NS)
    G = G.reshape(nz, (2 * R + 1) ** 2, grid.size)
    own = R * (2 * R + 1) + R
    G_conf = np.repeat(G[:, own:own + 1, :], (2 * R + 1) ** 2, axis=1)
    return grid, G_conf, np.gradient(G_conf, grid, axis=2).astype(np.float32)


def signals(pair: dict, seed: int = c.SEED) -> tuple:
    gcfg, g, aux, A, tt = c.load_pair(pair)
    prep = c.perf.prepare(gcfg, g, aux, A)
    grid, G_conf, dG = time_tables(tt, gcfg["material"], prep["nz"])
    rng = np.random.default_rng(seed)
    out = {}
    for name in ("ideal", "ics"):
        sig = c.perf.pixel_signals(name, prep, rng)
        assert sig["ent"]["key"].size == sig["pev"].size          # confined light: one entry per pixel
        row = np.empty(sig["pev"].size, np.int64)
        row[sig["kidx"]] = sig["ent"]["row"]
        sel = np.flatnonzero((sig["e"] >= min(c.THRESHOLDS)) & (sig["npe"] >= c.timing.TRIGGER_RANK))
        t = np.full(sig["e"].size, np.nan)
        t[sel] = c.perf._timestamps(sel, sig["ent"], sig["kidx"], G_conf, dG, sig["rw"]["t"], sig["npe"],
                                    sig["lam_tot"], grid, rng, c.timing.TRIGGER_RANK)
        sig["row"], sig["t_ns"], sig["t_true_ns"] = row, t, sig["rw"]["t"][row] * 1e-3
        out[name] = sig
    return gcfg, g, prep, out


def first_crystal(prep: dict) -> dict:
    """Per event id: crystal of the earliest deposit (GATE time, before threshold) and number of crystals hit."""
    rows = prep["rows"]
    n_ev = int(rows["ev"].max()) + 1
    order = np.lexsort((rows["t"], rows["ev"]))
    ev_s = rows["ev"][order]
    first = order[np.flatnonzero(np.r_[True, ev_s[1:] != ev_s[:-1]])]
    fx, fy = np.zeros(n_ev, int), np.zeros(n_ev, int)
    fx[rows["ev"][first]], fy[rows["ev"][first]] = rows["rx"][first], rows["ry"][first]
    return dict(fx=fx, fy=fy, first_row=first, n_dep=np.bincount(rows["ev"], minlength=n_ev))


def windows(prep: dict, sig: dict, T: float, first: dict) -> dict:
    e, pev, px, py = sig["e"], sig["pev"], sig["px"], sig["py"]
    vis = np.flatnonzero(e >= T)
    order = vis[np.lexsort((-e[vis], pev[vis]))]
    starts = np.flatnonzero(np.r_[True, pev[order][1:] != pev[order][:-1]])
    centre = order[starts]                                        # highest-energy hit of each photon
    ev = pev[centre]
    k = np.repeat(np.arange(ev.size), np.diff(np.r_[starts, order.size]))   # photon index of every visible hit
    row, col = px[order] - px[centre][k] + c.HALF, py[order] - py[centre][k] + c.HALF
    inside = (row >= 0) & (row < c.WINDOW) & (col >= 0) & (col < c.WINDOW)
    cell = row * c.WINDOW + col
    n, n_cell = ev.size, c.WINDOW * c.WINDOW
    E = np.zeros((n, n_cell), np.float32)
    t_real = np.zeros((n, n_cell))
    t_true = np.full((n, n_cell), np.inf)
    E[k[inside], cell[inside]] = e[order][inside]
    t_real[k[inside], cell[inside]] = sig["t_ns"][order][inside]
    t_true[k[inside], cell[inside]] = sig["t_true_ns"][order][inside]
    active = E > 0
    assert np.isfinite(t_real[active]).all()
    out = dict(ev=ev, cx=px[centre], cy=py[centre], E=E, active=active, t_real=t_real,
               t_true=np.where(active, t_true, 0.0), n_hits=active.sum(axis=1),
               target=np.where(active, t_true, np.inf).argmin(axis=1),
               is_ics=first["n_dep"][ev] >= 2, fx=first["fx"][ev], fy=first["fy"][ev],
               e_window=E.sum(axis=1), out_ev=k[~inside], out_px=px[order][~inside], out_py=py[order][~inside],
               out_e=e[order][~inside], out_t=sig["t_ns"][order][~inside])
    return out


def model_input(w: dict, time: str = "real") -> np.ndarray:
    """x (n, 3, 5, 5) float32 = [E, T − t_min, active], empty pixels −t_min (as photon_events.build)."""
    t = (w["t_real"] if time == "real" else w["t_true"]).astype(np.float32)
    t = np.where(w["active"], t, 0.0).astype(np.float32)
    t_min = np.where(w["active"], t, np.inf).min(axis=1).astype(np.float32)
    x = np.stack([w["E"], t - t_min[:, None], w["active"].astype(np.float32)], axis=1)
    return x.reshape(len(x), 3, c.WINDOW, c.WINDOW)


def one_hot(target: np.ndarray, is_ics: np.ndarray) -> np.ndarray:
    y = np.zeros((target.size, c.WINDOW * c.WINDOW), np.float32)
    y[np.flatnonzero(is_ics), target[is_ics]] = 1.0
    return y
