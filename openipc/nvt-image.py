#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Build the boot partition image of a Novatek NA51089 camera:
#
#   0x00000  the loader (it trains the DDR; the mask ROM starts it)
#   0x10000  the device tree the loader reads, then hands to U-Boot
#   0x20000  U-Boot, in an LZMA BCL1 container
#
# up to the environment at 0x40000. The loader finds U-Boot through the
# device tree's /nor/partition_uboot, unpacks it with the mask ROM's LZMA
# decoder to /nvt_memory_cfg/uboot and starts it at the HEADINFO's
# CodeEntry, once the checksum over the unpacked image adds up.
#
# usage: nvt-image.py LOADER DTB U-BOOT.BIN TEXT_BASE OUTPUT

import lzma
import struct
import sys

FDT_OFFSET = 0x10000
UBOOT_OFFSET = 0x20000
IMAGE_SIZE = 0x40000
# The loader reads a partition in whole flash blocks
FLASH_BLOCK = 0x10000

HEADINFO = 0x300
HI_CODE_ENTRY = HEADINFO + 0x00
HI_BIN_LENGTH = HEADINFO + 0x68
HI_CHECKSUM = HEADINFO + 0x6C

BCL1_LZMA = 11


def nvt_sum(data):
    """The Novatek checksum: sum of each 16-bit word plus its index"""
    words = struct.unpack('<%dH' % (len(data) // 2), data[:len(data) & ~1])
    return (sum(words) + sum(range(len(words)))) & 0xffff


def headinfo(uboot, text_base):
    """Fill in the length and the checksum the loader verifies"""
    img = bytearray(uboot)
    if len(img) & 1:
        img += b'\0'
    entry = struct.unpack_from('<I', img, HI_CODE_ENTRY)[0]
    if entry != text_base:
        sys.exit('HEADINFO CodeEntry %#x is not the link address %#x'
                 % (entry, text_base))
    struct.pack_into('<II', img, HI_BIN_LENGTH, len(img), 0)
    struct.pack_into('<I', img, HI_CHECKSUM, -nvt_sum(img) & 0xffff)
    assert nvt_sum(img) == 0
    return bytes(img)


def bcl1_lzma(data):
    """
    BCL1 container, as the SDK's bfc writes it: the FourCC, the checksum
    (u16 LE, over the whole container), the algorithm (u16 BE), the
    unpacked and packed sizes (u32 BE), then an .lzma stream with its
    real size in the header.
    """
    filters = [{'id': lzma.FILTER_LZMA1, 'preset': 9 | lzma.PRESET_EXTREME,
                'dict_size': 1 << 18, 'lc': 3, 'lp': 0, 'pb': 2}]
    packed = bytearray(lzma.compress(data, format=lzma.FORMAT_ALONE,
                                     filters=filters))
    struct.pack_into('<Q', packed, 5, len(data))
    # The loader reads the packed size, rounded up to whole flash blocks,
    # from the start of the container: the 16-byte header must fit too.
    if len(packed) % FLASH_BLOCK == 0 or \
            len(packed) % FLASH_BLOCK > FLASH_BLOCK - 16:
        packed += bytes(16)
    bcl = bytearray(b'BCL1' + struct.pack('>HHII', 0, BCL1_LZMA,
                                         len(data), len(packed)))
    bcl += packed
    struct.pack_into('<H', bcl, 4, -nvt_sum(bcl) & 0xffff)
    return bytes(bcl)


def main():
    if len(sys.argv) != 6:
        sys.exit(__doc__ or 'usage: nvt-image.py LOADER DTB U-BOOT.BIN '
                 'TEXT_BASE OUTPUT')
    loader = open(sys.argv[1], 'rb').read()
    dtb = open(sys.argv[2], 'rb').read()
    uboot = open(sys.argv[3], 'rb').read()
    text_base = int(sys.argv[4], 0)

    bcl = bcl1_lzma(headinfo(uboot, text_base))
    for name, blob, room in (('loader', loader, FDT_OFFSET),
                             ('device tree', dtb, UBOOT_OFFSET - FDT_OFFSET),
                             ('U-Boot', bcl, IMAGE_SIZE - UBOOT_OFFSET)):
        if len(blob) > room:
            sys.exit('%s is %d bytes, %d too many' %
                     (name, len(blob), len(blob) - room))

    img = bytearray(b'\xff' * IMAGE_SIZE)
    img[0:len(loader)] = loader
    img[FDT_OFFSET:FDT_OFFSET + len(dtb)] = dtb
    img[UBOOT_OFFSET:UBOOT_OFFSET + len(bcl)] = bcl
    open(sys.argv[5], 'wb').write(img)
    print('%s: loader %d, device tree %d, U-Boot %d packed (%d)' %
          (sys.argv[5], len(loader), len(dtb), len(bcl), len(uboot)))


if __name__ == '__main__':
    main()
