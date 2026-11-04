#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Build the boot partition image of a Novatek NA51089 camera, or of an
# NA51068 (NT98321 / NT98323) DVR/NVR board:
#
#   0x00000  the loader (it trains the DDR; the mask ROM starts it)
#   0x10000  the device tree the loader reads, then hands to U-Boot
#   0x20000  U-Boot, in an LZMA BCL1 container
#
# up to the environment at 0x40000. The loader finds U-Boot through the
# device tree's /nor/partition_uboot, unpacks it with an LZMA decoder
# (NA51089's mask ROM's, the one the NA51068 loader's Compress.a has, or
# the NA51055 / NS02501 loaders' own) to /nvt_memory_cfg/uboot and starts
# it at the HEADINFO's CodeEntry, once the checksum over the unpacked
# image adds up.
#
# With --fdt-lzma the device tree goes in an LZMA BCL1 container too: the
# NS02501 trees (~95 KiB) do not fit their 64 KiB plain, and that loader
# (board/novatek/ns02501/loader) unpacks one.
#
# With --nand the layout is the SPI NAND one, in 128 KiB blocks
# (openipc/nvt-nand.dtsi): the loader at 0, the tree at 0x40000 and its
# copy (fdt.restore) at 0x80000, U-Boot at 0xC0000. The image ends with
# U-Boot's last block; its partition runs to the environment at 0x2C0000.
# ${ubwrite} writes each part at its partition's start (a bad block is
# skipped), so every part must leave its partition a spare block: the
# loader and each tree one block of two, U-Boot all but one of sixteen.
#
# usage: nvt-image.py [--fdt-lzma] [--nand] LOADER DTB U-BOOT.BIN TEXT_BASE
#                     OUTPUT

import lzma
import struct
import sys

# SPI NOR: (block, tree, tree copy, U-Boot, end of U-Boot's room)
NOR = (0x10000, 0x10000, None, 0x20000, 0x40000)
NAND = (0x20000, 0x40000, 0x80000, 0xC0000, 0x2C0000)

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


def bcl1_lzma(data, flash_block):
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
    if len(packed) % flash_block == 0 or \
            len(packed) % flash_block > flash_block - 16:
        packed += bytes(16)
    bcl = bytearray(b'BCL1' + struct.pack('>HHII', 0, BCL1_LZMA,
                                         len(data), len(packed)))
    bcl += packed
    struct.pack_into('<H', bcl, 4, -nvt_sum(bcl) & 0xffff)
    return bytes(bcl)


def main():
    args = sys.argv[1:]
    fdt_lzma = '--fdt-lzma' in args
    nand = '--nand' in args
    args = [a for a in args if a not in ('--fdt-lzma', '--nand')]
    if len(args) != 5:
        sys.exit('usage: nvt-image.py [--fdt-lzma] [--nand] LOADER DTB '
                 'U-BOOT.BIN TEXT_BASE OUTPUT')
    block, fdt_off, fdt2_off, uboot_off, end = NAND if nand else NOR
    loader = open(args[0], 'rb').read()
    dtb = open(args[1], 'rb').read()
    uboot = open(args[2], 'rb').read()
    text_base = int(args[3], 0)
    dtb_len = len(dtb)
    if fdt_lzma:
        dtb = bcl1_lzma(dtb, block)

    bcl = bcl1_lzma(headinfo(uboot, text_base), block)
    spare = block if nand else 0
    for name, blob, room in (('loader', loader, fdt_off - spare),
                             ('device tree', dtb,
                              (fdt2_off or uboot_off) - fdt_off - spare),
                             ('U-Boot', bcl, end - uboot_off - spare)):
        if len(blob) > room:
            sys.exit('%s is %d bytes, %d too many' %
                     (name, len(blob), len(blob) - room))

    size = end
    if nand:
        size = uboot_off + -(-len(bcl) // block) * block
    img = bytearray(b'\xff' * size)
    img[0:len(loader)] = loader
    img[fdt_off:fdt_off + len(dtb)] = dtb
    if fdt2_off:
        img[fdt2_off:fdt2_off + len(dtb)] = dtb
    img[uboot_off:uboot_off + len(bcl)] = bcl
    open(args[4], 'wb').write(img)
    print('%s: loader %d, device tree %d%s, U-Boot %d packed (%d)' %
          (args[4], len(loader), len(dtb),
           ' packed (%d)' % dtb_len if fdt_lzma else '', len(bcl),
           len(uboot)))


if __name__ == '__main__':
    main()
