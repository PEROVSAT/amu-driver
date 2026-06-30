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
	uint32_t time_end;
	float voltage[IV_POINTS];
	float current[IV_POINTS];
} iv_sweep_t;

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
