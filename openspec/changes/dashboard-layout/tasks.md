<!-- All pixel values reference layout.md in this change. -->
<!-- Three gaps to close: HIGH zone color · graph threshold labels · R2 CGM layout -->

## 1. Fix zone_color() HIGH mapping

- [x] 1.1 In `zone_color()` (main.c ~line 275), change `case ZONE_HIGH:` to
  return `GColorChromeYellow` instead of `CLR_STATE_WARNING`
- [x] 1.2 Confirm `graph_layer_update_proc` already uses `GColorChromeYellow`
  for `high_thresh` line — no change needed there (already consistent)
- [x] 1.3 Confirm SLOT_CGM arc and `s_dash_glucose_layer` / `s_dash_trend_layer`
  call `zone_color()` — HIGH readings will now render yellow in all three places

## 2. Add graph threshold labels

In `graph_layer_update_proc` (main.c ~line 325), add `graphics_draw_text()`
calls immediately after each existing threshold line draw block.

- [x] 2.1 Before the existing `if (s_settings.low_thresh …)` block, declare
  the label font once:
  ```c
  GFont lbl = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  ```

- [x] 2.2 Inside the `if (s_settings.high_thresh …)` block, after drawing the
  dashed line, add:
  ```c
  char h_buf[8];
  snprintf(h_buf, sizeof(h_buf), "%d", (int)s_settings.high_thresh);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(ctx, "hyper", lbl,
    GRect(1, hy - 13, w / 2, 13),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, h_buf, lbl,
    GRect(w / 2, hy - 13, w / 2, 13),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  ```

- [x] 2.3 Inside the `if (s_settings.low_thresh …)` block, after drawing the
  dashed line, add:
  ```c
  char l_buf[8];
  snprintf(l_buf, sizeof(l_buf), "%d", (int)s_settings.low_thresh);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(ctx, "hypo", lbl,
    GRect(1, ly + 1, w / 2, 13),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, l_buf, lbl,
    GRect(w / 2, ly + 1, w / 2, 13),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  ```

- [x] 2.4 Guard check: `hy - 13 >= 0` before drawing hyper label (avoid
  negative rect origin if high threshold is near the top of the graph bounds).
  Wrap the hyper text block: `if (hy >= 13) { … }`

- [x] 2.5 Guard check: `ly + 14 <= h` before drawing hypo label.
  Wrap the hypo text block: `if (ly + 14 <= h) { … }`

## 3. Add s_dash_trend_name_layer declaration and creation

- [x] 3.1 In the static declarations section (~line 200), add after existing
  Dashboard layer declarations:
  ```c
  static TextLayer *s_dash_trend_name_layer;
  ```

- [x] 3.2 In `main_window_load()`, after the `s_dash_unit_layer` creation block:
  ```c
  s_dash_trend_name_layer = text_layer_create(GRect(0, 0, 96, 14));
  text_layer_set_background_color(s_dash_trend_name_layer, GColorClear);
  text_layer_set_text_color(s_dash_trend_name_layer, CLR_STATE_INACTIVE);
  text_layer_set_font(s_dash_trend_name_layer,
    fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_trend_name_layer, GTextAlignmentLeft);
  layer_set_hidden(text_layer_get_layer(s_dash_trend_name_layer), true);
  layer_add_child(s_window_layer,
    text_layer_get_layer(s_dash_trend_name_layer));
  ```

- [x] 3.3 In `main_window_unload()`, after `s_dash_unit_layer` destroy:
  ```c
  if (s_dash_trend_name_layer) {
    text_layer_destroy(s_dash_trend_name_layer);
    s_dash_trend_name_layer = NULL;
  }
  ```

## 4. Add trend_name() helper

