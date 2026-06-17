#!/bin/bash
#
# Greed Island MUD -- one-command deploy. Run this ON the server (e.g. over SSH):
#
#     ssh user@your-oracle-vm 'cd ~/Hunter-X-Hunter-Greed-Island-MUD && ./deploy.sh'
#
# It pulls the latest code, recompiles, smoke-tests the new binary, and (if the
# game is running under autorun) restarts it onto the new build. If the build
# fails to compile OR fails to boot, it aborts WITHOUT touching the live game.
#
# Env knobs:
#   SMOKE_PORT=4999   port the throwaway boot-test binds (must be free)
#   SKIP_SMOKE=1      skip the boot smoke-test (just pull + build + restart)
#
set -euo pipefail

cd "$(cd "$(dirname "$0")" && pwd)"
SMOKE_PORT="${SMOKE_PORT:-4999}"

echo "[deploy] Pulling latest source..."
git pull --ff-only

echo "[deploy] Compiling (cd src && make circle CFLAGS=-w)..."
( cd src && make circle CFLAGS=-w )
[ -x bin/circle ] || { echo "[deploy] ERROR: bin/circle missing after build -- aborting."; exit 1; }

if [ "${SKIP_SMOKE:-0}" != "1" ]; then
  echo "[deploy] Smoke-testing the new build on port $SMOKE_PORT (killed before any save)..."
  rm -f /tmp/gi_smoke.log
  bin/circle -d lib "$SMOKE_PORT" >/tmp/gi_smoke.log 2>&1 &
  smoke_pid=$!
  ok=0
  for _ in $(seq 1 45); do
    if grep -q "Boot db -- DONE" /tmp/gi_smoke.log 2>/dev/null; then ok=1; break; fi
    kill -0 "$smoke_pid" 2>/dev/null || break   # it died on its own
    sleep 1
  done
  kill -9 "$smoke_pid" 2>/dev/null || true       # -9 so it never reaches the save-on-shutdown path
  wait "$smoke_pid" 2>/dev/null || true
  if [ "$ok" != "1" ]; then
    echo "[deploy] ERROR: new build did not reach 'Boot db -- DONE' -- NOT restarting the live game."
    echo "--------- last lines of the smoke log ---------"; tail -20 /tmp/gi_smoke.log
    exit 1
  fi
  echo "[deploy] Smoke test passed."
fi

echo "[deploy] Restarting the live game..."
if pgrep -f 'bin/circle' >/dev/null; then
  if pgrep -f 'autorun' >/dev/null; then
    touch .fastboot                          # autorun reboots in ~5s instead of 60
    pkill -TERM -f 'bin/circle' || true       # clean shutdown (saves players); autorun relaunches with the new binary
    echo "[deploy] Reboot signaled -- autorun will relaunch on the new build in ~5s."
  else
    echo "[deploy] WARNING: the game is running but autorun is NOT, so it will not relaunch itself."
    echo "          Reboot it from inside the game ('shutdown reboot'), or stop it and run ./autorun.sh."
  fi
else
  echo "[deploy] Game not currently running. Start it under autorun with:"
  echo "             nohup ./autorun.sh >/dev/null 2>&1 &"
fi
echo "[deploy] Done."
