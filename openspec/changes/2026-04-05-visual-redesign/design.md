# Design: Visual Redesign

## Context

### Benchmark Analysis

**TTMM** (ttmm.is/pebble-time): one dominant number owns 50%+ of the screen. No zone
labels; color carries meaning. Everything else is metadata. Users read color faster
than words.

**Dual Arc** (rebble.io): Pebble-native visual language. Status symbols at corners,
never competing with center. Tick marks and dots replace verbose text. Multiple data
zones coexist without hierarchy conflict when each zone has a clear role.

**Shared principles across Pebble's best watchfaces:**
- Black background always. Power efficiency and maximum contrast.
- One hero element. Everything else is metadata.
- Color encodes state. Labels appear only when color alone is ambiguous.
- Pixel-aware: 1-2px strokes, elements snap to even grid coordinates.
- Pebble OS animations stay consistent with the platform (simple slide, no easing).

---

## Accessibility: Zone Labels

Color alone can fail in bright sunlight, peripheral vision, or for users with color
vision deficiencies. A short text label is additive - it confirms what color already
says, without replacing it.

**Label vocabulary:**
- `hypo.` = hypoglycemia = LOW + URGENT LOW zones
- `hyper.` = hyperglycemia = HIGH + URGENT HIGH zones
- No label = IN RANGE (green silence is the positive signal)

These two words are used in clinical and patient communities across languages. They are
short enough to fit any watch layout in GOTHIC_14 or GOTHIC_18.

**Display rule:** label appears only when out of range. When in range: label hidden
entirely. This keeps the "healthy" state clean and reduces label habituation.

---

## Three Layout Variants

The watchface ships with a single binary but a **Layout Mode** setting (1, 2, or 3)
selected in config. Each mode is a complete layout: different layer sizes, different
visual hierarchy, different information density.

---

### Variant A: Chrono Mode (Time-first)

**Who:** users who want a beautiful, readable watch that also happens to show glucose.
**Hero:** time. Glucose is secondary but never buried.

```
┌──────────────────────────────────┐
│  STATUS BAR                      │
│  [ bt ]          [ battery ···· ]│
├──────────────────────────────────┤
│                                  │
│                                  │
│         TIME HERO                │
│           HH:MM                  │
│                                  │
│                                  │
├──────────────────────────────────┤
│  GLUCOSE SECONDARY    TREND      │
│  [ value ]           [  ↗  ]    │
│  [ hypo. / hyper. ]             │
├──────────────────────────────────┤
│  DATE LEFT           DELTA RIGHT │
└──────────────────────────────────┘
```

**Zone roles:**
- `STATUS BAR` (top): BT icon left, battery indicator right (dots or bars)
- `TIME HERO` (center, ~50% screen height): time in largest available font
- `GLUCOSE SECONDARY` (below time): glucose value medium size, zone-colored, + accessibility label
- `TREND` (right of glucose): trend arrow, same zone color
- `DATE LEFT` / `DELTA RIGHT` (footer): date and delta flanking the bottom

**No graph in this variant.** Graph clutter conflicts with time legibility as the
primary purpose. Graph is available in Variants B and C.

---

### Variant B: Clinical Mode (Glucose-first)

**Who:** CGM-primary users. The watch is worn because of the condition. Glucose must
be readable in one glance, in any situation, from any angle.
**Hero:** glucose value.

```
┌──────────────────────────────────┐
│  STATUS BAR                      │
│  [ bt ]               [ HH:MM ] │
├──────────────────────────────────┤
│                                  │
│  GLUCOSE HERO            TREND   │
│  [ value ]               [  ↗ ] │
│  [ hypo. / hyper. ]             │
│                                  │
├──────────────────────────────────┤
│  DELTA               FRESHNESS   │
│  [ +0.4 ]             [ • 3m ]  │
├──────────────────────────────────┤
│ ─ ─ ─ ─  SEPARATOR  ─ ─ ─ ─ ─  │
├──────────────────────────────────┤
│                                  │
│         3H GRAPH                 │
│    (threshold lines visible)     │
│                                  │
├──────────────────────────────────┤
│  DATE LEFT            TIME RIGHT │
└──────────────────────────────────┘
```

**Zone roles:**
- `STATUS BAR` (top): BT icon left, time small right (not the hero here)
- `GLUCOSE HERO` (upper area, ~30% screen height): glucose in largest font, zone-colored
- `TREND` (right of glucose, same row): trend arrow, same zone color
- `hypo. / hyper.` (inline below value): accessibility label, only shown out of range
- `DELTA` (left of meta row): change since last reading
- `FRESHNESS` (right of meta row): `● 3m` style indicator, color-coded by staleness
- `SEPARATOR` (thin line): visual break between real-time data and history
- `3H GRAPH` (center-lower): sparkline with low/high threshold dashed lines
- `DATE / TIME` (footer): date left, time right (time repeated for quick glance when arm is down)

