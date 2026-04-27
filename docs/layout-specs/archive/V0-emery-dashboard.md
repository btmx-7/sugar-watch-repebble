# Emery · Dashboard Layout Spec

Platform: Emery (T2) — 200 × 228 px  
Layout: `LAYOUT_DASHBOARD`  
Source: `src/c/main.c`, function `prv_layout_for_bounds()` — T2 / dashboard branch  
Coordinates: origin top-left, y-axis downward. Match Figma 1:1.

---

## Coordinate & value conventions

| Concept | Value |
|---|---|
| Canvas size | 200 × 228 px |
| Coordinate origin | top-left (0, 0) |
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

Slot layers are custom-drawn (`slot_update_proc`). Coordinates below are the **layer frame** (absolute on screen). All internal sub-element coordinates are **relative to the slot layer origin**.

### Layer frames

| Element | X | Y | W | H | source-file | line |
|---|---|---|---|---|---|---|
| slot_layer[0] (left) | 8 | 4 | 56 | 56 | `src/c/main.c` | 1299 |
| slot_layer[1] (center) | 72 | 4 | 56 | 56 | `src/c/main.c` | 1300 |
| slot_layer[2] (right) | 136 | 4 | 56 | 56 | `src/c/main.c` | 1301 |

### Sub-elements (relative coords, all slots identical)

| Sub-element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Arc track (background) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 6 | `surface/border/subtle` (#005555) when has data; `state/disabled` (#555555) when no data | N/A | `src/c/main.c` | 690–694 |
| Arc fill (gauge) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 2 | icon_color (state-driven, see below) | `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 732–735 |
| Icon | 0 | 0 | 56 | 20 | 16 | `MATERIAL_SYMBOLS_16` (filled) / `MATERIAL_SYMBOLS_REGULAR_16` (outline) | icon_color (state-driven) | transparent | N/A | N/A | Same as arc fill | `src/c/main.c` | 745–748 |
| Value | 5 | 16 | 46 | 20 | 20 | `DATA_VALUE_20` | `text/inverted` (#FFFFFF) + 2px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 766–777 |
| Unit | 0 | 36 | 56 | 14 | 10 | `DATA_UNIT_10` | `text/inverted` (#FFFFFF) + 1px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 786–796 |

---

## Clock

All five layers use `TIME_DIGITS_56` (56 px cap-height, `s_time_font_dash`). bg-color = transparent. stroke-w = N/A. Created in `main_window_load`; positioned in `prv_layout_for_bounds`.

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| H1 (hour tens) | 8 | 72 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1327 |
| H2 (hour units) | 46 | 72 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1329 |
| Colon | 88 | 72 | 24 | 56 | 56 | `TIME_DIGITS_56` | `text/inverted` (#FFFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1331 |
| M1 (minute tens) | 112 | 72 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/default` (#00FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1333 |
| M2 (minute units) | 150 | 72 | 42 | 56 | 56 | `TIME_DIGITS_56` | `text/default` (#00FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1335 |

> Note: H1/H2 overlap by −4 px (H2.x = H1.x + 38). M1/M2 same. Colon is 24 px wide, no overlap.

---

## Status indicators

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bluetooth icon | 4 | 80 | 16 | 16 | 16 | `MATERIAL_SYMBOLS_16` | `icon/default` (#55FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1337 |
| Day | 176 | 74 | 20 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1339 |
| Month | 176 | 94 | 20 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1341 |

---

## CGM panel

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Graph (sparkline) | 4 | 130 | 120 | 80 | N/A | N/A | N/A | transparent | N/A | N/A | Dot color: state/danger, state/warning, state/positive per zone | `src/c/main.c` | 1347 |
| Trend name | — | — | — | — | — | — | — | — | — | — | **Hidden on T2** (BUG-09) | `src/c/main.c` | 1349 |
| Trend icon | 128 | 134 | 68 | 20 | 16 | `MATERIAL_SYMBOLS_16` | state-driven (see below) | transparent | N/A | N/A | `state/inactive` no data; `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 1352 |
| Glucose value | 128 | 156 | 68 | 36 | 20 | `DATA_VALUE_20` | state-driven (see below) | transparent | N/A | N/A | `text/default` no data; `state/positive` in-range; `state/warning` high/low; `state/danger` urgent; `state/inactive` stale | `src/c/main.c` | 1355 |
| Unit label | 128 | 194 | 68 | 14 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1360 |

> Text alignment: trend icon = right; glucose = right; unit = right.

---

## Figma spec — fill in from Figma export

Replace `?` values with your Figma measurements. Keep all other columns intact.

| Element | Figma X | Figma Y | Figma W | Figma H | Notes |
|---|---|---|---|---|---|
| slot_layer[0] | 4 | 4 | 56 | 56 | |
| slot_layer[1] | 72 | 4 | 56 | 56 | |
| slot_layer[2] | 140 | 4 | 56 | 56 | |
| H1 | 24 | 90 | 40 | 48 | Text is center aligned |
| H2 | 56 | 90 | 40 | 48 | Text is center aligned |
| Colon | 92 | 90 | 16 | 48 | Text is center aligned |
| M1 | 104 | 90 | 40 | 48 | Text is center aligned |
| M2 | 136 | 90 | 40 | 48 | Text is center aligned |
| Bluetooth icon | 4 | 104 | 20 | 20 | |
| Day | 176 | 94 | 20 | 16 | Text is right aligned |
| Month | 176 | 118 | 20 | 16 | Text is right aligned |
| Graph | 4 | 168 | 124 | 56 | Need to figure out the thresholsds min and max position, values and according graph signifers |
| Trend icon | 132 | 194 | 16 | 16 | |
| Glucose value | 150 | 192 | 46 | 20 | font-size 20px, line-height 20px, text is right aligned |
| Unit label | 150 | 214 | 46 | 10 | font-size 8px, line-height 10px, text is right aligned |
| Trend label | 132 | 176 | 64 | 14 | font-size 12px, line-height 14px, text is right aligned |
