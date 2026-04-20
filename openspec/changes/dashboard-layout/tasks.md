<!-- All pixel values reference layout.md in this change. -->
<!-- Three gaps to close: HIGH zone color · graph threshold labels · R2 CGM layout -->

## 1. Fix zone_color() HIGH mapping

- [ ] 1.1 In `zone_color()` (main.c ~line 275), change `case ZONE_HIGH:` to
  return `GColorChromeYellow` instead of `CLR_STATE_WARNING`
- [ ] 1.2 Confirm `graph_layer_update_proc` already uses `GColorChromeYellow`
  for `high_thresh` line — no change needed there (already consistent)
- [ ] 1.3 Confirm SLOT_CGM arc and `s_dash_glucose_layer` / `s_dash_trend_layer`
  call `zone_color()` — HIGH readings will now render yellow in all three places

## 2. Add graph threshold labels

In `graph_layer_update_proc` (main.c ~line 325), add `graphics_draw_text()`
calls immediately after each existing threshold line draw block.

- [ ] 2.1 Before the existing `if (s_settings.low_thresh …)` block, declare
  the label font once:
  ```c
  GFont lbl = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  ```

- [ ] 2.2 Inside the `if (s_settings.high_thresh …)` block, after drawing the
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

- [ ] 2.3 Inside the `if (s_settings.low_thresh …)` block, after drawing the
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

- [ ] 2.4 Guard check: `hy - 13 >= 0` before drawing hyper label (avoid
  negative rect origin if high threshold is near the top of the graph bounds).
  Wrap the hyper text block: `if (hy >= 13) { … }`

- [ ] 2.5 Guard check: `ly + 14 <= h` before drawing hypo label.
  Wrap the hypo text block: `if (ly + 14 <= h) { … }`

## 3. Add s_dash_trend_name_layer declaration and creation

- [ ] 3.1 In the static declarations section (~line 200), add after existing
  Dashboard layer declarations:
  ```c
  static TextLayer *s_dash_trend_name_layer;
  ```

- [ ] 3.2 In `main_window_load()`, after the `s_dash_unit_layer` creation block:
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

- [ ] 3.3 In `main_window_unload()`, after `s_dash_unit_layer` destroy:
  ```c
  if (s_dash_trend_name_layer) {
    text_layer_destroy(s_dash_trend_name_layer);
    s_dash_trend_name_layer = NULL;
  }
  ```

## 4. Add trend_name() helper

- [ ] 4.1 After the existing `trend_icon()` function (~line 310), add:
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

- [ ] 5.1 In `update_display_dashboard()`, after the existing BT block, add
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

- [ ] 6.1 Change `s_graph_layer` frame: height 76 → 60:
  ```c
  if (s_graph_layer)
    layer_set_frame(s_graph_layer, GRect(30, 148, 150, 60));
  ```

- [ ] 6.2 Reposition `s_dash_trend_name_layer` (row 1 left) and show it:
  ```c
  if (s_dash_trend_name_layer) {
    layer_set_frame(text_layer_get_layer(s_dash_trend_name_layer),
      GRect(42, 212, 96, 14));
    layer_set_hidden(text_layer_get_layer(s_dash_trend_name_layer), false);
  }
  ```

- [ ] 6.3 Reposition `s_dash_unit_layer` to row 1 right:
  ```c
  if (s_dash_unit_layer)
    layer_set_frame(text_layer_get_layer(s_dash_unit_layer),
      GRect(142, 212, 76, 14));
  ```

- [ ] 6.4 Reposition `s_dash_trend_layer` to row 2 left:
  ```c
  if (s_dash_trend_layer)
    layer_set_frame(text_layer_get_layer(s_dash_trend_layer),
      GRect(82, 226, 24, 20));
  ```

- [ ] 6.5 Reposition `s_dash_glucose_layer` to row 2 right:
  ```c
  if (s_dash_glucose_layer)
    layer_set_frame(text_layer_get_layer(s_dash_glucose_layer),
      GRect(110, 226, 70, 20));
  ```

- [ ] 6.6 Ensure trend, glucose, and unit frames that currently exist in the R2
  branch (for the old x=188 sidebar) are removed / replaced by 6.3–6.5 above.
  Delete old R2 frames for `s_dash_trend_layer`, `s_dash_glucose_layer`,
  `s_dash_unit_layer` at x=188.

## 7. Build, install, and visual verification

- [ ] 7.1 `pebble build` — zero warnings, zero errors on all 5 platforms
- [ ] 7.2 `pebble install --emulator emery` — Dashboard layout, in-range data
- [ ] 7.3 Screenshot T2: graph threshold labels ("hyper 230", "hypo 65") visible
- [ ] 7.4 Screenshot T2: HIGH glucose → sidebar text is yellow (GColorChromeYellow)
- [ ] 7.5 Screenshot T2: LOW glucose → sidebar text is orange (CLR_STATE_WARNING)
- [ ] 7.6 Screenshot T2: stale → sidebar shows "--" in GColorLightGray
- [ ] 7.7 `pebble install --emulator gabbro` — Dashboard layout, in-range data
- [ ] 7.8 Screenshot R2: CGM panel appears BELOW graph (not beside it)
- [ ] 7.9 Screenshot R2: row 1 = "Flat  mg/dL", row 2 = "→  120" (or current trend)
- [ ] 7.10 Screenshot R2: no text clipped at circular boundary
- [ ] 7.11 Quick View T2: slots and graph hidden, sidebar still shows trend + glucose + unit
- [ ] 7.12 Quick View R2: slots and graph hidden, below-graph panel still visible
