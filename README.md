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
