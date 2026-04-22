## Platform Dimensions

### Time 2 (emery)
Canvas: 200 × 228 px, rectangular, color e-paper.

### Round 2 (gabbro)
Canvas: 260 × 260 px, circular clip (inscribed circle diameter 260 px), color e-paper.
Circular boundary constraint: at y=246 the visible horizontal band narrows to
x ≈ 77..183 (106 px). All CGM content on R2 must fit within this envelope.

---

## Zone Breakdown

### Time 2 (200 × 228) — unchanged from full-redesign

| Zone | y | h | Purpose |
|---|---|---|---|
| slots | 0 | 64 | 3 top widget slots (56×56, 4 px margin) |
| time | 64 | 66 | HH:MM, BT icon, day/month |
| cgm-panel | 130 | 98 | Left 120 px: annotated sparkline graph · Right 68 px: CGM sidebar |
| **Total** | | **228** | |

### Round 2 (260 × 260) — cgm-panel split into graph + below-graph

| Zone | y | h | Purpose |
|---|---|---|---|
| slots | 0 | 80 | 3 top widget slots (56×56, inscribed) |
| time | 80 | 68 | HH:MM, BT icon, day/month |
| graph | 148 | 60 | Annotated sparkline (height reduced from 76 to 60 to fit below-graph panel) |
| below-graph | 210 | 38 | 2-row CGM panel: [trend name + unit] / [trend icon + glucose value] |
| **Total** | | **260** (8 px margin at bottom) | |

---

## Layer Inventory

**Legend:** `[existing]` = correct as shipped · `[corrected]` = frame changes on R2 ·
`[NEW]` = added by this change · `[T2 only]` / `[R2 only]` = platform-conditional

### Shared layers (both platforms)

| Layer | Type | Font | Default color |
|---|---|---|---|
| s_slot_layer[0..2] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | CLR_ICON_DEFAULT arc |
| s_slot_layer[3] `[existing]` | Layer | — | hidden always |
| s_dash_time_layer `[existing]` | TextLayer | LECO_36_BOLD_NUMBERS | GColorWhite |
| s_dash_bt_layer `[existing]` | TextLayer | MATERIAL_SYMBOLS_16 | CLR_ICON_DEFAULT |
| s_dash_day_layer `[existing]` | TextLayer | GOTHIC_14 | CLR_TEXT_SUBTLE |
| s_dash_month_layer `[existing]` | TextLayer | GOTHIC_14 | CLR_TEXT_SUBTLE |
| s_graph_layer `[corrected R2]` | Layer | — | zone-colored line |
| s_dash_trend_layer `[corrected R2]` | TextLayer | MATERIAL_SYMBOLS_16 | zone-colored |
| s_dash_glucose_layer `[corrected R2]` | TextLayer | GOTHIC_24_BOLD | zone-colored |
| s_dash_unit_layer `[corrected R2]` | TextLayer | GOTHIC_14 | GColorMediumAquamarine |
| s_dash_trend_name_layer `[NEW]` | TextLayer | GOTHIC_14 | CLR_STATE_INACTIVE |

### Time 2 — exact frames

| Layer | Frame (x,y,w,h) | Align | Note |
|---|---|---|---|
| s_slot_layer[0] | GRect(4,4,56,56) | — | existing |
| s_slot_layer[1] | GRect(72,4,56,56) | — | existing |
| s_slot_layer[2] | GRect(140,4,56,56) | — | existing |
| s_dash_time_layer | GRect(24,76,144,44) | Center | existing |
| s_dash_bt_layer | GRect(4,80,16,16) | Center | existing |
| s_dash_day_layer | GRect(176,74,20,14) | Right | existing |
| s_dash_month_layer | GRect(176,94,20,14) | Right | existing |
| s_graph_layer | GRect(4,130,120,80) | — | existing |
| s_dash_trend_name_layer | GRect(128,130,68,14) | Right | `[corrected]` row 1 visible (was hidden) |
| s_dash_trend_layer | GRect(128,146,68,18) | Right | `[corrected]` row 2 (y 130→146) |
| s_dash_glucose_layer | GRect(128,162,68,30) | Right | `[corrected]` row 3 (y 152→162) |
| s_dash_unit_layer | GRect(128,192,68,14) | Right | `[corrected]` row 4 (y 182→192) |

Rationale: Figma exports (`Dashboard_T2-1..5`) show a 4-row sidebar stack. The
trend-name row is visible on T2, not only R2 as the original proposal assumed.

### Round 2 — exact frames (`[corrected]` = changed from current code)

| Layer | Frame (x,y,w,h) | Align | Note |
|---|---|---|---|
| s_slot_layer[0] | GRect(20,20,56,56) | — | existing |
| s_slot_layer[1] | GRect(102,14,56,56) | — | existing |
| s_slot_layer[2] | GRect(184,20,56,56) | — | existing |
| s_dash_time_layer | GRect(50,90,160,44) | Center | existing |
| s_dash_bt_layer | GRect(10,94,16,16) | Center | existing |
| s_dash_day_layer | GRect(234,88,20,14) | Right | existing |
| s_dash_month_layer | GRect(234,108,20,14) | Right | existing |
| s_graph_layer | GRect(30,148,150,**60**) | — | `[corrected]` height 76→60 |
| s_dash_trend_name_layer | GRect(40,210,86,14) | Right | `[corrected]` row 1 left half, meets unit at center |
| s_dash_unit_layer | GRect(130,210,64,14) | Left | `[corrected]` row 1 right half |
| s_dash_trend_layer | GRect(82,226,24,22) | Center | `[corrected]` row 2 left |
| s_dash_glucose_layer | GRect(108,220,64,30) | Left | `[corrected]` row 2 right, h 20→30 for GOTHIC_24_BOLD |

