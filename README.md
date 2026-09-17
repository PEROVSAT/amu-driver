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
| `lib/amu_bus.h` | Named bus functions (`transfer`, `delay`) |
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

From a west workspace that includes this module:

```bash
west twister -T tests/unit -p native_sim
west build -b native_sim amu-driver/samples/repl
west build -t run
```

Unit tests compile `lib/` only. Uncomment the example in `tests/unit` and add
`amu_transfer` there when tests need a bus. They do not enable the
Zephyr driver.

## App integration (perovsat-app)

1. Add this repo as a west project.
2. Snippet `.conf`: `CONFIG_PEROVSAT_AMU=y` and a backend.
3. Devicetree node with `compatible = "aerospace,amu"`. Put it on I2C/SPI/UART when filling in hardware.
