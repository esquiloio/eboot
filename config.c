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
#include <stdio.h>
#include <string.h>

#include "fsl_i2c_master_driver.h"

#include "board.h"
#include "printd.h"
#include "pin_mux.h"
#include "config.h"

#define CONFIG_ID_END           0xff

#define CONFIG_MAGIC_VALUE      0xbabecafe
#define CONFIG_MAGIC_ADDR       0
#define CONFIG_MAGIC_SIZE       4

#define CONFIG_START_ADDR       (CONFIG_MAGIC_ADDR + CONFIG_MAGIC_SIZE)
#define CONFIG_END_ADDR         0x2000

#define I2C_READ_TIMEOUT        100
#define I2C_WRITE_TIMEOUT       100

#define MIN(a, b)               ((a) > (b) ? (b) : (a))

#define EEPROM_PAGE_SIZE        32

typedef struct tlv
{
    uint8_t     id;             ///< Unique variable identifier
    uint8_t     index;          ///< Variable instance index
    uint16_t    length;         ///< Total length of value
} tlv_t;

static i2c_master_state_t master;

static i2c_device_t eeprom =
{
    .address = BOARD_I2C_EEPROM_ADDR,
    .baudRate_kbps = 400
};

bool configRead(uint32_t address, void *buffer, uint32_t length)
{
    i2c_status_t            status;
    uint8_t                 data[2];

    data[0] = (address >> 8) & 0xff;
    data[1] = (address >> 0) & 0xff;

    status = I2C_DRV_MasterReceiveDataBlocking(
        BOARD_I2C_INSTANCE, &eeprom, data, sizeof(data),
        buffer, length, I2C_READ_TIMEOUT);

    if (status != kStatus_I2C_Success) {
        printd("i2c error %d\r\n", status);
        return false;
    }

    return true;
}

bool configWrite(uint32_t address, void *buffer, uint32_t length)
{
    i2c_status_t    status;
    uint8_t         data[2];
    uint32_t        size;

    // Align the first transfer to the page size
    size = MIN(length, EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE));

    while (length > 0) {
        data[0] = (address >> 8) & 0xff;
        data[1] = (address >> 0) & 0xff;

        status = I2C_DRV_MasterSendDataBlocking(
            BOARD_I2C_INSTANCE, &eeprom, data, sizeof(data),
            buffer, size, I2C_WRITE_TIMEOUT);

        if (status != kStatus_I2C_Success) {
            printd("i2c error %d\r\n", status);
            return false;
        }

        do {
            OSA_TimeDelay(1);

            status = I2C_DRV_MasterReceiveDataBlocking(
                BOARD_I2C_INSTANCE, &eeprom, NULL, 0,
                data, 1, I2C_READ_TIMEOUT);
        } while  (status == kStatus_I2C_ReceivedNak);

        buffer += size;
        length -= size;
        address += size;

        size = MIN(EEPROM_PAGE_SIZE, length);
    }

    return true;
}

static bool tlvRead(uint32_t addr, tlv_t *tlv)
{
    // Read the TLV
    if (!configRead(addr, tlv, sizeof(tlv_t)))
        return false;

    printd("id %d idx %d len %d\r\n", tlv->id, tlv->index, tlv->length);

    // End if it's an end TLV
    if (tlv->id == CONFIG_ID_END)
        return false;

    // End if the TLV exceeds the EEPROM
    if (addr + 2 * sizeof(tlv_t) + tlv->length >= CONFIG_END_ADDR)
        return false;

    return true;
}

uint32_t configFind(uint8_t id, uint8_t index, uint16_t *length)
{
    uint32_t    addr;
    tlv_t       tlv;

    addr = CONFIG_START_ADDR;
    
    while (true)
    {
        if (!tlvRead(addr, &tlv))
            return 0;

        // Stop if we have a match
        if (tlv.id == id && tlv.index == index) {
            *length = tlv.length;
            return addr + sizeof(tlv_t);
        }

        // Advance to next EEPROM address
        addr += tlv.length + sizeof(tlv_t);
    }
}

void configClear(void)
{
    uint8_t     buffer[CONFIG_END_ADDR];
    uint32_t    addr1;
    uint32_t    addr2;
    tlv_t       tlv;
    
    addr1 = CONFIG_START_ADDR;
    addr2 = CONFIG_START_ADDR;
    
    while (tlvRead(addr1, &tlv))
    {
        // Save certificates
        if (tlv.id >= CONFIG_ID_CERT_START && tlv.id <= CONFIG_ID_CERT_END) {
            // Write the TLV
            configWrite(addr2, &tlv, sizeof(tlv));

            addr1 += sizeof(tlv);
            addr2 += sizeof(tlv);

            // Copy the data
            configRead(addr1, &buffer, tlv.length);
            configWrite(addr2, &buffer, tlv.length);

            addr1 += tlv.length;
            addr2 += tlv.length;
        }
        else {
            // Skip over
            addr1 += sizeof(tlv) + tlv.length;
        }
    }

    tlv.id = CONFIG_ID_END;

    configWrite(addr2, &tlv, sizeof(tlv));
}

bool configInit(uint32_t instance)
{
    uint32_t magic = 0;

    I2C_DRV_MasterInit(instance, &master);

    if (!configRead(CONFIG_MAGIC_ADDR, &magic, sizeof(magic)))
        return false;

    if (magic != CONFIG_MAGIC_VALUE)
        return false;

    return true;
}

void configDeinit(uint32_t instance)
{
    I2C_DRV_MasterDeinit(instance);
}
