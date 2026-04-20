## Platform Dimensions

### Time 2 (emery)
Canvas: 200 × 228 px, rectangular, color e-paper.

### Round 2 (gabbro)
Canvas: 260 × 260 px, circular clip (inscribed circle diameter 260 px), color e-paper.
Note: corners are clipped. Keep critical content within the central 220 px diameter.
The right edge of the CGM sidebar (x ≈ 230) clips below y ≈ 215 — new layers on R2
must use reduced width or be omitted (see R2 notes in Layer Inventory).

---

## Zone Breakdown

### Time 2 (200 × 228)

| Zone | y | h | Purpose |
|---|---|---|---|
| slots | 0 | 64 | 3 widget circles (56×56, 4 px top margin) |
| time | 64 | 66 | HH:MM, BT icon, day/month |
| cgm-panel | 130 | 98 | Left 120 px: sparkline graph · Right 68 px: CGM sidebar |
| **Total** | | **228** | |

### Round 2 (260 × 260)

| Zone | y | h | Purpose |
|---|---|---|---|
| slots | 0 | 80 | 3 widget circles (56×56, inscribed) |
| time | 80 | 68 | HH:MM, BT icon, day/month |
| cgm-panel | 148 | 112 | Left 150 px: sparkline graph · Right 42 px: CGM sidebar |
| **Total** | | **260** | |

---

## Layer Inventory

Only Dashboard-specific layers are listed. `s_slot_layer[3]` is always hidden in
Dashboard. `s_slot_layer[0..2]` are hidden in compact mode on both platforms.

**Legend:** `[existing]` = as-shipped; `[NEW]` = added by this change.

### Time 2 (200 × 228)

| Layer | Type | Font | Frame (x,y,w,h) | Align | Default color |
|---|---|---|---|---|---|
| s_slot_layer[0] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(4,4,56,56) | — | CLR_ICON_DEFAULT arc |
| s_slot_layer[1] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(72,4,56,56) | — | CLR_ICON_DEFAULT arc |
| s_slot_layer[2] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(140,4,56,56) | — | CLR_ICON_DEFAULT arc |
| s_dash_time_layer `[existing]` | TextLayer | LECO_36_BOLD_NUMBERS | GRect(24,76,144,44) | Center | GColorWhite |
| s_dash_bt_layer `[existing]` | TextLayer | MATERIAL_SYMBOLS_16 | GRect(4,80,16,16) | Center | CLR_ICON_DEFAULT |
| s_dash_day_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(176,74,20,14) | Right | CLR_TEXT_SUBTLE |
| s_dash_month_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(176,94,20,14) | Right | CLR_TEXT_SUBTLE |
| s_graph_layer `[existing]` | Layer | — | GRect(4,130,120,80) | — | zone-colored line |
| s_dash_trend_layer `[existing]` | TextLayer | MATERIAL_SYMBOLS_16 | GRect(128,130,68,20) | Right | zone-colored |
| s_dash_glucose_layer `[existing]` | TextLayer | GOTHIC_24_BOLD | GRect(128,152,68,30) | Right | zone-colored |
| s_dash_unit_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(128,182,68,14) | Right | GColorMediumAquamarine |
| s_dash_delta_layer `[NEW]` | TextLayer | GOTHIC_14 | GRect(128,196,68,14) | Right | zone-colored |
| s_dash_fresh_layer `[NEW]` | TextLayer | GOTHIC_14 | GRect(128,210,68,14) | Right | CLR_STATE_INACTIVE |
| s_dash_zone_layer `[NEW]` | TextLayer | GOTHIC_14 | GRect(128,210,68,14) | Right | zone-colored |

`s_dash_fresh_layer` and `s_dash_zone_layer` share position y=210 and are
mutually exclusive: fresh shown when in-range, zone label shown when out of range.
Bottom of lowest element: y=224 (4 px margin to screen edge at 228).

### Round 2 (260 × 260)

| Layer | Type | Font | Frame (x,y,w,h) | Align | Default color |
|---|---|---|---|---|---|
| s_slot_layer[0] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(20,20,56,56) | — | CLR_ICON_DEFAULT arc |
| s_slot_layer[1] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(102,14,56,56) | — | CLR_ICON_DEFAULT arc |
| s_slot_layer[2] `[existing]` | Layer | MATERIAL_SYMBOLS_16 | GRect(184,20,56,56) | — | CLR_ICON_DEFAULT arc |
| s_dash_time_layer `[existing]` | TextLayer | LECO_36_BOLD_NUMBERS | GRect(50,90,160,44) | Center | GColorWhite |
| s_dash_bt_layer `[existing]` | TextLayer | MATERIAL_SYMBOLS_16 | GRect(10,94,16,16) | Center | CLR_ICON_DEFAULT |
| s_dash_day_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(234,88,20,14) | Right | CLR_TEXT_SUBTLE |
| s_dash_month_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(234,108,20,14) | Right | CLR_TEXT_SUBTLE |
| s_graph_layer `[existing]` | Layer | — | GRect(30,148,150,76) | — | zone-colored line |
| s_dash_trend_layer `[existing]` | TextLayer | MATERIAL_SYMBOLS_16 | GRect(188,148,42,20) | Right | zone-colored |
| s_dash_glucose_layer `[existing]` | TextLayer | GOTHIC_24_BOLD | GRect(188,170,42,30) | Right | zone-colored |
| s_dash_unit_layer `[existing]` | TextLayer | GOTHIC_14 | GRect(188,200,42,14) | Right | GColorMediumAquamarine |
| s_dash_delta_layer `[NEW]` | TextLayer | GOTHIC_14 | GRect(188,214,36,14) | Right | zone-colored |

