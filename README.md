# amu-driver

Device library for **AMU**, wrapped as a Zephyr module.

The bus is a named function (`amu_transfer` in
`lib/amu_bus.h`), defined at link time — not a pointer on the
device object. I2C, SPI, and UART belong in `src/hardware_transfer.c`. Change
the transfer signature if the device is not a register map.

## Where to write code

| File | Role |
|------|------|
| `include/amu.h` | API — types and function prototypes |
| `lib/amu.c` | Real implementations |
| `lib/amu_bus.h` | Named bus functions (`transfer`, `delay`, monotonic `time_ms`) |
| `lib/amu_regs.h` | Protocol constants |
| `mock/amu.c` | Same symbols, canned values (NDA-safe CI) |

**Adding a function:** declare it in the header, implement it in `lib/`, stub it in `mock/`. Do not edit `src/` for API work.

`src/` is Zephyr packaging (DT, bus bind, `from_dev`). Open it when bringing the chip up, not when adding API functions.

## Backends

Kconfig selects exactly one. Public mock links `mock/`. The other two link `lib/` and supply `amu_transfer` at link time.

| Backend | Links | `amu_transfer` comes from |
|---------|-------|--------------------------------------|
| Public mock (default) | `mock/amu.c` | not used |
| Library mock | `lib/amu.c` | `src/lib_mock_transfer.c` |
| Hardware | `lib/amu.c` | `src/hardware_transfer.c` |

App code:

```c
#include <amu.h>

amu_t *dev = amu_from_dev(DEVICE_DT_GET(DT_ALIAS(amu)));
```

## Tests and REPL

From the west workspace root (the directory containing the `amu-driver`
project):

```bash
west twister -T amu-driver/tests/unit -p qemu_cortex_m3
west build -b rpi_pico2/rp2350a/m33 amu-driver/samples/repl
west flash -r openocd --openocd ~/.local/bin/openocd
```

Use an RP2350-capable OpenOCD executable; `~/.local/bin/openocd` is an example
path and may vary by installation. If multiple probes are connected, add
`--cmd-pre-init 'adapter serial PROBE_USB_SERIAL'`, replacing the placeholder
with the Debug Probe's USB serial number. Connect the Raspberry Pi Debug Probe
to the Pico 2's SWDIO, SWCLK, and GND. Its UART lead also provides the shell:
connect probe TX to GP1 (UART RX), probe RX to GP0 (UART TX), and GND to GND.
Open the serial port enumerated by the Debug Probe at 115200 baud. The AMU
sample keeps its I2C0 bus at 400 kHz and address `0x0f` (payload index 0, face
`pos-z`). The sample selects the hardware backend, whose I2C transfer
implementation is present.

The Zephyr device readiness check covers software and bus initialization; it does
not contact the AMU. Control the AMU supply externally on the bench. After
switching it on, use the shell to check communication, wait for it to be idle,
and apply any configured devicetree overrides:

```text
amu probe
amu wait_ready 5000 10
amu apply_dt_config
amu get_config
amu trigger_sweep
```

Switch the AMU supply off and on externally to test recovery, then repeat the
commands. `amu probe` performs one revision read. `amu wait_ready 0 0` checks
once immediately; a responsive busy device returns `-EBUSY`. A positive timeout
requires a positive poll interval and returns `-ETIMEDOUT` if readiness expires.
The default sample has no configuration overrides, so `amu apply_dt_config`
does nothing unless overrides are added to its devicetree. Applying overrides
does not persist them: use `amu save_config` explicitly when
persistence is desired. The same sequence is available to application code via
`amu_probe`, `amu_wait_ready`, and `amu_apply_dt_config`.

Unit tests compile `lib/` only and provide a fake transfer implementation, delay,
and monotonic clock. They do not enable the Zephyr driver.

## App integration (perovsat-app)

1. Add this repo as a west project.
2. Snippet `.conf`: `CONFIG_PEROVSAT_AMU=y` and a backend.
3. I2C child node with `compatible = "aerospace,amu"`, `reg`, required `payload-index`, and
   `face` (`pos-z` / `pos-x` from `perovsat-face.yaml`). Enabled AMUs must use contiguous
   indices `0 .. NUM_AMUS-1` (see `perovsat-app/snippets/amu-hardware/`).
