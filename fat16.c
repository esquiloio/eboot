/******************************************************************************
*                                                  
*  (c) copyright Freescale Semiconductor 2008
*  ALL RIGHTS RESERVED
*
*  File Name: FAT16.c
*                                                                          
*  Purpose: This file is for a USB Mass-Storage Device bootloader.  This file 
*           mimics a FAT16 drive in order to enumerate as a disk drive
*                                                                          
*  Assembler:  Codewarrior for Microcontrollers V6.2
*                                            
*  Version:  1.3
*                                                                          
*                                                                          
*  Author: Derek Snell                             
*                                                                                       
*  Location: Indianapolis, IN. USA                                            
*                                                                                  
* UPDATED HISTORY:
*
* REV   YYYY.MM.DD  AUTHOR        DESCRIPTION OF CHANGE
* ---   ----------  ------        --------------------- 
* 1.3   2009.01.13  Derek Snell   Added linker SEGMENTs for S08 version
* 1.2   2008.11.24  Derek Snell   Added Volume label "BOOTLOADER" to FAT16 root directory
* 1.1   2008.09.17  Derek Snell   Updated to give S19 address error in status
* 1.0   2008.06.10  Derek Snell   Initial version
* 
*
******************************************************************************/                                                                        
/* Freescale  is  not  obligated  to  provide  any  support, upgrades or new */
/* releases  of  the Software. Freescale may make changes to the Software at */
/* any time, without any obligation to notify or provide updated versions of */
/* the  Software  to you. Freescale expressly disclaims any warranty for the */
/* Software.  The  Software is provided as is, without warranty of any kind, */
/* either  express  or  implied,  including, without limitation, the implied */
/* warranties  of  merchantability,  fitness  for  a  particular purpose, or */
/* non-infringement.  You  assume  the entire risk arising out of the use or */
/* performance of the Software, or any systems you design using the software */
/* (if  any).  Nothing  may  be construed as a warranty or representation by */
/* Freescale  that  the  Software  or  any derivative work developed with or */
/* incorporating  the  Software  will  be  free  from  infringement  of  the */
/* intellectual property rights of third parties. In no event will Freescale */
/* be  liable,  whether in contract, tort, or otherwise, for any incidental, */
/* special,  indirect, consequential or punitive damages, including, but not */
/* limited  to,  damages  for  any loss of use, loss of time, inconvenience, */
/* commercial loss, or lost profits, savings, or revenues to the full extent */
/* such  may be disclaimed by law. The Software is not fault tolerant and is */
/* not  designed,  manufactured  or  intended by Freescale for incorporation */
/* into  products intended for use or resale in on-line control equipment in */
/* hazardous, dangerous to life or potentially life-threatening environments */
/* requiring  fail-safe  performance,  such  as  in the operation of nuclear */
/* facilities,  aircraft  navigation  or  communication systems, air traffic */
/* control,  direct  life  support machines or weapons systems, in which the */
/* failure  of  products  could  lead  directly to death, personal injury or */
/* severe  physical  or  environmental  damage  (High  Risk Activities). You */
/* specifically  represent and warrant that you will not use the Software or */
/* any  derivative  work of the Software for High Risk Activities.           */
/* Freescale  and the Freescale logos are registered trademarks of Freescale */
/* Semiconductor Inc.                                                        */ 
/*****************************************************************************/


#include "fat16.h"

#include <string.h>

