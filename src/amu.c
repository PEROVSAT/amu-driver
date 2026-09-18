/*
 * Zephyr packaging: apply DT, bind the bus, call amu_init().
 * Do not add device functions here. See include/amu.h.
 */

#define DT_DRV_COMPAT aerospace_amu

#include "amu_priv.h"

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	#include "amu_bus.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(amu, CONFIG_LOG_DEFAULT_LEVEL);

amu_t *amu_from_dev(const struct device *dev)
{
	struct amu_driver_data *data = dev->data;

	return &data->chip;
}

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
void amu_delay(uint32_t ms)
{
	k_msleep(ms);
}
#endif

static bool amu_dt_has_overlay(const struct amu_dt_config *dt)
{
	return dt->has_type || dt->has_delay || dt->has_ratio || dt->has_power ||
	       dt->has_dac_gain || dt->has_sweep_averages || dt->has_adc_averages || dt->has_am0 ||
	       dt->has_area;
}

static void amu_dt_apply_overlay(amu_config_t *cfg, const struct amu_dt_config *dt)
{
	if (dt->has_type) {
		cfg->type = dt->type;
	}
	if (dt->has_delay) {
		cfg->delay = dt->delay;
	}
	if (dt->has_ratio) {
		cfg->ratio = dt->ratio;
	}
	if (dt->has_power) {
		cfg->power = dt->power;
	}
	if (dt->has_dac_gain) {
		cfg->dac_gain = dt->dac_gain;
	}
	if (dt->has_sweep_averages) {
		cfg->sweep_averages = dt->sweep_averages;
	}
	if (dt->has_adc_averages) {
		cfg->adc_averages = dt->adc_averages;
	}
	if (dt->has_am0) {
		cfg->am0 = (float)dt->am0_mw / 1000.0f;
	}
	if (dt->has_area) {
		cfg->area = (float)dt->area_ucm2 / 10000.0f;
	}
}

static int amu_driver_init(const struct device *dev)
{
	struct amu_driver_data *data = dev->data;
	const struct amu_driver_config *cfg = dev->config;
	amu_config_t on_device;
	int ret;

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	data->chip.bus_ctx = (void *)dev;

	ret = amu_transfer_init(dev);
	if (ret < 0) {
		return ret;
	}
#endif

	ret = amu_init(&data->chip);
	if (ret < 0) {
		return ret;
	}

	if (!amu_dt_has_overlay(&cfg->dt)) {
		return 0;
	}

	ret = amu_get_config(&data->chip, &on_device);
	if (ret < 0) {
		return ret;
	}

	amu_dt_apply_overlay(&on_device, &cfg->dt);

	return amu_save_config(&data->chip, &on_device);
}

#define AMU_INIT(inst)                                                                             \
	static struct amu_driver_data amu_data_##inst;                                             \
	static const struct amu_driver_config amu_config_##inst = {                                \
		.dt =                                                                              \
			{                                                                          \
				.type = DT_INST_PROP_OR(inst, amu_type, 0),                        \
				.has_type = DT_INST_NODE_HAS_PROP(inst, amu_type),                 \
				.delay = DT_INST_PROP_OR(inst, amu_delay, 0),                      \
				.has_delay = DT_INST_NODE_HAS_PROP(inst, amu_delay),               \
				.ratio = DT_INST_PROP_OR(inst, amu_ratio, 0),                      \
				.has_ratio = DT_INST_NODE_HAS_PROP(inst, amu_ratio),               \
				.power = DT_INST_PROP_OR(inst, amu_power, 0),                      \
				.has_power = DT_INST_NODE_HAS_PROP(inst, amu_power),               \
				.dac_gain = DT_INST_PROP_OR(inst, amu_dac_gain, 0),                \
				.has_dac_gain = DT_INST_NODE_HAS_PROP(inst, amu_dac_gain),         \
				.sweep_averages = DT_INST_PROP_OR(inst, amu_sweep_averages, 0),    \
				.has_sweep_averages =                                              \
					DT_INST_NODE_HAS_PROP(inst, amu_sweep_averages),           \
				.adc_averages = DT_INST_PROP_OR(inst, amu_adc_averages, 0),        \
				.has_adc_averages = DT_INST_NODE_HAS_PROP(inst, amu_adc_averages), \
				.am0_mw = DT_INST_PROP_OR(inst, amu_am0_mw, 0),                    \
				.has_am0 = DT_INST_NODE_HAS_PROP(inst, amu_am0_mw),                \
				.area_ucm2 = DT_INST_PROP_OR(inst, amu_area_ucm2, 0),              \
				.has_area = DT_INST_NODE_HAS_PROP(inst, amu_area_ucm2),            \
			},                                                                         \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, amu_driver_init, NULL, &amu_data_##inst, &amu_config_##inst,   \
			      BOOT_STAGE, BOOT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(AMU_INIT)