- [x] 4.1 After the existing `trend_icon()` function (~line 310), add:
  ```c
  static const char* trend_name(GlucoseTrend t) {
    switch (t) {
      case TREND_DOUBLE_UP:       return "Rise++";
      case TREND_SINGLE_UP:       return "Rising";
      case TREND_FORTY_FIVE_UP:   return "Rising";
      case TREND_FLAT:            return "Flat";
      case TREND_FORTY_FIVE_DOWN: return "Falling";
      case TREND_SINGLE_DOWN:     return "Falling";
      case TREND_DOUBLE_DOWN:     return "Fall++";
      default:                    return "--";
    }
  }
  ```

## 5. Update update_display_dashboard() for trend name layer

- [x] 5.1 In `update_display_dashboard()`, after the existing BT block, add
  trend name population (layer is hidden on T2; populated but only made visible
  on R2 via `prv_layout_for_bounds()`):
  ```c
  if (s_dash_trend_name_layer) {
    bool stale = data_is_stale();  // already computed above — reuse local
    const char *tname = (stale || s_glucose == 0)
      ? "--" : trend_name((GlucoseTrend)s_trend);
    text_layer_set_text(s_dash_trend_name_layer, tname);
    text_layer_set_text_color(s_dash_trend_name_layer,
      (stale || s_glucose == 0) ? CLR_STATE_INACTIVE : cgm_color);
  }
  ```
  Note: `stale` and `cgm_color` are already in scope in `update_display_dashboard()`.

## 6. Update prv_layout_for_bounds() R2 Dashboard branch

In `prv_layout_for_bounds()`, inside the `if (dashboard)` → `else` (R2) branch
(~line 1190 in main.c):

- [x] 6.1 Change `s_graph_layer` frame: height 76 → 60:
  ```c
  if (s_graph_layer)
    layer_set_frame(s_graph_layer, GRect(30, 148, 150, 60));
  ```

- [x] 6.2 Reposition `s_dash_trend_name_layer` (row 1 left) and show it:
  ```c
  if (s_dash_trend_name_layer) {
    layer_set_frame(text_layer_get_layer(s_dash_trend_name_layer),
      GRect(42, 212, 96, 14));
    layer_set_hidden(text_layer_get_layer(s_dash_trend_name_layer), false);
  }
  ```

- [x] 6.3 Reposition `s_dash_unit_layer` to row 1 right:
  ```c
  if (s_dash_unit_layer)
    layer_set_frame(text_layer_get_layer(s_dash_unit_layer),
      GRect(142, 212, 76, 14));
  ```

- [x] 6.4 Reposition `s_dash_trend_layer` to row 2 left:
  ```c
  if (s_dash_trend_layer)
    layer_set_frame(text_layer_get_layer(s_dash_trend_layer),
      GRect(82, 226, 24, 20));
  ```

- [x] 6.5 Reposition `s_dash_glucose_layer` to row 2 right:
  ```c
  if (s_dash_glucose_layer)
    layer_set_frame(text_layer_get_layer(s_dash_glucose_layer),
      GRect(110, 226, 70, 20));
  ```

- [x] 6.6 Ensure trend, glucose, and unit frames that currently exist in the R2
  branch (for the old x=188 sidebar) are removed / replaced by 6.3–6.5 above.
  Delete old R2 frames for `s_dash_trend_layer`, `s_dash_glucose_layer`,
  `s_dash_unit_layer` at x=188.

## 7. Build, install, and visual verification

Automation: `bash scripts/shoot_dashboard.sh` covers §7.1-7.10 across both
platforms. Outputs go to `resources/screenshots/states/dashboard/`. Dashboard
state matrix and AppMessage injection live in `scripts/send_mock.py` (states
11-22). Quick View (§7.11-7.12) is manual: see
`resources/screenshots/states/dashboard/README.md`.

