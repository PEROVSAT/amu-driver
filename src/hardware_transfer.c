#include "transfer.h"

#include "amu.h"

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(amu);

int amu_transfer(const void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	const struct device *dev = ctx;
	const struct amu_config *config = dev->config;
	int ret;

	if (read) {
		ret = i2c_burst_read_dt(&config->bus, reg, buf, len);
	} else {
		uint8_t tx_buf[len + 1];

		tx_buf[0] = reg;
		memcpy(&tx_buf[1], buf, len);

		ret = i2c_write_dt(&config->bus, tx_buf, len + 1);
	}

	if (ret < 0) {
		LOG_ERR("AMU %s: I2C %s failed (reg=0x%02x len=%zu ret=%d)", dev->name,
			read ? "read" : "write", reg, len, ret);
	}

	return ret;
}

int amu_transfer_init(const struct device *dev)
{
	const struct amu_config *config = dev->config;

	LOG_DBG("AMU %s: checking I2C bus %s addr 0x%02x", dev->name, config->bus.bus->name,
		config->bus.addr);

	if (!i2c_is_ready_dt(&config->bus)) {
		LOG_ERR("AMU %s: I2C bus not ready (bus=%s addr=0x%02x)", dev->name,
			config->bus.bus->name, config->bus.addr);
		return -ENODEV;
	}

	LOG_DBG("AMU %s: I2C bus ready", dev->name);

	return 0;
}
