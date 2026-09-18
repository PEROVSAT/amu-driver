/*
 * Unit tests for the portable device library.
 *
 * Links lib/amu.c only. Do not use src/. When a test talks to
 * the bus, define amu_transfer and amu_delay in
 * this file (see lib/amu_bus.h).
 */

#include <errno.h>
#include <string.h>

#include <zephyr/ztest.h>

#include "amu.h"
#include "amu_bus.h"
#include "amu_regs.h"

static uint8_t hw_rev;
static uint8_t cmd;
static amu_ivsweep_config_t sweep_cfg;

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	ARG_UNUSED(ctx);

	if (reg == AMU_REG_CMD) {
		if (len != 1) {
			return -EINVAL;
		}
		if (read) {
			*buf = cmd;
		} else {
			cmd = 0x00;
		}
		return 0;
	}

	if (reg == AMU_REG_SYSTEM_HARDWARE_REVISION) {
		if (!read || len != 1) {
			return -EINVAL;
		}
		*buf = hw_rev;
		return 0;
	}

	if (reg == AMU_REG_DATA_PTR_SWEEP_CONFIG) {
		if (len > sizeof(sweep_cfg)) {
			return -EINVAL;
		}
		if (read) {
			memcpy(buf, &sweep_cfg, len);
		} else {
			memcpy(&sweep_cfg, buf, len);
		}
		return 0;
	}

	return -EINVAL;
}

void amu_delay(uint32_t ms)
{
	ARG_UNUSED(ms);
}

ZTEST(amu_unit, test_init_rejects_null)
{
	amu_t dev = {
		.bus_ctx = (void *)1,
	};
	amu_config_t cfg = {0};

	zassert_equal(amu_init(NULL, &cfg), -EINVAL);
	zassert_equal(amu_init(&dev, NULL), -EINVAL);
}

ZTEST(amu_unit, test_init_rejects_disconnected)
{
	amu_t dev = {
		.bus_ctx = (void *)1,
	};
	amu_config_t cfg = {0};

	hw_rev = 0xFF;
	cmd = 0x00;
	zassert_equal(amu_init(&dev, &cfg), -ENODEV);

	hw_rev = 0x00;
	zassert_equal(amu_init(&dev, &cfg), -ENODEV);
}

ZTEST(amu_unit, test_init_probes_device)
{
	amu_t dev = {
		.bus_ctx = (void *)1,
	};
	amu_config_t cfg = {0};

	hw_rev = 0x10;
	cmd = 0x00;
	memset(&sweep_cfg, 0, sizeof(sweep_cfg));

	zassert_ok(amu_init(&dev, &cfg));
	zassert_equal(dev.hw_rev, 0x10);
	zassert_equal(sweep_cfg.numPoints, IV_POINTS);
}

ZTEST_SUITE(amu_unit, NULL, NULL, NULL, NULL, NULL);
