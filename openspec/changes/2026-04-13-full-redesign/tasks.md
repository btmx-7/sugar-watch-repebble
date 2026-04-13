<!-- Full redesign: Simple + Dashboard layouts, widget slots, bitmap fonts, Health API, Weather -->
<!-- Source of truth for pixel values: Figma https://www.figma.com/design/bKKqEkSN0q1rOdsEX8OpaE -->
<!-- Reference node IDs: Simple T2 Dark = 237:14165, Dashboard T2 Dark = 237:20791 -->

## 0. Font resources (blocking — must ship before any C work)

- [ ] 0.1 Download `Inter[wght].ttf` from Google Fonts (SIL OFL license)
- [ ] 0.2 Extract `InterBlack.ttf` (weight=900 static) or use variable font with wght=900
- [ ] 0.3 Add `resources/fonts/InterBlack.ttf` to project
- [ ] 0.4 Add font resource to `package.json` at 64px, digits only:
  ```json
  { "type": "font", "name": "RESOURCE_ID_TIME_DIGITS_64",
    "file": "fonts/InterBlack.ttf", "characterRegex": "[0-9]", "trackingAdjust": -2 }
  ```
- [ ] 0.5 Download `MaterialSymbolsRounded-Regular.ttf` (static) from google/material-design-icons or Google Fonts
- [ ] 0.6 Add `resources/fonts/MaterialSymbolsRounded.ttf` to project
- [ ] 0.7 Identify exact Unicode codepoints from the codepoints file for:
  bluetooth_connected, battery_android_3, favorite, arrow_forward,
  arrow_upward, arrow_downward, trending_up, trending_down,
  music_note, clear_day, partly_cloudy_day, cloud, rainy,
  thunderstorm, ac_unit, foggy, directions_walk
- [ ] 0.8 Add font resource to `package.json`:
  ```json
  { "type": "font", "name": "RESOURCE_ID_MATERIAL_SYMBOLS_16",
    "file": "fonts/MaterialSymbolsRounded.ttf",
    "characterRegex": "[<UTF-8 hex escapes for required codepoints>]",
    "trackingAdjust": 0 }
  ```
- [ ] 0.9 `pebble build` — verify both fonts load without warnings
- [ ] 0.10 Define C string constants for each icon glyph (UTF-8 encoded) in a `icons.h` section of main.c

## 1. Architecture additions (`src/c/main.c`)

- [ ] 1.1 Add `WatchLayout` enum: `LAYOUT_SIMPLE = 0`, `LAYOUT_DASHBOARD = 1`
- [ ] 1.2 Add `SlotType` enum: `SLOT_NONE`, `SLOT_BATTERY`, `SLOT_WEATHER`, `SLOT_HEART_RATE`, `SLOT_STEPS`, `SLOT_CGM`
- [ ] 1.3 Extend settings struct with `WatchLayout layout` and `SlotType slots[4]`
- [ ] 1.4 Add new data state statics: `s_weather_temp` (int8_t, INT8_MIN = unavailable), `s_weather_icon_id` (uint8_t), `s_heart_rate_bpm` (int), `s_step_count` (uint32_t)
- [ ] 1.5 Add new AppMessage keys to `package.json` appKeys: `KEY_WEATHER_TEMP`, `KEY_WEATHER_ICON`, `KEY_LAYOUT`, `KEY_SLOT_0`, `KEY_SLOT_1`, `KEY_SLOT_2`, `KEY_SLOT_3`
- [ ] 1.6 Load new keys in `inbox_received_handler` (weather temp + icon, layout, slots 0-3)
- [ ] 1.7 Save/load `layout` and `slots[4]` in persist read/write functions
- [ ] 1.8 Subscribe to Pebble Health API in `window_load`:
  `health_service_events_subscribe(prv_health_handler, NULL)`
- [ ] 1.9 Implement `prv_health_handler`: update `s_heart_rate_bpm` and `s_step_count`, call `update_display()`
- [ ] 1.10 Unsubscribe from Health API in `window_unload`:
  `health_service_events_unsubscribe()`

