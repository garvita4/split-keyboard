# Grand Line firmware — input bring-up baseline

This project is the **input bring-up milestone** for the Grand Line split keyboard, targeting ESP-IDF on ESP32-S3. It is not yet a complete keyboard firmware.

## Implemented in this baseline

- 4-row × 6-column matrix scan (24 positions; populate only the positions used by the 21-key half).
- Active-low column sampling with configurable scan/debounce constants.
- Matrix press/release events, including deferred emission if a caller passes an undersized event buffer.
- Rotary encoder quadrature decoding with ESP-IDF PCNT, overflow accumulation, detent conversion, a debounced push button, and simple acceleration.
- NVS initialization and standard recovery for full/outdated NVS partitions.
- Input task at `MATRIX_SCAN_HZ`, with serial logs to support hardware bring-up.
- Error returns and cleanup in the matrix/encoder initialization paths.

## Not implemented yet

- USB HID reports and keymap/layers
- Communication between left/right halves (UART or ESP-NOW)
- TFT driver/UI/animations
- NFC driver/protocol
- Per-key LED/backlight control
- Battery/charger telemetry and runtime sleep policy
- Production settings storage, diagnostics, and comprehensive tests

## Critical PCB/module issue

The current pin map assigns TFT CS to GPIO37 and TFT backlight to GPIO36, while the board is identified as using an ESP32-S3-WROOM-1-N4R8 octal-PSRAM module. Those pins are reserved/used by the module's octal PSRAM connection and must not be treated as available general-purpose GPIOs. Disabling PSRAM in software does not reroute the PCB or make a physical pin conflict disappear.

The current input-only milestone does not initialize the TFT, so do not connect/use TFT signals on those assignments. Before implementing the display, compare the actual KiCad schematic/netlist and exact module datasheet, then revise the PCB routing or select a module/board configuration that supports the intended display and memory simultaneously. Do not merely change `pins.h` unless the physical routing matches.

## Pin map summary

See `config/pins.h` for the current net-to-GPIO mapping. Important signals:

- Matrix rows: GPIO4–7
- Matrix columns: GPIO15, 16, 17, 18, 21, 38
- Encoder A/B/switch: GPIO10, 11, 12
- Shared SPI: GPIO40, 41, 42
- TFT CS/backlight conflict: GPIO37 / GPIO36
- TFT DC/reset: GPIO8 / GPIO9
- NFC CS/reset: GPIO39 / GPIO14
- USB D−/D+: GPIO19 / GPIO20
- Inter-half UART: GPIO43 / GPIO44

This list is copied from the current firmware pin header; it is not a substitute for validating the KiCad netlist against the physical PCB revision.

## Build and hardware validation

Use the ESP-IDF version installed for your project (this code uses the split GPIO/PCNT driver components found in newer ESP-IDF 5.x releases):

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board. If your project already has a matching `sdkconfig`, preserve it and review `sdkconfig.defaults` rather than blindly resetting the configuration.

### Suggested bring-up order

1. Build and resolve compiler/configuration errors.
2. Power the board safely and confirm boot/NVS logs.
3. Verify matrix row/column electrical levels with one switch at a time; confirm every press has one DOWN and one UP event.
4. Test rollover/chords and all populated positions; confirm no ghosting with the actual diode orientation.
5. Rotate the encoder slowly in both directions and verify one physical detent maps to one logical step; then test fast rotation and the push button.
6. Only after input tests pass, add a keymap and USB HID. Then add split transport and peripherals independently.

## Important verification note

The source has been reviewed and hardened, but this environment does not have ESP-IDF installed, so the project has **not** been compiled, flashed, or tested on the physical PCB here. A successful build and hardware test are still required before calling it functional.