---

### Variant C: Activity Mode (Dashboard)

**Who:** users who want their health data in one view. Time and glucose share equal
weight. Complementary data (steps, HR, battery, date) is always visible.
**Hero:** split - time and glucose side by side.

```
┌──────────────────────────────────┐
│  STATUS BAR                      │
│  [ bt ]    [ date ]   [ batt ]  │
├──────────────────────────────────┤
│  TIME ZONE       GLUCOSE ZONE   │
│                                  │
│  [ HH:MM ]     [ value ] [↗]   │
│                [ hypo./hyper. ] │
│                [ delta ]        │
├──────────────────────────────────┤
│                                  │
│         2H GRAPH                 │
│                                  │
├──────────────────────────────────┤
│  HEALTH TRAY                     │
│  [ steps ] [ hr ] [ •freshness ]│
└──────────────────────────────────┘
```

**Zone roles:**
- `STATUS BAR` (top): BT icon, date (center), battery indicator (right)
- `TIME ZONE` (left half, upper): time, medium size
- `GLUCOSE ZONE` (right half, upper): glucose value medium, trend arrow, zone label,
  delta stacked vertically in one column
- `2H GRAPH` (center, compressed): shorter history than Variant B to make room for health tray
- `HEALTH TRAY` (bottom): steps count, heart rate, freshness dot - all small, same row

**Notes on health tray:**
- Steps and HR require Pebble Health API; show `--` when unavailable
- Freshness dot replaces a stale row for compactness
- Weather would appear here if a weather key is added in a future change (not in scope now)

---

## Alert States (all variants)

Urgent alert behavior is identical across variants:
- Background flashes (700ms on/off) between zone color (red) and black
- Text inverts to GColorBlack on flash-on frame for readability
- Haptic: triple long pulse (300-200-300-200-300ms) on onset
- SELECT dismisses for 15 minutes

The accessibility label (`hypo.` / `hyper.`) remains visible during alert flash - it is
the clearest text signal in a high-urgency moment.

Non-urgent (LOW/HIGH only): double short haptic pulse, no flash.

---

## Comparative Zone Summary

| Zone               | Variant A | Variant B | Variant C |
|--------------------|-----------|-----------|-----------|
| Time               | Hero      | Corner    | Half-hero |
| Glucose            | Secondary | Hero      | Half-hero |
| Trend arrow        | Yes       | Yes       | Yes       |
| Accessibility label| Yes       | Yes       | Yes       |
| Delta              | Footer    | Meta row  | Stacked   |
| 3h Graph           | No        | Yes       | 2h only   |
| Date               | Footer    | Footer    | Status bar|
| Battery            | Status bar| —         | Status bar|
| Steps / HR         | No        | No        | Health tray|
| Freshness indicator| —         | Meta row  | Health tray|

---

## Decisions

### Decision 1: Three variants via Layout Mode setting, one binary

A single Pebble binary with a persisted layout mode (1/2/3) is simpler than three
separate apps. The user changes mode in the config page. Implementation cost: one
additional persist key, one conditional in `prv_layout_for_bounds` and
`main_window_load`.

### Decision 2: Accessibility label hypo./hyper. only when out of range

Showing the label at all times would create habituation (ignored noise). Showing it only
when out of range makes it a signal, not wallpaper. The period after the word ("hypo.")
is intentional: it suggests abbreviation and is consistent with medical shorthand.

### Decision 3: UI-agnostic zones for PenPot handoff

This document defines areas and hierarchy, not exact pixel coordinates or colors. Pixel
layout is implementation detail. PenPot mockups should work from the zone descriptions
above, applying the color system (see below) and the font hierarchy as constraints.

**Font hierarchy (reference, not implementation constraint):**
- Hero text: largest available bold numeric font
- Secondary text: medium bold font
- Metadata text: small font (14-18px equivalent)
- Labels: small font, same color as parent zone element

**Color system:**
- URGENT LOW / URGENT HIGH: red
- LOW: orange
- IN RANGE: green
- HIGH: yellow
- Unknown / stale: light gray
- Time, date, metadata: white
- Background: black

### Decision 4: Weather deferred

Weather requires a separate AppMessage key, a JS weather API call, and a display zone.
It is referenced in Variant C's health tray as a future slot but not implemented in this
change. A placeholder `[ W ]` area can be reserved in the PenPot mockup.

### Decision 5: Battery indicator style left to PenPot

Battery can be rendered as: dots (`···`), bars (`||||`), percentage, or icon. All are
valid Pebble conventions. The design doc specifies the zone (status bar right), not the
widget. PenPot mockup should explore 2-3 options and settle one before implementation.