- [ ] 7.1 `pebble build` — zero warnings, zero errors on all 5 platforms
- [ ] 7.2 `pebble install --emulator emery` — Dashboard layout, in-range data
- [ ] 7.3 Screenshot T2 (state d1): graph threshold labels ("hyper 180", "hypo 70") visible
- [ ] 7.4 Screenshot T2 (state d4): HIGH glucose → sidebar text yellow (GColorChromeYellow)
- [ ] 7.5 Screenshot T2 (state d3): LOW glucose → sidebar text orange (CLR_STATE_WARNING)
- [ ] 7.6 Screenshot T2 (state d6): stale → sidebar shows "--" in GColorLightGray
- [ ] 7.7 `pebble install --emulator gabbro` — Dashboard layout, in-range data
- [ ] 7.8 Screenshot R2 (state d1): CGM panel appears BELOW graph (not beside it)
- [ ] 7.9 Screenshot R2 (state d1): row 1 = "Flat  mg/dL", row 2 = "→  110"
- [ ] 7.10 Screenshot R2 (any state): no text clipped at circular boundary
- [ ] 7.11 Quick View T2 (manual): slots and graph hidden, sidebar still shows trend + glucose + unit
- [ ] 7.12 Quick View R2 (manual): slots and graph hidden, below-graph panel still visible

### 7.13 Slot variation coverage (states d9-d12)

- [ ] 7.13 T2 + R2 screenshots for {HR, steps, CGM}, {CGM, weather, battery},
  {weather, HR, CGM}, {battery, steps, CGM} — visual spot-check that each slot
  renders in each of the 3 dashboard positions

---

## 8. Batch 1: Font + color token migration

Figma references (cyan dark mode, `font-semantic.json`, `color-primitives.json`,
`color-semantic-dark/cyan.tokens.json`):
- T2: https://www.figma.com/design/IpIOLQi5xz0kWZP8ZNu27V/%E2%8C%9A%EF%B8%8F-SugarWatch-Watchface?node-id=40-20566
- R2: https://www.figma.com/design/IpIOLQi5xz0kWZP8ZNu27V/%E2%8C%9A%EF%B8%8F-SugarWatch-Watchface?node-id=40-20567

### 8.0 Font resources (package.json)

- [ ] 8.0.1 Add `fonts/Inter_18pt-SemiBold.ttf` to `resources/fonts/`
  (required by DATA_MEDIUM_12, DATA_SMALL_8)
- [ ] 8.0.2 Add `fonts/Inter_18pt-Bold.ttf` to `resources/fonts/`
  (required by DATE_14)
- [ ] 8.0.3 Add `TIME_DIGITS_80` resource: Inter Black 80pt, chars `[0-9:]`
  (Simple watchface time, `font-semantic/time/simple`).
  **BLOCKED:** at 80pt, Inter Black's `.notdef` glyph rasterizes to 301px,
  over Pebble SDK's 256px per-glyph limit. Codepoint 9647 (U+25AF WHITE
  VERTICAL RECTANGLE) triggers the limit. Pebble always generates `.notdef`
  regardless of `characterRegex`, so the regex cannot filter it out.
  Resolution options: (a) subset the TTF to remove/shrink `.notdef` glyph;
  (b) use Inter Black at 72pt instead (scales to ~271px — may still exceed);
  (c) use a lighter Inter weight (Bold/SemiBold) which has a smaller `.notdef`.
  Track as Batch 3.
- [ ] 8.0.4 Add `TIME_DIGITS_56` resource: Inter Black 56pt, chars `[0-9:]`
  (Dashboard watchface time, `font-semantic/time/dashboard`)
- [ ] 8.0.5 Add `DATA_MEDIUM_12` resource: Inter SemiBold 12pt, full set
  (unit label, trend name, `font-semantic/data/medium`)
- [ ] 8.0.6 Add `DATA_SMALL_8` resource: Inter SemiBold 8pt, chars `[0-9a-z]`
  (graph threshold labels, `font-semantic/data/small`)
- [ ] 8.0.7 Add `DATE_14` resource: Inter Bold 14pt, chars `[0-9a-zA-Z ]`
  (day + month, `font-semantic/date`, letterSpacing -0.5 not settable in SDK)
- [ ] 8.0.8 Remove `TIME_DIGITS_64` and `DATA_UNIT_10` (superseded)

