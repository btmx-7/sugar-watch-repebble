# Demo fixtures (QA only)

Visual QA harness for the Steady watchface. Compiled only when `DEMO_DATA=1` is
set in the environment. Release builds (`pebble build` with no env var) do not
include any of this code.

## Files

| File | Role |
|------|------|
| `demo.h` | `DemoScenario` struct + extern table declaration |
| `demo.c` | Scenario table |
| `../main.c` (`#ifdef DEMO_DATA`) | `apply_demo_state()`, button cycler |

## Scenario table

| # | Name          | Glucose | Trend       | Delta | Layout    | Graph   | Notes                   |
|---|---------------|---------|-------------|-------|-----------|---------|-------------------------|
| 0 | `urgent_low`  | 45      | Double Down | -15   | Simple    | Falling | Red flash + 3x haptic   |
| 1 | `low`         | 65      | Single Down | -8    | Simple    | Falling | Orange zone             |
| 2 | `in_range`    | 120     | Flat        | +2    | Simple    | Wave    | Default state (green)   |
| 3 | `high`        | 195     | Single Up   | +10   | Simple    | Rising  | Amber zone              |
| 4 | `urgent_high` | 270     | Double Up   | +18   | Simple    | Rising  | Red flash + 3x haptic   |
| 5 | `stale`       | 120     | None        | 0     | Simple    | Flat    | Last read 30 min ago    |
| 6 | `dashboard`   | 142     | Flat        | +3    | Dashboard | Wave    | Alt layout + graph      |
| 7 | `zero_state`  | 0       | None        | 0     | Simple    | Flat    | No data, all slots None |

The slot loadout for states 0-6 is `{Weather, Battery, CGM, Heart Rate}`.
State 7 uses `{None, None, None, None}` to verify empty-slot rendering.

## Usage

### Interactive cycler (recommended)

```bash
DEMO_DATA=1 pebble build
pebble install --emulator emery --logs
```

On the emulator, **UP** cycles forward through states, **DOWN** cycles backward.
The current state name is logged to the `--logs` stream.

### Pin one state at compile time

```bash
DEMO_DATA=1 DEMO_STATE=4 pebble build   # urgent_high
pebble install --emulator emery
```

`DEMO_STATE` defaults to `2` (in_range).

### Screenshot sweep

```bash
./scripts/screenshot-sweep.sh                   # all 8 states, emery
PLATFORM=gabbro ./scripts/screenshot-sweep.sh   # round
STATES="0 3 4"  ./scripts/screenshot-sweep.sh   # subset
```

Outputs to `screenshots/demo/<platform>_<i>_<name>.png`.

## Adding a new scenario

1. Bump `DEMO_SCENARIO_COUNT` in `demo.h`.
2. Append a row to `demo_scenarios[]` in `demo.c`.
3. Append the short name to `NAMES=(...)` in `scripts/screenshot-sweep.sh`.
4. Update the table above.

Trend / slot / layout / graph-pattern codes are listed in the header comment of
`demo.c`.

## Release checklist

Before `pebble publish`:

```bash
pebble clean
pebble build         # NO DEMO_DATA
```

Verify `build/` has no `-DDEMO_DATA` in any compile command and no `demo/*.o`
objects. Then publish.
