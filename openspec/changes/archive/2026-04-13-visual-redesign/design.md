# Design: Visual Redesign

## Benchmark References

| Principle | Source (benchmark.md §) | Application to this change |
|---|---|---|
| One hero element | Principle 1 | Variants A and B each have one dominant element owning 40-50% of screen height. Variant C intentionally relaxes this (split hero) - justified by the dashboard use case. |
| Color encodes state | Principle 2 | Zone color (green/yellow/orange/red) is the primary glucose signal in all three variants. Text labels confirm, never replace. |
| Symbols beat text | Principle 3 | Trend arrow (↗), freshness dot (●), BT icon - all symbols. Verbose rows ("Last read: N min ago") replaced with "●3m" compact format. |
| Staleness has a dedicated channel | Principle 4 | Freshness indicator is its own element in every variant (meta row or health tray). Never inferred from context. |
| Alerts are first-class | Principle 5 | Alert flash + haptic + SELECT dismiss is specified independently of layout variant. Same behavior across A/B/C. |
| Black background, always | Principle 6 | All three variants. No exceptions. |
| Quick View is a contract | Principle 8 | No existing CGM watchface handles Quick View. This is Sugar Watch's structural moat on Emery. All variants include a compact mode spec. |

**Competitive context (why existing apps fail each variant's user):**
- Urchin and nightscout-graph-pebble have no opinionated hierarchy - the user needing Variant A (beautiful time-first watch) gets no help from them.
- cgm-pebble has no color zones, no trend arrow, no accessibility labels - the user needing Variant B (glucose-first, clinical) gets the minimum viable product.
- No existing CGM watchface shows a health tray (steps, HR) alongside glucose - Variant C is a unique offering.
- T1000 has alert behavior but no Quick View, no three-variant layout, no accessibility labels.

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

### Variant A: Simple (Time-first)

**Who:** users who want a beautiful, readable watch that also happens to show glucose.
**Hero:** time. Glucose is secondary but never buried.
**Benchmark constraints:** Principles 1, 2, 3, 6, 7, 8.
**Competitor gap addressed:** TTMM proves a time-hero watch can win awards. No CGM watchface has tried this.
**Figma states:** In Range, High, Urgent High, Low, Urgent Low, Stale, Disconnected (Time 2); In Range (Round 2)

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

| Zone | Purpose | Key element |
|---|---|---|
| status-bar | BT + battery, non-intrusive | BT icon left, battery dots right |
| hero | Time dominates (~50% height) | HH:MM, largest font |
| glucose-secondary | Glucose readable, not dominant | Value medium, zone-colored + a11y label |
| trend | Same row as glucose | Arrow, same zone color |
| footer | Date + delta flanking | Date left, delta right |

**No graph in this variant.** Graph clutter conflicts with time legibility as the
primary purpose. Graph is available in Variants B and C.

---

### Variant B: Data (Glucose-first)

**Who:** CGM-primary users. The watch is worn because of the condition. Glucose must
be readable in one glance, in any situation, from any angle.
**Hero:** glucose value.
**Benchmark constraints:** Principles 1, 2, 3, 4, 5, 6, 7, 8.
**Competitor gap addressed:** cgm-pebble gives a glucose value with no color and no trend. Urchin gives a graph with no opinionated hero. This variant gives both a clear hero and a graph.
**Figma states:** In Range, High, Urgent High, Low, Urgent Low, Stale, Disconnected (Time 2); In Range (Round 2); all flash states on Alert States page

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

| Zone | Purpose | Key element |
|---|---|---|
| status-bar | BT left, time small right (not hero here) | BT icon, HH:MM small |
| hero | Glucose dominates (~30% height), zone-colored | Value LECO_42, trend arrow inline |
| meta | Delta + freshness side by side | "+0.4" left, "●3m" right, color-coded |
| separator | Visual break between live data and history | 1px DarkGray line |
| graph | 3h sparkline, threshold lines | Low/high dashed lines |
| footer | Date + time repeated for low-wrist glance | Date left, time right |

---

### Variant C: Dashboard (Activity + Health)

**Who:** users who want their health data in one view. Time and glucose share equal
weight. Complementary data (steps, HR, battery, date) is always visible.
**Hero:** split - time and glucose side by side.
**Benchmark constraints:** Principles 2, 3, 4, 5, 6, 7, 8. Principle 1 intentionally relaxed (split hero justified by dashboard use case).
**Competitor gap addressed:** No existing CGM watchface shows health data (steps, HR) alongside glucose. This variant is unique in the market.
**Figma states:** In Range, High, Urgent High, Low, Urgent Low, Stale, Disconnected (Time 2); In Range (Round 2)

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

| Zone | Purpose | Key element |
|---|---|---|
| status-bar | BT + date + battery all visible | BT left, date center, battery right |
| time-zone | Left half, medium hero | HH:MM |
| glucose-zone | Right half, medium hero | Value, trend, a11y label, delta stacked |
| graph | 2h sparkline, compressed | Shorter than Variant B to make room for tray |
| health-tray | Steps + HR + freshness | All small, same row, "--" when unavailable |

**Notes on health tray:**
- Steps and HR require Pebble Health API; show `--` when unavailable
- Freshness dot replaces a stale row for compactness
- Weather would appear here if a weather key is added in a future change (not in scope now)

---

## Alert States (all variants)

Urgent alert behavior is identical across variants (Principle 5):
- Background flashes (700ms on/off) between zone color (red) and black
- Text inverts to GColorBlack on flash-on frame for readability
- Haptic: triple long pulse (300-200-300-200-300ms) on onset
- SELECT dismisses for 15 minutes

The accessibility label (`hypo.` / `hyper.`) remains visible during alert flash - it is
the clearest text signal in a high-urgency moment.

Non-urgent (LOW/HIGH only): double short haptic pulse, no flash.

---

## Comparative Zone Summary

| Zone               | Simple (A) | Data (B) | Dashboard (C) |
|--------------------|------------|----------|---------------|
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

### Decision 3: UI-agnostic zones for Figma handoff

This document defines areas and hierarchy, not exact pixel coordinates or colors. Pixel
layout is implementation detail. Figma mockups should work from the zone descriptions
above, applying the color system (see below) and the font hierarchy as constraints.

**Font hierarchy (reference, not implementation constraint):**
- Hero text: largest available bold numeric font
- Secondary text: medium bold font
- Metadata text: small font (14-18px equivalent)
- Labels: small font, same color as parent zone element

**Color system (from Principle 2 + 6):**
- URGENT LOW / URGENT HIGH: red (#D50000)
- LOW: orange (#FF6D00)
- IN RANGE: green (#00C853)
- HIGH: yellow (#FFD600)
- Unknown / stale: light gray (#B0BEC5)
- Time, date, metadata: white (#FFFFFF)
- Background: black (#000000)

### Decision 4: Weather deferred

Weather requires a separate AppMessage key, a JS weather API call, and a display zone.
It is referenced in Variant C's health tray as a future slot but not implemented in this
change. A placeholder `[ W ]` area can be reserved in the Figma mockup.

### Decision 5: Battery indicator style left to Figma

Battery can be rendered as: dots (`···`), bars (`||||`), percentage, or icon. All are
valid Pebble conventions. The design doc specifies the zone (status bar right), not the
widget. Figma mockup should explore 2-3 options and settle one before implementation.

---

## Design Tool Link

**Figma:** https://www.figma.com/design/bKKqEkSN0q1rOdsEX8OpaE/SugarWatch-Watchface---rePebble

**Design system:** Space Grotesk Bold (hero numbers), Space Grotesk Regular (meta/labels). All frames use pure black (#000000) background with no zone-tint rectangles. Icon assets (BT states, battery levels, trend arrows, zone colors, freshness) documented on the "🎨 Components" page.

Frames generated per page:

- Page "Simple": Time 2 In Range + 6 states (High, Urgent High, Low, Urgent Low, Stale, Disconnected); Round 2 In Range
- Page "Data": Time 2 In Range + 6 states; Round 2 In Range
- Page "Dashboard": Time 2 In Range + 6 states; Round 2 In Range
- Page "Alert States": Urgent Low Flash ON + Flash OFF; Urgent High Flash ON + Flash OFF (Time 2, Data layout)
- Page "Quick View": In Range, High, Low, Urgent Low compact states (200×130)
- Page "🎨 Components": Typography comparison (Space Grotesk, Bebas Neue, DM Mono, Share Tech Mono); Icon asset library (BT, battery, trend arrows, zone colors, freshness)

---

## Selected Variant

```
Selected variant: B – Data (default)
Reason: CGM-primary users are the core audience; glucose as hero satisfies the primary job-to-be-done and is the strongest contest differentiator.
Benchmark principles satisfied: 1, 2, 3, 4, 5, 6, 7, 8 (all)
Deferred variants: A – Simple (Layout Mode 1, excellent for casual wearers who want a beautiful watch), C – Dashboard (Layout Mode 3, unique health overview with steps and HR)
```

---

## Open Questions

- **Battery indicator style:** Figma uses `●●●○` (filled/empty dots). Confirmed simpler and lower resolution. Decide final count (3 vs 4 dots) before layout.md.
- **Graph sparkline style:** Current Figma uses dots (individual ellipses). Should this be a connected line for a more TTMM feel? Decide before layout.md graph spec.
- **Sparkline color:** Always zone-colored to match current reading, or always green (in-range baseline)? Current Figma: zone-colored.
- **Health tray (Dashboard) unavailable state:** Show `--` (current Figma implementation) or hide the field entirely? `--` is less ambiguous.
- **Round 2 graph safe zone:** Graph left/right edges must stay inside inscribed circle. Verified in Figma mockups using `sx()` safe-zone calculation. Implementation must apply the same math dynamically.
- **Font choice for implementation:** Figma uses Space Grotesk as a proxy for Pebble's LECO/GOTHIC fonts. LECO is more condensed than Space Grotesk, so pixel values in layout.md will differ from Figma mockup dimensions. Treat Figma as hierarchy/color reference, not pixel reference.
