#!/usr/bin/env bash
# shoot_dashboard.sh — Build and screenshot all Dashboard-layout mock states on
# T2 (emery) + R2 (gabbro). Installs once per emulator, updates state via
# AppMessage injection (no reinstall).
#
# Covers three groups:
#   Zones (11-18):   in-range, urgent-low, low, high, urgent-high, stale, bt-off, error
#   Slot variations (19-22): swap the 3 dashboard slots across HR/steps/CGM/weather/battery
#
# Outputs go to resources/screenshots/states/dashboard/
#
# Usage:
#   bash scripts/shoot_dashboard.sh             # all 12 states on both platforms
#   STATES=11,14,15 bash scripts/shoot_dashboard.sh  # subset

set -euo pipefail
cd "$(dirname "$0")/.."

OUT_DIR="resources/screenshots/states/dashboard"
mkdir -p "$OUT_DIR"

# ── State definitions ────────────────────────────────────────────────────────
# Format: "state_num|slug|battery_pct|charging(0/1)|bt_connected(yes/no)"
# Slug prefix "d" distinguishes Dashboard captures from Simple ("s") captures.
ALL_STATES=(
  # Zone sweep
  "11|d1_inrange|78|0|yes"
  "12|d2_urgent_low|50|0|yes"
  "13|d3_low|50|0|yes"
  "14|d4_high|45|0|yes"
  "15|d5_urgent_high|70|0|yes"
  "16|d6_stale|50|0|yes"
  "17|d7_btoff|15|0|no"
  "18|d8_error|5|0|no"
  # Slot variations
  "19|d9_slots_hr_steps_cgm|78|0|yes"
  "20|d10_slots_cgm_weather_bat|78|0|yes"
  "21|d11_slots_weather_hr_cgm|78|0|yes"
  "22|d12_slots_battery_steps_cgm|78|1|yes"
)

# Filter by STATES=11,14,15 env
if [ -n "${STATES:-}" ]; then
  SELECTED=()
  IFS=',' read -r -a WANTED <<< "$STATES"
  for sd in "${ALL_STATES[@]}"; do
    IFS='|' read -r n _ _ _ _ <<< "$sd"
    for w in "${WANTED[@]}"; do
      if [ "$n" = "$w" ]; then SELECTED+=("$sd"); break; fi
    done
  done
  STATES_RUN=("${SELECTED[@]}")
else
  STATES_RUN=("${ALL_STATES[@]}")
fi

# ── Helpers ──────────────────────────────────────────────────────────────────

install_on() {
  local emulator="$1"
  local emu_json="/var/folders/rd/dzt_0bfs3y7bl1wfmtcbqm700000gn/T/pb-emulator.json"
  echo ""
  echo "── Installing on $emulator ─────────────────────────────────────────"

  # Kill existing processes by OS PID (bypasses pypkjs; pypkjs can hang after
  # a watch-app crash, e.g. if a previous run left the watch in a bad state).
  if [ -f "$emu_json" ]; then
    QEMU_PID=$(python3 -c "
import json,sys
d=json.load(open('$emu_json'))
e=d.get('$emulator',{})
if e:
  v=list(e.keys())[0]
  print(e[v]['qemu']['pid'])
" 2>/dev/null)
    PY_PID=$(python3 -c "
import json,sys
d=json.load(open('$emu_json'))
e=d.get('$emulator',{})
if e:
  v=list(e.keys())[0]
  print(e[v]['pypkjs']['pid'])
" 2>/dev/null)
    [ -n "$QEMU_PID" ] && kill "$QEMU_PID" 2>/dev/null || true
    [ -n "$PY_PID"   ] && kill "$PY_PID"   2>/dev/null || true
  fi
  sleep 1

  pebble install --emulator "$emulator" 2>&1 | grep -v "^N/A%"
  sleep 2.5
  echo "  install done"
}

shoot_state() {
  local state_num="$1"
  local slug="$2"
  local bat_pct="$3"
  local charging="$4"
  local bt_connected="$5"
  local emulator="$6"
  local platform="$7"

  echo ""
  echo "── ${platform} / ${slug} ───────────────────────────────────────────"

  # Inject mock data first (pypkjs WebSocket implies BT connected — set BT after)
  python3 scripts/send_mock.py --emulator "$emulator" --state "$state_num"

  # Battery (set after AppMessage so render uses injected data)
  if [ "$charging" = "1" ]; then
    pebble emu-battery --emulator "$emulator" --percent "$bat_pct" --charging
  else
    pebble emu-battery --emulator "$emulator" --percent "$bat_pct"
  fi

  # BT — set last so icon reflects final state, not the transient pypkjs link.
  # Non-fatal on bt=no: pypkjs socket closes, watch stays in its post-message state.
  pebble emu-bt-connection --emulator "$emulator" --connected "$bt_connected" 2>/dev/null || true
  sleep 1.5

  # Screenshot — retry up to 4 times with 3s backoff
  local out="${OUT_DIR}/${platform}_${slug}.png"
  local shot_ok=0
  for _try in 1 2 3 4; do
    if pebble screenshot --emulator "$emulator" "$out" 2>/dev/null; then
      shot_ok=1
      break
    fi
    echo "  [screenshot attempt ${_try} failed, retrying in 3s...]"
    sleep 3
  done
  if [ $shot_ok -eq 1 ]; then
    echo "  saved: $out"
  else
    # Fallback: QEMU screendump (bypasses pypkjs)
    echo "  [pebble screenshot failed — trying QEMU screendump fallback]"
    if python3 scripts/qemu_screenshot.py --emulator "$emulator" --output "$out" 2>&1; then
      shot_ok=1
    else
      echo "  [WARN] QEMU fallback also failed — skipping $out"
    fi
  fi
}

# ── Main ─────────────────────────────────────────────────────────────────────

echo "══ Building ════════════════════════════════════════════════════════"
pebble build 2>&1 | tail -3

# Install once on each emulator (layout is switched at runtime via AppMessage)
install_on "emery"
install_on "gabbro"

echo ""
echo "══ Shooting Dashboard states ══════════════════════════════════════"

for state_def in "${STATES_RUN[@]}"; do
  IFS='|' read -r state_num slug bat_pct charging bt_connected <<< "$state_def"

  echo ""
  echo "══ State ${state_num}: ${slug} ══════════════════════════════════════════"

  shoot_state "$state_num" "$slug" "$bat_pct" "$charging" "$bt_connected" "emery" "T2"
  shoot_state "$state_num" "$slug" "$bat_pct" "$charging" "$bt_connected" "gabbro" "R2"
done

echo ""
echo "Done. Screenshots in $OUT_DIR:"
ls -1 "$OUT_DIR"
