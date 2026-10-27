#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""
Build the 256 KiB boot partition of a SigmaStar infinity3 NOR flash:

  0x00000  zeros (the mask ROM looks for the IPL at 0x4000)
  0x03000  MXP partition table
  0x04000  IPL       the vendor's: trains the DDR, loads IPL_CUST
  0x10000  IPL_CUST  the vendor's: finds UBOOT in the MXP table, unpacks it
  0x1F000  KEY_CUST  zeros
  0x20000  UBOOT     U-Boot, xz-compressed in a legacy uImage
  ...      0xFF up to 0x40000, where the environment starts

The layout is the one on retail MSC313E cameras (and OpenIPC's NOR layout:
boot 256 KiB, environment at 0x40000). IPL_CUST takes the uImage as the
vendor's create_img.sh makes it: xz data under compression type "lzma",
load and entry 0, and unpacks it to where the vendor U-Boot is linked,
0x23E00000.

  infinity3-image.py <ipl> <ipl_cust> <u-boot.bin> <soc> <out>
"""

import lzma
import struct
import sys
import time
import zlib

BOOT_SIZE = 0x40000
MXP_OFF = 0x3000
IPL_OFF, IPL_SIZE = 0x4000, 0xC000
CUST_OFF, CUST_SIZE = 0x10000, 0xF000
KEY_OFF, KEY_SIZE = 0x1F000, 0x1000
UBOOT_OFF, UBOOT_SIZE = 0x20000, 0x20000
ENV_OFF, ENV_SIZE = 0x40000, 0x10000

# MXP record types: 1 a partition the loaders read, 3 one only listed
MXP_LOADER, MXP_LISTED = 1, 3

MXP_PARTS = [
    ("ROM", MXP_LOADER, 0, MXP_OFF),
    ("MXPT", MXP_LOADER, MXP_OFF, 0x1000),
    ("IPL", MXP_LOADER, IPL_OFF, IPL_SIZE),
    ("IPL_CUST", MXP_LOADER, CUST_OFF, CUST_SIZE),
    ("KEY_CUST", MXP_LOADER, KEY_OFF, KEY_SIZE),
    ("UBOOT", MXP_LOADER, UBOOT_OFF, UBOOT_SIZE),
    ("BOOT", MXP_LISTED, 0, BOOT_SIZE),
    ("UBOOT_ENV", MXP_LISTED, ENV_OFF, ENV_SIZE),
]


def mxp_record(name, kind, start, size):
    """One 0x88-byte record, as the vendor's mxp tool writes it."""
    rec = bytearray(b"\xff" * 0x88)
    rec[0:8] = b"MXPT" + bytes([1, kind, 0, 0xFF])
    rec[8:24] = struct.pack("<QQ", start, size)
    rec[0x20:0x40] = name.encode().ljust(0x20, b"\0")
    rec[0x7C:0x81] = b"\0TPXM"
    return bytes(rec)


def mxp_table():
    table = b"".join(mxp_record(*p) for p in MXP_PARTS)
    # The unused records: type 0, no start, no size
    empty = mxp_record("", 0, 2**64 - 1, 2**64 - 1)
    table += empty * ((0x1000 - len(table)) // 0x88)
    return table.ljust(0x1000, b"\xff")


def uimage(data, name):
    """A legacy uImage: ARM, OS U-Boot, type kernel, "lzma" (xz data)."""
    hdr = struct.pack(">IIIIIIIBBBB32s", 0x27051956, 0, int(time.time()),
                      len(data), 0, 0, zlib.crc32(data), 17, 2, 2, 3,
                      name.encode()[:32])
    hdr = hdr[:4] + struct.pack(">I", zlib.crc32(hdr)) + hdr[8:]
    return hdr + data


def main():
    if len(sys.argv) != 6:
        sys.exit(__doc__)
    ipl, cust, uboot, soc, out = sys.argv[1:]
    ipl = open(ipl, "rb").read()
    cust = open(cust, "rb").read()
    payload = lzma.compress(open(uboot, "rb").read(), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=9 | lzma.PRESET_EXTREME)
    img = uimage(payload, "OpenIPC U-Boot " + soc)

    for what, blob, size in (("IPL", ipl, IPL_SIZE),
                             ("IPL_CUST", cust, CUST_SIZE),
                             ("UBOOT", img, UBOOT_SIZE)):
        if len(blob) > size:
            sys.exit("%s: %u bytes, the partition holds %u" %
                     (what, len(blob), size))

    boot = bytearray(UBOOT_OFF)
    boot[MXP_OFF:MXP_OFF + 0x1000] = mxp_table()
    boot[IPL_OFF:IPL_OFF + len(ipl)] = ipl
    boot[CUST_OFF:CUST_OFF + len(cust)] = cust
    boot += img
    boot += b"\xff" * (BOOT_SIZE - len(boot))
    open(out, "wb").write(boot)
    print("%s: UBOOT %u of %u bytes" % (out, len(img), UBOOT_SIZE))


if __name__ == "__main__":
    main()
