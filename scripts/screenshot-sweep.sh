#!/usr/bin/env bash
# screenshot-sweep.sh
#
# Build + install + screenshot one PBW per demo scenario.
# Requires a running emulator: `pebble install --emulator emery` once first.
#
# Usage:
#   ./scripts/screenshot-sweep.sh                 # emery, all 8 states
#   PLATFORM=gabbro ./scripts/screenshot-sweep.sh # round
#   STATES="0 3 4"  ./scripts/screenshot-sweep.sh # subset

set -euo pipefail

PLATFORM="${PLATFORM:-emery}"
STATES="${STATES:-0 1 2 3 4 5 6 7}"
OUT_DIR="${OUT_DIR:-screenshots/demo}"

# Keep these names aligned with demo_scenarios[] in src/c/demo/demo.c.
NAMES=(urgent_low low in_range high urgent_high stale dashboard zero_state)

mkdir -p "$OUT_DIR"

for i in $STATES; do
  name="${NAMES[$i]:-state_$i}"
  echo ""
  echo "──────────────────────────────────────────"
  echo "  State $i  —  $name  ($PLATFORM)"
  echo "──────────────────────────────────────────"
  DEMO_DATA=1 DEMO_STATE="$i" pebble build
  pebble install --emulator "$PLATFORM"
  sleep 3
  out="$OUT_DIR/${PLATFORM}_${i}_${name}.png"
  pebble screenshot --emulator "$PLATFORM" "$out"
  echo "  Saved: $out"
done

echo ""
echo "Done. Contact sheet input: $OUT_DIR/"
