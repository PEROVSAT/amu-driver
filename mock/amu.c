/*
 * Public fake of the device API. Same symbols as lib/amu.c;
 * CMake links exactly one of the two.
 */

#include "amu.h"
#include "stddef.h"

int amu_init(amu_t *dev, const amu_config_t *cfg)
{
	(void)dev;
	(void)cfg;

	return 0;
}

int amu_apply_config(amu_t *dev, const amu_sweep_cfg_t *cfg)
{
	(void)dev;
	(void)cfg;

	return 0;
}

int amu_set_address(amu_t *dev, uint8_t addr)
{
	(void)dev;
	(void)addr;

	return 0;
}

int amu_do_iv_sweep(amu_t *dev, iv_sweep_t *sweep)
{
	int i;

	(void)dev;

	if (sweep == NULL) {
		return 0;
	}

	sweep->tsensor_start = 20.0f;
	sweep->tsensor_end = 25.0f;
	sweep->time_start = 0u;

	for (i = 0; i < IV_POINTS; ++i) {
		float t = (float)i / (float)(IV_POINTS - 1);

		sweep->voltage[i] = t * 1.0f;
		sweep->current[i] = t * 0.010f;
	}

	return 0;
}
