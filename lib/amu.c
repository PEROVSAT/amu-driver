#include "amu.h"

#include <errno.h>

int amu_init(amu_t *dev, const amu_config_t *cfg)
{
	if (dev == NULL || cfg == NULL) {
		return -EINVAL;
	}

	(void)cfg;

	/* FILL IN: probe the part, apply cfg, cache state on *dev.
	 * Call amu_transfer / amu_delay from
	 * lib/amu_bus.h. Do not add API functions in src/.
	 */

	return 0;
}
