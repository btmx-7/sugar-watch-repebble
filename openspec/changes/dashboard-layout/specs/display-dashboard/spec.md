# display-dashboard Specification

## Purpose

Defines what the Dashboard layout MUST render and how it MUST respond to all
data states on Time 2 (emery) and Round 2 (gabbro). All data states — in-range,
high, urgent-high, low, urgent-low, stale, disconnected — are identical to the
Simple layout. Only the spatial arrangement of elements differs between layouts.
All pixel values reference layout.md in this change.

---

## ADDED Requirements

### Requirement: Top Widget Slots

Dashboard MUST render exactly 3 widget slots (indices 0, 1, 2) in the slots
zone. Slot index 3 MUST always be hidden. Slots MUST be hidden in compact mode.

#### Scenario: Three slots visible in normal mode

- **WHEN** layout is LAYOUT_DASHBOARD and Quick View is not active
- **THEN** `s_slot_layer[0..2]` are visible and `s_slot_layer[3]` is hidden

#### Scenario: Slots hidden in compact mode

- **WHEN** layout is LAYOUT_DASHBOARD and unobstructed height < 185 px
- **THEN** `s_slot_layer[0..2]` are hidden
- **AND** the CGM display (T2 sidebar or R2 below-graph panel) remains visible

---

### Requirement: Time Row

Dashboard MUST display the current time as a single-row string using
`FONT_KEY_LECO_36_BOLD_NUMBERS`, centered in the time zone.
Format follows `clock_is_24h_style()`.

#### Scenario: 24h time

- **WHEN** `clock_is_24h_style()` is true and the time is 14:05
- **THEN** `s_dash_time_layer` shows "14:05"

#### Scenario: 12h time

- **WHEN** `clock_is_24h_style()` is false and the time is 14:05
- **THEN** `s_dash_time_layer` shows "02:05"

---

### Requirement: BT Icon

Dashboard MUST display a Bluetooth status icon using the filled glyph when
connected and the outlined glyph when disconnected.

#### Scenario: BT connected

- **WHEN** `connection_service_peek_pebble_app_connection()` is true
- **THEN** `s_dash_bt_layer` shows `ICON_BT_CONNECTED` in `CLR_ICON_DEFAULT`

#### Scenario: BT disconnected

- **WHEN** `connection_service_peek_pebble_app_connection()` is false
- **THEN** `s_dash_bt_layer` shows `ICON_BT_DISCONNECTED` in `CLR_STATE_DISABLED`

---

### Requirement: Sparkline Graph

Dashboard MUST render the sparkline graph in the graph zone. The graph MUST
be hidden in compact mode.

#### Scenario: Graph visible in normal mode

- **WHEN** layout is LAYOUT_DASHBOARD and Quick View is not active
- **THEN** `s_graph_layer` is visible and marked dirty on each `update_display()` call

#### Scenario: Graph hidden in compact mode

- **WHEN** Quick View is active
- **THEN** `s_graph_layer` is hidden

---

### Requirement: Graph Threshold Labels

`graph_layer_update_proc` MUST render text labels alongside the high and low
threshold lines using `graphics_draw_text()`:
- "hyper" left-aligned above the high threshold line
- High threshold value (e.g. "230") right-aligned above the high threshold line
- "hypo" left-aligned below the low threshold line
- Low threshold value (e.g. "65") right-aligned below the low threshold line

All four labels use `FONT_KEY_GOTHIC_14` in `GColorLightGray`.
Labels MUST only be drawn when the threshold falls within the visible y range
(condition already guarded by the existing line-draw `if` block).

#### Scenario: Threshold labels visible when thresholds in range

- **WHEN** `s_settings.high_thresh` is 230 and falls within `[min_val, max_val]`
- **THEN** "hyper" appears left-aligned above the high threshold line
- **AND** "230" appears right-aligned at the same y position

#### Scenario: Low threshold label

- **WHEN** `s_settings.low_thresh` is 65 and falls within `[min_val, max_val]`
- **THEN** "hypo" appears left-aligned below the low threshold line
- **AND** "65" appears right-aligned at the same y position

#### Scenario: Labels absent when threshold out of visible range

- **WHEN** `s_settings.high_thresh` is outside `[min_val, max_val]`
- **THEN** no "hyper" label is drawn (consistent with existing line suppression)

---

### Requirement: CGM Zone Color

