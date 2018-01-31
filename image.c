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

#include "fsl_interrupt_manager.h"
#include "fsl_mmcau_driver.h"
#include "fsl_crc_driver.h"
#include "fsl_clock_manager.h"
#include "SSD_FTFx.h"

#include "board.h"
#include "pin_mux.h"
#include "printd.h"
#include "image.h"

//#define IMAGE_VERIFY        1

#if DEBUG
#define IMAGE_START_ADDR    0x18000
#else
#define IMAGE_START_ADDR    0x8000
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define AES_BLOCK_SIZE      16
#define AES_KEY_SIZE        32
#define AES_KEY_SCH_SIZE    240
#define AES_NUM_ROUNDS      14

#define READ_MARGIN         1

#define P_FLASH_BASE        0x00000000
#define P_FLASH_SIZE        (FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE * FSL_FEATURE_FLASH_PFLASH_BLOCK_COUNT)
#define FLEXNVM_BASE        FSL_FEATURE_FLASH_FLEX_NVM_START_ADDRESS
#define EERAM_BASE          FSL_FEATURE_FLASH_FLEX_RAM_START_ADDRESS

#define DEBUGENABLE         0x00

#define FTFx_REG_BASE       FTFE_BASE
#define FTFx_PBLOCK_SIZE    FSL_FEATURE_FLASH_PFLASH_BLOCK_DATA_PATH_WIDTH
#define FTFx_PSECTION_SIZE  (FSL_FEATURE_FLASH_FLEX_RAM_SIZE / 4)
#define FTFx_PA_RAM_ADDR    FSL_FEATURE_FLASH_FLEX_RAM_START_ADDRESS

//////////////////////////////////////////////////////////////////////////////
// Local Types
//////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint32_t size;
    uint32_t crc;
    uint32_t start;
    uint32_t type;
} image_info_t;

typedef struct {
    uint32_t        remain;
    uint32_t        address;
    image_info_t    info;
    uint32_t        type;
    uint32_t        blockNum;
    uint8_t         magic[AES_BLOCK_SIZE];
    uint8_t         chain[AES_BLOCK_SIZE];
    uint8_t         key[AES_KEY_SIZE];
    uint8_t         keySch[AES_KEY_SCH_SIZE];
} image_t;

/*****************************************************************************
 * Local Variables 
 *****************************************************************************/

static pFLASHCOMMANDSEQUENCE flashCommand = (pFLASHCOMMANDSEQUENCE)0xFFFFFFFF;

static uint16_t ramFunc[100];

static FLASH_SSD_CONFIG flashConfig =
{
    FTFx_REG_BASE,          /*! FTFx control register base */
    P_FLASH_BASE,           /*! Base address of PFlash block */
    P_FLASH_SIZE,           /*! Size of PFlash block */
    FLEXNVM_BASE,           /*! Base address of DFlash block */
    0,                      /*! Size of DFlash block */
    EERAM_BASE,             /*! Base address of EERAM block */
    0,                      /*! Size of EEE block */
    DEBUGENABLE,            /*! Background debug mode enable bit */
    NULL_CALLBACK           /*! Pointer to callback function */
};

static image_t image;

//////////////////////////////////////////////////////////////////////////////
// Local Functions
//////////////////////////////////////////////////////////////////////////////
static void crcInit(void)
{
    crc_user_config_t config;

    config.crcWidth = kCrc32Bits;
    config.seed = 0xffffffff;
    config.polynomial = 0x04C11DB7U;
    config.writeTranspose = kCrcTransposeBoth;
    config.readTranspose = kCrcTransposeBoth;
    config.complementRead = true;

    CRC_DRV_Init(0, &config);
}

static void crcCheck(uint8_t *data, uint32_t num)
{
    while (num-- > 0) {
        CRC_HAL_SetDataLLReg(g_crcBase[0], *data++);
    }
}

static uint32_t crcResult(void)
{
    return CRC_HAL_GetCrcResult(g_crcBase[0]);
}

static void aesCbcInit(const void *key, const void *iv, void *keySch, void *chain)
{
    //MMCAU_AES_SetKey(key, AES_KEY_SIZE, keySch);
    mmcau_aes_set_key(key, 8 * AES_KEY_SIZE, keySch);
    
    memcpy(chain, iv, AES_BLOCK_SIZE);
}

