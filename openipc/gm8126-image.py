#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""
Lay out the boot partition of a Grain Media GM8126 camera, for a programmer
to write at offset 0.

  gm8126-image.py <nsboot.bin> <u-boot.bin> <u-boot offset> <env offset> <out>

The system header at 0 and nsboot at 0x1000 are read by the mask ROM in
32-bit SPI frames, so their words are byte-swapped in the flash. nsboot
itself reads bytes: U-Boot, and everything after it, is stored as it is.
Each image has a 256-byte header: the magic 0x805A474D, a checksum (0, as
the vendor leaves it), the size and a name.
"""

import array
import struct
import sys

MAGIC = 0x805A474D
HDR = 0x100
NSBOOT_OFF = 0x1000
# what the vendor's loader reads from after the header
NSBOOT_MAX = 20 * 1024


def image(data, name):
    data += b"\0" * (-len(data) % 4)
    hdr = struct.pack("<IIII16s", MAGIC, 0, len(data), 0, name)
    return hdr.ljust(HDR, b"\0") + data


def main():
    if len(sys.argv) != 6:
        sys.exit(__doc__)
    nsboot = image(open(sys.argv[1], "rb").read(), b"nsboot.bin")
    uboot = image(open(sys.argv[2], "rb").read(), b"u-boot.bin")
    uboot_off = int(sys.argv[3], 0)
    env_off = int(sys.argv[4], 0)

    if len(nsboot) - HDR > NSBOOT_MAX:
        sys.exit("nsboot is %d bytes, over %d" %
                 (len(nsboot) - HDR, NSBOOT_MAX))
    if NSBOOT_OFF + len(nsboot) > uboot_off:
        sys.exit("nsboot ends past U-Boot's 0x%x" % uboot_off)
    end = uboot_off + len(uboot)
    if end > env_off:
        sys.exit("U-Boot ends at 0x%x, past the environment at 0x%x" %
                 (end, env_off))

    first = bytearray(b"\xff" * uboot_off)
    hdr = bytearray(HDR)
    # bootm (what nsboot loads), burn-in, U-Boot and Linux addresses
    struct.pack_into("<8sIIII", hdr, 0, b"GM8126", uboot_off, uboot_off,
                     uboot_off, 0x50000)
    # page, sector and chip size (log2)
    struct.pack_into("<III", hdr, 0x34, 8, 12, 24)
    hdr[0xFE:0x100] = b"\x55\xaa"
    first[0:HDR] = hdr
    first[NSBOOT_OFF:NSBOOT_OFF + len(nsboot)] = nsboot
    words = array.array("I", bytes(first))
    assert words.itemsize == 4
    words.byteswap()

    open(sys.argv[5], "wb").write(words.tobytes() + uboot)
    print("%s: nsboot %d bytes, U-Boot %d bytes at 0x%x, %d bytes to spare" %
          (sys.argv[5], len(nsboot) - HDR, len(uboot) - HDR, uboot_off,
           env_off - end))


if __name__ == "__main__":
    main()
