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

static int amu_driver_init(const struct device *dev)
{
	struct amu_driver_data *data = dev->data;
	const struct amu_driver_config *cfg = dev->config;

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	int ret;

	data->chip.bus_ctx = (void *)dev;

	ret = amu_transfer_init(dev);
	if (ret < 0) {
		return ret;
	}
#endif

	return amu_init(&data->chip, &cfg->chip);
}

#define AMU_INIT(inst)                                                                             \
	static struct amu_driver_data amu_data_##inst;                                             \
	static const struct amu_driver_config amu_config_##inst = {                                \
		.chip =                                                                            \
			{                                                                          \
				.unused = 0, /* FILL IN: .foo = DT_INST_PROP(inst, foo), */        \
			},                                                                         \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, amu_driver_init, NULL, &amu_data_##inst, &amu_config_##inst,   \
			      BOOT_STAGE, BOOT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(AMU_INIT)
