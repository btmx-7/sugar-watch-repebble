<!-- All pixel values and font choices reference layout.md in this change. -->
<!-- Scope: close three missing CGM sidebar elements + fix HIGH zone color. -->

## 1. Fix zone_color() HIGH mapping

- [ ] 1.1 In `zone_color()` (~line 275 in main.c), change `case ZONE_HIGH:` to
  return `GColorChromeYellow` instead of `CLR_STATE_WARNING`
- [ ] 1.2 Verify the graph's `high_thresh` dashed line already uses
  `GColorChromeYellow` — confirm no double-change needed there
- [ ] 1.3 Run mental diff: SLOT_CGM arc color, s_dash_trend_layer, s_dash_glucose_layer
  — all call `zone_color()`; confirm HIGH will now be yellow in all three places

## 2. Add three new TextLayer declarations

In `main.c` static declarations section (~line 200 area), after existing
`s_dash_unit_layer` declaration, add:

- [ ] 2.1 `static TextLayer *s_dash_delta_layer;`
- [ ] 2.2 `static TextLayer *s_dash_fresh_layer;`
- [ ] 2.3 `static TextLayer *s_dash_zone_layer;`

## 3. Create layers in main_window_load()

After the existing `s_dash_unit_layer` creation block (~line 1376 in main.c):

- [ ] 3.1 Create `s_dash_delta_layer`:
  ```c
  s_dash_delta_layer = text_layer_create(GRect(128, 196, 68, 14));
  text_layer_set_background_color(s_dash_delta_layer, GColorClear);
  text_layer_set_text_color(s_dash_delta_layer, CLR_STATE_INACTIVE);
  text_layer_set_font(s_dash_delta_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_delta_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_delta_layer));
  ```

- [ ] 3.2 Create `s_dash_fresh_layer`:
  ```c
  s_dash_fresh_layer = text_layer_create(GRect(128, 210, 68, 14));
  text_layer_set_background_color(s_dash_fresh_layer, GColorClear);
  text_layer_set_text_color(s_dash_fresh_layer, CLR_STATE_INACTIVE);
  text_layer_set_font(s_dash_fresh_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_fresh_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_fresh_layer));
  ```

- [ ] 3.3 Create `s_dash_zone_layer`:
  ```c
  s_dash_zone_layer = text_layer_create(GRect(128, 210, 68, 14));
  text_layer_set_background_color(s_dash_zone_layer, GColorClear);
  text_layer_set_text_color(s_dash_zone_layer, CLR_STATE_INACTIVE);
  text_layer_set_font(s_dash_zone_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_zone_layer, GTextAlignmentRight);
  layer_set_hidden(text_layer_get_layer(s_dash_zone_layer), true);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_zone_layer));
  ```

## 4. Update update_display_dashboard() with new render logic

In `update_display_dashboard()` (~line 756 in main.c), after the existing
`s_dash_unit_layer` block:

- [ ] 4.1 Add delta rendering (after unit block):
  ```c
  // Delta — buf worst case: sign + 3 digits + null = 5 bytes; use 8 for safety
  static char s_dash_delta_buf[8];
  if (stale || s_glucose == 0) {
    snprintf(s_dash_delta_buf, sizeof(s_dash_delta_buf), "--");
  } else if (s_settings.use_mmol) {
    int32_t abs_delta = s_delta < 0 ? -s_delta : s_delta;
    int whole = (int)(abs_delta * 555 / 10000);
    int frac  = (int)((abs_delta * 555 % 10000) / 1000);
    snprintf(s_dash_delta_buf, sizeof(s_dash_delta_buf),
             s_delta >= 0 ? "+%d.%d" : "-%d.%d", whole, frac);
  } else {
    snprintf(s_dash_delta_buf, sizeof(s_dash_delta_buf),
             s_delta >= 0 ? "+%ld" : "%ld", (long)s_delta);
  }
  if (s_dash_delta_layer) {
    text_layer_set_text(s_dash_delta_layer, s_dash_delta_buf);
    text_layer_set_text_color(s_dash_delta_layer,
      (stale || s_glucose == 0) ? GColorLightGray : cgm_color);
  }
  ```

