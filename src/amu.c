#define DT_DRV_COMPAT aerospace_amu

#include "amu.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "transfer.h"

LOG_MODULE_REGISTER(amu, CONFIG_LOG_DEFAULT_LEVEL);

// Public API functions declared in amu.h are implemented here

static void amu_delay(uint32_t ms)
{
	k_msleep(ms);
}

int amu_set_address(const struct device *dev, uint8_t addr)
{
#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	return amu_lib_set_address(amu_transfer, (void *)dev, addr, amu_delay);
#else
	ARG_UNUSED(dev);
	ARG_UNUSED(addr);
	return 0;
#endif
}

int amu_do_iv_sweep(const struct device *dev, iv_sweep_t *sweep)
{
#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	return amu_lib_do_iv_sweep(amu_transfer, (void *)dev, amu_delay, sweep);
#else
	ARG_UNUSED(dev);

	sweep->tsensor_start = 20.0f;
	sweep->tsensor_end = 25.0f;
	sweep->time_start = 0u;

	for (int i = 0; i < IV_POINTS; ++i) {
		/* simple linear sweep from 0V to 1V and 0A to 10mA */
		float t = (float)i / (float)(IV_POINTS - 1);
		sweep->voltage[i] = t * 1.0f;
		sweep->current[i] = t * 0.010f;
	}

	return 0;
#endif
}

static int amu_init(const struct device *dev)
{
#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	LOG_INF("AMU init starting: %s", dev->name);

	// Initialize transfer backend and library
	int ret = amu_transfer_init(dev);
	if (ret < 0) {
		LOG_ERR("AMU %s: transfer init failed (%d)", dev->name, ret);
		return ret;
	}

	LOG_DBG("AMU %s: transfer init ok", dev->name);

	ret = amu_lib_init(amu_transfer, dev);
	if (ret < 0) {
		LOG_ERR("AMU %s: device probe failed (%d)", dev->name, ret);
		return ret;
	}

	LOG_INF("AMU contact success: %s", dev->name);

	const struct amu_config *cfg = dev->config;
	amu_sweep_cfg_t sweep_cfg = {
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

	ret = amu_lib_apply_config(amu_transfer, dev, &sweep_cfg, amu_delay);
	if (ret < 0) {
		LOG_ERR("AMU %s: apply config failed (%d)", dev->name, ret);
		return ret;
	}

	LOG_DBG("AMU %s: DT config applied", dev->name);

	return 0;
#else
	ARG_UNUSED(dev);
	return 0;
#endif
}

#define AMU_INIT(inst)                                                                             \
	static struct amu_data amu_data_##inst;                                                    \
	static const struct amu_config amu_config_##inst = {                                       \
		IF_ENABLED(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE,                                   \
			   (.bus = I2C_DT_SPEC_INST_GET(inst),)) .type =                             \
				    DT_INST_PROP_OR(inst, amu_type, 0),                            \
			    .has_type = DT_INST_NODE_HAS_PROP(inst, amu_type),                     \
			    .delay = DT_INST_PROP_OR(inst, amu_delay, 0),                          \
			    .has_delay = DT_INST_NODE_HAS_PROP(inst, amu_delay),                   \
			    .ratio = DT_INST_PROP_OR(inst, amu_ratio, 0),                          \
			    .has_ratio = DT_INST_NODE_HAS_PROP(inst, amu_ratio),                   \
			    .power = DT_INST_PROP_OR(inst, amu_power, 0),                          \
			    .has_power = DT_INST_NODE_HAS_PROP(inst, amu_power),                   \
			    .dac_gain = DT_INST_PROP_OR(inst, amu_dac_gain, 0),                    \
			    .has_dac_gain = DT_INST_NODE_HAS_PROP(inst, amu_dac_gain),             \
			    .sweep_averages = DT_INST_PROP_OR(inst, amu_sweep_averages, 0),        \
			    .has_sweep_averages = DT_INST_NODE_HAS_PROP(inst, amu_sweep_averages), \
			    .adc_averages = DT_INST_PROP_OR(inst, amu_adc_averages, 0),            \
			    .has_adc_averages = DT_INST_NODE_HAS_PROP(inst, amu_adc_averages),     \
			    .am0_mw = DT_INST_PROP_OR(inst, amu_am0_mw, 0),                        \
			    .has_am0 = DT_INST_NODE_HAS_PROP(inst, amu_am0_mw),                    \
			    .area_ucm2 = DT_INST_PROP_OR(inst, amu_area_ucm2, 0),                  \
			    .has_area = DT_INST_NODE_HAS_PROP(inst, amu_area_ucm2),                \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, amu_init, NULL, &amu_data_##inst, &amu_config_##inst,          \
			      BOOT_STAGE, BOOT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(AMU_INIT)
