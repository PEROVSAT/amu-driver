#define DT_DRV_COMPAT aerospace_amu

#include "amu.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	#include "transfer.h"
#endif

LOG_MODULE_REGISTER(amu, CONFIG_LOG_DEFAULT_LEVEL);

// Public API functions declared in amu.h are implemented here

#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
// Zephyr-backed delay injected into the Zephyr-agnostic library
static void amu_delay(uint32_t ms)
{
	k_msleep(ms);
}
#endif

int amu_do_iv_sweep(const struct amu_dt_spec *spec, iv_sweep_t *sweep)
{
#if !defined(CONFIG_PEROVSAT_AMU_BACKEND_PUBLIC_MOCK)
	return amu_lib_do_iv_sweep(amu_transfer, (void *)spec->dev, amu_delay, sweep);
#else
	ARG_UNUSED(spec);

	sweep->tsensor_start = 20.0f;
	sweep->tsensor_end = 25.0f;
	sweep->time_start = 0u;
	sweep->time_end = 1000u;

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

	return ret;
#else
	ARG_UNUSED(dev);
	return 0;
#endif
}

#define AMU_INIT(inst)                                                                             \
	static struct amu_data amu_data_##inst;                                                    \
	static const struct amu_config amu_config_##inst = {IF_ENABLED(CONFIG_PEROVSAT_AMU_BACKEND_HARDWARE,                                 \
			   (.bus = I2C_DT_SPEC_INST_GET(inst),)) };        \
	DEVICE_DT_INST_DEFINE(inst, amu_init, NULL, &amu_data_##inst, &amu_config_##inst,          \
			      BOOT_STAGE, BOOT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(AMU_INIT)