- [ ] 4.2 Add freshness + zone label mutual-exclusion logic:
  ```c
  // Freshness and zone label share y=210; exactly one visible or both hidden.
  bool show_zone = !stale && s_glucose != 0 &&
                   (zone == ZONE_LOW  || zone == ZONE_URGENT_LOW  ||
                    zone == ZONE_HIGH || zone == ZONE_URGENT_HIGH);
  bool show_fresh = !stale && s_glucose != 0 && !show_zone;

  if (s_dash_zone_layer) {
    layer_set_hidden(text_layer_get_layer(s_dash_zone_layer), !show_zone);
    if (show_zone) {
      bool is_hypo = (zone == ZONE_LOW || zone == ZONE_URGENT_LOW);
      text_layer_set_text(s_dash_zone_layer, is_hypo ? "hypo." : "hyper.");
      text_layer_set_text_color(s_dash_zone_layer, cgm_color);
    }
  }

  static char s_dash_fresh_buf[8];
  if (s_dash_fresh_layer) {
    layer_set_hidden(text_layer_get_layer(s_dash_fresh_layer), !show_fresh);
    if (show_fresh) {
      int32_t mins = minutes_since_last_read();
      snprintf(s_dash_fresh_buf, sizeof(s_dash_fresh_buf),
               "\xe2\x80\xa2%ldm", (long)(mins < 99 ? mins : 99));
      text_layer_set_text(s_dash_fresh_layer, s_dash_fresh_buf);
    }
  }
  ```
  Note: `\xe2\x80\xa2` is the UTF-8 encoding of the bullet character "•".

## 5. Update prv_layout_for_bounds() Dashboard branch

In `prv_layout_for_bounds()` Dashboard section, after the existing T2 and R2
coordinate blocks (~line 1145 in main.c):

- [ ] 5.1 T2 branch — add new layer frames after `s_dash_unit_layer` frame:
  ```c
  if (s_dash_delta_layer)
    layer_set_frame(text_layer_get_layer(s_dash_delta_layer), GRect(128, 196, 68, 14));
  if (s_dash_fresh_layer)
    layer_set_frame(text_layer_get_layer(s_dash_fresh_layer), GRect(128, 210, 68, 14));
  if (s_dash_zone_layer)
    layer_set_frame(text_layer_get_layer(s_dash_zone_layer), GRect(128, 210, 68, 14));
  ```

- [ ] 5.2 R2 branch — add delta only (fresh + zone not shown on R2 per layout.md):
  ```c
  if (s_dash_delta_layer)
    layer_set_frame(text_layer_get_layer(s_dash_delta_layer), GRect(188, 214, 36, 14));
  if (s_dash_fresh_layer)
    layer_set_hidden(text_layer_get_layer(s_dash_fresh_layer), true);
  if (s_dash_zone_layer)
    layer_set_hidden(text_layer_get_layer(s_dash_zone_layer), true);
  ```

## 6. Destroy layers in main_window_unload()

In `main_window_unload()`, after the existing `s_dash_unit_layer` destroy:

- [ ] 6.1 Add in reverse creation order:
  ```c
  if (s_dash_zone_layer)  { text_layer_destroy(s_dash_zone_layer);  s_dash_zone_layer  = NULL; }
  if (s_dash_fresh_layer) { text_layer_destroy(s_dash_fresh_layer); s_dash_fresh_layer = NULL; }
  if (s_dash_delta_layer) { text_layer_destroy(s_dash_delta_layer); s_dash_delta_layer = NULL; }
  ```

## 7. Build, install, and visual verification

- [ ] 7.1 `pebble build` — zero warnings, zero errors on all 5 platforms
- [ ] 7.2 `pebble install --emulator emery` — Dashboard layout, in-range data
- [ ] 7.3 Screenshot T2 in-range: delta visible (e.g. "+8"), freshness "•3m", no zone label
- [ ] 7.4 Screenshot T2 HIGH: glucose in yellow, delta in yellow, "hyper." label visible, freshness hidden
- [ ] 7.5 Screenshot T2 LOW: glucose in orange, delta in orange, "hypo." label visible
- [ ] 7.6 Screenshot T2 stale: all CGM layers show "--" or hidden, GColorLightGray color
- [ ] 7.7 Screenshot T2 URGENT_HIGH: glucose in red, "hyper." in red
- [ ] 7.8 `pebble install --emulator emery` — Quick View: slots + graph hidden, sidebar visible with delta
- [ ] 7.9 `pebble install --emulator gabbro` — R2 Dashboard: delta visible at GRect(188,214,36,14)
- [ ] 7.10 Screenshot R2 in-range: delta text not clipped at circular boundary
- [ ] 7.11 Confirm HIGH zone is yellow (GColorChromeYellow) in both SLOT_CGM arc and sidebar
