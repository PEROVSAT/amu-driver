#include "amu.h"
#include "amu_bus.h"
#include "amu_regs.h"

#include <errno.h>

static int amu_wait_for_ready(amu_t *dev, uint16_t interval_ms, uint16_t timeout_ms)
{
	uint8_t cmd_val = 0xFF;
	uint32_t elapsed = 0;

	while (elapsed < timeout_ms) {
		int ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd_val, 1, true);

		if (ret < 0) {
			return ret;
		}
		if (cmd_val == 0x00) {
			return 0;
		}

		amu_delay(interval_ms);
		elapsed += interval_ms;
	}

	return -ETIMEDOUT;
}

int amu_apply_config(amu_t *dev, const amu_sweep_cfg_t *cfg)
{
	amu_ivsweep_config_t wire;
	uint8_t save_cmd;
	int ret;

	if (cfg == NULL) {
		return 0;
	}

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire,
			   sizeof(wire), true);
	if (ret < 0) {
		return ret;
	}

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

	wire.numPoints = IV_POINTS;

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire,
			   sizeof(wire), false);
	if (ret < 0) {
		return ret;
	}

	save_cmd = AMU_CMD_SWEEP_CONFIG_SAVE;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &save_cmd, 1, false);
	if (ret < 0) {
		return ret;
	}

	return amu_wait_for_ready(dev, 10, 1000);
}

int amu_init(amu_t *dev, const amu_config_t *cfg)
{
	amu_sweep_cfg_t sweep_cfg;
	uint8_t hw_rev = 0;
	int ret;

	if (dev == NULL || cfg == NULL) {
		return -EINVAL;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_SYSTEM_HARDWARE_REVISION, &hw_rev, 1, true);
	if (ret < 0) {
		return ret;
	}

	if (hw_rev == 0x00 || hw_rev == 0xFF) {
		return -ENODEV;
	}

	dev->hw_rev = hw_rev;

	sweep_cfg = (amu_sweep_cfg_t){
		.type = cfg->type,
		.set_type = cfg->has_type,
		.delay = cfg->delay,
		.set_delay = cfg->has_delay,
		.ratio = cfg->ratio,
		.set_ratio = cfg->has_ratio,
		.power = cfg->power,
		.set_power = cfg->has_power,
		.dac_gain = cfg->dac_gain,
		.set_dac_gain = cfg->has_dac_gain,
		.sweep_averages = cfg->sweep_averages,
		.set_sweep_averages = cfg->has_sweep_averages,
		.adc_averages = cfg->adc_averages,
		.set_adc_averages = cfg->has_adc_averages,
		.am0 = (float)cfg->am0_mw / 1000.0f,
		.set_am0 = cfg->has_am0,
		.area = (float)cfg->area_ucm2 / 10000.0f,
		.set_area = cfg->has_area,
	};

	return amu_apply_config(dev, &sweep_cfg);
}

int amu_set_address(amu_t *dev, uint8_t addr)
{
	uint8_t cmd;
	int ret;

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_TRANSFER_PTR, &addr, 1, false);
	if (ret < 0) {
		return ret;
	}

	cmd = AMU_CMD_SET_ADDRESS;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd, 1, false);
	if (ret < 0) {
		return ret;
	}

	(void)amu_wait_for_ready(dev, 10, 1000);

	return 0;
}

int amu_do_iv_sweep(amu_t *dev, iv_sweep_t *sweep)
{
	amu_ivsweep_meta_t meta;
	uint8_t cmd_val;
	float dac_v;
	uint8_t dac_state;
	int ret;

	if (dev == NULL || sweep == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	cmd_val = AMU_CMD_SWEEP_TRIG_SWEEP;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd_val, 1, false);
	if (ret < 0) {
		return ret;
	}

	amu_delay(10);

	ret = amu_wait_for_ready(dev, 50, 2000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_META, (uint8_t *)&meta,
			   sizeof(meta), true);
	if (ret < 0) {
		return ret;
	}

	sweep->tsensor_start = meta.tsensor_start;
	sweep->tsensor_end = meta.tsensor_end;
	sweep->time_start = meta.timestamp;

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_VOLTAGE, (uint8_t *)sweep->voltage,
			   sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_CURRENT, (uint8_t *)sweep->current,
			   sizeof(float) * IV_POINTS, true);
	if (ret < 0) {
		return ret;
	}

	dac_v = meta.vmax;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_TRANSFER_PTR, (uint8_t *)&dac_v, sizeof(dac_v),
			   false);
	if (ret < 0) {
		return ret;
	}

	cmd_val = AMU_CMD_DAC_VOLTAGE;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd_val, 1, false);
	if (ret < 0) {
		return ret;
	}

	amu_delay(10);

	dac_state = 1;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_TRANSFER_PTR, &dac_state, sizeof(dac_state),
			   false);
	if (ret < 0) {
		return ret;
	}

	cmd_val = AMU_CMD_DAC_STATE;
	ret = amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd_val, 1, false);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
