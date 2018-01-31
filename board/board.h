#if !defined(__BOARD_H__)
#define __BOARD_H__

#include <stdint.h>
#include "pin_mux.h"
#include "gpio_pins.h"

/* The board name */
#define BOARD_NAME                      "ESQUILO AIR"

#define CORE_CLOCK_FREQ 120000000U

/* OSC0 configuration. */
#define OSC0_XTAL_FREQ 50000000U
#define OSC0_SC2P_ENABLE_CONFIG  false
#define OSC0_SC4P_ENABLE_CONFIG  false
#define OSC0_SC8P_ENABLE_CONFIG  false
#define OSC0_SC16P_ENABLE_CONFIG false
#define MCG_HGO0   kOscGainLow
#define MCG_RANGE0 kOscRangeVeryHigh
#define MCG_EREFS0 kOscSrcExt

/* EXTAL0 PTA18 */
#define EXTAL0_PORT   PORTA
#define EXTAL0_PIN    18
#define EXTAL0_PINMUX kPortPinDisabled

/* XTAL0 PTA19 */
#define XTAL0_PORT   PORTA
#define XTAL0_PIN    19
#define XTAL0_PINMUX kPortPinDisabled

/* RTC external clock configuration. */
#define RTC_XTAL_FREQ   32768U
#define RTC_SC2P_ENABLE_CONFIG       false
#define RTC_SC4P_ENABLE_CONFIG       false
#define RTC_SC8P_ENABLE_CONFIG       false
#define RTC_SC16P_ENABLE_CONFIG      false
#define RTC_OSC_ENABLE_CONFIG        true

#define BOARD_RTC_CLK_FREQUENCY     32768U;

/* The UART to use for debug messages. */
#ifndef BOARD_DEBUG_UART_INSTANCE
    #define BOARD_DEBUG_UART_PIN_MUX    kPinMuxUart4
    #define BOARD_DEBUG_UART_INSTANCE   4
    #define BOARD_DEBUG_UART_BASEADDR   UART4
#endif
#ifndef BOARD_DEBUG_UART_BAUD
    #define BOARD_DEBUG_UART_BAUD       115200
#endif

/* Define feature for the low_power_demo */
#define FSL_FEATURE_HAS_VLLS2 (1)

#define BOARD_I2C_GPIO_SCL              GPIO_MAKE_PIN(GPIOA_IDX, 12)
#define BOARD_I2C_GPIO_SDA              GPIO_MAKE_PIN(GPIOA_IDX, 13)

/* The i2c instance used for i2c connection by default */
#define BOARD_I2C_INSTANCE              2

/* EEPROM I2C address */
#define BOARD_I2C_EEPROM_ADDR           0x50

/* The SDHC instance/channel used for board */
#define BOARD_SDHC_INSTANCE             0

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void hardware_init(void);
void dbg_uart_init(void);
/*This function to used for power manager demo*/
void disable_unused_pins(void);
void enable_unused_pins(void);

/* Function to initialize clock base on board configuration. */
void BOARD_ClockInit(void);

/* Function to initialize OSC0 base on board configuration. */
void BOARD_InitOsc0(void);

/* Function to initialize RTC external clock base on board configuration. */
void BOARD_InitRtcOsc(void);

/*Function to handle board-specified initialization*/
uint8_t usb_device_board_init(uint8_t controller_id);
/*Function to handle board-specified initialization*/
uint8_t usb_host_board_init(uint8_t controller_id);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __BOARD_H__ */
