#include "amu.h"
#include "amu_bus.h"
#include "amu_priv.h"
#include "amu_regs.h"

#include <errno.h>
#include <string.h>

#include <zephyr/sys/util.h>

static uint8_t s_cmd;
static uint8_t s_hw_rev;
static uint8_t s_transfer_ptr[sizeof(float)];
static amu_config_t s_sweep_cfg;
static amu_ivsweep_meta_t s_meta;
static float s_voltage[IV_POINTS];
static float s_current[IV_POINTS];

struct reg_entry {
	uint8_t addr;
	void *data;
	size_t size;
};

static const struct reg_entry reg_table[] = {
	{AMU_REG_CMD, &s_cmd, sizeof(s_cmd)},
	{AMU_REG_SYSTEM_HARDWARE_REVISION, &s_hw_rev, sizeof(s_hw_rev)},
	{AMU_REG_TRANSFER_PTR, s_transfer_ptr, sizeof(s_transfer_ptr)},
	{AMU_REG_DATA_PTR_SWEEP_CONFIG, &s_sweep_cfg, sizeof(s_sweep_cfg)},
	{AMU_REG_DATA_PTR_SWEEP_META, &s_meta, sizeof(s_meta)},
	{AMU_REG_DATA_PTR_VOLTAGE, s_voltage, sizeof(s_voltage)},
	{AMU_REG_DATA_PTR_CURRENT, s_current, sizeof(s_current)},
};

static void complete_cmd(void)
{
	s_cmd = 0x00;
}

static const struct reg_entry *find_reg(uint8_t reg)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(reg_table); i++) {
		if (reg_table[i].addr == reg) {
			return &reg_table[i];
		}
	}

	return NULL;
}

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	const struct reg_entry *entry = find_reg(reg);

	ARG_UNUSED(ctx);

	if (entry == NULL || len > entry->size) {
		return -EINVAL;
	}

	if (read) {
		memcpy(buf, entry->data, len);
	} else {
		memcpy(entry->data, buf, len);

		if (reg == AMU_REG_CMD) {
			complete_cmd();
		}
	}

	return 0;
}

int amu_transfer_init(const struct device *dev)
{
	int i;

	ARG_UNUSED(dev);

	s_cmd = 0x00;
	s_hw_rev = 0x10;
	memset(s_transfer_ptr, 0, sizeof(s_transfer_ptr));
	memset(&s_sweep_cfg, 0, sizeof(s_sweep_cfg));
	s_sweep_cfg.numPoints = IV_POINTS;

	memset(&s_meta, 0, sizeof(s_meta));
	s_meta.tsensor_start = 30.0f;
	s_meta.tsensor_end = 35.0f;
	s_meta.timestamp = 5000;

	for (i = 0; i < IV_POINTS; i++) {
		s_voltage[i] = (float)i * 0.02f;
		s_current[i] = (float)(IV_POINTS - 1 - i) * 0.0003f;
	}

	return 0;
}