/********************************************************************
*********************************************************************
*       FAT16 Boot Sector
*********************************************************************
********************************************************************/
const uint8_t FAT16_BootSector[]=
{
    0xEB,           /*00 - BS_jmpBoot */
    0x3C,           /*01 - BS_jmpBoot */
    0x90,           /*02 - BS_jmpBoot */
    'E','S','Q','U','I','L','O',' ',    /* 03-10 - BS_OEMName */
    0x00,           /*11 - BPB_BytesPerSec = 512 */
    0x02,           /*12 - BPB_BytesPerSec = 512 */
    4,              /*13 - BPB_Sec_PerClus = 4 */
    1,              /*14 - BPB_RsvdSecCnt = 1 */
    0,              /*15 - BPB_RsvdSecCnt = 1 */
    2,              /*16 - BPB_NumFATs = 2 */
    0x0,            /*17 - BPB_RootEntCnt = 512 */
    0x2,            /*18 - BPB_RootEntCnt = 512 */
    0xA0,           /*19 - BPB_TotSec16 = 4,000 */
    0x0F,           /*20 - BPB_TotSec16 = 4,000 */
    0xF0,           /*21 - BPB_Media = 0xF0 */
    0xFA,           /*22 - BPBFATSz16 = 250 */
    0,              /*23 - BPBFATSz16 = 250 */
    0x32,           /*24 - BPB_SecPerTrk = 50 */
    0,              /*25 - BPB_SecPerTrk = 50 */
    5,              /*26 - BPB_NumHeads = 5 */
    0,              /*27 - BPB_NumHeads = 5 */
    0,              /*28 - BPB_HiddSec = 0 */
    0,              /*29 - BPB_HiddSec = 0 */
    0,              /*30 - BPB_HiddSec = 0 */
    0,              /*31 - BPB_HiddSec = 0 */
    0xA0,           /*32 - BPB_TotSec32 = 4,000 */
    0x0F,           /*33 - BPB_TotSec32 = 4,000 */
    0x00,           /*34 - BPB_TotSec32 = 4,000 */
    0x00,           /*35 - BPB_TotSec32 = 4,000 */
    0,              /*36 - BS_DrvNum = 0 */
    0,              /*37 - BS_Reserved1 = 0 */
    0x29,           /*38 - BS_BootSig = 0x29 */
    0xBD,           /*39 - BS_VolID = 0x02DDA5BD */
    0xA5,           /*40 - BS_VolID = 0x02DDA5BD */
    0xDD,           /*41 - BS_VolID = 0x02DDA5BD */
    0x02,           /*42 - BS_VolID = 0x02DDA5BD */
    'N','O',' ','N','A','M','E',' ',' ',' ',' ',  /*43-53 - BS_VolLab */
    'F','A','T','1','6',' ',' ',' '   /*54-61 - BS_FilSysType */
};


/********************************************************************
*********************************************************************
*       First Sector of FAT Table
*********************************************************************
********************************************************************/
const uint8_t FAT16_TableSector0[]=
{
    0xF0,0xFF,0xFF,0xFF,0xFF,0xFF
};


/********************************************************************
*********************************************************************
*       FAT Root Directory Sector
*********************************************************************
********************************************************************/

const uint8_t FAT16_HelpFileName[FATFileNameSize]=
{
    'H','E','L','P',' ',' ',' ',' ','T','X','T'    /*00-10 - Short File Name */
};

static const char help_txt[] =
"Welcome to the Esquilo bootloader!\r\n"
#if !_DEBUG
"\r\n"
"The Esquilo bootloader enables you to easily update your Esquilo Operating\r\n"
"System (EOS) by copying a new EOS software image onto the ESQUILO drive on\r\n"
"your computer.  With your web browser, go to http://esquilo.io/ to download\r\n"
"an updated EOS software image.  Save the image onto your computer and copy\r\n"
"it to the ESQUILO drive.  When the update is finished, the Esquilo will\r\n"
"reboot and the ESQUILO drive will disappear.  Do not unplug the USB cable\r\n"
"once the bootloader starts an update.\r\n"
"\r\n"
"The Esquilo status LED shows the progress of the bootloader.\r\n"
" * slow blink - ready for a software image\r\n"
" * fast blink - updating the software image (do not disturb)\r\n"
" * short blinks - an error occurred and the blink count indicates the error\r\n"
"   * 1 - flash initialization error\r\n"
"   * 2 - flash erase error\r\n"
"   * 3 - flash verify error\r\n"
"   * 4 - image failed integrity check\r\n"
"   * 5 - wrong image type\r\n"
"   * 6 - image size is incorrect\r\n"
"   * 7 - USB disk failure\r\n"
"   * 8 - USB communication failure\r\n"
"   * 9 - software error\r\n"
"\r\n"
"You can enter the bootloader at any time by holding down the program switch\r\n"
"while pressing the reset switch or applying power.\r\n"
#endif
;

#define HELP_SIZE (sizeof(help_txt) - 1)

#define BYTELL(v) ((v) & 0xff)
#define BYTEHL(v) (((v) >> 8) & 0xff)
#define BYTELH(v) (((v) >> 16) & 0xff)
#define BYTEHH(v) (((v) >> 24) & 0xff)

