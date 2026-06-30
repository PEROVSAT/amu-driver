#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

/**
 * Public sweep configuration. numPoints is intentionally absent, since its always determined by
 * IV_POINTS in code
 */
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

/* * Dependency Injection callbacks
 * ctx is passed transparently to allow your driver to inject the `amu_dt_spec`
 */
typedef int (*amu_transfer_fn_t)(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read);
typedef void (*amu_delay_fn_t)(uint32_t ms);

/**
 * @brief Orchestrates an IV sweep and parses the hardware memory map into iv_sweep_t
 * @return 0 on success, < 0 on hardware/transfer failure, -2 on timeout
 */
int amu_lib_do_iv_sweep(amu_transfer_fn_t transfer, void *ctx, amu_delay_fn_t delay,
			iv_sweep_t *sweep);

/**
 * @brief Set the AMU's I2C address.
 *
 * Writes @p addr to the transfer pointer register (0xFE), then issues the
 * set-address command (0x03) to the command register.  The new address takes
 * effect immediately but is not persisted; call amu_save_config_to_eeprom
 * afterward if persistence is required.
 *
 * @param transfer  Bus transfer callback.
 * @param ctx       Opaque context passed to @p transfer.
 * @param addr      New 7-bit I2C address.
 *
 * @return 0 on success, negative errno on failure.
 */
int amu_lib_set_address(amu_transfer_fn_t transfer, void *ctx, uint8_t addr, amu_delay_fn_t delay);

/**
 * @brief Write sweep configuration registers from an amu_sweep_cfg_t.
 *
 * Builds the packed hardware config struct internally, injecting
 * numPoints = IV_POINTS, and writes it to AMU_REG_SWEEP_CONFIG (0xB0).
 * Returns 0 immediately if @p cfg is NULL.
 *
 * @param transfer  Bus transfer callback.
 * @param ctx       Opaque context passed to @p transfer.
 * @param cfg       Sweep configuration to apply.
 *
 * @return 0 on success, negative errno on failure.
 */
int amu_lib_apply_config(amu_transfer_fn_t transfer, void *ctx, const amu_sweep_cfg_t *cfg,
			 amu_delay_fn_t delay);

/**
 * Probe the device and apply initial configuration.
 *
 * Use @p transfer and @p ctx to communicate with the device (e.g. read WHO_AM_I,
 * write config registers). Call once at startup.
 *
 * @param transfer  Bus transfer callback.
 * @param ctx       Opaque context passed to @p transfer.
 *
 * @return 0 on success, negative errno on failure.
 */
int amu_lib_init(amu_transfer_fn_t transfer, void *ctx);
