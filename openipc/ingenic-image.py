#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Assemble the Ingenic T-series NOR boot image: one file a programmer
writes at offset 0 and `sf write` takes as it is.

  0x0000  the SPL, its boot ROM header completed as the vendor
          tools/ingenic-tools/spi_checksum did: the CRC7 of the SPL from
          2 KiB on in byte 9, the SPL length in bytes 12..15
  0x6800  U-Boot, lzop-packed in a legacy uImage (what the SPL loads)

The image must end before the environment, at 0x40000.

The XBurst2 (T40) SPL goes through two more vendor tools first:
spl_pad_to_block (--pad-block: 0xff up to a multiple of 64 bytes) and, on
the targets whose boards.cfg line has SPL_PARAMS_FIXER, spl_params_fixer
(--params: the boot ROM's parameter block at offset 256, the CPM set-up it
runs before reading the rest of the SPL).

The SPI NAND image (--nand, the T40's SFC_NAND targets) has the vendor's
27 KiB SPL slot, 0x6c00, padded with 0xff as the vendor's u-boot-with-spl,
and must end before the NAND environment, at 0xc0000: the 768 KiB boot
partition of the openipc-hisilicon NAND layout.

usage: ingenic-image.py [--pad-block] [--params] [--nand] <u-boot-spl.bin>
                        <u-boot-lzo.img> <out>
"""

import os
import struct
import sys

SPL_SLOT = 0x6800
ENV_OFFSET = 0x40000
UNPACK_MAX = 0x80200000 - 0x80100000
NAND_SPL_SLOT = 0x6c00
NAND_ENV_OFFSET = 0xc0000
SKIP_SIZE = 2048
CRC_POSITION = 9
SPL_LENGTH_POSITION = 12


# crc7_syndrome_table of the vendor spi_checksum.c (the Linux lib/crc7.c one)
CRC7 = [
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
    0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
    0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
    0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
    0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
    0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
    0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
    0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
    0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
    0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
    0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
    0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
    0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
    0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
    0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
    0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
    0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
    0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
    0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
    0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
    0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
    0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
    0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
    0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
    0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
    0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
    0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
    0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
    0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79,
]


def crc7(data):
    crc = 0
    for b in data:
        crc = CRC7[((crc << 1) ^ b) & 0xff]
    return crc


# spl_params_fixer.c of u-boot-t40 (FPGA defined there: pll_freq and
# cpccr 0), the descriptors as {set_addr, poll_addr, value, poll_h_mask,
# poll_l_mask}
PARAMS_OFFSET = 256
PARAMS_LENGTH = 256
PARAMS_DESCS = [
    # APLL 600 MHz (M=50, N=1, OD1=2, OD0=1)
    (0xb0000010, 0xb0000010, 0x03205101, 0x8, 0),
    # MPLL 1200 MHz (M=100, N=1, OD1=2, OD0=1)
    (0xb0000014, 0xb0000014, 0x06405101, 0x8, 0),
    # CPCCR: dividers, then the clock selection
    (0xb0000000, 0xb00000d4, 0x557b5510, 0, 0x7),
    (0xb0000000, 0xffffffff, 0x9a0b5510, 0, 0),
    # SFC SSICDR[7:0]: 24 MHz (0x18)
    (0xb0000060, 0xb0000060, 0x60000018, 0, 0x10000000),
    (0xffffffff, 0xffffffff, 0, 0, 0),
]
PARAMS_NDESCS = 11


def spl_params(spl_length):
    length = spl_length if spl_length & 0x1ff == 0 \
        else (spl_length & ~0x1ff) + 0x200
    block = bytearray(PARAMS_LENGTH)
    struct.pack_into('<4sIII', block, 0, b'INGE', length, 0, 0)
    # nand_timing[4] stay 0
    for i, d in enumerate(PARAMS_DESCS):
        struct.pack_into('<5I', block, 32 + 20 * i, *d)
    assert 32 + 20 * PARAMS_NDESCS <= PARAMS_LENGTH
    return block


def main():
    args = sys.argv[1:]
    pad_block = '--pad-block' in args
    params = '--params' in args
    nand = '--nand' in args
    unpacked = [a.split('=', 1)[1] for a in args if a.startswith('--unpacked=')]
    slot, env, pad = (NAND_SPL_SLOT, NAND_ENV_OFFSET, b'\xff') if nand \
        else (SPL_SLOT, ENV_OFFSET, b'\x00')
    args = [a for a in args if not a.startswith('--')]
    spl_path, uboot_path, out_path = args[:3]
    spl = bytearray(open(spl_path, 'rb').read())

    if pad_block and len(spl) % 64:
        spl += b'\xff' * (64 - len(spl) % 64)
    if params:
        spl[PARAMS_OFFSET:PARAMS_OFFSET + PARAMS_LENGTH] = spl_params(len(spl))
    uboot = open(uboot_path, 'rb').read()
    # The SPL unpacks U-Boot from its link address (0x80100000) up to the
    # packed copy it reads to 0x80200000 (spl.c INGENIC_UNPACK_MAX)
    for u in unpacked:
        if os.path.getsize(u) > UNPACK_MAX:
            sys.exit('%s is %d bytes, more than the SPL unpacks (%d)' %
                     (u, os.path.getsize(u), UNPACK_MAX))

    if nand:
        # bytes 8..11: CRC7 (0 here), 0, pages per block / 32, bytes per
        # page / 1024
        if spl[:10] != bytes.fromhex('060504030255aa550000'):
            sys.exit('%s: no SFC NAND boot header' % spl_path)
    elif spl[:12] != bytes.fromhex('060504030255aa55aa000000'):
        sys.exit('%s: no SFC NOR boot header' % spl_path)
    if len(spl) > slot:
        sys.exit('SPL is %d bytes, the slot %d' % (len(spl), slot))

    # spi_checksum read 4-byte chunks and skipped the first SKIP_SIZE bytes
    spl[CRC_POSITION] = crc7(spl[SKIP_SIZE:])
    struct.pack_into('<I', spl, SPL_LENGTH_POSITION, len(spl))

    image = bytes(spl) + pad * (slot - len(spl)) + uboot
    if len(image) > env:
        sys.exit('image is %d bytes, it would overwrite the environment at '
                 '0x%x' % (len(image), env))
    open(out_path, 'wb').write(image)


if __name__ == '__main__':
    main()
