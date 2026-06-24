#pragma once

#define BOOT_STAGE    POST_KERNEL
#define BOOT_PRIORITY 100
// Put any other definitions here

#include <zephyr/device.h>

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
#include "amu_lib.h"
#endif

// This is the public API definition
struct amu_driver_api {
	// Add function definitions like
	// int (*read_sensor)(const struct device *dev, uint8_t *val);
};

// Static config data, filled at init
struct amu_config {
	// Mirror the devicetree properties here

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	// Add bus spec here
#endif
};

// Mutable state during runtime
struct amu_data {
	// Things like cached samples go here

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_SIMULATION)
	// Define things like a socket fd
#endif
};
