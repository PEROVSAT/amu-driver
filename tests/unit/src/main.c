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
static uint8_t last_cmd;
static uint8_t transfer_ptr[sizeof(float)];
static amu_config_t sweep_cfg;
static amu_sweep_meta_t sweep_meta;
static float voltage[IV_POINTS];
static float current[IV_POINTS];

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
			last_cmd = *buf;
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

	if (reg == AMU_REG_TRANSFER_PTR) {
		if (len > sizeof(transfer_ptr)) {
			return -EINVAL;
		}
		if (read) {
			memcpy(buf, transfer_ptr, len);
		} else {
			memcpy(transfer_ptr, buf, len);
		}
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

	if (reg == AMU_REG_DATA_PTR_SWEEP_META) {
		if (len > sizeof(sweep_meta)) {
			return -EINVAL;
		}
		if (read) {
			memcpy(buf, &sweep_meta, len);
		} else {
			memcpy(&sweep_meta, buf, len);
		}
		return 0;
	}

	if (reg == AMU_REG_DATA_PTR_VOLTAGE) {
		if (len > sizeof(voltage)) {
			return -EINVAL;
		}
		if (read) {
			memcpy(buf, voltage, len);
		} else {
			memcpy(voltage, buf, len);
		}
		return 0;
	}

	if (reg == AMU_REG_DATA_PTR_CURRENT) {
		if (len > sizeof(current)) {
			return -EINVAL;
		}
		if (read) {
			memcpy(buf, current, len);
		} else {
			memcpy(current, buf, len);
		}
		return 0;
	}

	return -EINVAL;
}

void amu_delay(uint32_t ms)
{
	ARG_UNUSED(ms);
}

static amu_t test_dev(void)
{
	return (amu_t){
		.bus_ctx = (void *)1,
	};
}

ZTEST(amu_unit, test_init_rejects_null)
{
	zassert_equal(amu_init(NULL), -EINVAL);
}

ZTEST(amu_unit, test_init_rejects_disconnected)
{
	amu_t dev = test_dev();

	hw_rev = 0xFF;
	cmd = 0x00;
	zassert_equal(amu_init(&dev), -ENODEV);

	hw_rev = 0x00;
	zassert_equal(amu_init(&dev), -ENODEV);
}

ZTEST(amu_unit, test_init_probes_device)
{
	amu_t dev = test_dev();

	hw_rev = 0x10;
	cmd = 0x00;

	zassert_ok(amu_init(&dev));
	zassert_equal(dev.hw_rev, 0x10);
}

ZTEST(amu_unit, test_get_set_save_config)
{
	amu_t dev = test_dev();
	amu_config_t got = {0};
	amu_config_t want = {
		.type = 2,
		.numPoints = IV_POINTS,
		.delay = 5,
		.ratio = 1,
		.power = 3,
		.dac_gain = 4,
		.sweep_averages = 2,
		.adc_averages = 8,
		.am0 = 1366.1f,
		.area = 1.0f,
	};

	cmd = 0x00;
	last_cmd = 0;
	memset(&sweep_cfg, 0, sizeof(sweep_cfg));

	zassert_ok(amu_set_config(&dev, &want));
	zassert_ok(amu_get_config(&dev, &got));
	zassert_mem_equal(&got, &want, sizeof(want));

	zassert_ok(amu_save_config(&dev));
	zassert_equal(last_cmd, AMU_CMD_SWEEP_CONFIG_SAVE);
}

ZTEST(amu_unit, test_sweep_and_dac)
{
	amu_t dev = test_dev();
	amu_sweep_meta_t meta = {0};
	amu_sweep_iv_t iv = {0};
	int i;

	cmd = 0x00;
	sweep_meta.voc = 0.7f;
	sweep_meta.vmax = 0.55f;
	sweep_meta.tsensor_start = 21.0f;
	sweep_meta.timestamp = 42;

	for (i = 0; i < IV_POINTS; i++) {
		voltage[i] = (float)i;
		current[i] = (float)i * 0.001f;
	}

	zassert_ok(amu_trigger_sweep(&dev));
	zassert_equal(last_cmd, AMU_CMD_SWEEP_TRIG_SWEEP);

	zassert_ok(amu_get_sweep_meta(&dev, &meta));
	zassert_equal(meta.timestamp, 42);
	zassert_equal(meta.voc, 0.7f);

	zassert_ok(amu_get_sweep_iv(&dev, &iv));
	zassert_mem_equal(iv.voltage, voltage, sizeof(voltage));
	zassert_mem_equal(iv.current, current, sizeof(current));

	zassert_ok(amu_dac_enable(&dev, 0.55f));
	zassert_equal(last_cmd, AMU_CMD_DAC_STATE);
	zassert_equal(transfer_ptr[0], 1);

	zassert_ok(amu_dac_disable(&dev));
	zassert_equal(last_cmd, AMU_CMD_DAC_STATE);
	zassert_equal(transfer_ptr[0], 0);
}

ZTEST(amu_unit, test_set_address)
{
	amu_t dev = test_dev();

	cmd = 0x00;
	zassert_ok(amu_set_address(&dev, 0x42));
	zassert_equal(last_cmd, AMU_CMD_SET_ADDRESS);
	zassert_equal(transfer_ptr[0], 0x42);
}

ZTEST_SUITE(amu_unit, NULL, NULL, NULL, NULL, NULL);
