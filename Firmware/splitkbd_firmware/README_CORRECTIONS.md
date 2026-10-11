# Grand Line firmware correction notes

## Corrections in this package

- Fixed the matrix event buffer in `main.c`: it now holds all 24 possible
  key-state transitions from a 4x6 matrix in a single scan.
- Added `MATRIX_MAX_EVENTS` and documented the capacity requirement.
- Added a defensive null/zero-capacity guard to `matrix_scan`.
- Configured encoder A/B GPIOs explicitly as inputs with pull-ups before using
  the PCNT driver, added a null guard to `encoder_poll`, and made encoder
  teardown safe when it has not been initialized.
- Added the standard NVS recovery path for full/old-version NVS partitions.

## Important hardware blocker: TFT pins

The current pin map assigns TFT CS to GPIO37 and TFT backlight to GPIO36. The
project identifies the module as ESP32-S3-WROOM-1-N4R8, whose octal PSRAM uses
GPIO35-37. Disabling PSRAM in `sdkconfig.defaults` does not electrically free
those pins. A source-only edit cannot correct this if the PCB traces are already
connected to GPIO36/37. Do not pretend that changing `pins.h` alone fixes the
board. Correct it by revising the PCB to route TFT CS/backlight to verified
free pins, or use a compatible module without that PSRAM wiring and confirm
the full board design. Update `pins.h` only to match the physically revised
PCB.

## Still not implemented

This remains an input-scanning firmware scaffold, not a finished keyboard
firmware. USB HID keyboard reports/keymaps, ESP-NOW split communication, TFT
rendering, NFC, backlighting, and power-management runtime behavior are not
implemented in this package.

## Build and test

Build with the ESP-IDF version selected for the project (`idf.py build`). Then
flash and test on hardware. The source has not been compiled or hardware-tested
in this environment. Confirm the actual PCB netlist and exact module variant
before connecting the TFT or relying on any pin assignment.
