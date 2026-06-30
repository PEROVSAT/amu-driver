#pragma once

// These values can be put in command register 0x00 to direct the AMU
// They are 8 bits each

#define CMD_SWEEP_ENABLE      0x41
#define CMD_SWEEP_TRIG_SWEEP  0x42
#define CMD_SWEEP_CONFIG_SAVE 0x45
#define CMD_SET_ADDRESS       0x03
