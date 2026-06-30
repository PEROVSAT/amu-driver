#include "amu_lib.h"

#include <errno.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(amu);

// AMU Memory Map and Registers
#define AMU_REG_CMD                      0x00
#define AMU_REG_TIME_MILLIS              0xA8
#define AMU_REG_SWEEP_META               0xC0
#define AMU_REG_DATA_PTR_VOLTAGE         0xF1
#define AMU_REG_DATA_PTR_CURRENT         0xF2
#define AMU_REG_SYSTEM_HARDWARE_REVISION 0x03

// 8-bit command values (Lower byte of CMD_SWEEP_TRIG_SWEEP 0x0142)
#define CMD_SWEEP_TRIG_SWEEP 0x42

// Polling configuration
#define AMU_POLL_INTERVAL_MS 100  // Check completion every 100ms
#define AMU_POLL_TIMEOUT_MS  5000 // Stop trying after 5s

// Memory-mapped struct for the active sweep meta data
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

int amu_lib_do_iv_sweep(amu_transfer_fn_t transfer, void *ctx, amu_delay_fn_t delay,
			iv_sweep_t *sweep)
{
	if (!transfer || !sweep) {
		return -1;
	}

	int ret;
	uint8_t cmd_val = CMD_SWEEP_TRIG_SWEEP;

	// 1. Trigger the sweep
	ret = transfer(ctx, AMU_REG_CMD, &cmd_val, 1, false);
	if (ret < 0) {
		return ret;
	}

	// 2. Poll until AMU_REG_CMD clears to 0x00 (indicating command completion)
	uint32_t elapsed = 0;
	bool done = false;

	while (elapsed < AMU_POLL_TIMEOUT_MS) {
		if (delay) {
			delay(AMU_POLL_INTERVAL_MS);
		}
		elapsed += AMU_POLL_INTERVAL_MS;

		ret = transfer(ctx, AMU_REG_CMD, &cmd_val, 1, true);
		if (ret < 0) {
			return ret;
		}

		if (cmd_val == 0x00) {
			done = true;
			break;
		}
	}

	if (!done) {
		return -2; // Sweep timeout
	}

	// 3. Read metadata block
	amu_ivsweep_meta_t meta;
	ret = transfer(ctx, AMU_REG_SWEEP_META, (uint8_t *)&meta, sizeof(meta), true);
	if (ret < 0) {
		return ret;
	}

	// 4. Read voltage array
	ret = transfer(ctx, AMU_REG_DATA_PTR_VOLTAGE, (uint8_t *)sweep->voltage,
		       sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	// 5. Read current array
	ret = transfer(ctx, AMU_REG_DATA_PTR_CURRENT, (uint8_t *)sweep->current,
		       sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	// 6. Read end time (Current device time upon sweep completion)
	uint32_t end_time = 0;
	ret = transfer(ctx, AMU_REG_TIME_MILLIS, (uint8_t *)&end_time, sizeof(end_time), true);
	if (ret < 0) {
		return ret;
	}

	// 7. Map raw AMU data to user struct
	sweep->tsensor_start = meta.tsensor_start;
	sweep->tsensor_end = meta.tsensor_end;
	sweep->time_start = meta.timestamp;
	sweep->time_end = end_time;

	return 0;
}

int amu_lib_init(amu_transfer_fn_t transfer, void *ctx)
{
	if (!transfer) {
		LOG_ERR("AMU probe failed: transfer callback is NULL");
		return -EINVAL;
	}

	uint8_t hw_rev = 0;

	LOG_DBG("AMU probe: reading hardware revision register 0x%02x",
		AMU_REG_SYSTEM_HARDWARE_REVISION);

	// Probe the device by reading its Hardware Revision register
	int ret = transfer(ctx, AMU_REG_SYSTEM_HARDWARE_REVISION, &hw_rev, 1, true);
	if (ret < 0) {
		LOG_ERR("AMU probe: hardware revision read failed (%d)", ret);
		return ret; // I2C Bus or transfer error
	}

	LOG_DBG("AMU probe: hardware revision register returned 0x%02x", hw_rev);

	// 0x00 or 0xFF indicates a floating bus or disconnected device.
	// Valid hardware revisions in amu_types.h are defined (e.g., 0x10, 0x20, 0x30).
	if (hw_rev == 0x00 || hw_rev == 0xFF) {
		LOG_ERR("AMU probe: invalid hardware revision 0x%02x (device missing or bus "
			"floating)",
			hw_rev);
		return -ENODEV;
	}

	// AMU has EEPROM to store configuration
	// Any overriding of those would be done here

	return 0;
}
