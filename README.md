# Steady

Your wrist, always in the know.

A polished CGM watchface for Pebble Time 2 and Round 2. Large clock, 4 configurable data slots, and a sparkline glucose graph. Everything that matters, readable at a glance.

Built for the rePebble Spring 2026 App Contest.

---

## Features

- CGM integration (Nightscout and Dexcom Share supported out of the box)
- 3-hour sparkline graph for glucose trend history
- 4 configurable widget slots (battery, weather, heart rate, steps, or CGM in any corner)
- Color-coded glucose zones: urgent low/high (red), low (orange), in range (theme accent color), high (yellow)
- Haptic and visual alerts on urgent glucose zones
- Weather via OpenMeteo, no API key required

## Layout

Simple layout. Large 2-row clock in Inter Black with 4 corner widget slots. Clean and glanceable for everyday wear.

## Widget Slots

4 fully configurable corner slots. Each slot can show:

| Slot | Data |
|------|------|
| Battery | Charge percent with arc progress ring |
| Weather | Temperature and condition icon (OpenMeteo) |
| Heart Rate | Live BPM from Pebble Health |
| Steps | Daily step count toward 10k goal |
| CGM | Current glucose with zone color and trend arrow |

## CGM Support

Fetches glucose data from:
- Nightscout (URL and optional access token)
- Dexcom Share (username and password, US and international servers)

Stale data shown in gray. Glucose display in mg/dL or mmol/L.

## Platforms

| Device | Resolution | Shape |
|--------|-----------|-------|
| Pebble Time 2 (emery) | 200x228 | Rectangular |
| Pebble Round 2 (gabbro) | 260x260 | Circular |
| Pebble Time (basalt) | 144x168 | Rectangular |
| Pebble Steel (diorite) | 144x168 | Rectangular |
| Pebble Round (chalk) | 180x180 | Circular |

## Settings

Configured via the Pebble app settings page:

- 4 widget slot assignments
- CGM data source and credentials
- Glucose display units (mg/dL or mmol/L)
- Graph window (1, 2, or 3 hours)
- Alert thresholds (low, high, urgent low, urgent high)

## Roadmap

- Dashboard layout: a glucose-first layout with single-row time, 3 top slots, full sparkline graph, and a CGM trend/value panel. Designed for active glucose monitoring.
- 8 color themes (Cyan is default)
- Light mode
- Auto-sunset mode switch
- Music playback indicator

## License

MIT, see [LICENSE](LICENSE)