const uint8_t FAT16_RootDirSector[]=
{
    0x20,           /*11 - Archive Attribute set */
    0x00,           /*12 - Reserved */
    0x4B,           /*13 - Create Time Tenth */
    0x9C,           /*14 - Create Time */
    0x42,           /*15 - Create Time */
    0x92,           /*16 - Create Date */
    0x38,           /*17 - Create Date */
    0x92,           /*18 - Last Access Date */
    0x38,           /*19 - Last Access Date */
    0x00,           /*20 - Not used in FAT16 */
    0x00,           /*21 - Not used in FAT16 */
    0x9D,           /*22 - Write Time */
    0x42,           /*23 - Write Time */
    0x92,           /*24 - Write Date */
    0x38,           /*25 - Write Date */
    0x02,           /*26 - First Cluster */
    0x00,           /*27 - First Cluster */
    BYTELL(HELP_SIZE), /*28 - File Size */
    BYTEHL(HELP_SIZE), /*29 - File Size */
    BYTELH(HELP_SIZE), /*30 - File Size */
    BYTEHH(HELP_SIZE), /*31 - File Size */
    'E','S','Q','U','I','L','O',' ',' ',' ',' ',  /*32-42 - Volume label */
    0x08,           /*43 - File attribute = Volume label */
    0x00,           /*44 - Reserved */
    0x00,           /*45 - Create Time Tenth */
    0x00,           /*46 - Create Time */
    0x00,           /*47 - Create Time */
    0x00,           /*48 - Create Date */
    0x00,           /*49 - Create Date */
    0x00,           /*50 - Last Access Date */
    0x00,           /*51 - Last Access Date */
    0x00,           /*52 - Not used in FAT16 */
    0x00,           /*53 - Not used in FAT16 */
    0x9D,           /*54 - Write Time */
    0x42,           /*55 - Write Time */
    0x92,           /*56 - Write Date */
    0x38,           /*57 - Write Date */
};

static void xmemcpy(uint8_t **dest, const void *src, size_t len)
{
    memcpy(*dest, src, len);
    *dest = (*dest + len);
}

static void xmemzero(uint8_t **dest, size_t len)
{
    memset(*dest, 0, len);
    *dest = (*dest + len);
}

/*********************************************************
* Name: FATReadLBA
*
* Desc: Read a Logical Block Address 
*
* Parameter: FAT_LBA - Logical Block Address to Read
*            pu8DataPointer - Pointer to array to store data read  
*
* Return: None
*             
**********************************************************/
void FATReadLBA
    (
        uint32_t FAT_LBA,
        uint8_t *data
    ) 
{
    /* Body */
    switch (FAT_LBA) 
    {
        /* Boot Sector */
        case FATBootSec: 
            /* Write Boot Sector info */
            xmemcpy(&data, FAT16_BootSector, sizeof(FAT16_BootSector));
            /* Rest of sector empty except last two bytes */
            xmemzero(&data, FATBytesPerSec - sizeof(FAT16_BootSector) - 2);
            /* Boot Sector requires these 2 bytes at end */
            *data++ = 0x55;
            *data++ = 0xaa;
            break;
        /* FAT Table Sector */
        case FATTable0Sec0:
        case FATTable1Sec0:
            /* Write FAT Table Sector */
            xmemcpy(&data, FAT16_TableSector0, sizeof(FAT16_TableSector0));
            /* Rest of sector empty */
            xmemzero(&data, FATBytesPerSec - sizeof(FAT16_TableSector0));
            break;
            
        /* Root Directory Sector */
        case FATRootDirSec0:
            /* Write the file name */
            xmemcpy(&data, FAT16_HelpFileName, FATFileNameSize);
            /* Write rest of file FAT structure */
            xmemcpy(&data, FAT16_RootDirSector, sizeof(FAT16_RootDirSector));
            /* Rest of sector empty to signify no more files */
            xmemzero(&data, FATBytesPerSec - FATFileNameSize - sizeof(FAT16_RootDirSector));
            break;

        default:
        {
            if (FAT_LBA >= FATDataSec0) {
                int offset = (FAT_LBA - FATDataSec0) * FATBytesPerSec;
                int helpsize = HELP_SIZE - offset;

                if (helpsize <= 0)
                {
                    xmemzero(&data, FATBytesPerSec);
                }
                else if (helpsize >= FATBytesPerSec)
                {
                    xmemcpy(&data, help_txt + offset, FATBytesPerSec);
                }
                else
                {
                    xmemcpy(&data, help_txt + offset, helpsize);
                    xmemzero(&data, FATBytesPerSec - helpsize);
                }

            }
            else
            {
                xmemzero(&data, FATBytesPerSec);
            }
            break;
        }
    } /* EndSwitch */
} /* EndBody */

  /* EOF */
