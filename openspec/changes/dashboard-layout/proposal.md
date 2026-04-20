## Why

The Dashboard layout shipped as part of the full-redesign change but was never
given its own standalone spec. Three v1 quality elements — delta value, freshness
indicator, and zone accessibility label — were specified in the visual-redesign
design.md but not carried through to `update_display_dashboard()`. A fourth gap:
`ZONE_HIGH` uses the same orange token (`CLR_STATE_WARNING`) as `ZONE_LOW`,
losing the yellow differentiation called out in the Figma design system. This
change creates the authoritative spec for the Dashboard layout and closes those
four gaps.

## What Changes

- **New spec: `display-dashboard`** — defines all Dashboard layout requirements:
  zones, layers, CGM sidebar content (trend, glucose, unit, delta, freshness,
  zone label), compact mode, and data-state behavior for T2 and R2.
- **Fix: Delta value absent from CGM sidebar** — `s_delta` is received and
  persisted but never passed to a text layer in Dashboard mode. Add
  `s_dash_delta_layer` (GOTHIC_14, right-aligned, zone-colored).
- **Fix: Freshness indicator absent** — `minutes_since_last_read()` is available
  but Dashboard never displays it. Add `s_dash_fresh_layer` (GOTHIC_14, "•Xm"
  format, CLR_STATE_INACTIVE) on T2; shared position with zone label.
- **Fix: Zone accessibility label absent** — `hypo.` / `hyper.` text not rendered
  in Dashboard. Add `s_dash_zone_layer` (GOTHIC_14), visible only when out of
  range, sharing the freshness row so only one is shown at a time.
- **Fix: HIGH zone color indistinguishable from LOW** — `zone_color()` maps both
  `ZONE_HIGH` and `ZONE_LOW` to `CLR_STATE_WARNING` (orange). HIGH must use
  `GColorChromeYellow` to match the Figma design system and the graph's existing
  `high_thresh` line color.

## Platform Targets

- [x] Time 2 (emery) - 200x228 px, color e-paper, rectangular
- [x] Round 2 (gabbro) - 260x260 px, circular clip, color e-paper

## Quick View Impact

Affected. Dashboard compact mode (Quick View active) already hides the 3 top
slots and the graph. The CGM sidebar — including the new delta, freshness, and
zone label layers — MUST remain visible in compact mode.

## Contest Angle

- [x] Team Judging - creativity, cleverness, good use of new platforms, design

## Capabilities

### New Capabilities

- `display-dashboard`: Full Dashboard render path — time row, 3 top slots,
  sparkline graph, and CGM sidebar with glucose value, delta, freshness
  indicator, and zone accessibility label. Both T2 and R2.

### Modified Capabilities

- `display`: `zone_color()` HIGH mapping changed from `CLR_STATE_WARNING` to
  `GColorChromeYellow` — affects all callers (graph threshold line already uses
  `GColorChromeYellow`, so this aligns the runtime color with the drawn line).

## Impact

- `src/c/main.c`: `zone_color()` — change `ZONE_HIGH` return value
- `src/c/main.c`: `update_display_dashboard()` — add delta, freshness, zone label
- `src/c/main.c`: `prv_layout_for_bounds()` Dashboard branch — add frames for
  new layers on both T2 and R2
- `src/c/main.c`: `main_window_load()` — declare and create 3 new TextLayers
- `src/c/main.c`: `main_window_unload()` — destroy 3 new TextLayers
