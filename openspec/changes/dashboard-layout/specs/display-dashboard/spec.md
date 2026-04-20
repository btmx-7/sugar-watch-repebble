# display-dashboard Specification

## Purpose

Defines what the Dashboard layout MUST render and how it MUST respond to all
data states on Time 2 (emery) and Round 2 (gabbro). All pixel values and font
constants reference layout.md in this change.

---

## Requirements

### Requirement: Top Widget Slots

Dashboard MUST render exactly 3 widget slots (slot indices 0, 1, 2) in the
slots zone (y=0..64 on T2, y=0..80 on R2). Slot index 3 MUST be hidden.

#### Scenario: Three slots visible in normal mode

- **WHEN** the layout is LAYOUT_DASHBOARD and Quick View is not active
- **THEN** `s_slot_layer[0]`, `s_slot_layer[1]`, and `s_slot_layer[2]` are visible
- **AND** `s_slot_layer[3]` is hidden

#### Scenario: Slots hidden in compact mode

- **WHEN** the layout is LAYOUT_DASHBOARD and Quick View is active (height < 185 px)
- **THEN** all three top slots are hidden
- **AND** the CGM sidebar remains fully visible

---

### Requirement: Time Row

Dashboard MUST display the current time as a single-row "HH:MM" string using
`FONT_KEY_LECO_36_BOLD_NUMBERS`, centered horizontally in the time zone.
12h/24h format MUST follow `clock_is_24h_style()`.

#### Scenario: Time renders in 24h format

- **WHEN** `clock_is_24h_style()` is true and the time is 14:05
- **THEN** `s_dash_time_layer` shows "14:05"

#### Scenario: Time renders in 12h format

- **WHEN** `clock_is_24h_style()` is false and the time is 14:05
- **THEN** `s_dash_time_layer` shows "02:05"

---

### Requirement: BT Icon

Dashboard MUST display a Bluetooth status icon (Material Symbol) to the left
of the time, using the filled glyph when connected and the outlined glyph when
disconnected.

#### Scenario: BT connected

- **WHEN** `connection_service_peek_pebble_app_connection()` is true
- **THEN** `s_dash_bt_layer` renders `ICON_BT_CONNECTED` in `CLR_ICON_DEFAULT`

#### Scenario: BT disconnected

- **WHEN** `connection_service_peek_pebble_app_connection()` is false
- **THEN** `s_dash_bt_layer` renders `ICON_BT_DISCONNECTED` in `CLR_STATE_DISABLED`

---

### Requirement: Date Display

Dashboard MUST show the current day-of-month and month number as two stacked
GOTHIC_14 labels in `CLR_TEXT_SUBTLE`, right-aligned to the right edge of the
time zone.

#### Scenario: Date renders correctly

- **WHEN** the date is April 20
- **THEN** `s_dash_day_layer` shows "20" and `s_dash_month_layer` shows "4"

---

### Requirement: Sparkline Graph

Dashboard MUST render the sparkline graph in the left portion of the cgm-panel
zone. The graph MUST be hidden in compact mode. The graph draw proc MUST draw
threshold lines using `GColorOrange` for `low_thresh` and `GColorChromeYellow`
for `high_thresh`.

#### Scenario: Graph visible in normal mode

- **WHEN** layout is LAYOUT_DASHBOARD and Quick View is not active
- **THEN** `s_graph_layer` is visible and `layer_mark_dirty` is called on each
  `update_display()` call

#### Scenario: Graph hidden in compact mode

- **WHEN** layout is LAYOUT_DASHBOARD and Quick View is active
- **THEN** `s_graph_layer` is hidden

---

### Requirement: CGM Sidebar Zone Color

All CGM sidebar elements that carry glucose state (trend, glucose value, delta)
MUST use a single `cgm_color` derived from `zone_color(zone)` when data is
fresh, or `GColorLightGray` when stale or absent.

The `zone_color()` function MUST map glucose zones to colors as follows:

| Zone | Color | Token |
|---|---|---|
| ZONE_URGENT_LOW | `CLR_STATE_DANGER` | #FF0000 |
| ZONE_LOW | `CLR_STATE_WARNING` | #FFAA00 |
| ZONE_IN_RANGE | `CLR_ICON_DEFAULT` | #55FFFF |
| ZONE_HIGH | `GColorChromeYellow` | #FFFF00 |
| ZONE_URGENT_HIGH | `CLR_STATE_DANGER` | #FF0000 |
| ZONE_UNKNOWN | `CLR_STATE_INACTIVE` | #AAAAAA |

#### Scenario: In-range glucose color

- **WHEN** `s_glucose` is 110 mg/dL (in range) and data is fresh
- **THEN** trend, glucose, and delta layers all use `CLR_ICON_DEFAULT`

#### Scenario: HIGH zone color is distinct from LOW

- **WHEN** `s_glucose` is 220 mg/dL (high) and data is fresh
- **THEN** trend, glucose, and delta layers use `GColorChromeYellow`
- **AND** NOT `CLR_STATE_WARNING` (orange)

#### Scenario: LOW zone color

- **WHEN** `s_glucose` is 65 mg/dL (low) and data is fresh
- **THEN** trend, glucose, and delta layers use `CLR_STATE_WARNING`

#### Scenario: Stale data color override

- **WHEN** `data_is_stale()` is true (last read > 15 minutes ago)
- **THEN** trend, glucose, and delta layers all use `GColorLightGray`

---

### Requirement: Trend Arrow

Dashboard MUST display the current glucose trend as a Material Symbol glyph
in `s_dash_trend_layer`, zone-colored, right-aligned.

#### Scenario: Trend icon for SingleUp