### 8.1 R2 top slot y-position fix

- [ ] 8.1.1 slot[0] frame y: 20 → 34 (R2 Dashboard branch)
- [ ] 8.1.2 slot[2] frame y: 20 → 34 (R2 Dashboard branch)
- [ ] 8.1.3 slot[1] stays y=14 (center-top, circular clip compliant)
- [ ] 8.1.4 Document circular clip rationale in `layout.md` R2 frames table

### 8.2 Dashboard time: 5-layer digit pattern

Dashboard time must match Simple's per-digit layer model. Figma
`font-semantic/time/dashboard` = Inter Black 56pt. User noted 56/48 —
interpret as font-size 56 with digit display width ≈ 48px post-tracking.
**Flagged batch 3 item if Figma frames reveal per-digit custom kerning
beyond what `trackingAdjust` can reproduce.**

- [ ] 8.2.1 Replace single `s_dash_time_layer` with 5 TextLayers: H1, H2,
  colon, M1, M2 (declared in static block, created in `main_window_load`)
- [ ] 8.2.2 Font: TIME_DIGITS_56 on all 5 layers
- [ ] 8.2.3 Color mapping:
  - H1, H2 → CLR_TEXT_SUBTLE (#AAFFFF)
  - colon  → CLR_TEXT_INVERTED (#FFFFFF)
  - M1, M2 → CLR_TEXT_DEFAULT (#00FFFF)
- [ ] 8.2.4 Digit overlap: match Figma Dashboard component frames. Start
  from Simple's overlap pattern (same TIME_DIGITS_80 approach, scaled to 56),
  adjust per Figma node 40-20566 / 40-20567. Verify build still renders.
- [ ] 8.2.5 Update `update_display_dashboard()` to write H1/H2/colon/M1/M2
  separately (split HH:MM buffer)
- [ ] 8.2.6 Remove old single `s_dash_time_layer` declaration, init, unload

### 8.3 Trend name: 7-label scheme

- [ ] 8.3.1 Refactor `trend_name()` in main.c (~line 324):
  - DOUBLE_UP       → "Rapid rise"
  - SINGLE_UP       → "Rise"
  - FORTY_FIVE_UP   → "Slow rise"
  - FLAT            → "Flat"
  - FORTY_FIVE_DOWN → "Slow fall"
  - SINGLE_DOWN     → "Fall"
  - DOUBLE_DOWN     → "Rapid fall"
  - default         → "--"
- [ ] 8.3.2 Verify layer frame widths accommodate longest label ("Rapid rise"
  at Inter SemiBold 12pt ≈ 60px). T2 frame is 68px wide ✓. R2 frame is 86px
  wide ✓. Flag batch 3 if clipping observed.

### 8.4 Dashboard CGM panel fonts + colors

- [ ] 8.4.1 `s_dash_trend_name_layer` font → DATA_MEDIUM_12 (was GOTHIC_14)
- [ ] 8.4.2 `s_dash_glucose_layer` font → DATA_VALUE_20 (unchanged, confirm)
- [ ] 8.4.3 `s_dash_glucose_layer` default color → CLR_TEXT_DEFAULT (#00FFFF,
  was GColorWhite). State colors via `zone_color()` unchanged.
- [ ] 8.4.4 `s_dash_unit_layer` font → DATA_MEDIUM_12 (was DATA_UNIT_10)
- [ ] 8.4.5 `s_dash_unit_layer` default color → CLR_TEXT_INVERTED (#FFFFFF,
  was GColorMediumAquamarine)
- [ ] 8.4.6 `s_dash_day_layer`, `s_dash_month_layer` font → DATE_14 (was
  system GOTHIC_14). Color CLR_TEXT_SUBTLE unchanged.
- [ ] 8.4.7 `s_dash_bt_layer` font and color unchanged (MATERIAL_SYMBOLS_16,
  CLR_ICON_DEFAULT)

### 8.5 Simple watchface font migration

- [ ] 8.5.1 Time digits (H1/H2/colon/M1/M2) font: TIME_DIGITS_64 → TIME_DIGITS_80
  (font-semantic/time/simple = Inter Black 80pt)
- [ ] 8.5.2 Date / day / month TextLayers font → DATE_14
- [ ] 8.5.3 Verify digit overlap math on T2 (200w) and R2 (260w) with 80pt
  glyphs. Adjust `trackingAdjust` or per-digit x positions. Flag batch 3 if
  80pt digits overflow T2 200px canvas.

### 8.6 Global color token audit

- [ ] 8.6.1 Grep `src/c/main.c` for direct `GColor*` literal usage
- [ ] 8.6.2 Replace with token macros:
  - GColorWhite → CLR_TEXT_INVERTED (text) or keep (frame bg)
  - GColorLightGray → CLR_STATE_INACTIVE
  - GColorChromeYellow → keep (maps to primitive chrome-yellow, used as
    state/warning variant for HIGH zone — not a semantic color)
  - GColorMediumAquamarine → CLR_TEXT_INVERTED (unit) or re-evaluate
  - GColorMintGreen → CLR_TEXT_DEFAULT
- [ ] 8.6.3 Keep primitive `GColor*` only for window bg (GColorBlack) and
  `zone_color()` branch returns

## 9. Batch 2: Graph threshold band + state coloring

### 9.1 Threshold line as bordered band (3px)

- [ ] 9.1.1 Replace dashed-line draw at `hy` with 3px horizontal band:
  - y=hy-1: 1px border line, color CLR_BORDER_SUBTLE (#005555)
  - y=hy:   1px fill line, color driven by state (see §9.3)
  - y=hy+1: 1px border line, color CLR_BORDER_SUBTLE
- [ ] 9.1.2 Same 3px band at `ly` (low threshold)
- [ ] 9.1.3 Pattern mirrors arc-slot stroke (track + progress)

### 9.2 Target zone background

- [ ] 9.2.1 Add `CLR_SURFACE_BG_SUBTLE` macro: argb 0xC5 (#005555 midnight-green)
- [ ] 9.2.2 Before drawing threshold bands + sparkline points, fill rect
  between `hy+2` and `ly-2` with CLR_SURFACE_BG_SUBTLE
- [ ] 9.2.3 Verify sparkline points remain visible above the subtle fill

### 9.3 State-colored threshold band + label

Band fill and label color follow CGM zone:

- [ ] 9.3.1 Determine `threshold_color()` helper mapping:
  - ZONE_IN_RANGE → CLR_STATE_INACTIVE (#AAAAAA)
  - ZONE_LOW → CLR_STATE_WARNING (#FFAA00) on low band + "hypo" label
  - ZONE_URGENT_LOW → CLR_STATE_DANGER (#FF0000) on low band + "hypo" label
  - ZONE_HIGH → GColorChromeYellow (#FFAA00) on high band + "hyper" label
  - ZONE_URGENT_HIGH → CLR_STATE_DANGER on high band + "hyper" label
  - Stale / error → CLR_STATE_INACTIVE
- [ ] 9.3.2 Non-active threshold (e.g. high band during hypo) stays
  CLR_STATE_INACTIVE — only the band matching the breached side colors up
- [ ] 9.3.3 Label font: DATA_SMALL_8 (Inter SemiBold 8pt, was GOTHIC_14)
- [ ] 9.3.4 Label color: matches the corresponding band fill color

### 9.4 Build + visual verification

- [ ] 9.4.1 `pebble build` — zero warnings on all 5 platforms
- [ ] 9.4.2 `STATES=11-22 bash scripts/shoot_dashboard.sh` (T2 + R2)
- [ ] 9.4.3 Compare captures vs `resources/screenshots/design-export/dashboard/`
- [ ] 9.4.4 Simple layout captures: `STATES=1-8 bash scripts/shoot_simple.sh`
  (if script exists; else manual)
- [ ] 9.4.5 Log any remaining gaps as Batch 3
