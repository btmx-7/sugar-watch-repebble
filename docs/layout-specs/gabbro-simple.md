# Gabbro · Simple Layout Spec

Platform: Gabbro (R2) — 260 × 260 px (square canvas; physical display clips to inscribed circle r=130)  
Layout: `LAYOUT_SIMPLE`  
Source: `src/c/main.c`, function `prv_layout_for_bounds()` — R2 / simple branch  
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
| `TIME_DIGITS_64` | 64 px | `[0-9:]` | `s_time_font` |
| `DATA_VALUE_20` | 20 px | `[-0-9.k]` | `s_value_font` |
| `DATA_UNIT_10` | 10 px | `[°CmgdLbp%stekou/l]` | `s_unit_font` |
| `MATERIAL_SYMBOLS_16` | 16 px | filled glyphs | `s_symbol_font` |
| `MATERIAL_SYMBOLS_REGULAR_16` | 16 px | outline glyphs | `s_symbol_font_regular` |
| `FONT_KEY_GOTHIC_14` | 14 px | system font | — |

---

## Slot badges (slot_layer[0..3])

Cross arrangement: Weather (center-left), Battery (top-center), CGM (bottom-center), HR (center-right). Figma ref: node 329:20387.

### Layer frames

| Element | X | Y | W | H | source-file | line |
|---|---|---|---|---|---|---|
| slot_layer[0] (center-left / Weather) | 4 | 102 | 56 | 56 | `src/c/main.c` | 1239 |
| slot_layer[1] (top-center / Battery) | 102 | 4 | 56 | 56 | `src/c/main.c` | 1240 |
| slot_layer[2] (bottom-center / CGM) | 102 | 200 | 56 | 56 | `src/c/main.c` | 1241 |
| slot_layer[3] (center-right / HR) | 200 | 102 | 56 | 56 | `src/c/main.c` | 1242 |

> slot_layer[2] and [3] are hidden in compact (Quick View) mode.  
> All four slots sit on cardinal axes of the circle. slot_layer[0] at x=4: leftmost edge clips slightly but content anchor is inset.

### Sub-elements (relative coords, all slots identical)

| Sub-element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Arc track (background) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 6 | `surface/border/subtle` (#005555) when has data; `state/disabled` (#555555) when no data | N/A | `src/c/main.c` | 690–694 |
| Arc fill (gauge) | 3 | 4 | 50 | 50 | N/A | N/A | N/A | N/A | 2 | icon_color (state-driven) | `state/positive` in-range; `state/warning` high/low; `state/danger` urgent | `src/c/main.c` | 732–735 |
| Icon | 0 | 0 | 56 | 20 | 16 | `MATERIAL_SYMBOLS_16` (filled) / `MATERIAL_SYMBOLS_REGULAR_16` (outline) | icon_color (state-driven) | transparent | N/A | N/A | Same as arc fill | `src/c/main.c` | 745–748 |
| Value | 5 | 16 | 46 | 20 | 20 | `DATA_VALUE_20` | `text/inverted` (#FFFFFF) + 2px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 766–777 |
| Unit | 0 | 36 | 56 | 14 | 10 | `DATA_UNIT_10` | `text/inverted` (#FFFFFF) + 1px black outline | transparent | N/A | N/A | N/A | `src/c/main.c` | 786–796 |

---

## Clock digits

Simple layout uses `TIME_DIGITS_64` (64 px cap-height, `s_time_font`). Each digit has two layers: a **stroke layer** (outline, drawn first) and a **fill text layer** (drawn on top).

### Fill layers (TextLayer)

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| H1 fill (hour tens) | 83 | 74 | 48 | 70 | 64 | `TIME_DIGITS_64` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1256 |
| H2 fill (hour units) | 129 | 74 | 48 | 70 | 64 | `TIME_DIGITS_64` | `text/inverted` (#FFFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1257 |
| M1 fill (minute tens) | 83 | 116 | 48 | 70 | 64 | `TIME_DIGITS_64` | `text/inverted` (#FFFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1258 |
| M2 fill (minute units) | 129 | 116 | 48 | 70 | 64 | `TIME_DIGITS_64` | `text/default` (#00FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1259 |

### Stroke layers (custom Layer, expanded ±4 px from fill frame)

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| H1 stroke | 79 | 70 | 56 | 78 | 64 | `TIME_DIGITS_64` | GColorBlack (outline) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1266 |
| H2 stroke | 125 | 70 | 56 | 78 | 64 | `TIME_DIGITS_64` | GColorBlack (outline) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1267 |
| M1 stroke | 79 | 112 | 56 | 78 | 64 | `TIME_DIGITS_64` | GColorBlack (outline) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1268 |
| M2 stroke | 125 | 112 | 56 | 78 | 64 | `TIME_DIGITS_64` | GColorBlack (outline) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1269 |

> Group spans x=83..177, y=74..186, centered at (130, 130).  
> H1/H2 overlap: H2.x = H1.x + 46 (−2 px). M1/M2 same. Hour row (y=74) and minute row (y=116) overlap by −28 px.

---

## Status indicators

| Element | X | Y | W | H | font-size | font-resource-ID | text-color | bg-color | stroke-w | stroke-color | state-color-variants | source-file | line |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bluetooth icon | 52 | 192 | 16 | 16 | 16 | `MATERIAL_SYMBOLS_16` | `icon/default` (#55FFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1277 |
| Day | 48 | 52 | 24 | 16 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1279 |
| Month | 188 | 52 | 24 | 16 | 14 | `FONT_KEY_GOTHIC_14` | `text/subtle` (#AAFFFF) | transparent | N/A | N/A | N/A | `src/c/main.c` | 1283 |

> Day and Month: center-aligned. Bluetooth icon: bottom-left area. Per Figma 329:20387.

---

## Figma spec — fill in from Figma export

Replace `?` values with your Figma measurements. Keep all other columns intact.

| Element | Figma X | Figma Y | Figma W | Figma H | Notes |
|---|---|---|---|---|---|
| slot_layer[0] | ? | ? | ? | ? | Weather — center-left |
| slot_layer[1] | ? | ? | ? | ? | Battery — top-center |
| slot_layer[2] | ? | ? | ? | ? | CGM — bottom-center |
| slot_layer[3] | ? | ? | ? | ? | HR — center-right |
| H1 fill | ? | ? | ? | ? | |
| H2 fill | ? | ? | ? | ? | |
| M1 fill | ? | ? | ? | ? | |
| M2 fill | ? | ? | ? | ? | |
| H1 stroke | ? | ? | ? | ? | Outer bounding box, expanded ±4 px |
| H2 stroke | ? | ? | ? | ? | Outer bounding box, expanded ±4 px |
| M1 stroke | ? | ? | ? | ? | Outer bounding box, expanded ±4 px |
| M2 stroke | ? | ? | ? | ? | Outer bounding box, expanded ±4 px |
| Bluetooth icon | ? | ? | ? | ? | |
| Day | ? | ? | ? | ? | |
| Month | ? | ? | ? | ? | |
