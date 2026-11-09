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
		.numPoints = PSP_IV_POINTS,
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

int amu_get_sweep_meta(amu_t *dev, psp_sweep_metadata *meta)
{
	(void)dev;

	if (meta == NULL) {
		return 0;
	}

	*meta = (psp_sweep_metadata){
		.tsensor_start = 20.0f,
		.tsensor_end = 25.0f,
	};

	return 0;
}

int amu_get_sweep_iv(amu_t *dev, psp_sweep_points *iv)
{
	int i;

	(void)dev;

	if (iv == NULL) {
		return 0;
	}

	for (i = 0; i < PSP_IV_POINTS; ++i) {
		float t = (float)i / (float)(PSP_IV_POINTS - 1);

		iv->voltage[i] = t * 1.0f;
		iv->current[i] = t * 0.010f;
	}

	return 0;
}

int amu_get_sweep(amu_t *dev, psp_sweep *sweep)
{
	(void)dev;

	if (sweep == NULL) {
		return 0;
	}

	amu_get_sweep_meta(dev, &sweep->meta);
	amu_get_sweep_iv(dev, &sweep->iv);

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