## 2. Widget slot draw system

- [ ] 2.1 Define `SlotRenderData` struct: `SlotType type`, `int value_normalized` (0-100), `char value_str[8]`, `char unit_str[8]`, `const char *icon_glyph`, `GColor icon_color`
- [ ] 2.2 Implement `prv_populate_slot_data(SlotRenderData *d, SlotType type)`:
  - Battery: value_normalized = battery%, value_str = "NN", unit_str = "%", icon = ICON_BATTERY_*, icon_color varies by level
  - Weather: value_normalized = 0, value_str = temp string, unit_str = "°C", icon = weather icon by icon_id
  - Heart Rate: value_normalized = (hr - 40) * 100 / 160 clamped, value_str = bpm string, unit_str = "bpm", icon = ICON_FAVORITE
  - Steps: value_normalized = step_count * 100 / 10000 clamped, value_str = step string, unit_str = "steps", icon = ICON_WALK
  - CGM: value_normalized = glucose_mg * 100 / 300 clamped, value_str = glucose string, unit_str = unit string, icon = trend arrow glyph, icon_color = zone color
  - SLOT_NONE: all empty, no draw
- [ ] 2.3 Implement `slot_update_proc(Layer *layer, GContext *ctx)`:
  - Read `SlotRenderData *d = layer_get_data(layer)`
  - Draw background track arc: GColorDarkGray, 3px, 240° (150° to 390°)
  - Draw active arc: GColorCyan, 3px, 150° to (150 + d->value_normalized * 240 / 100)°
  - Draw icon TextLayer content directly in proc (or use a child TextLayer)
  - Draw value text (GOTHIC_24_BOLD) centered
  - Draw unit text (GOTHIC_14) below value
- [ ] 2.4 Allocate slot layers with `layer_create_with_data(frame, sizeof(SlotRenderData))` — 4 instances max
- [ ] 2.5 Implement `prv_update_all_slots()`: calls `prv_populate_slot_data()` for each assigned slot, calls `layer_mark_dirty()` on each slot layer
- [ ] 2.6 Call `prv_update_all_slots()` from `update_display()`

## 3. Simple layout — T2 (200×228)

- [ ] 3.1 Declare layer statics for Simple: `s_hours_d1_layer`, `s_hours_d2_layer`, `s_minutes_d1_layer`, `s_minutes_d2_layer` (TextLayer, custom 64px font), `s_day_label_layer`, `s_month_label_layer` (TextLayer GOTHIC_14), `s_bt_center_layer` (TextLayer, Material Symbols 16px), `s_music_layer` (TextLayer, Material Symbols 16px, hidden by default)
- [ ] 3.2 Create Simple layout layers in `window_load` (inside `if (s_settings.layout == LAYOUT_SIMPLE)` block)
  - 4 digit layers: GRect(28, 58, 68, 68), GRect(100, 58, 68, 68), GRect(28, 126, 68, 68), GRect(100, 126, 68, 68)
  - Day: GRect(2, 102, 20, 16), right-aligned, GOTHIC_14, GColorMediumAquamarine
  - Month: GRect(178, 102, 20, 16), left-aligned, GOTHIC_14, GColorMediumAquamarine
  - BT icon: GRect(92, 4, 16, 16), Material Symbols 16px, GColorCyan
  - Music icon: GRect(92, 208, 16, 16), Material Symbols 16px, hidden
- [ ] 3.3 Update `update_display()` Simple branch:
  - Extract hours tens/units and minutes tens/units digits as single-char strings
  - Set colors: hours_d1 = GColorMediumAquamarine, hours_d2 = GColorWhite, minutes_d1 = GColorWhite, minutes_d2 = GColorCyan
  - Set day text (day of month), month text (month number)
  - Set BT icon glyph (connected vs disconnected)
- [ ] 3.4 Update `prv_layout_for_bounds()` Simple branch: compute slot frame GRects based on bounds (T2: TL=4,4 TR=140,4 BL=4,168 BR=140,168; R2: adjust to inscribed circle)
- [ ] 3.5 Destroy Simple-specific layers in `window_unload` (reverse creation order)

