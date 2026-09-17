#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Device API. Adding a function:
 *   1. Declare it here
 *   2. Implement it in lib/amu.c
 *   3. Stub it in mock/amu.c
 *
 * Do not edit src/ for API work.
 */

typedef struct {
	/* FILL IN: DT-backed settings used by init. Remove if unused. */
	uint8_t unused;
} amu_config_t;

typedef struct amu {
	/*
	 * Which instance to talk to. src/ sets this to the Zephyr device
	 * pointer; unit tests set it to any non-NULL token. App code should
	 * not read or write it.
	 *
	 * The bus implementation is a named function (amu_transfer),
	 * not a pointer on this object. See lib/amu_bus.h.
	 */
	void *bus_ctx;

	/* FILL IN: cached chip state used after init. */
} amu_t;

int amu_init(amu_t *dev, const amu_config_t *cfg);

/* App glue: Zephyr `struct device` → this object. Implemented in src/. */
struct device;
amu_t *amu_from_dev(const struct device *dev);

#ifdef __cplusplus
}
#endif
