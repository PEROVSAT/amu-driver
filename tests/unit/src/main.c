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

struct step {
	uint8_t reg;
	int ret;
	uint8_t value;
	uint32_t duration;
};
static const struct step *script;
static size_t script_len, script_pos;
static uint64_t now_ms;
static uint32_t last_delay;
static unsigned int writes;

static void set_script(const struct step *steps, size_t count)
{
	script = steps;
	script_len = count;
	script_pos = 0;
	now_ms = 0;
	last_delay = 0;
	writes = 0;
}

static uint8_t hw_rev;
static uint8_t cmd;
static uint8_t last_cmd;
static uint8_t transfer_ptr[sizeof(float)];
static amu_config_t sweep_cfg;
static psp_sweep_metadata sweep_meta;
static float voltage[PSP_IV_POINTS];
static float current[PSP_IV_POINTS];

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	ARG_UNUSED(ctx);
	if (!read) {
		writes++;
	}
	if (script != NULL) {
		zassert_true(script_pos < script_len, "unexpected transfer");
		const struct step *step = &script[script_pos++];
		zassert_equal(reg, step->reg);
		zassert_true(read);
		now_ms += step->duration;
		if (step->ret == 0) {
			*buf = step->value;
		}
		return step->ret;
	}

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
	last_delay = ms;
	now_ms += ms;
}

uint64_t amu_time_ms(void)
{
	return now_ms;
}

static amu_t test_dev(void)
{
	return (amu_t){
		.bus_ctx = (void *)1,
	};
}

ZTEST(amu_unit, test_probe_rejects_null)
{
	zassert_equal(amu_probe(NULL), -EINVAL);
}

ZTEST(amu_unit, test_probe_rejects_disconnected)
{
	amu_t dev = test_dev();

	hw_rev = 0xFF;
	cmd = 0x00;
	zassert_equal(amu_probe(&dev), -ENODEV);

	hw_rev = 0x00;
	zassert_equal(amu_probe(&dev), -ENODEV);
}

ZTEST(amu_unit, test_probe_probes_device)
{
	amu_t dev = test_dev();

	hw_rev = 0x10;
	cmd = 0x00;

	zassert_ok(amu_probe(&dev));
	zassert_equal(dev.hw_rev, 0x10);
}

ZTEST(amu_unit, test_wait_ready_retries_and_reuses_device)
{
	amu_t dev = test_dev();
	const struct step steps[] = {
		{AMU_REG_SYSTEM_HARDWARE_REVISION, -ETIMEDOUT, 0, 100},
		{AMU_REG_SYSTEM_HARDWARE_REVISION, -EIO, 0, 1},
		{AMU_REG_SYSTEM_HARDWARE_REVISION, 0, 0x10, 1},
		{AMU_REG_CMD, 0, 1, 1},
		{AMU_REG_SYSTEM_HARDWARE_REVISION, 0, 0x11, 1},
		{AMU_REG_CMD, 0, 0, 1},
	};
	set_script(steps, ARRAY_SIZE(steps));
	zassert_ok(amu_wait_ready(&dev, 10000, 100));
	zassert_equal(script_pos, ARRAY_SIZE(steps));
	zassert_equal(dev.hw_rev, 0x11);
	zassert_equal(writes, 0);
	/* A later operation fails once without implicitly retrying or writing. */
	const struct step lost[] = {{AMU_REG_CMD, -EIO, 0, 0}};
	psp_sweep_metadata meta = {0};
	set_script(lost, ARRAY_SIZE(lost));
	zassert_equal(amu_get_sweep_meta(&dev, &meta), -EIO);
	zassert_equal(script_pos, 1);
	zassert_equal(writes, 0);

	set_script(steps, ARRAY_SIZE(steps));
	zassert_ok(amu_wait_ready(&dev, 10000, 100));
	zassert_equal(writes, 0);
	script = NULL;
	cmd = 0;
	sweep_meta.timestamp = 42;
	zassert_ok(amu_get_sweep_meta(&dev, &meta));
	zassert_equal(meta.timestamp, 42);
}

