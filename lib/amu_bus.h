#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Link-time bus. lib/ calls these by name; the build supplies the bodies.
 *
 * Linked from exactly one place:
 *   - src/hardware_transfer.c
 *   - src/lib_mock_transfer.c
 *   - tests/unit
 *
 * amu_delay and monotonic amu_time_ms are provided by src/amu.c or tests.
 *
 * The signature below is a register read/write. Change it here and in every
 * definition if the device is not a register map.
 */

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read);
void amu_delay(uint32_t ms);
uint64_t amu_time_ms(void);
