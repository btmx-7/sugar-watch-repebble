# Proposal: Visual Redesign

## Why

The current layout distributes visual weight evenly across seven text layers, competing
for attention: glucose value, trend arrow, delta, zone label, time, date, stale row.
Nothing dominates. On a medical device worn on the wrist, the glucose value must be
readable at a glance - instantly, from any angle, in motion.

The benchmark analysis (TTMM, Dual Arc) confirms what Pebble's best watchfaces share:
one dominant number, minimal chrome, color carries meaning, words are removed in favour
of trained visual patterns.

## What Changes

- Glucose value promoted to full hero block with a larger font (LECO_42)
- Zone text label removed: color alone communicates zone, consistent with Pebble conventions
- Trend arrow relocated inline with glucose (same row, right side), larger font
- Delta and freshness consolidated on a single compact row below the hero block
- A 1px separator line draws a visual break between the data block and the graph
- Graph gains 12px of additional height from the space freed by removing the zone label
- Time and date move to the bottom bar (left/right split), freeing the top bar to breathe
- BT icon remains top-left; top-right is empty except when BT is disconnected (symmetry)
- Freshness indicator replaces the verbose "Last read: N min ago" with a compact "Nm" dot
- Quick View compact mode shows hero block only (glucose + trend + delta), graph hidden

## Capabilities

### Changed Capabilities

- `display-glucose`: Glucose font promoted from LECO_38 to LECO_42; larger visual footprint
- `display-trend`: Trend arrow moved to hero row, font promoted to GOTHIC_28_BOLD
- `display-zone`: Zone label layer removed; zone is communicated only through color
- `display-freshness`: Stale row replaced with compact "Nm ago" + zone-colored staleness dot
- `display-layout`: New 6-zone layout (status / hero / meta / separator / graph / footer)
- `display-quick-view`: Compact fallback hides graph and footer; shows hero + meta only

### Unchanged Capabilities

- `alert-flash`: Urgent alert behavior (background flash + haptic) unchanged
- `color-zones`: Zone color scheme (green/orange/yellow/red) unchanged
- `graph-sparkline`: Graph rendering logic unchanged; gains 12px height
- `bluetooth-status`: BT icon behavior unchanged

## Impact

- `src/c/main.c`: Layout, font constants, layer creation, removed zone label layer
- No JS or settings changes required
