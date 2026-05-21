@echo off
title Greed Island MUD

for /f "delims=" %%I in ('wsl.exe wslpath -a "%~dp0"') do set "WSL_ROOT=%%I"

echo [GI MUD] Starting Greed Island MUD on port 4000...
echo [GI MUD] Connect using: telnet localhost 4000
echo [GI MUD] Press Ctrl+C to stop the server.
echo.

wsl.exe bash "%WSL_ROOT%start.sh"

pause
