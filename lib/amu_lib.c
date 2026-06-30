// The majority of the logic for this library is from Aerospace's AMULIB:
// https://github.com/the-aerospace-corporation/amulib/

#include "amu_lib.h"
#include "amu_regs.h"
#include "amu_cmds.h"
#include "amu_types.h"

#include <errno.h>

#include <zephyr/logging/log.h>
#include <inttypes.h>

LOG_MODULE_DECLARE(amu);

static int amu_wait_for_ready(amu_transfer_fn_t transfer, void *ctx, uint16_t interval_ms,
			      uint16_t timeout_ms, amu_delay_fn_t delay)
{
	uint8_t cmd_val = 0xFF;
	uint32_t elapsed = 0;

	while (elapsed < timeout_ms) {
		int ret = transfer(ctx, AMU_REG_CMD, &cmd_val, 1, true);
		if (ret < 0) {
			return ret;
		}
		if (cmd_val == 0x00) {
			return 0; // Ready
		}

		if (delay) {
			delay(interval_ms);
		}
		elapsed += interval_ms;
	}
	return -ETIMEDOUT;
}

static void amu_log_sweep_config(const char *label, const amu_ivsweep_config_t *cfg)
{
	LOG_INF("%s sweep config: type=%u numPoints=%u delay=%u ratio=%u power=%u "
		"dac_gain=%u sweep_averages=%u adc_averages=%u am0=%ld mW/m2 area=%ld ucm2",
		label, cfg->type, cfg->numPoints, cfg->delay, cfg->ratio, cfg->power, cfg->dac_gain,
		cfg->sweep_averages, cfg->adc_averages, (long)(cfg->am0 * 1000.0f),
		(long)(cfg->area * 10000.0f));
}

