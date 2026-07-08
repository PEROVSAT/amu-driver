#include "transfer.h"

#include "amu.h"

#include <errno.h>
#include <string.h>

#include <zephyr/sys/util.h>

/* Register addresses (mirror lib/amu_lib.c) */
#define AMU_REG_CMD          0x00
#define AMU_REG_HW_REV       0x03
#define AMU_REG_TIME_MILLIS  0xA8
#define AMU_REG_SWEEP_META   0xC0
#define AMU_REG_DATA_VOLTAGE 0xF1
#define AMU_REG_DATA_CURRENT 0xF2

#define CMD_SWEEP_TRIG_SWEEP 0x42

typedef struct __attribute__((packed)) {
	float voc;
	float isc;
	float tsensor_start;
	float tsensor_end;
	float ff;
	float eff;
	float vmax;
	float imax;
	float pmax;
	float adc;
	uint32_t timestamp;
	uint32_t crc;
} amu_ivsweep_meta_t;

static uint8_t s_cmd;
static uint8_t s_hw_rev;
static uint32_t s_time_millis;
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
	{AMU_REG_HW_REV, &s_hw_rev, sizeof(s_hw_rev)},
	{AMU_REG_TIME_MILLIS, &s_time_millis, sizeof(s_time_millis)},
	{AMU_REG_SWEEP_META, &s_meta, sizeof(s_meta)},
	{AMU_REG_DATA_VOLTAGE, s_voltage, sizeof(s_voltage)},
	{AMU_REG_DATA_CURRENT, s_current, sizeof(s_current)},
};

typedef void (*cmd_handler_t)(void);

static void handle_iv_sweep(void)
{
	s_cmd = 0x00;
}

struct cmd_entry {
	uint8_t cmd;
	cmd_handler_t handler;
};

static const struct cmd_entry cmd_table[] = {
	{CMD_SWEEP_TRIG_SWEEP, handle_iv_sweep},
};

static const struct reg_entry *find_reg(uint8_t reg)
{
	for (size_t i = 0; i < ARRAY_SIZE(reg_table); i++) {
		if (reg_table[i].addr == reg) {
			return &reg_table[i];
		}
	}

	return NULL;
}

static void dispatch_cmd(uint8_t cmd)
{
	for (size_t i = 0; i < ARRAY_SIZE(cmd_table); i++) {
		if (cmd_table[i].cmd == cmd) {
			cmd_table[i].handler();
			return;
		}
	}
}

int amu_transfer(const void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	ARG_UNUSED(ctx);

	const struct reg_entry *entry = find_reg(reg);

	if (!entry || len > entry->size) {
		return -EINVAL;
	}

	if (read) {
		memcpy(buf, entry->data, len);
	} else {
		memcpy(entry->data, buf, len);

		if (reg == AMU_REG_CMD) {
			dispatch_cmd(buf[0]);
		}
	}

	return 0;
}

int amu_transfer_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	s_cmd = 0x00;
	s_hw_rev = 0x10;
	s_time_millis = 6000;

	memset(&s_meta, 0, sizeof(s_meta));
	s_meta.tsensor_start = 30.0f;
	s_meta.tsensor_end = 35.0f;
	s_meta.timestamp = 5000;

	for (int i = 0; i < IV_POINTS; i++) {
		s_voltage[i] = (float)i * 0.02f;
		s_current[i] = (float)(IV_POINTS - 1 - i) * 0.0003f;
	}

	return 0;
}
