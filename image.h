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
#ifndef _IMAGE_H_
#define _IMAGE_H_

#define IMAGE_MAGIC_SIZE        16
#define IMAGE_KEY_SIZE          32
#define IMAGE_HEADER_SIZE       48

typedef enum
{
    NO_ERROR,
    FLASH_INIT,
    FLASH_ERASE,
    FLASH_PROGRAM,
    CRC_CHECK,
    BAD_IMAGE,
    IMAGE_SIZE,
    BLOCK_SIZE,
    MEDIA_FAILURE,
    ABORT,
} image_error_t;

uint32_t imageRemain(void);

void imageBoot(void);

void imageClear(void);

image_error_t imageWrite(void *data, uint32_t size);

image_error_t imageInit(const uint8_t *magic, uint32_t type, const uint8_t *key);

#endif

