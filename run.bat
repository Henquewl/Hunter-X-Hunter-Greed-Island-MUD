@echo off
setlocal EnableExtensions DisableDelayedExpansion

title Greed Island MUD

set "SCRIPT_DIR=%~dp0"
set "SRC_DIR=%SCRIPT_DIR%src"
set "TEMP_SH=%TEMP%\run_gimud_%RANDOM%%RANDOM%.sh"

for /f "delims=" %%I in ('wsl.exe wslpath -a "%SCRIPT_DIR%"') do set "WSL_ROOT=%%I"
for /f "delims=" %%I in ('wsl.exe wslpath -a "%TEMP_SH%"') do set "WSL_SH=%%I"

(
    echo cd "%WSL_ROOT%"
    echo mkdir -p bin
    echo cd src
    echo.
    echo if [ ! -x ../bin/circle ]; then
    echo   echo '[GI MUD] Compiling server...'
    echo   make circle CFLAGS=\"-w\"
    echo.
    echo   if [ $? -ne 0 ]; then
    echo     echo '[GI MUD] Compilation failed.'
    echo     exit 1
    echo   fi
    echo fi
    echo.
    echo if [ ! -x ../bin/circle ]; then
    echo   echo '[GI MUD] Executable was not generated.'
    echo   exit 1
    echo fi
    echo.
    echo echo '[GI MUD] Launching server...'
    echo cd ..
    echo exec ./bin/circle 4000
) > "%TEMP_SH%"

echo [GI MUD] Starting Greed Island MUD on port 4000...
echo [GI MUD] Connect using: telnet localhost 4000
echo [GI MUD] Press Ctrl+C to stop the server.
echo.

wsl.exe -d Ubuntu-24.04 /bin/sh -lc "tr -d '\r' < '%WSL_SH%' | /bin/sh"

del "%TEMP_SH%" >nul 2>&1

pause