- **WHEN** `s_trend` is `TREND_SINGLE_UP` and data is fresh
- **THEN** `s_dash_trend_layer` renders `ICON_TREND_SINGLE_UP` in `cgm_color`

#### Scenario: No trend when data absent

- **WHEN** `s_glucose` is 0
- **THEN** `s_dash_trend_layer` text is empty string ""

---

### Requirement: Glucose Value

Dashboard MUST display the current glucose value in `s_dash_glucose_layer`
using `FONT_KEY_GOTHIC_24_BOLD`. Format follows `format_glucose()` (mg/dL or
mmol/L per `s_settings.use_mmol`). Show "--" when stale or absent.

#### Scenario: In-range glucose value in mg/dL

- **WHEN** `s_glucose` is 110 and `use_mmol` is false and data is fresh
- **THEN** `s_dash_glucose_layer` shows "110" in `CLR_ICON_DEFAULT`

#### Scenario: In-range glucose value in mmol/L

- **WHEN** `s_glucose` is 110 and `use_mmol` is true and data is fresh
- **THEN** `s_dash_glucose_layer` shows "6.1" (110 × 0.0555)

#### Scenario: Stale data shows placeholder

- **WHEN** `data_is_stale()` is true
- **THEN** `s_dash_glucose_layer` shows "--" in `GColorLightGray`

---

### Requirement: Unit Label

Dashboard MUST display the active unit ("mg/dL" or "mmol/L") in
`s_dash_unit_layer` using `FONT_KEY_GOTHIC_14` in `GColorMediumAquamarine`,
right-aligned below the glucose value.

#### Scenario: Unit label matches setting

- **WHEN** `s_settings.use_mmol` is false
- **THEN** `s_dash_unit_layer` shows "mg/dL"

---

### Requirement: Delta Value

Dashboard MUST display the glucose rate-of-change (delta) in
`s_dash_delta_layer` using `FONT_KEY_GOTHIC_14`, zone-colored, right-aligned
below the unit label. Format: "+N" / "-N" in mg/dL or "+N.N" / "-N.N" in
mmol/L. Show "--" when stale or absent.

Buffer size: worst case "-399" + null = 5 bytes minimum; use char buf[8].

#### Scenario: Positive delta in mg/dL

- **WHEN** `s_delta` is +8 mg/dL and `use_mmol` is false and data is fresh
- **THEN** `s_dash_delta_layer` shows "+8"

#### Scenario: Negative delta in mmol/L

- **WHEN** `s_delta` is -8 mg/dL and `use_mmol` is true and data is fresh
- **THEN** `s_dash_delta_layer` shows "-0.4"
- **AND** does NOT show "0.-4"

#### Scenario: Delta hidden when stale

- **WHEN** `data_is_stale()` is true
- **THEN** `s_dash_delta_layer` shows "--" in `GColorLightGray`

#### Scenario: R2 delta visible

- **WHEN** the platform is Round 2 (bounds.size.w == 260)
- **THEN** `s_dash_delta_layer` frame is GRect(188,214,36,14)
- **AND** content renders without clipping at the circular boundary

---

### Requirement: Freshness Indicator

On Time 2, Dashboard MUST display a compact freshness indicator in
`s_dash_fresh_layer` (GOTHIC_14, `CLR_STATE_INACTIVE`, right-aligned) when
glucose zone is IN_RANGE. Format: "•Xm" where X = `minutes_since_last_read()`.
`s_dash_fresh_layer` MUST be hidden when the zone is out of range (zone label
takes priority at the same position).

#### Scenario: Freshness shows when in range

- **WHEN** zone is ZONE_IN_RANGE and last read was 3 minutes ago
- **THEN** `s_dash_fresh_layer` shows "•3m" and `s_dash_zone_layer` is hidden

#### Scenario: Freshness hidden when out of range

- **WHEN** zone is ZONE_HIGH or ZONE_LOW or URGENT
- **THEN** `s_dash_fresh_layer` is hidden and `s_dash_zone_layer` is visible

#### Scenario: Freshness hidden when stale

- **WHEN** `data_is_stale()` is true
- **THEN** both `s_dash_fresh_layer` and `s_dash_zone_layer` are hidden (glucose
  value and trend already communicate stale state via `GColorLightGray`)

---

### Requirement: Zone Accessibility Label

On Time 2, Dashboard MUST display a zone label in `s_dash_zone_layer` (GOTHIC_14,
zone-colored, right-aligned) when glucose is out of range. Labels:
- `ZONE_URGENT_LOW` or `ZONE_LOW`: "hypo."
- `ZONE_URGENT_HIGH` or `ZONE_HIGH`: "hyper."
- `ZONE_IN_RANGE` or `ZONE_UNKNOWN`: hidden

`s_dash_zone_layer` shares position y=210 with `s_dash_fresh_layer`; exactly one
MUST be visible at a time (or both hidden when stale).

#### Scenario: Zone label shows hypo

- **WHEN** zone is ZONE_LOW or ZONE_URGENT_LOW and data is fresh
- **THEN** `s_dash_zone_layer` shows "hypo." in `cgm_color`
- **AND** `s_dash_fresh_layer` is hidden

#### Scenario: Zone label shows hyper

- **WHEN** zone is ZONE_HIGH or ZONE_URGENT_HIGH and data is fresh
- **THEN** `s_dash_zone_layer` shows "hyper." in `cgm_color`
- **AND** `s_dash_fresh_layer` is hidden

#### Scenario: Zone label hidden when in range

- **WHEN** zone is ZONE_IN_RANGE
- **THEN** `s_dash_zone_layer` is hidden
- **AND** `s_dash_fresh_layer` is visible
