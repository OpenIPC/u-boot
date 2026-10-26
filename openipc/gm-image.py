#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""
Lay out the boot partition of a Grain Media GM8135S / GM8136 / GM8136S
camera: the NOR system header at 0, nsboot at 0x1000 and U-Boot after it,
all to end before the environment.

  gm-image.py <nsboot.bin> <u-boot.bin> <u-boot offset> <env offset> <out>

The mask ROM checks the header's signature and loads nsboot -- 24 KiB at
most -- from the second 4 KiB sector into SRAM. nsboot reads the header,
loads its first image to DDR 0 and jumps there. The header is the SDK's
NOR_SYSHDR (nsboot include/spi_flash.h, the SDK flash guide's table 1-1),
filled as the Longse cameras' flash has it.
"""

import struct
import sys

HDR_SIZE = 0x100
NSBOOT_OFF = 0x1000
NSBOOT_MAX = 24 * 1024
PAGE = 0x100


def header(uboot_off, uboot_size):
    hdr = bytearray(HDR_SIZE)
    hdr[0:8] = b"GM8136".ljust(8, b"\0")
    # bootm_addr, bootm_size, reserved: the image nsboot loads, as image 0
    struct.pack_into("<III", hdr, 0x08, uboot_off, uboot_size, 0)
    # image[0..9]: {addr, size, name[8], reserved}; only U-Boot. The
    # OpenIPC kernel takes its partitions from mtdparts=.
    struct.pack_into("<II8sI", hdr, 0x14, uboot_off, uboot_size, b"UBOOT", 0)
    # norfixup: page, sector and chip size (log2), clock divider
    struct.pack_into("<IIII", hdr, 0xDC, 8, 12, 24, 4)
    hdr[0xFE:0x100] = b"\x55\xaa"
    return bytes(hdr)


def main():
    if len(sys.argv) != 6:
        sys.exit(__doc__)
    nsboot = open(sys.argv[1], "rb").read()
    uboot = open(sys.argv[2], "rb").read()
    uboot_off = int(sys.argv[3], 0)
    env_off = int(sys.argv[4], 0)

    if len(nsboot) > NSBOOT_MAX:
        sys.exit("nsboot is %d bytes, the boot ROM loads %d" %
                 (len(nsboot), NSBOOT_MAX))
    if uboot_off < NSBOOT_OFF + NSBOOT_MAX or uboot_off % PAGE:
        sys.exit("U-Boot at 0x%x overlaps nsboot's 24 KiB or is not "
                 "page aligned" % uboot_off)
    # nsboot reads whole pages
    size = (len(uboot) + PAGE - 1) // PAGE * PAGE
    end = uboot_off + size
    if end > env_off:
        sys.exit("U-Boot ends at 0x%x, past the environment at 0x%x "
                 "(%d bytes too many)" % (end, env_off, end - env_off))

    image = bytearray(b"\xff" * uboot_off)
    image[0:HDR_SIZE] = header(uboot_off, size)
    image[NSBOOT_OFF:NSBOOT_OFF + len(nsboot)] = nsboot
    image += uboot
    open(sys.argv[5], "wb").write(image)
    print("%s: nsboot %d bytes, U-Boot %d bytes at 0x%x, %d bytes to spare"
          % (sys.argv[5], len(nsboot), len(uboot), uboot_off,
             env_off - end))


if __name__ == "__main__":
    main()
