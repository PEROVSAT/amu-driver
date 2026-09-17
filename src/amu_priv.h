#pragma once

/*
 * Zephyr packaging. Do not add device functions here.
 */

#define BOOT_STAGE    POST_KERNEL
#define BOOT_PRIORITY 100

#include <zephyr/device.h>

#include <amu.h>

struct amu_driver_config {
	amu_config_t chip;

	/* FILL IN when bringing up hardware, e.g. struct i2c_dt_spec bus; */
};

struct amu_driver_data {
	amu_t chip;
};

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
/* Defined in hardware_transfer.c or lib_mock_transfer.c. */
int amu_transfer_init(const struct device *dev);
#endif
