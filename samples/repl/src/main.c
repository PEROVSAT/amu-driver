#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/shell/shell.h>

#include <amu.h>

static amu_t *sh_amu(const struct shell *sh)
{
	const struct device *zdev = DEVICE_DT_GET(DT_ALIAS(amu));

	if (!device_is_ready(zdev)) {
		shell_error(sh, "device not ready");
		return NULL;
	}

	return amu_from_dev(zdev);
}

static int sh_ret(const struct shell *sh, int ret)
{
	if (ret < 0) {
		shell_error(sh, "error %d", ret);
		return ret;
	}

	return 0;
}

static void print_config(const struct shell *sh, const amu_config_t *cfg)
{
	shell_print(sh, "type: %u", cfg->type);
	shell_print(sh, "numPoints: %u", cfg->numPoints);
	shell_print(sh, "delay: %u", cfg->delay);
	shell_print(sh, "ratio: %u", cfg->ratio);
	shell_print(sh, "power: %u", cfg->power);
	shell_print(sh, "dac_gain: %u", cfg->dac_gain);
	shell_print(sh, "sweep_averages: %u", cfg->sweep_averages);
	shell_print(sh, "adc_averages: %u", cfg->adc_averages);
	shell_print(sh, "am0: %f", (double)cfg->am0);
	shell_print(sh, "area: %f", (double)cfg->area);
}

static int parse_u8(const char *s, uint8_t *out)
{
	char *end;
	unsigned long v = strtoul(s, &end, 0);

	if (s[0] == '\0' || end == s || *end != '\0' || v > 0xFFul) {
		return -EINVAL;
	}

	*out = (uint8_t)v;
	return 0;
}

static int parse_float(const char *s, float *out)
{
	char *end;
	float v = strtof(s, &end);

	if (s[0] == '\0' || end == s || *end != '\0') {
		return -EINVAL;
	}

	*out = v;
	return 0;
}

static int cmd_get_config(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	amu_config_t cfg;
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = amu_get_config(dev, &cfg);
	if (sh_ret(sh, ret) < 0) {
		return ret;
	}

	print_config(sh, &cfg);
	return 0;
}

static int cmd_set_config(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	amu_config_t cfg;
	const char *field = argv[1];
	int ret;

	ARG_UNUSED(argc);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = amu_get_config(dev, &cfg);
	if (ret < 0) {
		return sh_ret(sh, ret);
	}

	if (strcmp(field, "type") == 0) {
		ret = parse_u8(argv[2], &cfg.type);
	} else if (strcmp(field, "numPoints") == 0) {
		ret = parse_u8(argv[2], &cfg.numPoints);
	} else if (strcmp(field, "delay") == 0) {
		ret = parse_u8(argv[2], &cfg.delay);
	} else if (strcmp(field, "ratio") == 0) {
		ret = parse_u8(argv[2], &cfg.ratio);
	} else if (strcmp(field, "power") == 0) {
		ret = parse_u8(argv[2], &cfg.power);
	} else if (strcmp(field, "dac_gain") == 0) {
		ret = parse_u8(argv[2], &cfg.dac_gain);
	} else if (strcmp(field, "sweep_averages") == 0) {
		ret = parse_u8(argv[2], &cfg.sweep_averages);
	} else if (strcmp(field, "adc_averages") == 0) {
		ret = parse_u8(argv[2], &cfg.adc_averages);
	} else if (strcmp(field, "am0") == 0) {
		float am0;

		ret = parse_float(argv[2], &am0);
		if (ret == 0) {
			cfg.am0 = am0;
		}
	} else if (strcmp(field, "area") == 0) {
		float area;

		ret = parse_float(argv[2], &area);
		if (ret == 0) {
			cfg.area = area;
		}
	} else {
		shell_error(sh, "unknown field '%s'", field);
		return -EINVAL;
	}

	if (ret < 0) {
		shell_error(sh, "invalid value");
		return ret;
	}

	ret = amu_set_config(dev, &cfg);
	if (sh_ret(sh, ret) < 0) {
		return ret;
	}

	print_config(sh, &cfg);
	return 0;
}

