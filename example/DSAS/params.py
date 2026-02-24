# params.py
# Define your parametric study here.
# Two modes:
#   PARAM_GRID : explicit list of configs (use this for hand-picked combos)
#   build_grid : combinatorial helper (use this for full sweeps)

import itertools


def build_grid(**kwargs):
    """
    Cartesian product of all parameter lists.
    Each key maps to a list of values to sweep over.
    Example:
        build_grid(CRYSTAL_SIZE=[1.2, 1.5], CRYSTAL_DEPTH=[2.5, 3.0])
        -> 4 configs
    """
    keys = list(kwargs.keys())
    values = list(kwargs.values())
    return [dict(zip(keys, combo)) for combo in itertools.product(*values)]


# -----------------------------------------------------------------------
# BASE CONFIG  — default values for every parameter
# Anything not overridden in a sweep uses these values.
# -----------------------------------------------------------------------
BASE = {
    # --- Geometry ---
    "SIZE_MATRIX":           30,
    "CRYSTAL_MATERIAL":      "LYSO",
    "FRONT_MATERIAL":        "Bismuth",
    "REFLECTOR_MATERIAL":    "PTFE",
    "CRYSTAL_DEPTH":         2.5,
    "CRYSTAL_SIZE":          1.2,
    "REFLECTOR_DEPTH":       0.2,
    "METAL_REFLECTOR_DEPTH": 0.5,
    "PIXEL_DEPTH":           0.01,
    "METAL_FRONT_DEPTH":     0.1,
    "GLOBAL_POS_Z":          -20.0,
    "CRYSTAL_DEPTH1": 2.7,
    "GLOBAL_POS_Z1":  -20.1,
    # --- Source ---
    "SOURCE_ENERGY":         80,
    "SOURCE_ACTIVITY":       2000.0,
    "SOURCE_FREQUENCY":      50000000,
    "SOURCE_RADIUS":         0.01,
    "SOURCE_POS_X":          0.0,
    "SOURCE_POS_Y":          0.0,
    "SOURCE_POS_Z":          120.0,
    "SOURCE_DIR_X":          0.0,
    "SOURCE_DIR_Y":          0.0,
    "SOURCE_DIR_Z":          -1.0,
    "SOURCE_MINTHETA":       0.0,
    "SOURCE_MAXTHETA":       8.2,
    "SOURCE_MINPHI":         0.0,
    "SOURCE_MAXPHI":         360.0,
    "ENERGY_HIST_FILE":      "energy.txt",

    # --- Digitizer ---
    "DIGITIZER_THRESHOLD":   1,

    # --- Acquisition ---
    "TIME_SLICE":            0.001,
    "TIME_START":            0.0,
    "TIME_STOP":             1.0,

    # --- Phantom ---
    "PHANTOM_FILE":          "Dorenzo.mac",

    # --- Actor ---
    "SPAD_CONFIG_PATH":      "/app/dsas/source/externals/flatSPAD/config/example.cfg",
    # --- Random ---
    "RANDOM_SEED":           6,
}


# -----------------------------------------------------------------------
# PARAM_GRID  — list of configs to run
# Each entry only needs to specify what differs from BASE.
# -----------------------------------------------------------------------

# Option A: explicit hand-picked configs
PARAM_GRID_EXPLICIT = [
    {**BASE, "CRYSTAL_SIZE": 1.2, "CRYSTAL_DEPTH": 2.5, "REFLECTOR_MATERIAL": "PTFE"},
    {**BASE, "CRYSTAL_SIZE": 1.5, "CRYSTAL_DEPTH": 3.0, "REFLECTOR_MATERIAL": "PTFE"},
    {**BASE, "CRYSTAL_SIZE": 1.2, "CRYSTAL_DEPTH": 2.5, "REFLECTOR_MATERIAL": "ESR",
             "PHANTOM_FILE": "water_cylinder.mac"},
]

# Option B: combinatorial sweep (uncomment to use)
# PARAM_GRID_SWEEP = [
#     {**BASE, **overrides}
#     for overrides in build_grid(
#         CRYSTAL_SIZE          = [1.2, 1.5, 2.0],
#         CRYSTAL_DEPTH         = [2.5, 3.0],
#         REFLECTOR_MATERIAL    = ["PTFE", "ESR"],
#         PHANTOM_FILE          = ["Dorenzo.mac", "water_cylinder.mac"],
#         SOURCE_ENERGY         = [80, 140],
#     )
# ]

# -----------------------------------------------------------------------
# Active grid — switch between EXPLICIT and SWEEP here
# -----------------------------------------------------------------------
PARAM_GRID = PARAM_GRID_EXPLICIT
# PARAM_GRID = PARAM_GRID_SWEEP
