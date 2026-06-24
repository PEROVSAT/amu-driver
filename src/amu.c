#define DT_DRV_COMPAT aerospace_amu

#include "amu.h"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
#include "transfer.h"
#endif

LOG_MODULE_REGISTER(amu, CONFIG_LOG_DEFAULT_LEVEL);

// Static API functions that are defined in amu.h get implemented here

/*
static int read_sensor(const struct device *dev, uint8_t *val)
{
	#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
		return amu_lib_read_sensor(amu_transfer, dev, val);
	#else
		ARG_UNUSED(dev);
		*val = 0x01;
		return 0;
	#endif
}
*/

const struct amu_driver_api amu_api = {
	// .read_sensor = read_sensor,
};

static int amu_init(const struct device *dev)
{
#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	// Initialize transfer backend and library
	int ret = amu_transfer_init(dev);
	if (ret < 0) {
		return ret;
	}

	return amu_lib_init(amu_transfer, dev);
#else
	ARG_UNUSED(dev);
	return 0;
#endif
}

#define AMU_INIT(inst)                                                                             \
	static struct amu_data amu_data_##inst;                                                    \
	static const struct amu_config amu_config_##inst = {                                       \
		/* FILL IN: .bus = I2C_DT_SPEC_INST_GET(inst), */                                  \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, amu_init, NULL, &amu_data_##inst, &amu_config_##inst,          \
			      BOOT_STAGE, BOOT_PRIORITY, &amu_api);

DT_INST_FOREACH_STATUS_OKAY(AMU_INIT)
