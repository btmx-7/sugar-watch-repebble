# UX/UI Benchmark: Sugar Watch Watchface

Saved: 2026-04-06

Two axes of reference: **CGM watchfaces** (direct competitors, same use case) and
**best-in-class Pebble UX** (design references, not CGM-specific). Both inform
Sugar Watch's visual redesign.

---

## Axis 1: CGM Watchfaces (Pebble)

### 1. Urchin CGM
- **Source:** [github.com/mddub/urchin-cgm](https://github.com/mddub/urchin-cgm)
- **Tagline:** "Unopinionated, Ridiculously Configurable Human INterface"
- **Layout:**
  - Graph dominates the screen (1h–12h of history, configurable point size/spacing)
  - Status bar (top): up to 3 stacked lines — date, uploader battery, raw readings,
    active insulin, basal rate, Loop/OpenAPS status, custom text
  - Overlays: bolus ticks, basal bar graph, predicted glucose line
  - No fixed hierarchy: users reorder zones, adjust heights, toggle borders
- **UX strengths:**
  - Graph-first matches advanced users (Loop, OpenAPS, pump wearers)
  - Gap detection in the graph itself signals stale data without an extra row
  - Connection status icons (Rig > NS > Phone > Pebble) diagnose pipeline failures
- **UX weaknesses:**
  - Zero opinionated hierarchy: no clear "hero element" out of the box
  - Configuration complexity is a barrier for non-technical users
  - Status bar text requires reading, not glancing
- **Benchmark lesson:** Maximum configurability and minimum opinions are opposites of
  each other. Sugar Watch takes the opposite position: opinionated, glanceable hierarchy.

---

### 2. nightscout-graph-pebble
- **Source:** [github.com/oamyoamy/nightscout-graph-pebble](https://github.com/oamyoamy/nightscout-graph-pebble)
- **Layout:**
  - Large graph (central)
  - Current reading prominent
  - Status bar top; time display adapts by layout variant
  - Bolus/basal overlays optional
- **UX strengths:**
  - Graph-gap method for staleness (no redundant text row)
  - Three connection-path icons cover all failure modes clearly
- **UX weaknesses:**
  - Same configurability-first philosophy as Urchin; no strong default
  - Time is secondary to graph, which is wrong for casual wearers
- **Benchmark lesson:** Connection status needs three states (phone, NS, sensor). A
  single BT icon is insufficient for CGM users who need to diagnose where data broke.

---

### 3. cgm-pebble (Nightscout official)
- **Source:** [github.com/nightscout/cgm-pebble](https://github.com/nightscout/cgm-pebble)
- **Layout:** Minimal. Glucose value + graph. No companion app required (fetches directly
  via PebbleKit JS).
- **UX strengths:** Simple, low-friction setup
- **UX weaknesses:**
  - Designed for old 144x168 Basalt screen; no color zone support
  - No trend arrow, no delta, no accessibility labels
  - Not updated for Emery (200x228, 64-color)
- **Benchmark lesson:** The baseline the community has lived with for years. Every
  capability Sugar Watch ships beyond this is a clear differentiator.

---

### 4. T1000 CGM
- **Source:** [apps.rebble.io — T1000 CGM](https://apps.rebble.io/en_US/application/6972fd68ae32660009f7c242?section=watchfaces)
- **Features:** Real-time Dexcom CGM data, configurable High/Low Soon alerts
- **UX note:** Alert-forward design (alerts are the product, display is secondary). No
  public repository; design details unavailable for deep analysis.
- **Benchmark lesson:** Alert UX is a first-class feature, not an afterthought. Users
  specifically download watchfaces for alert behavior, not display aesthetics.

---

## Axis 2: Best-in-Class Pebble UX (Design References)

### 5. TTMM
- **Source:** [ttmm.is/pebble](https://ttmm.is/pebble/) |
  [Rebble store — TTMM developer](https://apps.rebble.io/en_US/developer/6828c2bbfcffa4000710b5da/1)
- **Awards:** GOOD DESIGN Special Award (Ministry of Culture Poland, 2017) + GOOD DESIGN
  in New Technologies
- **Design philosophy:**
  - 200+ watchface models across three Pebble form factors
  - One dominant number: the current time owns 50%+ of the screen
  - Color carries meaning; labels are removed in favor of trained visual patterns
  - No chrome: borders, labels, and secondary text are stripped unless essential
  - Typography-driven: each design is a typographic experiment, not an icon set
- **Layout pattern:**
  ```
  ┌──────────────────────────┐
  │                          │
  │         HH:MM            │  ~50% screen height, hero
  │                          │
  │   [small secondary data] │  compact, metadata-only
  └──────────────────────────┘
  ```
- **Benchmark lesson:** The "one hero element" principle is the single most impactful
  lesson. Sugar Watch applies it to glucose (Variant B) and time (Variant A).

---

### 6. Dual Arc
- **Source:** rebble.io (Pebble-native design collection)
- **Design philosophy:**
  - Native Pebble visual language: arcs, dots, tick marks replace text labels
  - Status symbols at corners, never competing with the center
  - Multiple data zones coexist without hierarchy conflict when each zone has a role
  - 1–2px strokes, even-pixel grid, no sub-pixel positioning
- **Layout pattern:**
  ```
  ┌──────────────────────────┐
  │ [status]          [bt]   │  corner symbols only
  │    ◦  ◦  ◦  ◦  ◦        │  arc ticks
  │         HH:MM            │  center hero
  │    ◦  ◦  ◦  ◦  ◦        │  arc ticks
  │ [date]          [meta]   │  footer
  └──────────────────────────┘
  ```
- **Benchmark lesson:** Status indicators belong at corners and edges, not competing
  with center data. Symbols (dots, arcs) are faster to read than abbreviated text.

---

## Cross-Reference: Feature Coverage Matrix

| Feature                   | Urchin | NS-graph | cgm-pebble | T1000 | TTMM | Dual Arc |
|---------------------------|--------|----------|------------|-------|------|----------|
| Glucose hero              | No     | Partial  | Yes        | Yes   | N/A  | N/A      |
| Trend arrow               | Yes    | Yes      | No         | Yes   | N/A  | N/A      |
| Delta                     | Yes    | Yes      | No         | No    | N/A  | N/A      |
| Color zones               | Yes    | Yes      | No         | Yes   | Yes  | Yes      |
| 3h graph                  | Yes    | Yes      | Yes        | No    | No   | No       |
| Freshness indicator       | Yes    | Graph gap| No         | No    | N/A  | N/A      |
| Connection diagnostics    | Yes    | Yes      | No         | No    | N/A  | N/A      |
| Alert haptic              | Yes    | Yes      | No         | Yes   | N/A  | N/A      |
| Alert dismiss             | No     | No       | No         | No    | N/A  | N/A      |
| Quick View / compact mode | No     | No       | No         | No    | N/A  | N/A      |
| Emery (200x228) native    | No     | No       | No         | No    | No   | No       |
| One-hero hierarchy        | No     | No       | No         | No    | Yes  | Yes      |
| Accessibility label       | No     | No       | No         | No    | N/A  | N/A      |

**Sugar Watch closes every gap in this matrix.**

---

## Consolidated UX Principles (synthesis)

These are the rules that appear across multiple references and should be treated as
non-negotiable constraints on Sugar Watch's design:

1. **One hero.** Every best watchface has one dominant element that owns the center at
   the expense of everything else. Either time (Variant A) or glucose (Variant B), never both.

2. **Color encodes state, not decoration.** Zone color (green/orange/yellow/red) is the
   primary signal. Text labels confirm it, never replace it.

3. **Symbols beat text.** Dots, arcs, and arrows are read in <100ms. Abbreviated text
   ("3m", "↗") is faster than sentences ("Last read: 3 min ago").

4. **Staleness has a dedicated visual channel.** Whether via graph gaps (Urchin),
   color-coded dot (Sugar Watch), or grayed-out value, freshness must never be inferred.

5. **Alerts are first-class features.** T1000 users download watchfaces for alert
   behavior. Flash + haptic pattern + dismiss UX is product, not polish.

6. **Black background, always.** Power efficiency + maximum contrast for e-paper. No
   white or gray backgrounds in any reference.

7. **Pixel-aware construction.** 1–2px strokes, even-grid snapping. Pebble e-paper
   magnifies sloppy alignment; Dual Arc and TTMM are pixel-perfect.

8. **Quick View is a contract.** No existing CGM watchface handles Quick View. This is
   Sugar Watch's structural advantage on Emery.

---

## Sources

- [Urchin CGM — GitHub](https://github.com/mddub/urchin-cgm)
- [nightscout-graph-pebble — GitHub](https://github.com/oamyoamy/nightscout-graph-pebble)
- [cgm-pebble — GitHub](https://github.com/nightscout/cgm-pebble)
- [T1000 CGM — Rebble](https://apps.rebble.io/en_US/application/6972fd68ae32660009f7c242?section=watchfaces)
- [TTMM — ttmm.is](https://ttmm.is/pebble/)
- [TTMM — Rebble store](https://apps.rebble.io/en_US/developer/6828c2bbfcffa4000710b5da/1)
- [TTMM — designboom 2015](https://www.designboom.com/technology/albert-salamon-ttmm-pebble-smartwatch-displays-10-19-2015/)
- [TTMM — designboom 2017](https://www.designboom.com/design/ttmm-pebble-round-smartwatch-customizable-interface-08-02-2017/)
- [Nightscout wearable docs](https://nightscout.github.io/nightscout/wearable/)
