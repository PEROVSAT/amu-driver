/*
 * Public fake of the device API. Same symbols as lib/amu.c;
 * CMake links exactly one of the two.
 */

#include "amu.h"

int amu_init(amu_t *dev, const amu_config_t *cfg)
{
	(void)dev;
	(void)cfg;

	return 0;
}