## 4. Dashboard layout — T2 (200×228)

Before implementation: call `get_design_context` on Figma node `237:20792` to confirm exact pixel positions for top slots, time row, and graph panel frames.

- [ ] 4.1 Declare layer statics for Dashboard: `s_dash_time_layer` (TextLayer, custom 64px font, 1-row HH:MM), `s_dash_day_layer`, `s_dash_month_layer` (GOTHIC_14), `s_dash_bt_layer` (Material Symbols), `s_dash_trend_label_layer` (TextLayer, GOTHIC_18_BOLD), `s_dash_glucose_panel_layer` (TextLayer, GOTHIC_24_BOLD)
- [ ] 4.2 Create Dashboard layout layers in `window_load` (inside `if (s_settings.layout == LAYOUT_DASHBOARD)` block)
  - Top 3 slots: GRect(4,4,56,46), GRect(72,4,56,46), GRect(140,4,56,46) — slot system from Task 2
  - Time: GRect(28, 56, 144, 60), custom 64px font, centered
  - Day/Month: positioned flanking the time row
  - Graph: GRect(4, 130, 128, 80) — re-use `s_graph_layer` with updated frame
  - Trend label: GRect(138, 130, 58, 24), GOTHIC_18_BOLD
  - Glucose panel: GRect(138, 154, 58, 40), GOTHIC_24_BOLD, right-aligned, zone color
  - Threshold labels: GRect(4, 212, 192, 14), GOTHIC_14
- [ ] 4.3 Update `update_display()` Dashboard branch:
  - Set time as "HH:MM" string
  - Set day/month labels
  - Set trend label text ("Flat", "Slow Rise ↑", "Rise ↑", "Rapid Rise ↑↑", "Slow Fall ↓", "Fall ↓", "Rapid Fall ↓↓")
  - Set glucose panel text (glucose value + unit), color = zone color
  - Update graph layer (existing logic, re-use)
- [ ] 4.4 Update `prv_layout_for_bounds()` Dashboard branch for both T2 and R2
- [ ] 4.5 Destroy Dashboard-specific layers in `window_unload`

## 5. Round 2 adaptation (R2 — 260×260)

- [ ] 5.1 In `prv_layout_for_bounds()`, detect R2 via `bounds.size.w == 260`
- [ ] 5.2 Simple R2 slot positions (inside 130px inscribed circle):
  - TL: GRect(38, 38, 56, 56), TR: GRect(166, 38, 56, 56)
  - BL: GRect(38, 166, 56, 56), BR: GRect(166, 166, 56, 56)
  - Time rows: GRect(60, 80, 140, 70) and GRect(60, 150, 140, 70)
  - Day: GRect(2, 122, 30, 16), Month: GRect(228, 122, 30, 16)
- [ ] 5.3 Dashboard R2 slot positions (upper arc area):
  - Top 3 slots: GRect(20,20,56,46), GRect(102,14,56,46), GRect(184,20,56,46)
  - Time: GRect(60, 76, 140, 60)
  - Graph: GRect(30, 148, 150, 76)
  - Trend+glucose panel: GRect(186, 148, 44, 76)
- [ ] 5.4 Verify on emulator: no elements clipped outside circular boundary
- [ ] 5.5 Adjust if needed — iterate until clean visual

## 6. Clay settings update (`src/pkjs/config.html`)

- [ ] 6.1 Add Layout section before existing settings:
  - Toggle/select: "Layout" — options: "Simple (Clock)" / "Dashboard (Glucose)" — key: `KEY_LAYOUT`
- [ ] 6.2 Add "Widget Slots" section with 4 dropdowns:
  - "Top Left / Slot A" — key `KEY_SLOT_0`, options: None, Battery, Weather, Heart Rate, Steps, CGM
  - "Top Right / Slot B" — key `KEY_SLOT_1`
  - "Bottom Left / Slot C" — key `KEY_SLOT_2`
  - "Bottom Right / Slot D" — key `KEY_SLOT_3`
  - Help text: "In Dashboard layout, Slots C and D are not displayed"
