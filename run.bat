@echo off
echo [GI MUD] Iniciando Greed Island MUD na porta 4000...
echo [GI MUD] Conecte via: telnet localhost 4000
echo [GI MUD] Pressione Ctrl+C para encerrar.
echo.

wsl bash -c "cd '$(wslpath \"%SCRIPT_DIR%\")' && bin/circle 4000"

pause
