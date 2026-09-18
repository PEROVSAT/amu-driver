/*
 * Public fake of the device API. Same symbols as lib/amu.c;
 * CMake links exactly one of the two.
 */

#include "amu.h"

#include <stddef.h>

int amu_init(amu_t *dev)
{
	(void)dev;

	return 0;
}

int amu_get_config(amu_t *dev, amu_config_t *cfg)
{
	(void)dev;

	if (cfg == NULL) {
		return 0;
	}

	*cfg = (amu_config_t){
		.numPoints = IV_POINTS,
	};

	return 0;
}

int amu_set_config(amu_t *dev, const amu_config_t *cfg)
{
	(void)dev;
	(void)cfg;

	return 0;
}

int amu_save_config(amu_t *dev)
{
	(void)dev;

	return 0;
}

int amu_trigger_sweep(amu_t *dev)
{
	(void)dev;

	return 0;
}

int amu_get_sweep_meta(amu_t *dev, amu_sweep_meta_t *meta)
{
	(void)dev;

	if (meta == NULL) {
		return 0;
	}

	*meta = (amu_sweep_meta_t){
		.tsensor_start = 20.0f,
		.tsensor_end = 25.0f,
	};

	return 0;
}

int amu_get_sweep_iv(amu_t *dev, amu_sweep_iv_t *iv)
{
	int i;

	(void)dev;

	if (iv == NULL) {
		return 0;
	}

	for (i = 0; i < IV_POINTS; ++i) {
		float t = (float)i / (float)(IV_POINTS - 1);

		iv->voltage[i] = t * 1.0f;
		iv->current[i] = t * 0.010f;
	}

	return 0;
}

int amu_dac_enable(amu_t *dev, float voltage)
{
	(void)dev;
	(void)voltage;

	return 0;
}

int amu_dac_disable(amu_t *dev)
{
	(void)dev;

	return 0;
}

int amu_set_address(amu_t *dev, uint8_t addr)
{
	(void)dev;
	(void)addr;

	return 0;
}
