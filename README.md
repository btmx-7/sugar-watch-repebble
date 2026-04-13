# Steady

A polished CGM watchface for Pebble Time 2 and Round 2. Two layouts, 4 configurable data slots, and a sparkline graph. Glucose, heart rate, steps, weather, and battery at a glance.

Built for the rePebble Spring 2026 App Contest.

## Layouts

**Simple** — time-first. Large 2-row clock in Inter Black 64px with 4 corner widget slots. For casual wearers who want the time front and center with context around it.

**Dashboard** — glucose-first. Single-row time at center with 3 top slots, a full CGM sparkline graph, and a trend/value panel. For active glucose monitoring.

## Widget Slots

4 fully configurable slots (Simple: 4 corners; Dashboard: 3 top). Each slot shows:
- **Battery** — charge percent with arc progress ring
- **Weather** — temperature + icon (via OpenMeteo, no API key)
- **Heart Rate** — live BPM from Pebble Health
- **Steps** — daily step count toward 10k goal
- **CGM** — current glucose with zone color and trend arrow

## CGM Support

Fetches glucose data from:
- **Nightscout** (URL + optional token)
- **Dexcom Share** (username + password, US + international servers)

Color-coded zones: urgent low (red), low (orange), in range (green), high (yellow), urgent high (red). Stale data shown in gray. Urgent zones trigger screen flash + haptic pattern.

## Platforms

- Pebble Time 2 (emery) — 200×228, rectangular
- Round 2 (gabbro) — 260×260, circular
- Pebble Time (basalt), Steel (diorite), Round (chalk)

## Settings

Configured via the Pebble app:
- Layout (Simple / Dashboard)
- 4 widget slot assignments
- CGM data source + credentials
- Glucose units (mg/dL / mmol/L)
- Graph window (1, 2, or 3 hours)
- Alert thresholds (low, high, urgent low, urgent high)

## Coming Soon

- 8 color themes (Cyan is default)
- Light mode + auto-sunset
- Music playback indicator

## License

MIT — see [LICENSE](LICENSE)
