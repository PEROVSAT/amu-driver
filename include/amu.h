#pragma once

#include <stdbool.h>
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

#ifndef IV_POINTS
	#define IV_POINTS 40
#endif

typedef struct {
	float tsensor_start;
	float tsensor_end;
	uint32_t time_start;
	float voltage[IV_POINTS];
	float current[IV_POINTS];
} iv_sweep_t;

typedef struct {
	uint8_t type;
	bool set_type;

	uint8_t delay;
	bool set_delay;

	uint8_t ratio;
	bool set_ratio;

	uint8_t power;
	bool set_power;

	uint8_t dac_gain;
	bool set_dac_gain;

	uint8_t sweep_averages;
	bool set_sweep_averages;

	uint8_t adc_averages;
	bool set_adc_averages;

	float am0;
	bool set_am0;

	float area;
	bool set_area;
} amu_sweep_cfg_t;

typedef struct {
	uint8_t type;
	bool has_type;
	uint8_t delay;
	bool has_delay;
	uint8_t ratio;
	bool has_ratio;
	uint8_t power;
	bool has_power;
	uint8_t dac_gain;
	bool has_dac_gain;
	uint8_t sweep_averages;
	bool has_sweep_averages;
	uint8_t adc_averages;
	bool has_adc_averages;
	uint32_t am0_mw;
	bool has_am0;
	uint32_t area_ucm2;
	bool has_area;
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

int amu_init(amu_t *dev, const amu_config_t *cfg);
int amu_apply_config(amu_t *dev, const amu_sweep_cfg_t *cfg);
int amu_set_address(amu_t *dev, uint8_t addr);
int amu_do_iv_sweep(amu_t *dev, iv_sweep_t *sweep);

/* App glue: Zephyr `struct device` → this object. Implemented in src/. */
struct device;
amu_t *amu_from_dev(const struct device *dev);

#ifdef __cplusplus
}
#endif
