## Platform Dimensions

<!-- Delete platforms not targeted by this change. -->

### Time 2 (basalt-plus)
Canvas: 200 x 228 px, rectangular, color e-paper.

### Round 2 (gabbro)
Canvas: 260 x 260 px, circular clip (inscribed circle diameter 260px), color e-paper.
Note: corners are clipped. Keep critical content within the central 200px diameter.

## Zone Breakdown

<!-- Contiguous vertical zones. Verify rows sum to screen height. -->
<!-- Add/remove rows as needed. Separate tables per platform if they differ. -->

### Time 2 (200 x 228)

| Zone | y | h | Purpose |
|---|---|---|---|
| status-bar | 0 | 24 | BT icon, secondary info |
| hero | 24 | <!-- h --> | Glucose value + trend arrow |
| meta | <!-- y --> | <!-- h --> | Delta + freshness |
| separator | <!-- y --> | 4 | Visual break |
| graph | <!-- y --> | <!-- h --> | Sparkline graph |
| footer | <!-- y --> | 24 | Time + date |
| **Total** | | **228** | |

### Round 2 (260 x 260) *(if targeted)*

| Zone | y | h | Purpose |
|---|---|---|---|
<!-- Repeat table for Round 2 if layout differs. -->

## Layer Inventory

<!-- Only layers new, moved, resized, or removed by this change. -->
<!-- REMOVED layers: mark as ~~strikethrough~~ in the Layer name column. -->

| Layer name | Type | Font | Frame (x, y, w, h) | Alignment | Default color |
|---|---|---|---|---|---|
| s_glucose_layer | TextLayer | FONT_KEY_LECO_42_BOLD_NUMBERS | GRect(<!--x,y,w,h-->) | Center | GColorWhite |
| s_trend_layer | TextLayer | FONT_KEY_GOTHIC_28_BOLD | GRect(<!--x,y,w,h-->) | Right | GColorWhite |
| <!-- name --> | <!-- type --> | <!-- font --> | GRect(<!--x,y,w,h-->) | <!-- align --> | <!-- color --> |

## Font Reference

<!-- List every FONT_KEY_* constant used, with approximate pixel height. -->

| Constant | Approx height | Used for |
|---|---|---|
| FONT_KEY_LECO_42_BOLD_NUMBERS | ~54 px | Glucose hero value |
| FONT_KEY_GOTHIC_28_BOLD | ~28 px | Trend arrow |
| FONT_KEY_GOTHIC_24_BOLD | ~24 px | Delta value |
| FONT_KEY_GOTHIC_14 | ~14 px | Freshness, secondary text |

## Color Palette

<!-- GColor constants and their semantic meaning in this watchface. -->

| GColor | Semantic meaning |
|---|---|
| GColorGreen | Glucose in normal range |
| GColorChromeYellow | Glucose approaching limit |
| GColorOrange | Glucose mildly out of range |
| GColorRed | Glucose urgently out of range |
| GColorLightGray | Stale / no data |
| GColorWhite | Default text |
| GColorDarkGray | Separator, secondary chrome |

## Quick View / Compact Mode

<!-- Describe the compact layout when Quick View is active. -->
<!-- Which layers are hidden? Which reposition? -->

Hidden in compact mode:
-

Repositioned in compact mode:
-

### Compact Zone Table *(if significantly different)*

| Zone | y | h | Purpose |
|---|---|---|---|
<!-- Fill in if compact mode layout differs substantially from normal. -->

## ASCII Diagram

<!-- Simple diagram showing zone boundaries. Use | and - characters. -->
<!-- Helps verify coordinate math before implementation. -->

```
Time 2 (200 x 228)          Compact / Quick View
┌──────────────────────┐    ┌──────────────────────┐
│  status-bar  (y=0)   │    │  status-bar  (y=0)   │
├──────────────────────┤    ├──────────────────────┤
│  hero        (y=24)  │    │  hero        (y=24)  │
├──────────────────────┤    ├──────────────────────┤
│  meta                │    │  meta                │
├──────────────────────┤    └──────────────────────┘
│  separator           │     (graph + footer hidden)
├──────────────────────┤
│  graph               │
├──────────────────────┤
│  footer      (y=204) │
└──────────────────────┘
```

<!-- Replace placeholder y values and heights with actual values from Zone Breakdown. -->
