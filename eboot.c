/*
 * Esquilo Bootloader
 * 
 * Copyright 2014-2018 Esquilo Corporation - https://esquilo.io/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <string.h>

#include "fsl_debug_console.h"

#include "ff.h"
#include "diskio.h"

#include "board.h"
#include "gpio_pins.h"
#include "printd.h"
#include "usb_msc.h"
#include "image.h"
#include "config.h"
#include "image_key.h"

#define IMAGE_TYPE_ESQUILO_AIR  1

#define USB_IDLE_TIME           1000
#define LED_READY_PERIOD        1000
#define LED_ERROR_DELAY         1000
#define LED_ERROR_PERIOD        250
#define LED_TOGGLE_RATE         16

#define CONFIG_CLEAR_DELAY      10000

extern uint32_t OSA_TimeDiff(uint32_t time_start, uint32_t time_end);

typedef enum {
    USB_STATE_READY,
    USB_STATE_PROGRAM,
    USB_STATE_BOOT,
    USB_STATE_ERROR
} UsbState;

const char eboot_ident[] __attribute__((section(".ident"))) =
    "eboot " EBOOT_VERSION;

const uint8_t flash_config[] __attribute__((section(".FlashConfig"))) = {
    // Backdoor comparison key
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    // FPROT0-3
    0xff, 0xff, 0xff, 0xff,

    // FSEC
#if RELEASE 
    0xfb, // Secure
#else
    0xfe, // Insecure
#endif

    // FOPT
    0xf9, // Disable NMI and EzPort

    // Reserved
    0xff, 0xff,

};

static const uint8_t imageMagic[IMAGE_MAGIC_SIZE] = {
    0xdd, 0xaa, 0x1c, 0x11, 0xa1, 0x05, 0x71, 0x27,
    0x06, 0xa8, 0x50, 0xe8, 0x96, 0x03, 0x83, 0x10
};

static uint32_t             usb_next_lba;
static volatile UsbState    usb_state;
static volatile uint32_t    usb_last_access;
static image_error_t        usb_error;

static char                 sd_update_file[17]; // sd:/ + FAT 8.3 + nil

static void reportError(image_error_t error, bool forever)
{
    uint32_t count = 5;

    printd("Error %d\r\n", error);

    while (forever || count--)
    {
        GPIO_DRV_WritePinOutput(kGpioStatusLed, 1);
        OSA_TimeDelay(LED_ERROR_DELAY);
        for (uint32_t i = 0; i < error; i++)
        {
            GPIO_DRV_WritePinOutput(kGpioStatusLed, 0);
            OSA_TimeDelay(LED_ERROR_PERIOD);
            GPIO_DRV_WritePinOutput(kGpioStatusLed, 1);
            OSA_TimeDelay(LED_ERROR_PERIOD);
        }
    }
}

static void usbError(void)
{
    // Close USB
    usbMscDeinit();

    // Clear the image unless the flash wasn't touched
    if (usb_error != FLASH_INIT && usb_error != BAD_IMAGE)
        imageClear();

    // Report the error forever
    reportError(usb_error, true);

    // We should never come back
}

static void usbDataCallback(uint32_t lba, void *data, uint32_t size)
{
    printd("USB DATA %ld %p %ld\r\n", lba, data, size);

    if (usb_state == USB_STATE_READY)
    {
        if (memcmp(data, imageMagic, sizeof(imageMagic)) == 0)
        {
            printd("USB_STATE_PROGRAM\r\n");
            usb_state = USB_STATE_PROGRAM;
            usb_next_lba = lba;
        }
    }

    if (usb_state == USB_STATE_PROGRAM && lba == usb_next_lba)
    {
        usb_next_lba++;
        usb_error = imageWrite(data, size);
        if (usb_error != NO_ERROR)
            usb_state = USB_STATE_ERROR;

        if (lba % LED_TOGGLE_RATE == 0)
            GPIO_DRV_TogglePinOutput(kGpioStatusLed);
    }
}

static void usbReadCallback(uint32_t lba, uint32_t size)
{
    printd("USB READ %ld %ld\r\n", lba, size);
    usb_last_access = OSA_TimeGetMsec();
}

static void usbWriteCallback(uint32_t lba, uint32_t size)
{
    printd("USB WRITE %ld %ld\r\n", lba, size);
    usb_last_access = OSA_TimeGetMsec();
}

static void usbUpdate(void)
{
    uint32_t        readyTime;
    uint32_t        accessTime;

    usb_error = imageInit(imageMagic, IMAGE_TYPE_ESQUILO_AIR, imageKey);
    if (usb_error != NO_ERROR)
        usbError();

    readyTime = OSA_TimeGetMsec();

    usbMscInit(usbReadCallback, usbWriteCallback, usbDataCallback);

    pin_mux_configure(kPinMuxStatusLed);
    GPIO_DRV_OutputPinInit(&gpioStatusLedPin);

    while (1)
    {
        accessTime = OSA_TimeDiff(usb_last_access, OSA_TimeGetMsec());

        switch (usb_state)
        {
        case USB_STATE_READY:
            if (OSA_TimeDiff(readyTime, OSA_TimeGetMsec()) >= LED_READY_PERIOD)
            {
                readyTime = OSA_TimeGetMsec();
                GPIO_DRV_TogglePinOutput(kGpioStatusLed);
            }
            break;
        case USB_STATE_PROGRAM:
            if (accessTime >= USB_IDLE_TIME)
            {
                printd("short image\r\n");
                usb_error = IMAGE_SIZE;
                usb_state = USB_STATE_ERROR;
            }
            if (imageRemain() == 0)
            {
                usb_state = USB_STATE_BOOT;
            }
            break;
        case USB_STATE_BOOT:
            if (accessTime >= USB_IDLE_TIME)
            {
                printd("resetting...\n");
	            usbMscDeinit();
                NVIC_SystemReset();
            }
            break;
        case USB_STATE_ERROR:
            if (accessTime >= USB_IDLE_TIME)
            {
                usbError();
            }
            break;
        }
    }
}

static void sdError(image_error_t error)
{
    // Clear the image unless the flash wasn't touched
    if (error != FLASH_INIT && error != BAD_IMAGE)
        imageClear();

    // Report the error
    reportError(error, false);

    // Reset after the report
    NVIC_SystemReset();
}

static bool sdReady(void)
{
    uint16_t    addr;
    uint16_t    len;

    pin_mux_configure(kPinMuxSdhcCd);
    GPIO_DRV_InputPinInit(&gpioSdhcCdPin);

    // We can't do an SD update without an SD card
    if(GPIO_DRV_ReadPinInput(kGpioSdhcCd)) {
        printd("no SD card\r\n");
        return false;
    }

    // Set filename to empty string
    sd_update_file[0] = '\0';

    // Configure the pin mux for the config I2C
    pin_mux_configure(kPinMuxI2c2);

    // Initialize the config
    if (!configInit(BOARD_I2C_INSTANCE)) {
        printd("unable to intialize config\r\n");
        return false;
    }

    // Look for the update SD filename in the config
    addr = configFind(CONFIG_ID_SYS_UPDATE, 0, &len);
    if (addr != 0 && len < sizeof(sd_update_file)) {
        if (configRead(addr, sd_update_file, len)) {
            // Make sure the string is terminated
            sd_update_file[len + 1] = '\0';

            // Clear the update filename in the config
            configWrite(addr, "\0", 1);
        }
    }

    // Reset the config
    configDeinit(BOARD_I2C_INSTANCE);

    // Check if filename is valid
    if (memcmp(sd_update_file, "sd:", 3) != 0) {
        printd("no update in config\r\n");
        return false;
    }

    return true;
}

static void sdUpdate(void)
{
    FRESULT         result;
    FATFS           fs;
    FIL             fp;
    uint8_t         buffer[512];
    uint32_t        read;
    image_error_t   error;
    uint32_t        block;

    // Configure the mux
    pin_mux_configure(kPinMuxSdhc);

    // Initialize the image
    error = imageInit(imageMagic, IMAGE_TYPE_ESQUILO_AIR, imageKey);
    if (error != NO_ERROR) {
        sdError(error);
        return;
    }

    // Mount the SD card
    result = f_mount(SD, &fs);
    if (result != FR_OK) {
        printd("volume mount error 0x%x\r\n", result);
        sdError(MEDIA_FAILURE);
        return;
    }

    // Open the update file
    result = f_open(&fp, &sd_update_file[3], FA_READ | FA_OPEN_EXISTING);
    if (result != FR_OK) {
        printd("file open error 0x%x\r\n", result);
        sdError(MEDIA_FAILURE);
        return;
    }

    // Initialize the status LED
    pin_mux_configure(kPinMuxStatusLed);
    GPIO_DRV_OutputPinInit(&gpioStatusLedPin);

    // Process the update file
    block = 0;
    do {
        if (block++ % LED_TOGGLE_RATE == 0)
            GPIO_DRV_TogglePinOutput(kGpioStatusLed);

        result = f_read(&fp, buffer, sizeof(buffer), &read);
        if (result != FR_OK) {
            printd("file read error 0x%x\r\n", result);
            sdError(MEDIA_FAILURE);
            return;
        }

        error = imageWrite(buffer, read);
        if (error != NO_ERROR) {
            sdError(error);
            return;
        }
    } while (f_eof(&fp) == 0);

    // Check if we got it all
    if (imageRemain() != 0) {
        sdError(IMAGE_SIZE);
        return;
    }

    printd("resetting...\r\n");
    NVIC_SystemReset();
}

void abort(void)
{
#if DEBUG
    printd("program abort\r\n");
    asm volatile ("bkpt");
#endif
    pin_mux_configure(kPinMuxStatusLed);
    GPIO_DRV_OutputPinInit(&gpioStatusLedPin);
    reportError(ABORT, true);
    while (1);
}

int main(void)
{
    hardware_init();
    OSA_Init();

    pin_mux_configure(kPinMuxJtag);

#ifdef DEBUG
    dbg_uart_init();
#endif

    printd("Esquilo Boot " EBOOT_VERSION "\r\n");

    pin_mux_configure(kPinMuxProgSw);
    GPIO_DRV_InputPinInit(&gpioProgSwPin);

    // Is the PROG switch held down?
    if(!GPIO_DRV_ReadPinInput(kGpioProgSw)) {
        // Measure how long PROG is held
        while (!GPIO_DRV_ReadPinInput(kGpioProgSw)) {
            OSA_TimeDelay(100);
            if (OSA_TimeGetMsec() > CONFIG_CLEAR_DELAY)
                break;
        }

        // Do a config clear if PROG is held down a long time
        if (OSA_TimeGetMsec() > CONFIG_CLEAR_DELAY) {
            // Turn off the status LED
            GPIO_DRV_OutputPinInit(&gpioStatusLedPin);
            GPIO_DRV_WritePinOutput(kGpioStatusLed, 1);
            pin_mux_configure(kPinMuxStatusLed);

            printd("clearing config\r\n");
            pin_mux_configure(kPinMuxI2c2);
            if (configInit(BOARD_I2C_INSTANCE))
                configClear();

            imageBoot();
        }

        usbUpdate();
    }

    // Do an SD update if there is one ready
    if (sdReady())
        sdUpdate();

    // Try to boot the image
    imageBoot();

    // Force a USB update if we didn't boot
    usbUpdate();

    return 0;
}

