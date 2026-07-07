# hitori

A minimal Wayland control panel using GTK4 Layer Shell.

## Features

- **Launch entry** — type a command and press Enter to run it
- **Clock** — live date/time display
- **Battery** — capacity and charging icon
- **Power save** — toggle between `power-saver` and `balanced` via `powerprofilesctl`
- **Charge limit** — toggle 80% battery charge threshold
- **Brightness** — slider with Apply button (uses `pkexec`)
- **Volume** — live slider via `wpctl`
- **Suspend** — one-click system suspend
- **Poweroff** — one-click system poweroff
- **Custom buttons** — user-defined command buttons from config
- **Escape** — press Escape to close the panel

## Dependencies

- `gtk4`, `gtk4-layer-shell`
- `powerprofilesctl` (optional — power save)
- `pkexec` (brightness, charge limit)
- `wpctl` / `wireplumber` (volume)
- `systemd` (suspend, poweroff)

## Build

```
make
```

## Run

```
./hitori
./hitori -c /path/to/config.ini
```

## Configuration

By default hitori reads `~/.config/hitori/config.ini`. Use `-c` / `--config` to specify a different path:

```
hitori -c myconfig.ini
```

Example config:

```ini
[widgets]
# Set any of these to false to hide the widget
clock=true
battery=true
power_save=true
charge_limit=true
brightness=true
volume=true
suspend=true

[custom_buttons]
# Add custom command buttons (label=command)
# Lock Screen=gtklock
# Logout=systemctl --user exit

[style]
# Path to a custom CSS file (overrides built-in style)
# See example-style.css in the repo for a full reference.
# css_path = /home/user/hitori-style.css
```

Press **Escape** to dismiss the panel. The **launch entry** at the top accepts any command and closes the panel on Enter.
