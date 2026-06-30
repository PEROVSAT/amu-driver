#pragma once

#define BOOT_STAGE    POST_KERNEL
#define BOOT_PRIORITY 100

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	#include <zephyr/drivers/i2c.h>
#endif

#include "amu_lib.h"

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

/**
 * @brief Set the I2C address of an AMU device.
 *
 * Calls amu_lib_set_address via the device's transfer backend.  The new
 * address is not persisted to EEPROM; call amu_save_config_to_eeprom
 * afterward if persistence is required.
 *
 * @param spec  AMU device and channel spec.
 * @param addr  New 7-bit I2C address.
 *
 * @return 0 on success, negative errno on failure.
 */
int amu_set_address(const struct amu_dt_spec *spec, uint8_t addr);

#ifdef __cplusplus
}
#endif

// Static config data, filled at init
struct amu_config {
#if defined(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE)
	struct i2c_dt_spec bus;
#endif

	/* Sweep configuration mirrored from DeviceTree.
	 * Fields default to 0; a zero value means "do not override EEPROM".
	 * am0 and area are fixed-point encoded: am0 in mW/m², area in µcm². */
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

// Mutable state during runtime
struct amu_data {
	// Things like cached samples go here

#if defined(CONFIG_PEROVSAT_AMU_BACKEND_SIMULATION)
	// TODO: socket fd
#endif
};
