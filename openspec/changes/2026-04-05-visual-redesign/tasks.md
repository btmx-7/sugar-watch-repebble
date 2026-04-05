# Tasks: Visual Redesign

All changes are isolated to `src/c/main.c`.

## Task 1: Promote glucose font to LECO_42

In `main_window_load`, change the glucose layer font from `FONT_KEY_LECO_38_BOLD_NUMBERS`
to `FONT_KEY_LECO_42_BOLD_NUMBERS`.

Update the layer height to match: the 42px font needs ~54px of vertical clearance (font
height + descender). Current layer is h=52, update to h=56.

## Task 2: Remove zone label layer

Delete `s_zone_layer` and all references:
- Declaration: `static TextLayer *s_zone_layer;`
- Creation block in `main_window_load` (~7 lines)
- All `text_layer_set_text(s_zone_layer, ...)` calls in `update_display`
- `text_layer_destroy(s_zone_layer)` in `main_window_unload`
- `layer_set_frame(text_layer_get_layer(s_zone_layer), ...)` in `prv_layout_for_bounds`

## Task 3: Relocate trend arrow to hero row

Current: trend layer at y = glucose_y + 48 (below the glucose number).
New: trend layer right-aligned at the same y as glucose, sized to h=56 to match.

In `main_window_load`:
- Font: `FONT_KEY_GOTHIC_28_BOLD`
- Frame: `GRect(w - 50, glucose_y, 48, 56)` (right-aligned, hero row height)
- Alignment: `GTextAlignmentRight`

Update `prv_layout_for_bounds` for both normal and compact cases accordingly.

## Task 4: Consolidate delta + freshness on meta row

**Delta (left):**
- Move from `glucose_y + 48` to `glucose_y + 58` (below hero block)
- Frame: `GRect(4, meta_y, 80, 24)` where `meta_y = glucose_y + 58`
- Font: keep `FONT_KEY_GOTHIC_24_BOLD`

**Freshness (right, replaces stale row):**
- Rename `s_stale_layer` to `s_fresh_layer` (or keep name for minimal diff, just change behavior)
- Frame: `GRect(w - 70, meta_y + 4, 68, 18)` (right-aligned, vertically centered in meta row)
- Font: `FONT_KEY_GOTHIC_14`
- Alignment: `GTextAlignmentRight`

In `update_display`, replace the stale text logic:

```c
static char s_fresh_buf[16];
int32_t mins = minutes_since_last_read();
GColor fresh_color;
if (mins > 9000) {
  snprintf(s_fresh_buf, sizeof(s_fresh_buf), "* --");
  fresh_color = GColorLightGray;
} else if (mins < 5) {
  snprintf(s_fresh_buf, sizeof(s_fresh_buf), "* %ldm", (long)mins);
  fresh_color = GColorMintGreen;
} else if (mins <= 15) {
  snprintf(s_fresh_buf, sizeof(s_fresh_buf), "* %ldm", (long)mins);
  fresh_color = GColorChromeYellow;
} else {
  snprintf(s_fresh_buf, sizeof(s_fresh_buf), "* %ldm", (long)mins);
  fresh_color = GColorRed;
}
text_layer_set_text(s_stale_layer, s_fresh_buf);
text_layer_set_text_color(s_stale_layer, fresh_color);
```

Note: use `*` as a substitute for `●` if the bullet character doesn't render cleanly in
GOTHIC_14. Test on device/emulator to confirm. If `●` renders, prefer it.

## Task 5: Add separator line

Add a `s_separator_layer` (a plain `Layer`, not `TextLayer`) between the meta row and
the graph. The update proc draws one horizontal line.

```c
static Layer *s_separator_layer;

static void separator_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(0, 0), GPoint(b.size.w, 0));
}
```

Frame: `GRect(4, separator_y, w - 8, 4)` where `separator_y = meta_y + 26`.

Create in `main_window_load`, destroy in `main_window_unload`.

## Task 6: Move time to footer, update layout

**Footer layout (normal mode):**
- Date (left): `GRect(4, h - 20, w / 2 - 4, 18)` — GOTHIC_14, left-aligned, GColorWhite
- Time (right): `GRect(w / 2, h - 22, w / 2 - 4, 24)` — GOTHIC_24_BOLD, right-aligned, GColorWhite

**Status bar (normal mode):**
- BT icon: `GRect(2, 4, 20, 20)` — unchanged
- Top-right: empty (no time element here in normal mode)

**Quick View compact mode:**
- Footer hidden (graph hidden, date hidden)
- Time moves to top-right: `GRect(w - 75, 2, 73, 22)` — GOTHIC_24_BOLD, right-aligned
- Meta row (delta + freshness) remains visible

Update `prv_layout_for_bounds` for both `compact` and normal branches.

## Task 7: Recompute all Y coordinates in prv_layout_for_bounds

After all layer changes, audit `prv_layout_for_bounds` to ensure:
- `glucose_y` = 24 (normal), 22 (compact)
- `meta_y` = glucose_y + 58
- `separator_y` = meta_y + 26
- `graph_y` = separator_y + 6
- `graph_h` = h - graph_y - 24 (normal), clamp to min 30
- Footer at h - 22 (normal only)
