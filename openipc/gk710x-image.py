#!/usr/bin/env python3
"""Wrap u-boot.bin in the GK7101/GK7102 boot ROM header.

    gk710x-image.py <soc>.rvs u-boot.bin <load address> <out>

The mask ROM reads the header from flash offset 0, in CPU words:

    0, 1   "GK71" magic, version      (0x474B3731, 0x30310801)
    2      U-Boot length in words
    3      load address, where it also starts U-Boot
    4      the load address in words
    5, 6   DDR test address and pattern
    7..    the .rvs table: <register, value> to write, or 0xFFFFFFFE,
           <register, value> to wait for; 0 ends it

and U-Boot follows the 0. Words 0, 1 and 4-6 are as in every SDK prebuilt
U-Boot and camera flash seen.

The flash controller packs bytes MSB first, so the chip holds every 32-bit
word byte-swapped (drivers/mtd/spi/gk_sflash.c). The output is in that
chip order: a programmer writes it as it is, qemu takes it as flash, and
sf write -- the vendor U-Boot's and this one's -- recognises the swapped
magic at offset 0 and swaps it back.
"""
import re
import struct
import sys

MAGIC = 0x474B3731
VERSION = 0x30310801
DDR_TEST = (0xC4000000, 0x5A5A5A5A)
POLL = 0xFFFFFFFE


def table(path):
    words = []
    for line in open(path):
        m = re.match(r'\s*(setup|rdcom):\s*(0x[0-9a-fA-F]+)\s*=\s*'
                     r'(0x[0-9a-fA-F]+)', line)
        if not m:
            continue
        if m.group(1) == 'rdcom':
            words.append(POLL)
        words += [int(m.group(2), 16), int(m.group(3), 16)]
    if not words:
        sys.exit('%s: no setup lines' % path)
    return words + [0]


def main():
    rvs, src, load, dst = sys.argv[1], sys.argv[2], int(sys.argv[3], 0), \
        sys.argv[4]
    code = open(src, 'rb').read()
    code += b'\0' * (-len(code) % 4)
    head = [MAGIC, VERSION, len(code) // 4, load, load // 4, *DDR_TEST]
    head += table(rvs)
    image = struct.pack('<%dI' % len(head), *head) + code
    with open(dst, 'wb') as f:
        f.write(b''.join(image[i:i + 4][::-1]
                         for i in range(0, len(image), 4)))


if __name__ == '__main__':
    main()
