#!/usr/bin/env bash
# shoot_mockups.sh — Build and screenshot all 5 mock states on T2 (emery) + R2 (gabbro)
# Installs once per emulator, then updates state via AppMessage injection (no reinstall).
# Outputs go to resources/screenshots/states/
# Usage: bash scripts/shoot_mockups.sh

set -euo pipefail
cd "$(dirname "$0")/.."

OUT_DIR="resources/screenshots/states"
mkdir -p "$OUT_DIR"

# ── State definitions ────────────────────────────────────────────────────────
# Format: "state_num|slug|battery_pct|charging(0/1)|bt_connected(yes/no)"
STATES=(
  "1|s1_inrange|78|0|yes"
  "2|s2_lower|20|0|yes"
  "3|s3_btoff|15|0|no"
  "4|s4_higher|80|1|yes"
  "5|s5_error|5|0|no"
)

# ── Helpers ──────────────────────────────────────────────────────────────────

install_on() {
  local emulator="$1"
  local emu_json="/var/folders/rd/dzt_0bfs3y7bl1wfmtcbqm700000gn/T/pb-emulator.json"
  echo ""
  echo "── Installing on $emulator ─────────────────────────────────────────"

  # Kill existing processes by OS PID (bypasses pypkjs — needed when pypkjs is unresponsive
  # after a watch-app crash, e.g., gabbro state 5 leaving WatchVersionRequest timing out)
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

  # Inject mock data first (pypkjs WebSocket implies BT connected — must set BT state after)
  python3 scripts/send_mock.py --emulator "$emulator" --state "$state_num"

  # Battery (set after AppMessage so render uses injected data)
  if [ "$charging" = "1" ]; then
    pebble emu-battery --emulator "$emulator" --percent "$bat_pct" --charging
  else
    pebble emu-battery --emulator "$emulator" --percent "$bat_pct"
  fi

  # BT — set last so the icon reflects final state, not the transient pypkjs connection.
  # Non-fatal: on gabbro state 5 the app crash-restarts, breaking pypkjs — BT is already
  # disconnected from the WebSocket close in send_mock.py, so the failure is harmless.
  pebble emu-bt-connection --emulator "$emulator" --connected "$bt_connected" 2>/dev/null || true
  sleep 1.5

  # Screenshot — retry up to 4 times with 3s backoff (watch may be briefly unresponsive)
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
    # Fallback: QEMU screendump (bypasses pypkjs — used when watch app crash-restarts
    # after certain AppMessages, breaking the pypkjs WatchVersionRequest protocol)
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

# Install once on each emulator
install_on "emery"
install_on "gabbro"

echo ""
echo "══ Shooting states ════════════════════════════════════════════════"

for state_def in "${STATES[@]}"; do
  IFS='|' read -r state_num slug bat_pct charging bt_connected <<< "$state_def"

  echo ""
  echo "══ State ${state_num}: ${slug} ══════════════════════════════════════════"

  shoot_state "$state_num" "$slug" "$bat_pct" "$charging" "$bt_connected" "emery" "T2"
  shoot_state "$state_num" "$slug" "$bat_pct" "$charging" "$bt_connected" "gabbro" "R2"
done

echo ""
echo "Done. Screenshots in $OUT_DIR:"
ls -1 "$OUT_DIR"
