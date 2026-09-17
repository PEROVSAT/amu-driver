/*
 * Unit tests for the portable device library.
 *
 * Links lib/amu.c only. Do not use src/. When a test talks to
 * the bus, define amu_transfer and amu_delay in
 * this file (see lib/amu_bus.h).
 */

#include <zephyr/ztest.h>

#if 0
	#include "amu.h"
	#include "amu_bus.h"

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	ARG_UNUSED(ctx);
	ARG_UNUSED(reg);
	ARG_UNUSED(buf);
	ARG_UNUSED(len);
	ARG_UNUSED(read);

	return 0;
}

void amu_delay(uint32_t ms)
{
	ARG_UNUSED(ms);
}

ZTEST(amu_unit, test_init)
{
	amu_t dev = {
		.bus_ctx = (void *)1,
	};
	amu_config_t cfg = {0};

	zassert_ok(amu_init(&dev, &cfg));
}
#endif

ZTEST_SUITE(amu_unit, NULL, NULL, NULL, NULL, NULL);
