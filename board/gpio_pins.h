#ifndef __gpio_pins_H
#define __gpio_pins_H

/* MODULE gpio_pins. */

/* Include inherited beans */
#include "fsl_gpio_driver.h"


extern gpio_input_pin_user_config_t gpioProgSwPin;

extern const gpio_input_pin_user_config_t gpioSdhcCdPin;

extern const gpio_output_pin_user_config_t gpioStatusLedPin;


/*! @brief Pin names */
enum _gpio_pins_pinNames{
  kGpioProgSw       = GPIO_MAKE_PIN(GPIOC_IDX, 2U),
  kGpioSdhcCd       = GPIO_MAKE_PIN(GPIOE_IDX, 6U),
  kGpioStatusLed    = GPIO_MAKE_PIN(GPIOC_IDX, 2U),
};
#endif

