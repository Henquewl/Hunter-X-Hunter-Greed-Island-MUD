#!/bin/bash
# Inicia o Greed Island MUD
# Uso: bash start.sh [porta]
# Exemplo no WSL: bash start.sh 4000

PORT=${1:-4000}
MUDDIR="$(cd "$(dirname "$0")" && pwd)"

cd "$MUDDIR"
echo "[GI MUD] Iniciando Greed Island 0.89 na porta $PORT..."
echo "[GI MUD] Pressione Ctrl+C para encerrar."
echo "[GI MUD] Conecte via: telnet localhost $PORT"
echo ""

bin/circle "$PORT"
