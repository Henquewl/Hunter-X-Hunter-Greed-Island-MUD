# Deploying Greed Island MUD on an Oracle Cloud VM (over SSH)

## One-time setup on the VM

1. **Install build deps** (Ubuntu/Oracle Linux):
   ```bash
   sudo apt update && sudo apt install -y git build-essential libcrypt-dev   # Debian/Ubuntu
   # or:  sudo dnf install -y git gcc make glibc-devel libxcrypt-devel        # Oracle Linux
   ```

2. **Clone and build:**
   ```bash
   git clone https://github.com/Henquewl/Hunter-X-Hunter-Greed-Island-MUD.git
   cd Hunter-X-Hunter-Greed-Island-MUD
   chmod +x deploy.sh autorun.sh start.sh
   (cd src && make circle CFLAGS=-w)
   ```

3. **Open the port (4000).** Oracle Cloud needs this in **two** places:
   - **VCN ingress rule:** Networking → your VCN → Security List (or the instance's NSG) →
     add an Ingress rule: Source `0.0.0.0/0`, IP Protocol `TCP`, Destination port `4000`.
   - **Instance firewall:**
     ```bash
     sudo iptables -I INPUT -p tcp --dport 4000 -j ACCEPT && sudo netfilter-persistent save  # Ubuntu
     # or:  sudo firewall-cmd --permanent --add-port=4000/tcp && sudo firewall-cmd --reload   # firewalld
     ```
   (Oracle images ship with a restrictive local firewall — opening only the VCN rule is the
   usual "why can't I connect?" trap.)

4. **Run it under autorun** (auto-reboots on crash, lets `deploy.sh` restart it). Pick one:
   ```bash
   # quick: detached background process
   nohup ./autorun.sh >/dev/null 2>&1 &

   # or a tmux/screen session you can reattach to
   tmux new -s gi -d './autorun.sh'

   # or a systemd service (survives reboots) -- /etc/systemd/system/gimud.service:
   #   [Unit]
   #   Description=Greed Island MUD
   #   After=network.target
   #   [Service]
   #   User=ubuntu
   #   WorkingDirectory=/home/ubuntu/Hunter-X-Hunter-Greed-Island-MUD
   #   ExecStart=/home/ubuntu/Hunter-X-Hunter-Greed-Island-MUD/autorun.sh
   #   Restart=always
   #   [Install]
   #   WantedBy=multi-user.target
   # then: sudo systemctl enable --now gimud
   ```

## Deploying an update

From your dev machine, push, then run the server-side deploy over SSH:

```bash
git push origin master
ssh user@your-oracle-vm 'cd ~/Hunter-X-Hunter-Greed-Island-MUD && ./deploy.sh'
```

Make it a one-liner by adding an SSH alias to `~/.ssh/config`:
```
Host gimud
    HostName  <vm-public-ip>
    User      ubuntu
    IdentityFile ~/.ssh/your_oracle_key
```
then just: `ssh gimud 'cd Hunter-X-Hunter-Greed-Island-MUD && ./deploy.sh'`

### What `deploy.sh` does
`git pull --ff-only` → recompile → **smoke-test** the new binary on a throwaway port (killed
before it can write any player file) → if it compiles AND reaches `Boot db -- DONE`, signal a
fast reboot (`touch .fastboot` + `pkill -TERM circle`) so autorun relaunches on the new build.
**If the build fails to compile or to boot, it aborts and leaves the running game untouched.**
Env: `SMOKE_PORT=4999` (test port), `SKIP_SMOKE=1` (skip the boot test).

The gentlest restart is from inside the game instead: `shutdown reboot` (autorun relaunches on
the new binary at a moment of your choosing).

## Notes / gotchas
- **Line endings:** shell scripts must be LF (a `\r` in the shebang gives
  `bad interpreter: /bin/bash^M`). `.gitattributes` now forces `*.sh`/`autorun*` to LF; if you
  ever hit it on an old checkout, run `dos2unix deploy.sh autorun.sh start.sh autorun`.
- **Sentinel files** (in the MUD root): `.fastboot` = reboot after 5s, `pause` = hold reboots,
  `.killscript` = stop the autorun loop. `shutdown {reboot|pause|die}` in-game creates them.
- **Logs:** the autorun loop appends to `syslog` and rotates highlights into `log/`.
- **Player data** lives in `lib/plrfiles`/`lib/plrobjs` (gitignored). The deploy never touches
  it; the smoke-test instance is `kill -9`'d before it can save, so it can't race the live game.
