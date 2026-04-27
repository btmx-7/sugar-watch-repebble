#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/dev-hub"

if ! command -v node &>/dev/null; then
  echo "✗ Node.js not found — install from https://nodejs.org"
  exit 1
fi

if [ ! -d node_modules ]; then
  echo "→ Installing dependencies..."
  npm install --silent
fi

echo "→ Starting Steady Dev Hub..."
node server.js &
SERVER_PID=$!

sleep 0.8
open http://localhost:3333

trap "kill $SERVER_PID 2>/dev/null; exit 0" INT TERM
wait $SERVER_PID
