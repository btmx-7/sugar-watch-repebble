## Why

The original Sugar Watch was a functional CGM watchface but visually indistinguishable from a medical device. The Figma redesign introduces two distinct layouts, a widget slot system, and a design system to make the watchface appealing to both diabetic and non-diabetic wearers — competing with trendy watchfaces while remaining clinically useful. The Pebble Spring 2026 Contest (deadline April 19) is the forcing function.

Figma source: https://www.figma.com/design/bKKqEkSN0q1rOdsEX8OpaE/SugarWatch-Watchface---rePebble

## What Changes

- **New: Simple layout** — time-first, 2-row hero clock display (Inter Black 64px, hours/minutes split), 4 corner widget slots, no graph. Targets casual wearers and non-diabetics.
- **New: Dashboard layout** — time center (single row), 3 top widget slots, sparkline graph retained, CGM trend+value panel right of graph. Targets active glucose monitoring.
- **New: Widget slot system** — 4 fully configurable slots per layout. Each slot user-assignable to: Battery, Weather, Heart Rate, Steps, or CGM. Renders as 56×56px circle with arc-stroke progress indicator, Material Symbol icon, value, and unit label.
- **New: Material Symbols Rounded bitmap font (16px)** — icon set for all slot types and status indicators (BT, music placeholder). Replaces bt_icon.png bitmap for BT.
- **New: Custom Inter Black 64px bitmap font** — for 2-row time display in Simple layout.
- **New: Pebble Health API integration** — HR and Steps values read from HealthService, update on significant change event.
- **New: Weather via OpenMeteo (pkjs)** — fetches temperature + WMO weather code on connect, maps to icon index. No API key required.
- **New: Clay settings additions** — layout selector (Simple/Dashboard), 4 slot assignment dropdowns (None/Battery/Weather/HR/Steps/CGM).
- **Modified: display-layout** — two independent layout render paths, both supporting T2 and R2.
- **Modified: display-glucose** — glucose in CGM slot widget (Simple) or right-panel sidebar (Dashboard) rather than standalone hero.
- **Modified: display-trend** — trend arrow rendered as Material Symbol inside CGM slot.
- **Removed: s_zone_layer** — zone text label removed entirely; zone communicated by color.
- **Rename: Sugar Watch → Steady** — new name across package.json, README, config, and store description. Internal app UUID unchanged.
- **Deferred (not in scope):** light/dark mode, auto-sunset, 8 color themes (architecture ready), audio status.

## Platform Targets

- [x] Time 2 (emery) - 200x228 px, color e-paper, rectangular
- [x] Round 2 (gabbro) - 260x260 px, circular clip, color e-paper

## Quick View Impact

Affected. Both layouts must support Quick View compact mode:
- Simple compact: slots hidden, time compressed to single-row, CGM value visible if CGM slot assigned
- Dashboard compact: graph hidden, top slots compressed, time + CGM right-panel remain

## Contest Angle

- [x] Most Hearts - community appeal, visual polish, shareability
- [x] Team Judging - creativity, cleverness, good use of new platforms, design

Specific angles:
- Most visually polished CGM watchface submitted (design system, arc strokes, icon font)
- Uses both new platforms (T2 + R2) — explicitly rewarded by judges
- Non-medical aesthetic: looks like a premium watchface, not a medical device
- Roadmap teased in description: themes, light mode, audio indicator — signals depth

## Capabilities

### New Capabilities

- `display-simple-layout`: Full Simple layout render path (2-row time, 4 corner slots, T2+R2)
- `display-dashboard-layout`: Full Dashboard layout render path (3 top slots, time, graph+CGM panel, T2+R2)
- `widget-slots`: 4 configurable slot system with arc-stroke draw proc, all 5 slot types
- `data-health`: Pebble Health API subscription for HR and steps
- `data-weather`: OpenMeteo fetch in pkjs, AppMessage to watch, weather slot rendering

### Modified Capabilities

- `display-layout`: now routes to Simple or Dashboard based on settings
- `display-glucose`: glucose now rendered inside CGM slot widget or Dashboard right panel
- `display-trend`: trend arrow now Material Symbol inside CGM slot
- `settings`: layout selector + 4 slot dropdowns added to Clay config

## Impact

- `src/c/main.c`: complete refactor (810 → ~1400-1600 lines)
- `src/pkjs/index.js`: weather fetch + new AppMessage sends
- `src/pkjs/config.html`: new Clay settings sections
- `package.json`: 2 new font resources + new AppMessage keys
- `resources/fonts/`: MaterialSymbolsRounded.ttf, InterBlack.ttf (new)
