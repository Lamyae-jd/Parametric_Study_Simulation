# Gate Parametric Study

## Project structure

```
gate_parametric/
├── templates/
│   ├── main.mac              # Entry point template
│   └── macro/
│       ├── Geometry.mac      # Top-level aliases injected by Python, derived math by Gate
│       ├── Source.mac        # All source parameters templated
│       ├── actor.mac         # Per-run output paths
│       ├── StartDAQ.mac      # Acquisition timing
│       └── Visualisation.mac # Static (copied as-is)
├── params.py                 # Parameter grid — edit this file
├── run_study.py              # Orchestrator — run this
└── runs/                     # Auto-generated, one folder per run
    ├── run_000/
    │   ├── main.mac          # Filled macro (ready to inspect)
    │   ├── macro/            # Filled sub-macros
    │   ├── output/           # Gate output files (.root, ascii)
    │   ├── params.json       # Exact parameters used for this run
    │   └── gate.log          # Full Gate stdout/stderr
    ├── run_001/
    └── study_summary.json    # Status of all runs
```

## Placeholder convention

- `{{DOUBLE_BRACES}}` — filled by Python before Gate runs
- `{single_braces}` — Gate's own alias system, untouched by Python

These two syntaxes coexist without collision in Geometry.mac.

## Usage

### 1. Edit your parameter grid

Open `params.py` and modify `PARAM_GRID_EXPLICIT` for hand-picked configs,
or uncomment and configure `PARAM_GRID_SWEEP` for a full combinatorial sweep.

### 2. Dry run first (always recommended)

```bash
python run_study.py --dry-run
```

This generates all run directories and fills all macros WITHOUT launching Gate.
Inspect `runs/run_000/main.mac` and `runs/run_000/macro/Geometry.mac` to verify
that all placeholders were correctly replaced before committing to a full run.

### 3. Run the study

```bash
python run_study.py
```

### 4. Resume after a partial run

```bash
python run_study.py --start-from 5   # skip run_000 to run_004
```

## Debugging

- **Gate errors**: check `runs/run_NNN/gate.log` — all Gate output is captured there
- **Missing placeholder**: the orchestrator raises immediately before Gate runs,
  showing exactly which placeholder was not found in params
- **Failed runs**: listed at the end of the run and in `runs/study_summary.json`
- **Verify a single config**: set `PARAM_GRID = PARAM_GRID_EXPLICIT[:1]` and do a dry run

## Adding a new parameter

1. Add `{{NEW_PARAM}}` in the relevant template `.mac` file
2. Add `"NEW_PARAM": default_value` to `BASE` in `params.py`
3. Override it in your configs as needed