static void aesCbcDecrypt(void *keySch, const void *ciphertext, void *plaintext, void *chain)
{
    uint8_t ciphersave[AES_BLOCK_SIZE];

    // Save the ciphertext for the next chain block
    memcpy(ciphersave, ciphertext, sizeof(ciphersave));

    // Process through the AES block cipher
    //MMCAU_AES_DecryptEcb(ciphertext, keySch, AES_NUM_ROUNDS, plaintext);
    mmcau_aes_decrypt(ciphertext, keySch, AES_NUM_ROUNDS, plaintext);

    // XOR the plaintext with the chain block
    for (uint32_t i = 0; i < AES_BLOCK_SIZE; i++)
        ((uint8_t*)plaintext)[i] ^= ((uint8_t*)chain)[i];

    // Save the ciphertext as the next chain block
    memcpy(chain, ciphersave, AES_BLOCK_SIZE);
}

uint32_t imageRemain(void)
{
    return image.remain;
}

void imageBoot(void)
{
    printd("boot image @ 0x%x\r\n", IMAGE_START_ADDR);

    if(*(uint32_t *)IMAGE_START_ADDR == 0xffffffff) {
        printd("image is erased\r\n");
        return;
    }

    asm volatile
    (
        "bl %0" : : "I" (IMAGE_START_ADDR)
    );
}

void imageClear(void)
{
    INT_SYS_DisableIRQGlobal();
    FlashEraseSector(&flashConfig, IMAGE_START_ADDR, FTFx_PSECTOR_SIZE, flashCommand);
    INT_SYS_EnableIRQGlobal();
}

static image_error_t eraseFlash(void)
{
    uint32_t result;

    printd("erase sector 0x%lx\r\n", image.address);

    INT_SYS_DisableIRQGlobal();
    result = FlashEraseSector(&flashConfig, image.address, FTFx_PSECTOR_SIZE, flashCommand);
    INT_SYS_EnableIRQGlobal();
    if (result != FTFx_OK)
    {
        printd("erase sector error %ld\r\n", result);
        return FLASH_ERASE;
    }

#if IMAGE_VERIFY
    printd("verify erase sector\r\n");

    INT_SYS_DisableIRQGlobal();
    result = FlashVerifySection(&flashConfig, image.address,
                                FTFx_PSECTOR_SIZE / FTFx_PBLOCK_SIZE,
                                READ_MARGIN, flashCommand);
    INT_SYS_EnableIRQGlobal();
    if (result != FTFx_OK)
    {
        printd("verify erase error %ld\r\n", result);
        return FLASH_ERASE;
    }
#endif

    return NO_ERROR;
}

static image_error_t programFlash(void)
{
    uint32_t result;
    uint32_t address;
    uint32_t blocks;
    
    // Check if this is a full section or partial section
    if (image.address % FTFx_PSECTION_SIZE == 0)
    {
        address = image.address - FTFx_PSECTION_SIZE;
        blocks = FTFx_PSECTION_SIZE / FTFx_PBLOCK_SIZE;
    }
    else
    {
        address = image.address & ~(FTFx_PSECTION_SIZE - 1);
        blocks = (image.address % FTFx_PSECTION_SIZE + FTFx_PBLOCK_SIZE - 1) / FTFx_PBLOCK_SIZE;
    }

    printd("program section %ld blocks @ 0x%lx\r\n", blocks, image.address);

    INT_SYS_DisableIRQGlobal();
    result = FlashProgramSection(&flashConfig, address,
                                 blocks, flashCommand);
    INT_SYS_EnableIRQGlobal();
    if (result != FTFx_OK)
    {
        printd("program section error %ld\r\n", result);
        return FLASH_PROGRAM;
    }

#if IMAGE_VERIFY
    uint32_t failAddr;

    printd("verify program\r\n");

    INT_SYS_DisableIRQGlobal();
    result = FlashProgramCheck(&flashConfig, address,
                               blocks * FTFx_PBLOCK_SIZE,
                               (uint8_t*)FTFx_PA_RAM_ADDR,
                               &failAddr, READ_MARGIN, flashCommand);
    INT_SYS_EnableIRQGlobal();
    if (result != FTFx_OK)
    {
        printd("verify program error %ld 0x%lx\r\n", result, failAddr);
        return FLASH_PROGRAM;
    }
#else
    if (memcmp((void*)FTFx_PA_RAM_ADDR, (void*)address, blocks * FTFx_PBLOCK_SIZE) != 0)
    {
        printd("verify program error\r\n");
        return FLASH_PROGRAM;
    }

#endif

    return NO_ERROR;
}

