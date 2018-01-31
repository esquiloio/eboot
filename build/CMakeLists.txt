INCLUDE(CMakeForceCompiler)

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)
 
# BUILD DIRECTORY
SET(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# KSDK DIRECTORY
SET(KSDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ksdk)

# BOARD DIRECTORY
SET(BOARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../board)

# SOURCE DIRECTORY
SET(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# VERSION
list(APPEND CMAKE_MODULE_PATH "${BUILD_DIR}")
include(GetGitRevisionDescription)
git_describe(EBOOT_VERSION)

# DEFINES
set(DEFINES " -DCPU_MK64FN1M0VMD12 -DSD_DISK_ENABLE -DSDHC_USING_PIO -DCD_USING_GPIO -DFSL_FEATURE_SDHC_NO_IRQ -DFSL_FEATURE_SDHC_NO_ADMA1 -DFSL_FEATURE_I2C_NO_SLAVE -DFSL_FEATURE_MMCAU_AES_NO_ENCRYPT -DEBOOT_VERSION=\\\"${EBOOT_VERSION}\\\"")

# ASM FLAGS
SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mthumb  -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -mapcs  -std=gnu99 -x assembler-with-cpp ${DEFINES}")

#  C FLAGS
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mthumb  -MMD  -MP  -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -mapcs  -std=gnu99 ${DEFINES}")

# LD FLAGS
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${BUILD_DIR}/MK64FN1M0xxx12_flash.ld  -static -Xlinker --defsym=__ram_vector_table__=1  -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  --specs=nano.specs  -lm  -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -mthumb  -mapcs  -Xlinker --gc-sections  -Xlinker -static  -Xlinker -z  -Xlinker muldefs  -Xlinker --defsym=__stack_size__=0x2000  -Xlinker --defsym=__heap_size__=0x2000")

# ASM
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -g -DDEBUG")
SET(CMAKE_ASM_FLAGS_DEVELOP "${CMAKE_ASM_FLAGS_DEVELOP} -g -DNDEBUG")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -g -DNDEBUG -DRELEASE")

# C 
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
SET(CMAKE_C_FLAGS_DEVELOP "${CMAKE_C_FLAGS_DEVELOP} -g -Os -DNDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -Os -DNDEBUG -DRELEASE")

# LD
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
SET(CMAKE_EXE_LINKER_FLAGS_DEVELOP "${CMAKE_EXE_LINKER_FLAGS_DEVELOP} -g")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -g")

# INCLUDE_DIRECTORIES
INCLUDE_DIRECTORIES(${SOURCE_DIR})
INCLUDE_DIRECTORIES(${BOARD_DIR})
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/osa/inc)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/utilities/inc)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/CMSIS/Include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/devices)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/devices/MK64F12/include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/devices/MK64F12/startup)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/hal/inc)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/drivers/inc)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/system/inc)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/drivers/src/flash)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/drivers/src/mmcau/asm-cm4/src)
INCLUDE_DIRECTORIES(${KSDK_DIR}/platform/composite/inc)

INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/sources/classes/include)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/adapter/sources)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/adapter/sources/sdk)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/include/MK64F12)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/hal)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/sources/controller)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/sources/classes/common)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/sources/classes/include/config)
INCLUDE_DIRECTORIES(${KSDK_DIR}/usb/usb_core/device/sources/controller/khci)

INCLUDE_DIRECTORIES(${KSDK_DIR}/middleware/filesystem/fatfs)
INCLUDE_DIRECTORIES(${KSDK_DIR}/middleware/filesystem/fatfs/fsl_sd_disk)

