# Dashboard layout state captures

Captures produced by `scripts/shoot_dashboard.sh`.

## File naming

`{platform}_{slug}.png` where:
- platform: `T2` (emery) or `R2` (gabbro)
- slug: `d{index}_{name}` matching the state definitions in `send_mock.py`

## State matrix

Zone sweep (slots = {weather, battery, CGM}):
- d1_inrange: glucose 110 flat
- d2_urgent_low: glucose 45 double-down
- d3_low: glucose 65 single-down
- d4_high: glucose 195 single-up (sidebar yellow per v1 fix)
- d5_urgent_high: glucose 270 double-up
- d6_stale: stale CGM, trend none
- d7_btoff: stale CGM, BT disconnected
- d8_error: glucose 0, weather unavailable

Slot variations (in-range data, 3 slots swapped):
- d9_slots_hr_steps_cgm: {HR, steps, CGM}
- d10_slots_cgm_weather_bat: {CGM, weather, battery}
- d11_slots_weather_hr_cgm: {weather, HR, CGM}
- d12_slots_battery_steps_cgm: {battery, steps, CGM}, charging

## Manual captures

Quick View (tasks §7.11, §7.12) is not covered by the sweep. To capture manually:
1. Run the sweep to get the watch into a dashboard state.
2. Trigger Quick View: `pebble emu-notification "test"` then screenshot while the modal is up, or open a timeline pin.
3. Save as `{platform}_d_quickview.png`.
