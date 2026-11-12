#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "flight/sweep/sweep.h"

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

/*
 * Sweep config as stored on the AMU. Layout matches the device register
 * block; flight software reads and writes this directly.
 */
typedef struct __attribute__((packed)) {
	uint8_t type;
	uint8_t numPoints;
	uint8_t delay;
	uint8_t ratio;
	uint8_t power;
	uint8_t dac_gain;
	uint8_t sweep_averages;
	uint8_t adc_averages;
	float am0;
	float area;
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

	uint8_t hw_rev;
} amu_t;

/* Probe reads the revision once. The caller controls target power. */
int amu_probe(amu_t *dev);
int amu_wait_ready(amu_t *dev, uint32_t timeout_ms, uint32_t poll_interval_ms);

int amu_get_config(amu_t *dev, amu_config_t *cfg);
int amu_set_config(amu_t *dev, const amu_config_t *cfg);
int amu_save_config(amu_t *dev);

int amu_trigger_sweep(amu_t *dev);
int amu_get_sweep_meta(amu_t *dev, psp_sweep_metadata *meta);
int amu_get_sweep_iv(amu_t *dev, psp_sweep_points *iv);
int amu_get_sweep(amu_t *dev, psp_sweep *sweep);

int amu_dac_enable(amu_t *dev, float voltage);
int amu_dac_disable(amu_t *dev);

int amu_set_address(amu_t *dev, uint8_t addr);

/* App glue: Zephyr `struct device` → this object. Implemented in src/. */
struct device;
amu_t *amu_from_dev(const struct device *dev);
/* Supply a ready Zephyr AMU device; apply DT overrides without saving. */
int amu_apply_dt_config(const struct device *dev);

#ifdef __cplusplus
}
#endif
