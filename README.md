# Hunter X Hunter: Greed Island MUD

A text-based multiplayer RPG (MUD) set in the Hunter x Hunter universe, based on TbaMUD 3.67 (heavily modified). Features a Nen system with 7 classes, Jajanken combat, and the Greed Island card mechanics.

## Requirements

- Windows 10/11 with **WSL** (Windows Subsystem for Linux) installed
- Ubuntu on WSL (any recent version)

If you don't have WSL yet, open PowerShell as Administrator and run:
```powershell
wsl --install
```

## Running the Server

### Option 1 — Double-click (easiest)

Double-click `run.bat` in the project folder. A CMD window will open and the server starts on port **4000**.

### Option 2 — From CMD or PowerShell

```cmd
cd C:\Users\henrique.lobo\Downloads\Hunter-X-Hunter-Greed-Island-MUD
run
```

### Option 3 — From WSL terminal

```bash
cd /mnt/c/Users/henrique.lobo/Downloads/Hunter-X-Hunter-Greed-Island-MUD
bin/circle 4000
```

## Connecting

Once the server is running, connect with any telnet/MUD client to:

```
Host: localhost
Port: 4000
```

**Recommended clients:**
- [MUSHclient](https://www.mushclient.com/) (Windows, full ANSI color support)
- [Mudlet](https://www.mudlet.org/) (Windows/Mac/Linux)
- PuTTY (Raw mode, port 4000)

> The server is ready when you see `Boot db -- DONE` in the terminal.

## Compiling from Source (WSL)

If you need to recompile after changing source files:

```bash
# Install build dependencies (first time only)
sudo apt install gcc make

# Compile
cd /mnt/c/Users/henrique.lobo/Downloads/Hunter-X-Hunter-Greed-Island-MUD/src
make circle
```

The binary is output to `bin/circle`.

## Player Files

Player save files are stored in `lib/plrfiles/` and `lib/plrobjs/`. These are excluded from version control (see `.gitignore`) so the server always starts fresh for new deployments.

## Stopping the Server

Press `Ctrl+C` in the server window, or from WSL:

```bash
pkill circle
```
