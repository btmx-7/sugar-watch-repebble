# Pebble Watchface UI Design Guide

> Based on official Pebble/Rebble developer documentation. Last compiled: 2026-04-07.

## Step 1: Know your canvas (hardware constraints)

Before designing anything, lock down which platforms you target. Each has different dimensions:

| Platform | Device | Screen | Resolution | Colors |
|----------|--------|--------|------------|--------|
| Aplite | Pebble Classic / Steel | Rectangular | 144 x 168 px | 2 (B/W) |
| Basalt | Pebble Time | Rectangular | 144 x 168 px | 64 |
| Chalk | Pebble Time Round | Circular | 180 x 180 px | 64 |
| Diorite | Pebble 2 | Rectangular | 144 x 168 px | 2 (B/W) |
| Emery | Pebble Time 2 | Rectangular | 200 x 228 px | 64 |
| Gabbro | Pebble Round 2 | Circular | 180 x 180 px | 64 |

Key constraints to internalize:
- No touch screen. Navigation is 4 physical buttons only.
- For watchfaces, **none of the buttons are available** to you (Up/Down go to Timeline, Back/Select are system-reserved).
- The screen is always-on, reflective memory LCD. Battery is precious.

Reference: https://developer.rebble.io/guides/tools-and-resources/hardware-information/

---

## Step 2: Establish information hierarchy

Pebble's core design philosophy is **glanceability**. The user looks at their wrist for 2-3 seconds.

Rules to follow:
- Identify **one primary data point** (for a glucose watch: the current BG reading). Make it the largest, most visually dominant element.
- Pick **two secondary data points** max (trend arrow, time of last reading). Use smaller fonts.
- Add **tertiary info** (date, battery) only if it doesn't compete visually.
- Remove everything else.

From the official guidelines: *"only as much information displayed as is immediately required"* and *"font size 28 for primary content, minimum 18 for secondary elements."*

---

## Step 3: Design your layout grid

### For rectangular displays (144x168 or 200x228)

A common and effective watchface layout:

```
+------------------+
|   [date / label] |  <- 20-30px tall, top zone
|                  |
|  [PRIMARY DATA]  |  <- 60-80px tall, center zone (dominant)
|  [secondary]     |  <- 30-40px, below primary
|                  |
|  [tertiary info] |  <- 20-30px, bottom zone
+------------------+
```

Practical rules:
- Keep a minimum **4px margin** on all edges.
- Align text consistently (all centered, or all left-aligned).
- Don't mix alignment within the same visual zone.

### For round displays (Chalk/Gabbro, 180x180 circular)

The circle clips corners. Safe content area is roughly a **140x140 inscribed square**.