image_error_t imageWrite(void *data, uint32_t size)
{
    uint32_t        crc;
    image_error_t   error;

    printd("image write %ld\r\n", size);

    // Data must come in multiples of the AES block size (16 bytes)
    if (size % AES_BLOCK_SIZE != 0) {
        printd("block size 0x%lx\r\n", size);
        return BLOCK_SIZE;
    }

    while (size > 0)
    {
        if (image.blockNum == 0)
        {
            // Check the image magic value
            if (memcmp(data, image.magic, sizeof(image.magic)) != 0) {
                printd("bad magic value");
                return BAD_IMAGE;
            }
        }
        else if (image.blockNum == 1)
        {
            // Next block contains the AES IV
            aesCbcInit(image.key, data, image.keySch, image.chain);

            // Initialize the CRC check
            crcInit();
        }
        else if (image.blockNum == 2)
        {
            // AES decrypt the image info block
            aesCbcDecrypt(image.keySch, data, &image.info, image.chain);
#if !DEBUG
            // Check the start address
            if (image.info.start != IMAGE_START_ADDR) {
                printd("bad start address\r\n");
                return BAD_IMAGE;
            }
#endif
            // Check the size
            if (image.info.size > P_FLASH_SIZE - IMAGE_START_ADDR) {
                printd("bad image size\r\n");
                return BAD_IMAGE;
            }

            // Make sure the image is correct
            if (image.info.type != image.type) {
                printd("bad image type\r\n");
                return BAD_IMAGE;
            }

            image.address = IMAGE_START_ADDR;
            image.remain = image.info.size;
        }
        else if (image.remain > 0)
        {
            // AES decrypt the image data
            aesCbcDecrypt(image.keySch, data, data, image.chain);

            // CRC check the block
            crcCheck(data, MIN(image.remain, AES_BLOCK_SIZE));

            // Copy the image data to the PA RAM
            memcpy((void*)(FTFx_PA_RAM_ADDR + image.address % FTFx_PSECTION_SIZE), data, AES_BLOCK_SIZE);

            // Erase the flash if at the sector start
            if (image.address % FTFx_PSECTOR_SIZE == 0)
            {
                error = eraseFlash();
                if (error != NO_ERROR)
                    return error;
            }

            // Advance to the next block
            image.address += AES_BLOCK_SIZE;
            image.remain -= MIN(image.remain, AES_BLOCK_SIZE);

            // If the section is full, program it in the flash via the PA RAM
            if (image.address % FTFx_PSECTION_SIZE == 0)
            {
                error = programFlash();
                if (error != NO_ERROR)
                    return error;
            }

            // Check if it at the end of the image
            if (image.remain == 0)
            {
                // Program any remaining data in the PA RAM
                if (image.address % FTFx_PSECTION_SIZE != 0)
                {
                    error = programFlash();
                    if (error != NO_ERROR)
                        return error;
                }

                // Check CRC at the end
                crc = crcResult();
                if (crc != image.info.crc) {
                    printd("CRC fail 0x%lx != 0x%lx\r\n", crc, image.info.crc);
                    return CRC_CHECK;
                }
            }
        }

        data += AES_BLOCK_SIZE;
        size -= AES_BLOCK_SIZE;
        image.blockNum++;
    }

    return NO_ERROR;
}

image_error_t imageInit(const uint8_t *magic, uint32_t type, const uint8_t *key)
{
    uint32_t        result;

    memset(&image, 0, sizeof(image));

    memcpy(image.magic, magic, IMAGE_MAGIC_SIZE);
    image.type = type;
    memcpy(image.key, key, IMAGE_KEY_SIZE);

    result = FlashInit(&flashConfig);
    if (result != FTFx_OK)
        return FLASH_INIT;

    flashCommand = (pFLASHCOMMANDSEQUENCE)RelocateFunction(
        (uint32_t)ramFunc, sizeof(ramFunc),(uint32_t)FlashCommandSequence);

    return NO_ERROR;
}

