#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""axera-image.py <u-boot.bin> <out.bin> <limit>

Put u-boot.bin behind the 1 KiB Axera image header the AX620U vendor SPL
loads from the boot partition: header and image go to 0x40000000, the
SPL jumps to 0x40000400 (SYS_TEXT_BASE).

The SPL checks two 32-bit little-endian word sums: img_check_sum (+0x14)
over the image, and check_sum (+0x00) over the header from +0x08 to
+0x2bc. The header's other words are those of the stock AX620U U-Boot
(the capability and flash words the SPL reads); its RSA key and signature
are left zero, as an SPL without secure boot does not read them.

The output must fit <limit> bytes, the boot partition.
"""
import struct
import sys

HDR_SIZE = 0x400
MAGIC = 0x55543322

# +0x08..+0x2c as in the stock AX620U boot image
STOCK_WORDS = {
    0x08: 0x00007afe,	# capability
    0x1c: 0x06010504,	# nand_nor_cfg
    0x20: 0x000e0000,
    0x24: 0x001c0000,
    0x28: 0x001e0000,
    0x2c: 0x02000800,	# RSA-2048 key header
    0x130: 0x02010020,	# exponent header
    0x1b8: 0x01000800,	# signature header
}


def wsum(data):
    data += b'\0' * (-len(data) % 4)
    return sum(struct.unpack('<%dI' % (len(data) // 4), data)) & 0xffffffff


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    img = open(sys.argv[1], 'rb').read()
    limit = int(sys.argv[3], 0)
    if HDR_SIZE + len(img) > limit:
        sys.exit('%s: %d bytes with the header, over the %d-byte boot '
                 'partition' % (sys.argv[1], HDR_SIZE + len(img), limit))
    hdr = bytearray(HDR_SIZE)
    struct.pack_into('<I', hdr, 0x04, MAGIC)
    for off, val in STOCK_WORDS.items():
        struct.pack_into('<I', hdr, off, val)
    struct.pack_into('<I', hdr, 0x0c, len(img))
    struct.pack_into('<I', hdr, 0x14, wsum(img))
    struct.pack_into('<I', hdr, 0x00, wsum(bytes(hdr[0x08:0x2bc])))
    with open(sys.argv[2], 'wb') as f:
        f.write(bytes(hdr) + img)


if __name__ == '__main__':
    main()
