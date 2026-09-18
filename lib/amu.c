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

static int amu_write_cmd(amu_t *dev, uint8_t cmd)
{
	return amu_transfer(dev->bus_ctx, AMU_REG_CMD, &cmd, 1, false);
}

static int amu_write_transfer_ptr(amu_t *dev, uint8_t *buf, size_t len)
{
	return amu_transfer(dev->bus_ctx, AMU_REG_TRANSFER_PTR, buf, len, false);
}

static int amu_dac_set_state(amu_t *dev, uint8_t enabled)
{
	int ret;

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_transfer_ptr(dev, &enabled, sizeof(enabled));
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_cmd(dev, AMU_CMD_DAC_STATE);
	if (ret < 0) {
		return ret;
	}

	return amu_wait_for_ready(dev, 10, 1000);
}

int amu_get_config(amu_t *dev, amu_config_t *cfg)
{
	int ret;

	if (dev == NULL || cfg == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	return amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)cfg,
			    sizeof(*cfg), true);
}

int amu_set_config(amu_t *dev, const amu_config_t *cfg)
{
	amu_config_t wire;
	int ret;

	if (dev == NULL || cfg == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	wire = *cfg;
	return amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_CONFIG, (uint8_t *)&wire,
			    sizeof(wire), false);
}

int amu_save_config(amu_t *dev)
{
	int ret;

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_cmd(dev, AMU_CMD_SWEEP_CONFIG_SAVE);
	if (ret < 0) {
		return ret;
	}

	return amu_wait_for_ready(dev, 10, 1000);
}

int amu_init(amu_t *dev)
{
	uint8_t hw_rev = 0;
	int ret;

	if (dev == NULL) {
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

	return 0;
}

int amu_set_address(amu_t *dev, uint8_t addr)
{
	int ret;

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_write_transfer_ptr(dev, &addr, sizeof(addr));
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_cmd(dev, AMU_CMD_SET_ADDRESS);
	if (ret < 0) {
		return ret;
	}

	(void)amu_wait_for_ready(dev, 10, 1000);

	return 0;
}

int amu_trigger_sweep(amu_t *dev)
{
	int ret;

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_cmd(dev, AMU_CMD_SWEEP_TRIG_SWEEP);
	if (ret < 0) {
		return ret;
	}

	amu_delay(10);

	return amu_wait_for_ready(dev, 50, 2000);
}

int amu_get_sweep_meta(amu_t *dev, amu_sweep_meta_t *meta)
{
	int ret;

	if (dev == NULL || meta == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	return amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_SWEEP_META, (uint8_t *)meta,
			    sizeof(*meta), true);
}

int amu_get_sweep_iv(amu_t *dev, amu_sweep_iv_t *iv)
{
	int ret;

	if (dev == NULL || iv == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_VOLTAGE, (uint8_t *)iv->voltage,
			   sizeof(iv->voltage), true);
	if (ret < 0) {
		return ret;
	}

	return amu_transfer(dev->bus_ctx, AMU_REG_DATA_PTR_CURRENT, (uint8_t *)iv->current,
			    sizeof(iv->current), true);
}

int amu_dac_enable(amu_t *dev, float voltage)
{
	int ret;

	if (dev == NULL) {
		return -EINVAL;
	}

	ret = amu_wait_for_ready(dev, 10, 1000);
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_transfer_ptr(dev, (uint8_t *)&voltage, sizeof(voltage));
	if (ret < 0) {
		return ret;
	}

	ret = amu_write_cmd(dev, AMU_CMD_DAC_VOLTAGE);
	if (ret < 0) {
		return ret;
	}

	amu_delay(10);

	return amu_dac_set_state(dev, 1);
}

int amu_dac_disable(amu_t *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	return amu_dac_set_state(dev, 0);
}