SET(SOURCE "${SOURCE}"
    "${SOURCE_DIR}/eboot.c"
    "${SOURCE_DIR}/hardware_init.c"
    "${SOURCE_DIR}/usb_descriptor.c"
    "${SOURCE_DIR}/usb_descriptor.h"
    "${SOURCE_DIR}/printd.h"
    "${SOURCE_DIR}/fat16.c"
    "${SOURCE_DIR}/fat16.h"
    "${SOURCE_DIR}/usb_msc.c"
    "${SOURCE_DIR}/usb_msc.h"
    "${SOURCE_DIR}/image.c"
    "${SOURCE_DIR}/image.h"
    "${SOURCE_DIR}/ffconf.h"
    "${SOURCE_DIR}/config.c"
    "${SOURCE_DIR}/config.h"
    "${SOURCE_DIR}/stubs.c"

    "${BOARD_DIR}/gpio_pins.c"
    "${BOARD_DIR}/gpio_pins.h"
    "${BOARD_DIR}/pin_mux.c"
    "${BOARD_DIR}/pin_mux.h"
    "${BOARD_DIR}/board.c"
    "${BOARD_DIR}/board.h"

    "${KSDK_DIR}/platform/system/src/clock/fsl_clock_manager.c"
    "${KSDK_DIR}/platform/system/src/clock/fsl_clock_manager_common.c"
    "${KSDK_DIR}/platform/system/src/clock/MK64F12/fsl_clock_MK64F12.c"

    "${KSDK_DIR}/platform/hal/src/sim/MK64F12/fsl_sim_hal_MK64F12.c"
    "${KSDK_DIR}/platform/hal/src/uart/fsl_uart_hal.c"
    "${KSDK_DIR}/platform/hal/src/mcg/fsl_mcg_hal.c"
    "${KSDK_DIR}/platform/hal/src/osc/fsl_osc_hal.c"
    "${KSDK_DIR}/platform/hal/src/mcg/fsl_mcg_hal_modes.c"
    "${KSDK_DIR}/platform/hal/src/gpio/fsl_gpio_hal.c"
    "${KSDK_DIR}/platform/hal/src/crc/fsl_crc_hal.c"
    "${KSDK_DIR}/platform/hal/src/lptmr/fsl_lptmr_hal.c"
    "${KSDK_DIR}/platform/hal/src/sdhc/fsl_sdhc_hal.c"
    "${KSDK_DIR}/platform/hal/src/i2c/fsl_i2c_hal.c"

    "${KSDK_DIR}/platform/composite/src/sdcard/fsl_sdhc_card.c"

    "${KSDK_DIR}/platform/drivers/src/gpio/fsl_gpio_driver.c"
    "${KSDK_DIR}/platform/drivers/src/gpio/fsl_gpio_common.c"
    "${KSDK_DIR}/platform/drivers/src/crc/fsl_crc_driver.c"
    "${KSDK_DIR}/platform/drivers/src/crc/fsl_crc_common.c"
    "${KSDK_DIR}/platform/drivers/src/mmcau/asm-cm4/src/mmcau_aes_functions.s"
    "${KSDK_DIR}/platform/drivers/src/sdhc/fsl_sdhc_driver.c"
    "${KSDK_DIR}/platform/drivers/src/sdhc/fsl_sdhc_common.c"
    "${KSDK_DIR}/platform/drivers/src/sdhc/fsl_sdhc_irq.c"
    "${KSDK_DIR}/platform/drivers/src/i2c/fsl_i2c_master_driver.c"
    "${KSDK_DIR}/platform/drivers/src/i2c/fsl_i2c_common.c"
    "${KSDK_DIR}/platform/drivers/src/i2c/fsl_i2c_shared_function.c"
    "${KSDK_DIR}/platform/drivers/src/i2c/fsl_i2c_slave_driver.c"

    "${KSDK_DIR}/platform/osa/src/fsl_os_abstraction_bm.c"

    "${KSDK_DIR}/platform/system/src/interrupt/fsl_interrupt_manager.c"

    "${KSDK_DIR}/usb/usb_core/device/sources/bsp/MK64F12/usb_dev_bsp.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/classes/common/usb_class.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/classes/msd/usb_msc.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/classes/msd/usb_msc_scsi.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/controller/khci/khci_dev.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/controller/khci/device_khci_interface.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/controller/usb_dev.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/controller/usb_framework.c"
    "${KSDK_DIR}/usb/usb_core/device/sources/controller/khci/khci_dev.c"
    "${KSDK_DIR}/usb/adapter/sources/sdk/adapter_sdk.c"

    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/CopyToRam.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashCommandSequence.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashEraseSector.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashInit.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashProgramSection.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashProgramCheck.c"
    "${KSDK_DIR}/platform/drivers/src/flash/C90TFS/drvsrc/source/FlashVerifySection.c"

    "${KSDK_DIR}/middleware/filesystem/fatfs/diskio.c"
    "${KSDK_DIR}/middleware/filesystem/fatfs/ff.c"
    "${KSDK_DIR}/middleware/filesystem/fatfs/fsl_sd_disk/src/fsl_sd_disk.c"
    #"${KSDK_DIR}/middleware/filesystem/fatfs/option/unicode.c"

    "${KSDK_DIR}/platform/devices/MK64F12/startup/system_MK64F12.c"
    "${KSDK_DIR}/platform/devices/MK64F12/startup/gcc/startup_MK64F12.S"
    "${KSDK_DIR}/platform/devices/startup.c"

)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
SET(SOURCE "${SOURCE}"
    "${KSDK_DIR}/platform/utilities/src/fsl_debug_console.c"
    "${KSDK_DIR}/platform/utilities/src/print_scan.c"
)
ENDIF()

ADD_EXECUTABLE(eboot ${SOURCE})
SET_TARGET_PROPERTIES(eboot PROPERTIES OUTPUT_NAME "eboot.elf")

TARGET_LINK_LIBRARIES(eboot -Wl,--start-group)
TARGET_LINK_LIBRARIES(eboot m)
TARGET_LINK_LIBRARIES(eboot c)
TARGET_LINK_LIBRARIES(eboot gcc)
TARGET_LINK_LIBRARIES(eboot nosys)
TARGET_LINK_LIBRARIES(eboot -Wl,--end-group)

# MAP FILE
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}  -Xlinker -Map=debug/eboot.map")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}  -Xlinker -Map=release/eboot.map")

# HEX
ADD_CUSTOM_COMMAND(TARGET eboot POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Oihex ${EXECUTABLE_OUTPUT_PATH}/eboot.elf ${EXECUTABLE_OUTPUT_PATH}/eboot.hex)

# BIN
ADD_CUSTOM_COMMAND(TARGET eboot POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/eboot.elf ${EXECUTABLE_OUTPUT_PATH}/eboot.bin)

# IMG
ADD_CUSTOM_COMMAND(TARGET eboot POST_BUILD COMMAND bin2img ${EXECUTABLE_OUTPUT_PATH}/eboot.bin ${EXECUTABLE_OUTPUT_PATH}/eboot.img 2 0x0)

# CLEAN
SET(CLEAN_FILES
    ${EXECUTABLE_OUTPUT_PATH}/eboot.map
    ${EXECUTABLE_OUTPUT_PATH}/eboot.hex
    ${EXECUTABLE_OUTPUT_PATH}/eboot.bin
    ${EXECUTABLE_OUTPUT_PATH}/eboot.img
)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")

