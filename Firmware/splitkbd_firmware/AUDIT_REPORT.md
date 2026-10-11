# Grand Line source audit

## Changes in this revision

1. **Matrix event loss:** state transitions are no longer committed if the caller's event buffer is full. A debounced transition remains pending and can be emitted on a later scan.
2. **Matrix initialization/cleanup:** GPIO errors are returned instead of forcing an immediate `ESP_ERROR_CHECK` abort from inside the module. Added initialization state checks and cleanup.
3. **Encoder initialization/cleanup:** checked every PCNT/GPIO setup operation, recorded channel handles, and cleanly tears down partially-created resources on failure.
4. **Encoder PCNT count race:** enabled PCNT overflow accumulation and installed high/low watch points; polling now differences the accumulated count instead of reading and immediately clearing it each cycle.
5. **Encoder startup acceleration:** the first detent is not incorrectly treated as a rapid continuation of a previous detent.
6. **Encoder startup button state:** initial switch state is sampled after GPIO setup to avoid inventing a press/release transition at boot.
7. **Task creation:** failure to create the input task is reported and input peripherals are cleaned up.
8. **Build dependencies:** removed unused Wi-Fi/event/power-management dependencies from this input-only milestone.
9. **Documentation:** recorded the implemented scope, pin assignments, hardware blocker, and recommended validation sequence.

## Known limitations / risks

- No ESP-IDF toolchain is installed in the authoring environment, so compilation has not been verified.
- The TFT CS/backlight pin conflict remains a **hardware/PCB issue**, not a software-only fix.
- `ENCODER_COUNTS_PER_DETENT` is set to 4. Confirm this against the exact encoder part and test both directions; some encoders have different detent behavior.
- Acceleration multipliers are provisional UX values and should be tuned on the actual encoder.
- Matrix diode direction and populated switch positions must match the PCB. Validate with the KiCad schematic/netlist and hardware.
- USB HID, keymap, split communication, display, NFC, lighting, battery management, and sleep behavior are not implemented.

## Acceptance criteria for the next milestone

- `idf.py build` completes on the selected ESP-IDF version.
- All 21 populated keys on each half generate exactly one DOWN and one UP event per press/release.
- Chords and fast typing do not drop events.
- Encoder produces stable signed detent counts, with no unexpected counts at rest.
- No GPIO assigned to an active firmware feature conflicts with module-reserved pins or another PCB net.