- [ ] 6.3 Set sensible defaults: Layout = Simple, Slot A = Weather, Slot B = Battery, Slot C = CGM, Slot D = Heart Rate
- [ ] 6.4 On settings save, include layout + slot keys in `Pebble.sendAppMessage()` call

## 7. Weather via pkjs (`src/pkjs/index.js`)

- [ ] 7.1 Add `fetchWeather()` function using navigator.geolocation + OpenMeteo API:
  `https://api.open-meteo.com/v1/forecast?latitude=LAT&longitude=LON&current=temperature_2m,weather_code`
- [ ] 7.2 Implement `weatherCodeToIconIndex(code)`: map WMO codes to 0-7 icon index
  (0=sunny, 1=partly cloudy, 2=cloudy, 3=rain, 4=storm, 5=snow, 6=fog, 7=default)
- [ ] 7.3 Call `fetchWeather()` in `Pebble.addEventListener('ready', ...)` after existing CGM fetch
- [ ] 7.4 Refresh weather every 30 minutes: use `setInterval(fetchWeather, 30 * 60 * 1000)`
- [ ] 7.5 Handle geolocation permission denied: send `KEY_WEATHER_TEMP = INT8_MIN` to signal unavailable

## 8. Quick View support

- [ ] 8.1 Simple compact mode (`is_unobstructed == false`): hide slot layers, show compressed time (single row GOTHIC_28_BOLD or LECO_38), show CGM value if CGM slot assigned
- [ ] 8.2 Dashboard compact mode: keep existing compact behavior (hide graph), compress top slots to icon-only (no value/unit text), keep time + glucose panel
- [ ] 8.3 Update `prv_layout_for_bounds()` with Quick View branches for both layouts
- [ ] 8.4 Test both layouts in Quick View on T2 and R2 emulators

## 9. Build, install, and visual verification

- [ ] 9.1 `pebble build` — zero warnings, zero errors, all 5 platforms (emery, gabbro, basalt, diorite, chalk)
- [ ] 9.2 `pebble install --emulator emery` — Simple layout, all 4 slots assigned, arc strokes visible
- [ ] 9.3 Screenshot T2 Simple: 2-row time prominent, 4 corner slots, arc strokes, BT icon
- [ ] 9.4 `pebble install --emulator emery` — switch to Dashboard layout, verify graph + top slots
- [ ] 9.5 Screenshot T2 Dashboard: time center, 3 top slots, sparkline graph, trend panel
- [ ] 9.6 `pebble install --emulator gabbro` — Simple layout on R2, no clipping
- [ ] 9.7 Screenshot R2 Simple: slots within circular boundary, time centered
- [ ] 9.8 `pebble install --emulator gabbro` — Dashboard layout on R2
- [ ] 9.9 Data states on T2: hypo (< 70 mg/dL), in-range, high (> 180), stale (> 15 min), no data
- [ ] 9.10 Clay: toggle layout → live update; reassign slot → slot content changes
- [ ] 9.11 Steps + HR: confirm values appear in slots (use mock data or real device)
- [ ] 9.12 Weather: confirm temperature + icon display in Weather slot on connect

## 10. Rename + Publish (contest submission)

- [ ] 10.0 Rename watchface from "Sugar Watch" to "Steady" across all files:
  - `package.json`: update `"name"` and `"displayName"` fields
  - `src/pkjs/config.html`: update title if present
  - `README.md`: replace all occurrences of "Sugar Watch"
  - `openspec/config.yaml`: update context description
  - Verify: no remaining "Sugar Watch" or "GlucoseGuard" strings in user-visible content
- [ ] 10.1 Use `resources/appstore-description-draft.md` as the store description (already drafted as "Steady")
- [ ] 10.2 Prepare contest screenshots: T2 Simple dark + T2 Dashboard dark + R2 Simple dark
- [ ] 10.3 `pebble publish` — push PBW to Pebble App Store
- [ ] 10.4 Verify on apps.repebble.com: screenshots correct, description complete, both new platforms listed
