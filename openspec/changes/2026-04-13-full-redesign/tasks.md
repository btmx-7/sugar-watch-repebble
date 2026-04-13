<!-- Full redesign: Simple + Dashboard layouts, widget slots, bitmap fonts, Health API, Weather -->
<!-- Source of truth for pixel values: Figma https://www.figma.com/design/bKKqEkSN0q1rOdsEX8OpaE -->
<!-- Reference node IDs: Simple T2 Dark = 237:14165, Dashboard T2 Dark = 237:20791 -->

## 0. Font resources (blocking — must ship before any C work)

- [x] 0.1 Download `Inter[wght].ttf` from Google Fonts (SIL OFL license)
- [x] 0.2 Extract `InterBlack.ttf` (weight=900 static) — using `Inter_18pt-Black.ttf`
- [x] 0.3 Add font to `resources/fonts/` — `Inter_18pt-Black.ttf` present
- [x] 0.4 Add font resource to `package.json` at 64px, digits only (with `"size": 64`)
- [x] 0.5 Download `MaterialSymbolsRounded-Regular.ttf` (static)
- [x] 0.6 Add font to `resources/fonts/` — `MaterialSymbolsRounded_28pt-Medium.ttf` present
- [x] 0.7 Identified 20 Unicode codepoints for all required glyphs
- [x] 0.8 Add font resource to `package.json` with full characterRegex and `"size": 16`
- [ ] 0.9 `pebble build` — verify both fonts load without warnings (SDK not installed; pending emulator test)
- [x] 0.10 Define C string constants for each icon glyph (UTF-8 encoded) in `main.c`

## 1. Architecture additions (`src/c/main.c`)

- [x] 1.1 Add `WatchLayout` enum: `LAYOUT_SIMPLE = 0`, `LAYOUT_DASHBOARD = 1`
- [x] 1.2 Add `SlotType` enum: `SLOT_NONE`, `SLOT_BATTERY`, `SLOT_WEATHER`, `SLOT_HEART_RATE`, `SLOT_STEPS`, `SLOT_CGM`
- [x] 1.3 Extend settings struct with `WatchLayout layout` and `SlotType slots[4]`
- [x] 1.4 Add new data state statics: `s_weather_temp` (int8_t), `s_weather_icon` (uint8_t), `s_heart_rate` (int), `s_step_count` (uint32_t)
- [x] 1.5 Add new AppMessage keys to `package.json`: keys 10-16 (WEATHER_TEMP, WEATHER_ICON, LAYOUT, SLOT_0..3)
- [x] 1.6 Load new keys in `inbox_received_handler`
- [x] 1.7 Persist/restore layout and slots (PERSIST_LAYOUT=109, PERSIST_SLOT_0..3=110..113, PERSIST_WEATHER_TMP=114, PERSIST_WEATHER_ICN=115)
- [x] 1.8 Subscribe to Pebble Health API in `window_load`
- [x] 1.9 Implemented `prv_health_handler` (HealthEventHeartRateUpdate + HealthEventMovementUpdate)
- [x] 1.10 Unsubscribe from Health API in `window_unload`

## 2. Widget slot draw system

- [x] 2.1 Defined `SlotRenderData` struct
- [x] 2.2 Implemented `prv_populate_slot_data()` for all 5 slot types
- [x] 2.3 Implemented `slot_update_proc()` with arc track + active arc + icon + value + unit
- [x] 2.4 Allocated 4 slot layers with `layer_create_with_data()`
- [x] 2.5 Implemented `prv_update_all_slots()`; hides slot[3] in Dashboard
- [x] 2.6 Called from `update_display()`

## 3. Simple layout — T2 (200×228)

- [x] 3.1 Declared `s_simple_digit[4]`, `s_simple_day_layer`, `s_simple_month_layer`, `s_simple_bt_layer`, `s_simple_music_layer`
- [x] 3.2 Created all Simple layers in `window_load` (always created, hidden when Dashboard active)
- [x] 3.3 Implemented `update_display_simple()`: 4 single-digit strings, BT icon, day/month
- [x] 3.4 Positions in `prv_layout_for_bounds()`: T2 corners (4,4/140,4/4,168/140,168), R2 inscribed circle
- [x] 3.5 Destroyed in `main_window_unload`

