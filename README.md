# hitori

A minimal Wayland control panel using GTK4 Layer Shell.

## Features

- Clock display
- Battery status (capacity, charging icon)
- Power save toggle (`powerprofilesctl`)
- Charging limit toggle (80% threshold)
- Brightness slider with Apply button
- Volume slider (`wpctl`)
- Suspend button

## Dependencies

- `gtk4`, `gtk4-layer-shell`
- `powerprofilesctl` (optional — power save)
- `pkexec` (brightness, charge limit)
- `wpctl` / `wireplumber` (volume)
- `systemd` (suspend)

## Build

```
make
```

## Run

```
./hitori
```

## Configuration

Create `~/.config/hitori/config.ini` to customize the panel:

```ini
[widgets]
; Set any of these to false to hide the widget
clock=true
battery=true
power_save=true
charge_limit=true
brightness=true
volume=true
suspend=true

[custom_buttons]
; Add custom command buttons (label=command)
# Lock Screen=gtklock
# Logout=systemctl --user exit
```