R2 boundary check at y=252 (bottom of row 2): x_min ≈ 85, x_max ≈ 175.
Row 1 sits at y=210..224 (wide band); row 2 trailing edge x=108+64=172 < 175 ✓.
Row 1 pair is centered at x≈130 as a single visual group (Figma `Dashboard_R2-1..5`),
not spread edge-to-edge. Row 2 uses height 30 so GOTHIC_24_BOLD digits do not clip.

---

## Graph Threshold Labels (both platforms)

Drawn inside `graph_layer_update_proc` using `graphics_draw_text()`.
No new layers required — rendered directly on the graph canvas.

| Text | Position within layer | Font | Color |
|---|---|---|---|
| "hyper" | above high line (falls back below if clipped at top) | FONT_KEY_GOTHIC_14 | GColorLightGray |
| high value ("230") | above high line (mirrors "hyper") | FONT_KEY_GOTHIC_14 | GColorLightGray |
| "hypo" | below low line (falls back above if clipped at bottom) | FONT_KEY_GOTHIC_14 | GColorLightGray |
| low value ("65") | below low line (mirrors "hypo") | FONT_KEY_GOTHIC_14 | GColorLightGray |

`hy` = `VAL_TO_Y(s_settings.high_thresh)`, `ly` = `VAL_TO_Y(s_settings.low_thresh)`.
Guard: only draw if the threshold is within `[min_val, max_val]`. Label y picks
`above` or `below` based on available room — prevents hypo clipping when the
low line sits near the graph's bottom edge (observed in every T2 state capture).

---

## Font Reference

| Constant | Approx height | Used for |
|---|---|---|
| FONT_KEY_LECO_36_BOLD_NUMBERS | ~36 px | Dashboard time (HH:MM) |
| RESOURCE_ID_MATERIAL_SYMBOLS_16 | 16 px | BT icon, trend arrow |
| FONT_KEY_GOTHIC_24_BOLD | ~24 px | Glucose value (T2 sidebar) |
| FONT_KEY_GOTHIC_14 | ~14 px | Day, month, unit, trend name, graph labels |

---

## Color Palette

| Token / GColor | Hex | Semantic |
|---|---|---|
| CLR_STATE_DANGER | #FF0000 | URGENT_LOW, URGENT_HIGH |
| CLR_STATE_WARNING | #FFAA00 | LOW zone |
| GColorChromeYellow | #FFFF00 | HIGH zone `[changed by this spec]` |
| CLR_ICON_DEFAULT | #55FFFF | IN_RANGE zone, arc stroke, BT connected |
| GColorLightGray | #AAAAAA | Stale/no data, graph threshold labels |
| CLR_STATE_INACTIVE | #AAAAAA | Trend name default (no data) |
| GColorWhite | #FFFFFF | Time text |
| CLR_TEXT_SUBTLE | #AAFFFF | Day, month |
| GColorMediumAquamarine | #00AAAA | Unit label |

`zone_color()` must return `GColorChromeYellow` for `ZONE_HIGH`
(currently returns `CLR_STATE_WARNING`).

---

## Quick View / Compact Mode

Hidden in compact mode (both platforms):
- `s_slot_layer[0..2]` — top 3 widget slots
- `s_graph_layer` — sparkline graph

Visible and unchanged in compact mode:
- Time row: `s_dash_time_layer`, `s_dash_bt_layer`, `s_dash_day_layer`, `s_dash_month_layer`
- T2 sidebar: `s_dash_trend_layer`, `s_dash_glucose_layer`, `s_dash_unit_layer`
- R2 below-graph panel: all four repositioned layers + `s_dash_trend_name_layer`

---

## ASCII Diagram

```
Time 2 (200 × 228)                          Compact / Quick View
┌──────────────────────────────────────┐     ┌──────────────────────────────────────┐
│  [slot0 56×56]  [slot1]  [slot2]     │     │  (slots hidden)                      │
│  y=4            y=4      y=4         │     │                                      │
├──────────────────────────────────────┤     │  [BT] HH:MM (LECO_36) [DD]           │
│  [BT]  HH:MM (LECO_36)  [DD]        │     │        y=76               [MM]        │
│         y=76              [MM]       │     ├──────────────────┬───────────────────┤
├──────────────────┬───────────────────┤     │  (graph hidden)  │  [name]    y=130 │
│  graph  120×80   │  [name]     y=130 │     │                  │  [arrow]   y=146 │
│  y=130..210      │  [arrow]    y=146 │     │                  │  [glucose] y=162 │
│  hyper---230---  │  [glucose]  y=162 │     │                  │  [unit]    y=192 │
│  hypo----65----  │  [unit]     y=192 │     └──────────────────┴───────────────────┘
└──────────────────┴───────────────────┘

Round 2 (260 × 260) — circular clip
┌──────────────────────────────────────────┐
│    [slot0 y=20]  [slot1 y=14]  [slot2]   │
│                                          │
│  [BT]    HH:MM (LECO_36)       [DD][MM] │
│           y=90                           │
│                                          │
│     graph  150×60  y=148..208            │
│     hyper-----------230-----------       │
│     hypo-----------65-------------       │
│                                          │
│       [trend name] [unit]        y=210   │
│        [→] [glucose value]       y=222   │
└──────────────────────────────────────────┘
```