int amu_lib_do_iv_sweep(amu_transfer_fn_t transfer, void *ctx, amu_delay_fn_t delay,
			iv_sweep_t *sweep)
{
	if (!transfer || !sweep) {
		return -1;
	}

	int ret;

	// Check not busy for safety
	ret = amu_wait_for_ready(transfer, ctx, 10, 1000, delay);
	if (ret < 0) {
		return ret;
	}

	// Read the current config from hardware so unset fields preserve EEPROM values
	amu_ivsweep_config_t wire;
	ret = transfer(ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire, sizeof(wire), true);
	if (ret < 0) {
		return ret;
	}

	amu_log_sweep_config("Current", &wire);
	// Command to start the sweep
	uint8_t cmd_val = CMD_SWEEP_TRIG_SWEEP;
	ret = transfer(ctx, AMU_REG_CMD, &cmd_val, 1, false);
	if (ret < 0) {
		return ret;
	}

	delay(10); // So the instant poll after doesn't crash it

	// Poll for completion
	ret = amu_wait_for_ready(transfer, ctx, 50, 2000, delay);
	if (ret < 0) {
		return ret;
	}

	// Read metadata
	amu_ivsweep_meta_t meta;
	ret = transfer(ctx, AMU_REG_DATA_PTR_SWEEP_META, (uint8_t *)&meta, sizeof(meta), true);
	if (ret < 0) {
		return ret;
	}
	sweep->tsensor_start = meta.tsensor_start;
	sweep->tsensor_end = meta.tsensor_end;
	sweep->time_start = meta.timestamp;

	LOG_INF("Sweep metadata: voc=%ld mV isc=%ld uA ff=%ld%% eff=%ld%%",
		(long)(meta.voc * 1000.0f), (long)(meta.isc * 1e6f), (long)(meta.ff * 100.0f),
		(long)(meta.eff * 100.0f));
	LOG_INF("  vmax=%ld mV imax=%ld uA pmax=%ld uW", (long)(meta.vmax * 1000.0f),
		(long)(meta.imax * 1e6f), (long)(meta.pmax * 1e6f));
	LOG_INF("  tsensor_start=%ld tsensor_end=%ld adc=%ld ts=%" PRIu32 " crc=0x%08" PRIx32,
		(long)meta.tsensor_start, (long)meta.tsensor_end, (long)(meta.adc * 1000.0f),
		meta.timestamp, meta.crc);

	// Read voltages
	ret = transfer(ctx, AMU_REG_DATA_PTR_VOLTAGE, (uint8_t *)sweep->voltage,
		       sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	// Read currents
	ret = transfer(ctx, AMU_REG_DATA_PTR_CURRENT, (uint8_t *)sweep->current,
		       sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int amu_lib_set_address(amu_transfer_fn_t transfer, void *ctx, uint8_t addr, amu_delay_fn_t delay)
{
	if (!transfer || !delay) {
		return -EINVAL;
	}

	int ret;

	ret = transfer(ctx, AMU_REG_TRANSFER_PTR, &addr, 1, false);
	if (ret < 0) {
		return ret;
	}

	uint8_t cmd = CMD_SET_ADDRESS;
	ret = transfer(ctx, AMU_REG_CMD, &cmd, 1, false);
	if (ret < 0) {
		return ret;
	}

	// Ensure it goes through
	amu_wait_for_ready(transfer, ctx, 10, 1000, delay);

	return 0;
}

int amu_lib_apply_config(amu_transfer_fn_t transfer, void *ctx, const amu_sweep_cfg_t *cfg,
			 amu_delay_fn_t delay)
{
	if (!cfg) {
		return 0;
	}

	if (!transfer || !delay) {
		return -EINVAL;
	}

	// Check not busy for safety
	int ret = amu_wait_for_ready(transfer, ctx, 10, 1000, delay);
	if (ret < 0) {
		return ret;
	}

	// Read the current config from hardware so unset fields preserve EEPROM values
	amu_ivsweep_config_t wire;
	ret = transfer(ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire, sizeof(wire), true);
	if (ret < 0) {
		return ret;
	}

	amu_log_sweep_config("Current", &wire);

	// Only overwrite fields that the caller explicitly set
	if (cfg->set_type) {
		wire.type = cfg->type;
	}
	if (cfg->set_delay) {
		wire.delay = cfg->delay;
	}
	if (cfg->set_ratio) {
		wire.ratio = cfg->ratio;
	}
	if (cfg->set_power) {
		wire.power = cfg->power;
	}
	if (cfg->set_dac_gain) {
		wire.dac_gain = cfg->dac_gain;
	}
	if (cfg->set_sweep_averages) {
		wire.sweep_averages = cfg->sweep_averages;
	}
	if (cfg->set_adc_averages) {
		wire.adc_averages = cfg->adc_averages;
	}
	if (cfg->set_am0) {
		wire.am0 = cfg->am0;
	}
	if (cfg->set_area) {
		wire.area = cfg->area;
	}

	// numPoints is always authoritative from the compile-time constant
	wire.numPoints = IV_POINTS;

	amu_log_sweep_config("Updated", &wire);

	// Save configuration to AMU RAM
	ret = transfer(ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire, sizeof(wire), false);
	if (ret < 0) {
		return ret;
	}

	// Command - Save to EEPROM
	uint8_t save_cmd = CMD_SWEEP_CONFIG_SAVE;
	ret = transfer(ctx, AMU_REG_CMD, &save_cmd, 1, false);
	if (ret < 0) {
		return ret;
	}

	// Wait for command
	ret = amu_wait_for_ready(transfer, ctx, 10, 1000, delay);
	if (ret < 0) {
		return ret;
	}

	ret = transfer(ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire, sizeof(wire), true);
	if (ret < 0) {
		return ret;
	}

	amu_log_sweep_config("After Write", &wire);

	return 0;
}

int amu_lib_init(amu_transfer_fn_t transfer, void *ctx)
{
	if (!transfer) {
		return -EINVAL;
	}

	uint8_t hw_rev = 0;

	// Probe the device by reading its Hardware Revision register
	int ret = transfer(ctx, AMU_REG_SYSTEM_HARDWARE_REVISION, &hw_rev, 1, true);
	if (ret < 0) {
		return ret; // I2C Bus or transfer error
	}

	// 0x00 or 0xFF indicates a floating bus or disconnected device.
	// Valid hardware revisions in amu_types.h are defined (e.g., 0x10, 0x20, 0x30).
	if (hw_rev == 0x00 || hw_rev == 0xFF) {
		return -ENODEV;
	}

	return 0;
}
