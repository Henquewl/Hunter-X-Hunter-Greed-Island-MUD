#!/bin/bash
PORT=${1:-4000}
MUDDIR="$(cd "$(dirname "$0")" && pwd)"
cd "$MUDDIR"
VERSION=$(cat "$MUDDIR/VERSION" 2>/dev/null | tr -d '[:space:]')

if [ ! -x bin/circle ]; then
  echo "[GI MUD] Compiling server..."
  cd src && make circle CFLAGS=-w
  if [ $? -ne 0 ]; then
    echo "[GI MUD] Compilation failed."
    exit 1
  fi
  cd ..
fi

echo "[GI MUD] Starting Greed Island $VERSION on port $PORT..."
echo "[GI MUD] Connect using: telnet localhost $PORT"
echo "[GI MUD] Press Ctrl+C to stop."
echo ""
exec bin/circle "$PORT"