ZTEST(amu_unit, test_wait_ready_deadline_and_immediate)
{
	amu_t dev = test_dev();
	const struct step slow[] = {{AMU_REG_SYSTEM_HARDWARE_REVISION, -ETIMEDOUT, 0, 7}};
	set_script(slow, ARRAY_SIZE(slow));
	zassert_equal(amu_wait_ready(&dev, 10, 20), -ETIMEDOUT);
	zassert_equal(last_delay, 3);
	zassert_equal(now_ms, 10);
	zassert_equal(script_pos, 1);
	set_script(slow, ARRAY_SIZE(slow));
	zassert_equal(amu_wait_ready(&dev, 5, 1), -ETIMEDOUT);
	zassert_equal(last_delay, 0);
	set_script(slow, ARRAY_SIZE(slow));
	zassert_equal(amu_wait_ready(&dev, 0, 0), -ETIMEDOUT);
	zassert_equal(last_delay, 0);
	const struct step busy[] = {
		{AMU_REG_SYSTEM_HARDWARE_REVISION, 0, 1, 0},
		{AMU_REG_CMD, 0, 1, 0},
	};
	set_script(busy, ARRAY_SIZE(busy));
	zassert_equal(amu_wait_ready(&dev, 0, 0), -EBUSY);
	zassert_equal(amu_wait_ready(NULL, 1, 1), -EINVAL);
	zassert_equal(amu_wait_ready(&dev, 1, 0), -EINVAL);
	const struct step bad[] = {{AMU_REG_SYSTEM_HARDWARE_REVISION, -EINVAL, 0, 0}};
	set_script(bad, ARRAY_SIZE(bad));
	zassert_equal(amu_wait_ready(&dev, 50, 1), -EINVAL);
	script = NULL;
}

ZTEST(amu_unit, test_get_set_save_config)
{
	amu_t dev = test_dev();
	amu_config_t got = {0};
	amu_config_t want = {
		.type = 2,
		.numPoints = PSP_IV_POINTS,
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
	psp_sweep_metadata meta = {0};
	psp_sweep_points iv = {0};
	int i;

	cmd = 0x00;
	sweep_meta.voc = 0.7f;
	sweep_meta.vmax = 0.55f;
	sweep_meta.tsensor_start = 21.0f;
	sweep_meta.timestamp = 42;

	for (i = 0; i < PSP_IV_POINTS; i++) {
		voltage[i] = (float)i;
		current[i] = (float)i * 0.001f;
	}

	zassert_ok(amu_trigger_sweep(&dev));
	zassert_equal(last_cmd, AMU_CMD_SWEEP_TRIG_SWEEP);

	zassert_ok(amu_get_sweep_meta(&dev, &meta));
	zassert_equal(meta.timestamp, 42);
	zassert_equal(meta.voc, 0.7f);
	zassert_equal(meta.vmax, 0.55f);

	zassert_ok(amu_get_sweep_iv(&dev, &iv));
	zassert_mem_equal(iv.voltage, voltage, sizeof(voltage));
	zassert_mem_equal(iv.current, current, sizeof(current));

	psp_sweep sweep = {0};
	zassert_ok(amu_get_sweep(&dev, &sweep));
	zassert_equal(sweep.meta.timestamp, 42);
	for (i = 0; i < PSP_IV_POINTS; i++) {
		zassert_equal(sweep.iv.voltage[i], voltage[i]);
		zassert_equal(sweep.iv.current[i], current[i]);
	}

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

static void reset_script(void *fixture)
{
	ARG_UNUSED(fixture);
	set_script(NULL, 0);
}

ZTEST_SUITE(amu_unit, NULL, NULL, reset_script, NULL, NULL);
