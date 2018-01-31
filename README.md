# Esquilo Bootloader

This project is the bootloader for the Esquilo Air using the NXP K64 processor.  The bootloader supports secure firmware updates via an SD card or by emulating a USB mass storage device (i.e. USB flash drive).  Security is provided by using the AES-256 algorithm on the firmware image.  The bootloader also supports clearing the Esquilo OS (EOS) configuration if the PROG button is held down during boot.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You must have an [ARM gcc toolchain](https://launchpad.net/gcc-arm-embedded/) and cmake installed to build the bootloader.

Set the ARMGCC_DIR environment variable to the root of the ARM gcc toolchain install.

```
export ARMGCC_DIR=/usr/local/arm-gcc
```

### Installing

To configure the bootloader build, go into the build directory and run the appropriate shell script to create the Makefile.

```
cd build
./cmake_release.sh
```

There are three build versions:

|Version|Optimization|Debug Output|JTAG Support|Flash Security|
|-------|:----------:|:----------:|:----------:|:------------:|
|debug  | no         | yes        | yes        | no           |
|develop| yes        | no         | yes        | no           |
|release| yes        | no         | no         | yes          |

To build eboot, execute make:

```
make
```

NOTE: You will probably want to create a unique AES-256 key for encrypting the images.  This key may be found in the image\_key.h file.

## Deployment

If the Esquilo Air is running EOS 0.6+, then the bootloader may be updated via OTA update.  See the update RPC call in EOS for more information.  For earlier EOS versions or for a completely erased system, you can install the bootloader using a JTAG programmer using the JTAG header (J4) on the Esquilo Air.

## License

Esquilo developed portions of this project are licensed under the MIT License.  Portions developed by Freescale/NXP use a BSD-like license.  See the headers in each individual file for specifics.

