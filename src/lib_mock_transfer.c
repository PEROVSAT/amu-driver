#include "transfer.h"

#include "amu.h"

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>

#define AMU_REGISTER_MAP_SIZE 256

static uint8_t register_map[AMU_REGISTER_MAP_SIZE];

int amu_transfer(void *ctx, uint8_t reg, uint8_t *buf, size_t len, bool read)
{
	ARG_UNUSED(ctx);

	if (reg + len > sizeof(register_map)) {
		return -EINVAL;
	}

	if (read) {
		memcpy(buf, &register_map[reg], len);
	} else {
		memcpy(&register_map[reg], buf, len);
	}

	return 0;
}

int amu_transfer_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	memset(register_map, 0, sizeof(register_map));

	return 0;
}