## 4. Dashboard layout — T2 (200×228)

- [x] 4.1 Declared `s_dash_time_layer`, `s_dash_day_layer`, `s_dash_month_layer`, `s_dash_bt_layer`, `s_dash_trend_layer`, `s_dash_glucose_layer`, `s_dash_unit_layer`
- [x] 4.2 Created all Dashboard layers in `window_load`; time uses FONT_KEY_LECO_36_BOLD_NUMBERS (includes colon)
- [x] 4.3 Implemented `update_display_dashboard()`: strftime HH:MM, day/month, BT icon, CGM panel with zone color
- [x] 4.4 Positions in `prv_layout_for_bounds()`: T2 (slots at 4,4/72,4/140,4; graph at 4,130,120,80; panel at 128,x), R2 adapted
- [x] 4.5 Destroyed in `main_window_unload`

## 5. Round 2 adaptation (R2 — 260×260)

- [x] 5.1 Detect R2 via `bounds.size.w == 260` in `prv_layout_for_bounds()`
- [x] 5.2 Simple R2 slot positions: TL(38,38) TR(166,38) BL(38,166) BR(166,166), digits centered in 62..198 area
- [x] 5.3 Dashboard R2: slots (20,20 / 102,14 / 184,20), graph (30,148,150,76), panel (188,148..200)
- [ ] 5.4 Verify on emulator: no clipping outside circular boundary (pending build)
- [ ] 5.5 Adjust if needed

## 6. Clay settings update (`src/pkjs/config.html`)

- [x] 6.1 Layout section added (dropdown: Simple=0 / Dashboard=1)
- [x] 6.2 Widget Slots section: 4 dropdowns (slot0..3), each with None/Battery/Weather/HeartRate/Steps/CGM
- [x] 6.3 Defaults: slot0=Weather, slot1=Battery, slot2=CGM, slot3=HeartRate; layout=Simple
- [x] 6.4 save() includes layout + slot0..3 keys; index.js sends them in webviewclosed handler

## 7. Weather via pkjs (`src/pkjs/index.js`)

- [x] 7.1 Implemented `fetchWeather()` using `navigator.geolocation` + OpenMeteo API (XHR, no API key)
- [x] 7.2 Implemented `weatherCodeToIconIndex(code)`: WMO codes → 0-7 icon index
- [x] 7.3 Called in `ready` handler alongside CGM fetch
- [x] 7.4 `setInterval(fetchWeather, 30 * 60 * 1000)` in ready handler
- [x] 7.5 Geolocation denied: sends `KEY_WEATHER_TEMP = -128` (INT8_MIN sentinel)

## 8. Quick View support

- [x] 8.1 Simple compact (`h < 185`): bottom slots hidden, digit layers hidden, stale row remains
- [x] 8.2 Dashboard compact: graph hidden, top slots hidden; time + CGM panel remain
- [x] 8.3 `prv_layout_for_bounds()` includes `compact` flag; `UnobstructedAreaService` callbacks subscribed
- [ ] 8.4 Test Quick View on T2 and R2 emulators (pending build)

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

- [x] 10.0 Renamed "Sugar Watch" → "Steady": `package.json` (name + displayName), `config.html` (title + header), `index.js` (log strings + localStorage prefix "steady_")
- [x] 10.1 Updated README, openspec/config.yaml; remaining references are internal docs only
- [ ] 10.2 Prepare contest screenshots: T2 Simple dark + T2 Dashboard dark + R2 Simple dark (after emulator install)
- [ ] 10.3 `pebble publish` — push PBW to Pebble App Store
- [ ] 10.4 Verify on apps.repebble.com: screenshots correct, description complete, both new platforms listed
