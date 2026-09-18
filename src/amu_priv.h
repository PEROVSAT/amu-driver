#pragma once

/*
 * Zephyr packaging. Do not add device functions here.
 */

#define BOOT_STAGE    POST_KERNEL
#define BOOT_PRIORITY 100

#include <zephyr/device.h>

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	#include <zephyr/drivers/i2c.h>
#endif

#include <amu.h>

struct amu_dt_config {
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
};

struct amu_driver_config {
	struct amu_dt_config dt;
#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	struct i2c_dt_spec bus;
#endif
};

struct amu_driver_data {
	amu_t chip;
};

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
/* Defined in hardware_transfer.c or lib_mock_transfer.c. */
int amu_transfer_init(const struct device *dev);
#endif
