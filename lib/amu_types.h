#pragma once

// These are wire data formats from the AMU firmware, not types we create
// Taken from AMULIB

#include <stdint.h>

// The packed attribute ensures that the compiler doesn't try and compress the data format

// IV Sweep Configuration Mapping
// numPoints is managed internally by the library (always set to IV_POINTS).
// Use amu_sweep_cfg_t from amu_lib.h to configure sweeps.
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

// IV Sweep Metadata Mapping
typedef struct __attribute__((packed)) {
	float voc;
	float isc;
	float tsensor_start;
	float tsensor_end;
	float ff;  // Fill factor
	float eff; // Efficiency
	float vmax;
	float imax;
	float pmax;
	float adc;
	uint32_t timestamp;
	uint32_t crc;
} amu_ivsweep_meta_t;
