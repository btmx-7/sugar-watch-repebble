## Why

The Dashboard layout shipped without an authoritative spec, and three gaps exist
between the current implementation and the Figma design. The sparkline graph
draws threshold lines but never renders the "hyper"/"hypo" labels or threshold
values that appear in both T2 and R2 frames. On Round 2, the CGM panel (trend,
glucose, unit) is positioned to the right of the graph in code but must sit
below the graph per Figma — a fundamentally different layout. And `zone_color()`
maps both `ZONE_LOW` and `ZONE_HIGH` to orange, losing the yellow differentiation
the Figma design system requires for HIGH.

## What Changes

- **New spec: `display-dashboard`** — authoritative requirements for the Dashboard
  layout on T2 and R2. State behavior (in-range, high, urgent, low, stale,
  disconnected) is identical to Simple; only the spatial arrangement of elements
  differs.
- **Fix: Graph threshold labels absent** — `graph_layer_update_proc` draws
  threshold lines but no text. Add "hyper" + high-threshold value above the
  high line, and "hypo" + low-threshold value below the low line, drawn in
  `FONT_KEY_GOTHIC_14` / `GColorLightGray` inside the graph layer on both
  platforms.
- **Fix: R2 CGM layout** — Round 2 must render the CGM panel BELOW the graph,
  not beside it. Requires reducing the R2 graph height (76 → 60 px) and
  repositioning trend, glucose, and unit layers into a 2-row below-graph panel:
  row 1 = trend name text ("Flat", "Rising", …) + unit label; row 2 = trend
  icon + glucose value. A new `s_dash_trend_name_layer` is required for the
  trend-name text (R2 only; hidden on T2).
- **Fix: HIGH zone color** — `zone_color()` returns `CLR_STATE_WARNING` (orange)
  for `ZONE_HIGH`. Must return `GColorChromeYellow` to match Figma and align
  with the graph's existing `high_thresh` line color.

## Platform Targets

- [x] Time 2 (emery) - 200x228 px, color e-paper, rectangular
- [x] Round 2 (gabbro) - 260x260 px, circular clip, color e-paper

## Quick View Impact

Affected. Compact mode already hides the 3 top slots and the graph. The T2 CGM
sidebar and R2 below-graph panel must remain visible in compact mode.

## Contest Angle

- [x] Team Judging - design fidelity on both new platforms

## Capabilities

### New Capabilities

- `display-dashboard`: Full Dashboard layout on T2 and R2 — time row, 3 top
  slots, annotated sparkline graph (with threshold labels), and CGM display
  (right sidebar on T2 / below-graph 2-row panel on R2). All data states match
  Simple layout.

### Modified Capabilities

- `display`: `zone_color()` HIGH mapping changed from `CLR_STATE_WARNING` to
  `GColorChromeYellow`. Affects sidebar zone coloring, slot CGM arc, and graph
  data-point coloring for high-zone readings.

## Impact

- `src/c/main.c`: `zone_color()` — change `ZONE_HIGH` return value
- `src/c/main.c`: `graph_layer_update_proc` — add threshold label rendering
- `src/c/main.c`: `main_window_load()` / `main_window_unload()` — add and
  destroy `s_dash_trend_name_layer`
- `src/c/main.c`: `update_display_dashboard()` — populate trend name layer on R2
- `src/c/main.c`: `prv_layout_for_bounds()` Dashboard R2 branch — reduce graph
  height, reposition CGM layers below graph, frame trend name layer
