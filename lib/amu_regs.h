#pragma once

// Register information from AMULIB

// --- Base System Registers ---
#define AMU_REG_CMD                      0x00
#define AMU_REG_SYSTEM_HARDWARE_REVISION 0x03

// --- Direct Struct Registers ---
// Note: These point to the START of the structs in memory.
// Writing to AMU_REG_SWEEP_CONFIG (0xB0) is fine for applying config,
// but reading bulk data requires the Data Pointers below.
#define AMU_REG_SWEEP_CONFIG 0xB0
#define AMU_REG_SWEEP_META   0xC0

// --- Bulk Data Pointers ---
// Use these registers for burst-reading entire arrays or structs over I2C
// Base offset: 0xF0
#define AMU_REG_DATA_PTR_VOLTAGE      0xF1 // float array[IV_POINTS]
#define AMU_REG_DATA_PTR_CURRENT      0xF2 // float array[IV_POINTS]
#define AMU_REG_DATA_PTR_SWEEP_CONFIG 0xF5 // amu_ivsweep_config_t (16 bytes)
#define AMU_REG_DATA_PTR_SWEEP_META   0xF6 // amu_ivsweep_meta_t (48 bytes)

// --- Communication / Utility ---
#define AMU_REG_TRANSFER_PTR 0xFE // Used for setting I2C address
