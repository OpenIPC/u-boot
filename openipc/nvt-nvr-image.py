#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Build the U-Boot slot image of a Novatek NVR die, NA51103 (NT98332) or
# NA51090 (NT98633). The board's own loader (DDR training, BL31) and its
# device tree stay in the flash: only the U-Boot slot is OpenIPC's.
#
#   nt98332  SPI NOR, Xiongmai's FW98332A loader: the device tree's
#            partition_boot (0x20000) holds U-Boot as a bare LZMA stream
#            ("compress header 0x5D"); OpenIPC's environment follows at
#            0x40000, so the slot is 128 KiB.
#   nt98633  SPI NAND, the SDK's loader: partition_uboot (0xC0000,
#            0xE0000 long) holds a BCL1 LZMA container.
#
# The loader unpacks it to /nvt_memory_cfg/uboot and BL31 enters it at
# the HEADINFO's CodeEntry, in AArch64 at EL2.
#
# usage: nvt-nvr-image.py SOC U-BOOT.BIN TEXT_BASE OUTPUT [FLASH]
#
# With FLASH (a dump of the board's flash, NOR as read, or NAND data
# without OOB) the slot is also written into a copy of it, OUTPUT.flash,
# for an external programmer.

import lzma
import struct
import sys

HEADINFO = 0x300
HI_CODE_ENTRY = HEADINFO + 0x00
HI_BIN_LENGTH = HEADINFO + 0x68
HI_CHECKSUM = HEADINFO + 0x6C

BCL1_LZMA = 11

SOCS = {
    # slot offset, slot size, container
    'nt98332': (0x20000, 0x20000, 'lzma'),
    'nt98633': (0xC0000, 0xE0000, 'bcl1'),
}


def nvt_sum(data):
    """The Novatek checksum: sum of each 16-bit word plus its index"""
    words = struct.unpack('<%dH' % (len(data) // 2), data[:len(data) & ~1])
    return (sum(words) + sum(range(len(words)))) & 0xffff


def headinfo(uboot, text_base):
    """Fill in the length and the checksum, as for the NA51089 loader"""
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


def lzma_alone(data, dict_size):
    filters = [{'id': lzma.FILTER_LZMA1, 'preset': 9 | lzma.PRESET_EXTREME,
                'dict_size': dict_size, 'lc': 3, 'lp': 0, 'pb': 2}]
    return bytearray(lzma.compress(data, format=lzma.FORMAT_ALONE,
                                   filters=filters))


def bare_lzma(data):
    """
    What Xiongmai's vendor U-Boot slot holds: an .lzma stream with a
    16 MiB dictionary and an unknown size (an end marker), as LZMA Utils
    write it.
    """
    return bytes(lzma_alone(data, 1 << 24))


def bcl1_lzma(data):
    """
    BCL1 container, as the SDK's bfc writes it: the FourCC, the checksum
    (u16 LE, over the whole container), the algorithm (u16 BE), the
    unpacked and packed sizes (u32 BE), then an .lzma stream whose 64-bit
    size field holds the 32-bit length twice (the board's own U-Boot is
    packed that way).
    """
    packed = lzma_alone(data, 1 << 18)
    struct.pack_into('<II', packed, 5, len(data), len(data))
    bcl = bytearray(b'BCL1' + struct.pack('>HHII', 0, BCL1_LZMA,
                                         len(data), len(packed)))
    bcl += packed
    struct.pack_into('<H', bcl, 4, -nvt_sum(bcl) & 0xffff)
    return bytes(bcl)


def main():
    if len(sys.argv) not in (5, 6) or sys.argv[1] not in SOCS:
        sys.exit('usage: nvt-nvr-image.py {%s} U-BOOT.BIN TEXT_BASE OUTPUT '
                 '[FLASH]' % ','.join(SOCS))
    soc = sys.argv[1]
    offset, size, kind = SOCS[soc]
    uboot = open(sys.argv[2], 'rb').read()
    text_base = int(sys.argv[3], 0)

    img = headinfo(uboot, text_base)
    blob = bare_lzma(img) if kind == 'lzma' else bcl1_lzma(img)
    if len(blob) > size:
        sys.exit('U-Boot is %d bytes packed, %d too many for its %d KiB slot'
                 % (len(blob), len(blob) - size, size >> 10))
    slot = blob + b'\xff' * (size - len(blob))
    open(sys.argv[4], 'wb').write(slot)
    print('%s: U-Boot %d packed (%d), for flash %#x..%#x' %
          (sys.argv[4], len(blob), len(uboot), offset, offset + size))

    if len(sys.argv) == 6:
        flash = bytearray(open(sys.argv[5], 'rb').read())
        if len(flash) < offset + size:
            sys.exit('%s is too small' % sys.argv[5])
        flash[offset:offset + size] = slot
        open(sys.argv[4] + '.flash', 'wb').write(flash)
        print('%s.flash: %s with this U-Boot' % (sys.argv[4], sys.argv[5]))


if __name__ == '__main__':
    main()