- Center your primary data point exactly at the visual center.
- Avoid placing text in the four corner regions (they'll be clipped).
- Use the Round App UI guide for the `grect_inset()` API to apply safe insets.
- Horizontal rules or full-width rectangles look bad on round. Use arcs or radial layouts instead.

---

## Step 4: Build with the Layer system

Every visual element is a **Layer**. Layers stack from bottom to top.

The layer types you'll use most for a watchface:

| Layer | Use case |
|-------|----------|
| `TextLayer` | Any text: time, date, BG value, labels |
| `BitmapLayer` | Icons, trend arrows, background images |
| `Layer` (custom) | Custom drawn graphics via `graphics_draw_*` primitives |
| `StatusBarLayer` | System-style top bar (optional, takes 16px) |

Layer lifecycle pattern:

```c
// 1. Create when window loads
TextLayer *time_layer = text_layer_create(GRect(0, 60, 144, 60));

// 2. Configure
text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
text_layer_set_background_color(time_layer, GColorClear);

// 3. Add to parent layer
layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

// 4. Destroy when window unloads (memory management is critical)
text_layer_destroy(time_layer);
```

Reference: https://developer.rebble.io/docs/c/User_Interface/

---

## Step 5: Choose fonts deliberately

### System fonts (free, no size cost)

Key fonts for watchfaces:

| Font key | Best for |
|----------|----------|
| `FONT_KEY_BITHAM_42_BOLD` | Large time display |
| `FONT_KEY_BITHAM_30_BLACK` | Large secondary value |
| `FONT_KEY_ROBOTO_CONDENSED_21` | Date, labels |
| `FONT_KEY_GOTHIC_18_BOLD` | Small labels |
| `FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM` | Numeric-only displays |
| `FONT_KEY_ROBOTO_49_BOLD` | Very large numbers (digits + colon only) |

Rules:
- Use **no more than 2-3 font sizes** in the entire watchface.
- `ROBOTO_49_BOLD` and `BITHAM_*_NUMBERS` only contain digits and colons. Don't use them for text.
- Maximum recommended custom font size: **48px**.
- When adding custom fonts, limit character sets to reduce binary size (e.g., `[0-9:APM ]` for a time font).

Reference: https://developer.rebble.io/guides/app-resources/fonts/

---

## Step 6: Use color with intent

For color platforms (Basalt, Chalk, Emery, Gabbro): 64 colors from a fixed GColor palette.

Design rules:
- Use color to **convey meaning without text** (green = in range, red = alert).
- Only use red/orange/green when they carry their expected meaning. Using red text for non-error states confuses users.
- Maintain sufficient contrast. White text on light colors fails on the reflective LCD in sunlight.
- Test in both **bright and dark ambient light** conditions.

For B/W platforms (Aplite, Diorite): you only have black, white, and dithered patterns. Make sure your design degrades gracefully using `#ifdef PBL_COLOR` compile guards.

---

## Step 7: Handle the Unobstructed Area

When a user has **Timeline Peek** active, the bottom ~20% of your watchface is covered by the system peek bar.

You must handle this:

```c
// Subscribe to unobstructed area changes
UnobstructedAreaHandlers handlers = {
  .change = prv_unobstructed_area_change_handler
};
unobstructed_area_service_subscribe(handlers, NULL);

// In your handler: resize layers to fit the available area
static void prv_unobstructed_area_change_handler(AnimationProgress progress, void *context) {
  GRect available = unobstructed_area_get_bounds();
  // Reposition layers to fit within available
}
```

Reference: https://developer.rebble.io/guides/user-interfaces/unobstructed-area/

---

## Step 8: Set up the tick timer (the heartbeat)

This is how your watchface updates:

```c
// In init(), subscribe to tick events
tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Update time display
  // Update BG reading if available
}
```

Rules:
- Use `MINUTE_UNIT` by default. Using `SECOND_UNIT` drains battery significantly.
- Only switch to seconds if your watchface explicitly requires it (e.g., a stopwatch complication).

---

## Step 9: Handle platform differences correctly

Use compile-time guards to adapt your layout:

```c
// Round vs rectangular layout
#if defined(PBL_ROUND)
  // Chalk/Gabbro: apply safe insets, radial layouts
  GRect bounds = grect_inset(layer_get_bounds(root_layer), GEdgeInsets(15));
#else
  // Rectangular: full-width layouts work
  GRect bounds = layer_get_bounds(root_layer);
#endif

// Color vs B/W
#if defined(PBL_COLOR)
  text_layer_set_text_color(bg_layer, GColorGreen);
#else
  text_layer_set_text_color(bg_layer, GColorBlack);
#endif
```

Reference: https://developer.rebble.io/guides/best-practices/building-for-every-pebble/

---

## Step 10: Polish - animations and feedback

Animations add character but must be used sparingly on a battery-constrained device:

- Use `Animation` + `PropertyAnimationImplementation` for smooth transitions.
- Animate only on meaningful state changes (new BG reading arriving, alert state change).
- Keep animations short: 200-400ms is the sweet spot.
- Never animate continuously (spinning, pulsing). It kills battery.

From the guidelines: *"use animations to add character and flourish while drawing attention to changing information."*

---

## Watchface UI checklist

Before shipping:

- [ ] One clearly dominant primary data point
- [ ] Max 3 font sizes used
- [ ] Margins respected (4px minimum)
- [ ] Round display safe zone handled (if targeting Chalk/Gabbro)
- [ ] Unobstructed area handled (Timeline Peek compatible)
- [ ] Color degrades gracefully to B/W on Aplite/Diorite
- [ ] Tick timer uses `MINUTE_UNIT` unless seconds are needed
- [ ] All layers destroyed in `window_unload` (no memory leaks)
- [ ] Tested on all target platforms in the emulator

---

## Sources

- [Benefits of Design Guidelines](https://developer.rebble.io/developer.pebble.com/guides/design-and-interaction/benefits/index.html)
- [Recommended Guidelines and Patterns](https://developer.rebble.io/developer.pebble.com/guides/design-and-interaction/recommended/index.html)
- [User Interface API](https://developer.rebble.io/docs/c/User_Interface/)
- [Fonts guide](https://developer.rebble.io/guides/app-resources/fonts/)
- [Hardware Information](https://developer.rebble.io/guides/tools-and-resources/hardware-information/)
- [Building for Every Pebble](https://developer.rebble.io/guides/best-practices/building-for-every-pebble/)
- [Unobstructed Area](https://developer.rebble.io/guides/user-interfaces/unobstructed-area/)
