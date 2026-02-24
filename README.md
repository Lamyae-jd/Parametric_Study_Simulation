# Parametric_Study_Gate_Simulation

A Python orchestration framework for running automated parametric studies with [GATE Monte Carlo simulation](http://www.opengatecollaboration.org/). Instead of manually editing `.mac` files for each simulation configuration, this pipeline generates run directories, fills macro templates, and launches GATE sequentially, keeping your results organized and reproducible.

GATE is coupled to [DSAS](https://github.com/DigitalSpadArraySimulator/DigitalSpadArraySimulator) in the configuration. DSAS emulate the behavior of SiPMs.  

---

## Why this tool?

GATE simulations involve multiple interdependent macro files. Running a parametric study by hand means editing files, renaming outputs, and tracking configurations: a process that is error-prone and hard to reproduce. This pipeline automates all of that:

- Define all your configurations once in `params.py`
- Run a single command to launch the full study
- Every run gets its own directory with its parameters saved as `params.json`
- Resume interrupted studies with `--start-from`

---

## Requirements

- Python 3.8+
- [GATE](http://www.opengatecollaboration.org/) installed and accessible as `Gate` in your `PATH`
- No external Python libraries required (uses only the standard library)

Verify your GATE installation:
```bash
Gate --version
```

---

## Project Structure

```
Parametric_Study_Gate_Simulation/example/DSAS/
├── templates/                  # Your .mac files with {{PLACEHOLDER}} tokens
│   ├── main.mac                # Entry point called by Gate
│   ├── geometry.mac
│   ├── source.mac
│   ├── digitizer.mac
│   ├── Visualisation.mac       # Static (no placeholders, copied as-is)
│   └── Verbose.mac             # Static
│
├── runs/                       # Auto-generated — one folder per simulation run
│   ├── run_001/
│   │   ├── main.mac            # Filled macro files for this run
│   │   ├── geometry.mac
│   │   ├── params.json         # Exact parameters used (traceability)
│   │   └── output/             # GATE output files
│   ├── run_002/
│   └── ...
│
├── params.py                   # Parameter grid definition
└── run_study.py                # Main orchestrator script
```

---

## How It Works

### 1. Convert your macros to templates

Replace hardcoded values in your `.mac` files with `{{PLACEHOLDER}}` tokens:

```mac
# geometry.mac
/gate/crystal/geometry/setXLength   {{CRYSTAL_SIZE}} mm
/gate/crystal/geometry/setYLength   {{CRYSTAL_SIZE}} mm
/gate/crystal/geometry/setZLength   {{CRYSTAL_DEPTH}} mm
/gate/reflector/setMaterial         {{REFLECTOR_MATERIAL}}
```

```mac
# source.mac
/gate/source/mySource/setEnergy     {{SOURCE_ENERGY}} keV
/gate/source/mySource/setActivity   {{SOURCE_ACTIVITY}} becquerel
```

```mac
# digitizer.mac
/gate/digitizer/Singles/energyWindow/setMin  {{EWIN_MIN}} keV
/gate/digitizer/Singles/energyWindow/setMax  {{EWIN_MAX}} keV
```

```mac
# main.mac
/control/execute {{PHANTOM_FILE}}
```

Place all template macros in the `templates/` directory. Files listed in `STATIC_MACROS` inside `run_study.py` are copied without modification (e.g. `Visualisation.mac`, `Verbose.mac`).

---

### 2. Define your parameter grid (`params.py`)

You have two modes: **explicit** (hand-picked configs) or **combinatorial sweep**.

**Start by defining shared defaults in `BASE`:**
```python
# params.py

BASE = {
    "CRYSTAL_SIZE":       1.5,
    "CRYSTAL_DEPTH":      3.0,
    "REFLECTOR_MATERIAL": "PTFE",
    "SOURCE_ENERGY":      511,
    "SOURCE_ACTIVITY":    1000000,
    "EWIN_MIN":           400,
    "EWIN_MAX":           650,
    "PHANTOM_FILE":       "phantoms/nema.mac",
}
```

**Option A — Explicit configs (hand-picked):**
```python
PARAM_GRID = [
    {**BASE, "CRYSTAL_SIZE": 1.2, "CRYSTAL_DEPTH": 2.5, "REFLECTOR_MATERIAL": "PTFE"},
    {**BASE, "CRYSTAL_SIZE": 1.5, "CRYSTAL_DEPTH": 3.0, "REFLECTOR_MATERIAL": "ESR"},
    {**BASE, "CRYSTAL_SIZE": 2.0, "CRYSTAL_DEPTH": 3.0, "REFLECTOR_MATERIAL": "ESR",
             "PHANTOM_FILE": "phantoms/water_cylinder.mac"},
]
```

**Option B — Combinatorial sweep (all combinations):**
```python
from run_study import build_grid

PARAM_GRID = [
    {**BASE, **overrides}
    for overrides in build_grid(
        CRYSTAL_SIZE       = [1.2, 1.5, 2.0],
        CRYSTAL_DEPTH      = [2.5, 3.0],
        REFLECTOR_MATERIAL = ["PTFE", "ESR"],
        SOURCE_ENERGY      = [511],
        EWIN_MIN           = [400],
        EWIN_MAX           = [650],
        PHANTOM_FILE       = ["phantoms/nema.mac", "phantoms/water_cylinder.mac"],
    )
]
```

`build_grid()` computes the full Cartesian product of all parameter lists. For the example above, that is 2 × 3 × 2 × 2 = 24 configurations.

---

### 3. Run the study

```bash
# Run all configurations
python run_study.py

# Preview without launching Gate (generates run dirs and filled macros only)
python run_study.py --dry-run

# Resume from a specific run index (e.g., if run_005 crashed)
python run_study.py --start-from 5
```

The script will:
1. Create `runs/run_001/`, `runs/run_002/`, etc.
2. Fill all template macros with the corresponding parameters
3. Raise a clear error if any `{{PLACEHOLDER}}` was left unfilled
4. Save `params.json` in each run directory
5. Launch `Gate main.mac` from within each run directory
6. Print a pass/fail summary at the end

---

## Output Structure

Each run directory is self-contained:

```
runs/run_001/
├── main.mac            # Filled macros (can be re-run manually)
├── geometry.mac
├── source.mac
├── digitizer.mac
├── Visualisation.mac
├── Verbose.mac
├── params.json         # Exact parameters used for this run
└── output/             # All Gate output files (ROOT, txt, etc.)
```

`params.json` example:
```json
{
  "CRYSTAL_SIZE": 1.5,
  "CRYSTAL_DEPTH": 3.0,
  "REFLECTOR_MATERIAL": "PTFE",
  "SOURCE_ENERGY": 511,
  "SOURCE_ACTIVITY": 1000000,
  "EWIN_MIN": 400,
  "EWIN_MAX": 650,
  "PHANTOM_FILE": "phantoms/nema.mac"
}
```

These files are particularly useful when writing post-processing scripts — load `params.json` alongside the ROOT output to know exactly what configuration produced each result.

---

## Debugging

**Unfilled placeholder error**
The orchestrator raises a `ValueError` listing which `{{TOKEN}}` was not resolved. Check that the key exists in your `params.py` dict with the exact same spelling (case-sensitive).

**Gate not found**
Make sure `Gate` is in your `PATH`. Verify with `which Gate`. If using an environment module, load it before running the script.

**Run failed mid-study**
Use `--start-from N` to skip already-completed runs. Each `runs/run_XXX/` directory is self-contained and can also be re-run manually:
```bash
cd runs/run_005
Gate main.mac
```

**Saving a full log**
Redirect stdout and stderr to a file:
```bash
python run_study.py > study.log 2>&1
```

---

## Contributing

Contributions are welcome. To add support for parallel execution, HPC job submission, or new parameter types, open an issue or submit a pull request.
