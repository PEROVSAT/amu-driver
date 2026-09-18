#pragma once

#include <stdint.h>

/* Register information from AMULIB */

#define AMU_REG_CMD                      0x00
#define AMU_REG_SYSTEM_HARDWARE_REVISION 0x03

#define AMU_REG_SWEEP_CONFIG 0xB0
#define AMU_REG_SWEEP_META   0xC0

#define AMU_REG_DATA_PTR_VOLTAGE      0xF1
#define AMU_REG_DATA_PTR_CURRENT      0xF2
#define AMU_REG_DATA_PTR_SWEEP_CONFIG 0xF5
#define AMU_REG_DATA_PTR_SWEEP_META   0xF6

#define AMU_REG_TRANSFER_PTR 0xFE

#define AMU_CMD_SWEEP_ENABLE      0x41
#define AMU_CMD_SWEEP_TRIG_SWEEP  0x42
#define AMU_CMD_SWEEP_CONFIG_SAVE 0x45
#define AMU_CMD_SET_ADDRESS       0x03
#define AMU_CMD_DAC_STATE         0x50
#define AMU_CMD_DAC_VOLTAGE       0x53

typedef struct __attribute__((packed)) {
	uint8_t type;
	uint8_t numPoints;
	uint8_t delay;
	uint8_t ratio;
	uint8_t power;
	uint8_t dac_gain;
	uint8_t sweep_averages;
	uint8_t adc_averages;
	float am0;
	float area;
} amu_ivsweep_config_t;

typedef struct __attribute__((packed)) {
	float voc;
	float isc;
	float tsensor_start;
	float tsensor_end;
	float ff;
	float eff;
	float vmax;
	float imax;
	float pmax;
	float adc;
	uint32_t timestamp;
	uint32_t crc;
} amu_ivsweep_meta_t;
