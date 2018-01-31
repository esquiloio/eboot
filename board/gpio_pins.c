#include "gpio_pins.h"
#include <stdbool.h>

gpio_input_pin_user_config_t gpioProgSwPin = {
    .pinName = kGpioProgSw,
    .config.isPullEnable = true,
    .config.pullSelect = kPortPullUp,
    .config.isPassiveFilterEnabled = false,
    .config.interrupt = kPortIntDisabled
};

const gpio_input_pin_user_config_t gpioSdhcCdPin = {
    .pinName = kGpioSdhcCd,
    .config.isPullEnable = true,
    .config.pullSelect = kPortPullUp,
    .config.isPassiveFilterEnabled = false,
    .config.interrupt = kPortIntDisabled
};

const gpio_output_pin_user_config_t gpioStatusLedPin = {
    .pinName = kGpioStatusLed,
    .config.outputLogic = 1,
    .config.slewRate = kPortSlowSlewRate,
    .config.isOpenDrainEnabled = false,
    .config.driveStrength = kPortHighDriveStrength,
};

