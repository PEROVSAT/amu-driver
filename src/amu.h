#pragma once

#define BOOT_STAGE    POST_KERNEL
#define BOOT_PRIORITY 100
#define IV_POINTS     40
// Put any other definitions here

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	#include "amu_lib.h"
#endif

typedef struct {
	float tsensor_start;
	float tsensor_end;
	uint32_t time_start;
	uint32_t time_end;
	float voltage[IV_POINTS];
	float current[IV_POINTS];
} iv_sweep_t;

// DeviceTree spec used to access a specific cell
struct amu_dt_spec {
	const struct device *dev;
	uint8_t channel;
};

// Macro to grab the correct AMU devce and channel from a cell definition in the DeviceTree
#define AMU_DT_SPEC_GET(node_id)                                                                   \
	{.dev = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(node_id)),                                       \
	 .channel = DT_IO_CHANNELS_INPUT(node_id)}

// Since this driver is so direct, we are skipping doing the whole API struct,
// 	and just declaring this as a public function
#ifdef __cplusplus
extern "C" {
#endif

int amu_do_iv_sweep(const struct amu_dt_spec *spec, iv_sweep_t *sweep);

#ifdef __cplusplus
}
#endif

// Static config data, filled at init
struct amu_config {
	// Mirror the devicetree properties here

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	struct i2c_dt_spec bus;
#endif
};

// Mutable state during runtime
struct amu_data {
	// Things like cached samples go here

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_SIMULATION)
	// TODO: socket fd
#endif
};