R2 note: delta uses w=36 (ends at x=224) to stay within the inscribed circle at
y=214 (x_max ≈ 229). Freshness and zone label are not rendered in R2 Dashboard —
the circular boundary provides insufficient space below y=228 at sidebar x.

---

## Font Reference

| Constant | Approx height | Used for |
|---|---|---|
| FONT_KEY_LECO_36_BOLD_NUMBERS | ~36 px | Dashboard time (HH:MM, colon included) |
| RESOURCE_ID_MATERIAL_SYMBOLS_16 | 16 px | BT icon, trend arrow in sidebar |
| FONT_KEY_GOTHIC_24_BOLD | ~24 px | Glucose value in sidebar |
| FONT_KEY_GOTHIC_14 | ~14 px | Day, month, unit, delta, freshness, zone label |

---

## Color Palette

| Token / GColor | Hex | Semantic meaning |
|---|---|---|
| CLR_STATE_DANGER | #FF0000 | URGENT_LOW, URGENT_HIGH zone |
| CLR_STATE_WARNING | #FFAA00 | LOW zone |
| GColorChromeYellow | #FFFF00 | HIGH zone `[changed by this spec]` |
| CLR_ICON_DEFAULT | #55FFFF | IN_RANGE zone, arc stroke, BT connected |
| CLR_STATE_INACTIVE | #AAAAAA | Stale / no data, freshness indicator |
| CLR_STATE_DISABLED | #555555 | BT disconnected |
| GColorWhite | #FFFFFF | Time text |
| CLR_TEXT_SUBTLE | #AAFFFF | Day, month |
| GColorMediumAquamarine | #00AAAA | Unit label |

The `zone_color()` function MUST return `GColorChromeYellow` for `ZONE_HIGH`
(currently returns `CLR_STATE_WARNING`). This aligns the sidebar color with the
graph's `high_thresh` dashed line, which already uses `GColorChromeYellow`.

---

## Quick View / Compact Mode

Compact mode is triggered when unobstructed height < 185 px (Quick View bar active).

Hidden in compact mode:
- `s_slot_layer[0..2]` — top 3 widget slots
- `s_graph_layer` — sparkline graph

Visible and unchanged in compact mode:
- `s_dash_time_layer`, `s_dash_bt_layer`, `s_dash_day_layer`, `s_dash_month_layer`
- `s_dash_trend_layer`, `s_dash_glucose_layer`, `s_dash_unit_layer`
- `s_dash_delta_layer`, `s_dash_fresh_layer`, `s_dash_zone_layer` — stay visible

No layer repositioning occurs in compact mode for Dashboard; the CGM sidebar
occupies the bottom of the screen and remains unobstructed by the Quick View bar.

---

## ASCII Diagram

```
Time 2 (200 × 228)                    Compact / Quick View
┌──────────────────────────────────┐   ┌──────────────────────────────────┐
│  [slot0 56×56] [slot1] [slot2]   │   │  (slots hidden)                  │
│  y=4           y=72    y=140     │   │                                  │
├──────────────────────────────────┤   │                                  │
│  [BT]  HH:MM (LECO_36)  [DD]    │   │  [BT]  HH:MM (LECO_36)  [DD]    │
│         y=76                [MM] │   │         y=76                [MM] │
├─────────────────┬────────────────┤   ├─────────────────┬────────────────┤
│  graph          │  [trend icon]  │   │  (graph hidden) │  [trend icon]  │
│  120×80         │  y=130         │   │                 │  y=130         │
│  y=130..210     │  [glucose]     │   │                 │  [glucose]     │
│                 │  y=152         │   │                 │  y=152         │
│                 │  [unit]        │   │                 │  [unit]        │
│                 │  y=182         │   │                 │  y=182         │
│                 │  [delta]       │   │                 │  [delta]       │
│                 │  y=196         │   │                 │  y=196         │
│                 │  [fresh|zone]  │   │                 │  [fresh|zone]  │
│                 │  y=210         │   │                 │  y=210         │
└─────────────────┴────────────────┘   └─────────────────┴────────────────┘

Round 2 (260 × 260) — circular clip shown as rounded corners
┌────────────────────────────────────────┐
│    [slot0]   [slot1]   [slot2]         │
│    y=20      y=14      y=20            │
│                                        │
│  [BT]    HH:MM (LECO_36)       [DD]   │
│           y=90                  [MM]   │
│                                        │
│  graph 150×76        [trend]           │
│  y=148..224          y=148             │
│                      [glucose]         │
│                      y=170             │
│                      [unit]            │
│                      y=200             │
│                      [delta w=36]      │
│                      y=214             │
└────────────────────────────────────────┘
```
