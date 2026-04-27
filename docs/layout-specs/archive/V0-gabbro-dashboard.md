# Gabbro · Dashboard Layout Spec

Platform: Gabbro (R2) — 260 × 260 px (square canvas; physical display clips to inscribed circle r=130)  
Layout: `LAYOUT_DASHBOARD`  
Source: `src/c/main.c`, function `prv_layout_for_bounds()` — R2 / dashboard branch  
Coordinates: origin top-left, y-axis downward. Figma 260×260 square maps directly — no translation needed.

---

## Coordinate & value conventions

| Concept | Value |
|---|---|
| Canvas size | 260 × 260 px |
| Coordinate origin | top-left (0, 0) |
| Circular clip | inscribed circle center (130, 130), r = 130 — applied by OS at render time |
| Font size unit | cap-height px (equals Figma px) |
| GColor format | `argb` byte — 2 bits/channel (00=0x00, 01=0x55, 10=0xAA, 11=0xFF) |
| bg-color `transparent` | `GColorClear` |
| bg-color `black` | `GColorBlack` (#000000) |

---

## Color tokens

| Token alias | Hex | argb |
|---|---|---|
| `text/subtle` | #AAFFFF | 0xEF |
| `text/inverted` | #FFFFFF | 0xFF |
| `text/default` | #00FFFF | 0xCF |
| `icon/default` | #55FFFF | 0xDF |
| `icon/subtle` | #00AAAA | 0xCA |
| `surface/border/subtle` | #005555 | 0xC5 |
| `surface/bg/subtle` | #005555 | 0xC5 |
| `state/danger` | #FF0000 | 0xF0 |
| `state/warning` | #FFAA00 | 0xF8 |
| `state/positive` | #00FFAA | 0xCE |
| `state/inactive` | #AAAAAA | 0xEA |
| `state/disabled` | #555555 | 0xD5 |

---

## Font resource IDs

| Resource ID | Cap-height | Character set | Variable |
|---|---|---|---|
| `TIME_DIGITS_56` | 56 px | `[0-9:]` | `s_time_font_dash` |
| `TIME_DIGITS_64` | 64 px | `[0-9:]` | `s_time_font` |
| `DATA_VALUE_20` | 20 px | `[-0-9.k]` | `s_value_font` |
| `DATA_UNIT_10` | 10 px | `[°CmgdLbp%stekou/l]` | `s_unit_font` |
| `MATERIAL_SYMBOLS_16` | 16 px | filled glyphs | `s_symbol_font` |
| `MATERIAL_SYMBOLS_REGULAR_16` | 16 px | outline glyphs | `s_symbol_font_regular` |
| `FONT_KEY_GOTHIC_14` | 14 px | system font | — |

---

## Slot badges (slot_layer[0..2])

slot_layer[3] is always hidden in Dashboard on R2. Slot layers are custom-drawn (`slot_update_proc`). Coordinates below are the **layer frame** (absolute on 260×260 canvas). All internal sub-element coordinates are **relative to the slot layer origin**.

### Layer frames

| Element | X | Y | W | H | source-file | line |
|---|---|---|---|---|---|---|
| slot_layer[0] (left) | 42 | 34 | 56 | 56 | `src/c/main.c` | 1381 |
| slot_layer[1] (center top) | 102 | 14 | 56 | 56 | `src/c/main.c` | 1382 |
| slot_layer[2] (right) | 162 | 34 | 56 | 56 | `src/c/main.c` | 1383 |

> Slot 0/2 are staggered 20 px lower than slot 1 to respect the circular boundary. At y=34 the chord starts at x≈43, so x=42 keeps content inside the circle.

### Sub-elements (relative coords, all slots identical)

| Sub-element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Arc track (background) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 6 | `surface/border/subtle` (#005555) when has data; `state/disabled` (#555555) when no data | N/A | `src/c/main.c` | 690–694 |
| Arc fill (gauge) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 2 | icon_color (state-driven) | `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 732–735 |
| Icon | 0 | 0 | 56 | 20 | 16 | `MATERIAL_SYMBOLS_16` (filled) / `MATERIAL_SYMBOLS_REGULAR_16` (outline) | icon_color (state-driven) | transparent | N/A | N/A | Same as arc fill | `src/c/main.c` | 745–748 |
| Value | 5 | 16 | 46 | 20 | 20 | `DATA_VALUE_20` | `text/inverted` (#FFFFFF) + 2px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 766–777 |
| Unit | 0 | 36 | 56 | 14 | 10 | `DATA_UNIT_10` | `text/inverted` (#FFFFFF) + 1px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 786–796 |

---

## Clock

All five layers use `TIME_DIGITS_56` (56 px cap-height, `s_time_font_dash`). bg-color = transparent. stroke-w = N/A. Created in `main_window_load`; positioned in `prv_layout_for_bounds`.

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| H1 (hour tens) | 38 | 86 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1399 |
| H2 (hour units) | 76 | 86 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1401 |
| Colon | 118 | 86 | 24 | 56 | 56 | `TIME_DIGITS_56` | `text/inverted` (#FFFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1403 |
| M1 (minute tens) | 142 | 86 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/default` (#00FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1405 |
| M2 (minute units) | 180 | 86 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/default` (#00FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1407 |

> Group spans x=38..222, centered at x=130. H1/H2 overlap by −4 px. M1/M2 same.

---

## Status indicators

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bluetooth icon | 10 | 94 | 16 | 16 | 16 | `MATERIAL_SYMBOLS_16` | `icon/default` (#55FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1409 |
| Day | 224 | 88 | 22 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1411 |
| Month | 224 | 108 | 22 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1413 |

> Day/Month placed right of M2 (ends at x=222). At y=88–122 the chord allows x up to ≈253, so x=224+22=246 stays inside the circle.

---

## CGM panel

The CGM panel on R2 sits below the graph (y≥210) and uses a two-row layout: trend name + unit on row 1, trend arrow + glucose on row 2.

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Graph (sparkline) | 30 | 148 | 150 | 60 | N/A | N/A | N/A | transparent | N/A | N/A | Dot color: state/danger, state/warning, state/positive per zone | `src/c/main.c` | 1417 |
| Trend name (row 1, right) | 40 | 210 | 86 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `state/inactive` (#AAAAAA) default; state-driven at runtime | transparent | N/A | N/A | `state/inactive` no data; `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 1422 |
| Unit label (row 1, left) | 130 | 210 | 64 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1428 |
| Trend icon (row 2, left) | 82 | 226 | 24 | 22 | 16 | `MATERIAL_SYMBOLS_16` | state-driven | transparent | N/A | N/A | `state/inactive` no data; `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 1435 |
| Glucose value (row 2, right) | 108 | 220 | 64 | 30 | 20 | `DATA_VALUE_20` | state-driven | transparent | N/A | N/A | `text/default` no data; `state/positive` in-range; `state/warning` high/low; `state/danger` urgent; `state/inactive` stale | `src/c/main.c` | 1439 |

> Text alignment: trend name = right; unit = left; trend icon = center; glucose = left.  
> Circular clip at y=252 allows x∈[85,175]. Arrow at x≈94, glucose ends ≤246 — verify against circle at those y values.

---

## Figma spec — fill in from Figma export

Replace `?` values with your Figma measurements. Keep all other columns intact.

| Element | Figma X | Figma Y | Figma W | Figma H | Notes |
|---|---|---|---|---|---|
| slot_layer[0] | ? | ? | ? | ? | |
| slot_layer[1] | ? | ? | ? | ? | |
| slot_layer[2] | ? | ? | ? | ? | |
| H1 | ? | ? | ? | ? | |
| H2 | ? | ? | ? | ? | |
| Colon | ? | ? | ? | ? | |
| M1 | ? | ? | ? | ? | |
| M2 | ? | ? | ? | ? | |
| Bluetooth icon | ? | ? | ? | ? | |
| Day | ? | ? | ? | ? | |
| Month | ? | ? | ? | ? | |
| Graph | ? | ? | ? | ? | |
| Trend name | ? | ? | ? | ? | |
| Unit label | ? | ? | ? | ? | |
| Trend icon | ? | ? | ? | ? | |
| Glucose value | ? | ? | ? | ? | |