All CGM elements that carry glucose state (trend icon, glucose value) MUST use
`cgm_color` = `zone_color(zone)` when data is fresh, or `GColorLightGray` when
stale or absent. `zone_color()` MUST map zones as follows:

| Zone | Return value |
|---|---|
| ZONE_URGENT_LOW | CLR_STATE_DANGER |
| ZONE_LOW | CLR_STATE_WARNING |
| ZONE_IN_RANGE | CLR_ICON_DEFAULT |
| ZONE_HIGH | **GColorChromeYellow** |
| ZONE_URGENT_HIGH | CLR_STATE_DANGER |
| ZONE_UNKNOWN | CLR_STATE_INACTIVE |

#### Scenario: HIGH zone is yellow, distinct from LOW

- **WHEN** `s_glucose` is 220 mg/dL (ZONE_HIGH) and data is fresh
- **THEN** `cgm_color` is `GColorChromeYellow`
- **AND NOT** `CLR_STATE_WARNING` (orange)

#### Scenario: LOW zone is orange

- **WHEN** `s_glucose` is 65 mg/dL (ZONE_LOW) and data is fresh
- **THEN** `cgm_color` is `CLR_STATE_WARNING`

#### Scenario: Stale overrides zone color

- **WHEN** `data_is_stale()` is true
- **THEN** `cgm_color` is `GColorLightGray` regardless of zone

---

### Requirement: T2 CGM Sidebar

On Time 2, Dashboard MUST render a vertical CGM sidebar to the right of the
graph containing trend icon, glucose value, and unit label — in that order,
top to bottom. Frames per layout.md T2 table.

#### Scenario: T2 in-range sidebar

- **WHEN** platform is T2, `s_glucose` is 120, zone is IN_RANGE, data is fresh
- **THEN** `s_dash_trend_layer` shows the trend arrow glyph in `CLR_ICON_DEFAULT`
- **AND** `s_dash_glucose_layer` shows "120" in `CLR_ICON_DEFAULT`
- **AND** `s_dash_unit_layer` shows "mg/dL" in `GColorMediumAquamarine`

#### Scenario: T2 stale state

- **WHEN** platform is T2 and `data_is_stale()` is true
- **THEN** `s_dash_glucose_layer` shows "--" in `GColorLightGray`
- **AND** `s_dash_trend_layer` text is empty string ""

---

### Requirement: R2 Below-Graph CGM Panel

On Round 2, Dashboard MUST render the CGM panel in two rows directly below the
graph. The graph height MUST be 60 px (not 76 px) to provide space for the
panel. Frames per layout.md R2 table.

**Row 1** (y=212): trend name text (left) + unit label (right)
**Row 2** (y=226): trend icon glyph (left) + glucose value (right)

Trend name text is produced by a `trend_name()` helper returning:
`"Rise++"`, `"Rising"`, `"Rising"`, `"Flat"`, `"Falling"`, `"Falling"`,
`"Fall++"` for TREND_DOUBLE_UP through TREND_DOUBLE_DOWN respectively;
`"--"` for TREND_NONE or when stale.

`s_dash_trend_name_layer` MUST be hidden on T2 and visible on R2.

#### Scenario: R2 in-range Flat state

- **WHEN** platform is R2, `s_glucose` is 120, `s_trend` is TREND_FLAT, data is fresh
- **THEN** `s_dash_trend_name_layer` shows "Flat" (row 1 left)
- **AND** `s_dash_unit_layer` shows "mg/dL" (row 1 right, GColorMediumAquamarine)
- **AND** `s_dash_trend_layer` shows the flat arrow glyph (row 2 left, CLR_ICON_DEFAULT)
- **AND** `s_dash_glucose_layer` shows "120" (row 2 right, CLR_ICON_DEFAULT)

#### Scenario: R2 HIGH state

- **WHEN** platform is R2 and zone is ZONE_HIGH
- **THEN** trend name, trend glyph, and glucose value all use `GColorChromeYellow`

#### Scenario: R2 stale state

- **WHEN** platform is R2 and `data_is_stale()` is true
- **THEN** `s_dash_trend_name_layer` shows "--" in `CLR_STATE_INACTIVE`
- **AND** `s_dash_glucose_layer` shows "--" in `GColorLightGray`
- **AND** `s_dash_trend_layer` text is empty string ""

#### Scenario: R2 content stays within circular boundary

- **WHEN** platform is R2
- **THEN** row 2 content (x=82..180, y=226..246) does not clip against the
  inscribed circle boundary (x_min ≈ 77, x_max ≈ 183 at y=246) ✓