static int cmd_save_config(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	return sh_ret(sh, amu_save_config(dev));
}

static int cmd_trigger_sweep(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	return sh_ret(sh, amu_trigger_sweep(dev));
}

static int cmd_get_sweep_meta(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	amu_sweep_meta_t meta;
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = amu_get_sweep_meta(dev, &meta);
	if (sh_ret(sh, ret) < 0) {
		return ret;
	}

	shell_print(sh, "voc: %f", (double)meta.voc);
	shell_print(sh, "isc: %f", (double)meta.isc);
	shell_print(sh, "tsensor_start: %f", (double)meta.tsensor_start);
	shell_print(sh, "tsensor_end: %f", (double)meta.tsensor_end);
	shell_print(sh, "ff: %f", (double)meta.ff);
	shell_print(sh, "eff: %f", (double)meta.eff);
	shell_print(sh, "vmax: %f", (double)meta.vmax);
	shell_print(sh, "imax: %f", (double)meta.imax);
	shell_print(sh, "pmax: %f", (double)meta.pmax);
	shell_print(sh, "adc: %f", (double)meta.adc);
	shell_print(sh, "timestamp: %u", meta.timestamp);
	shell_print(sh, "crc: 0x%08x", meta.crc);
	return 0;
}

static int cmd_get_sweep_iv(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	amu_sweep_iv_t iv;
	int ret;
	int i;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = amu_get_sweep_iv(dev, &iv);
	if (sh_ret(sh, ret) < 0) {
		return ret;
	}

	shell_print(sh, "i voltage current");
	for (i = 0; i < IV_POINTS; i++) {
		shell_print(sh, "%d %f %f", i, (double)iv.voltage[i], (double)iv.current[i]);
	}

	return 0;
}

static int cmd_dac_enable(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	float voltage;
	int ret;

	ARG_UNUSED(argc);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = parse_float(argv[1], &voltage);
	if (ret < 0) {
		shell_error(sh, "invalid voltage");
		return ret;
	}

	return sh_ret(sh, amu_dac_enable(dev, voltage));
}

static int cmd_dac_disable(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (dev == NULL) {
		return -ENODEV;
	}

	return sh_ret(sh, amu_dac_disable(dev));
}

static int cmd_set_address(const struct shell *sh, size_t argc, char **argv)
{
	amu_t *dev = sh_amu(sh);
	uint8_t addr;
	int ret;

	ARG_UNUSED(argc);

	if (dev == NULL) {
		return -ENODEV;
	}

	ret = parse_u8(argv[1], &addr);
	if (ret < 0) {
		shell_error(sh, "invalid address");
		return ret;
	}

	return sh_ret(sh, amu_set_address(dev, addr));
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	amu_cmds, SHELL_CMD_ARG(get_config, NULL, "Read sweep config", cmd_get_config, 1, 0),
	SHELL_CMD_ARG(set_config, NULL,
		      "Set one config field: <field> <value>\n"
		      "fields: type numPoints delay ratio power dac_gain "
		      "sweep_averages adc_averages am0 area",
		      cmd_set_config, 3, 0),
	SHELL_CMD_ARG(save_config, NULL, "Save config on the device", cmd_save_config, 1, 0),
	SHELL_CMD_ARG(trigger_sweep, NULL, "Trigger an IV sweep", cmd_trigger_sweep, 1, 0),
	SHELL_CMD_ARG(get_sweep_meta, NULL, "Read sweep metadata", cmd_get_sweep_meta, 1, 0),
	SHELL_CMD_ARG(get_sweep_iv, NULL, "Read sweep IV points", cmd_get_sweep_iv, 1, 0),
	SHELL_CMD_ARG(dac_enable, NULL, "Set DAC voltage and enable: <voltage>", cmd_dac_enable, 2,
		      0),
	SHELL_CMD_ARG(dac_disable, NULL, "Disable DAC", cmd_dac_disable, 1, 0),
	SHELL_CMD_ARG(set_address, NULL, "Set AMU bus address: <addr>", cmd_set_address, 2, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(amu, &amu_cmds, "AMU driver", NULL);

int main(void)
{
	return 0;
}
