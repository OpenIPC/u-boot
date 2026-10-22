#!/usr/bin/env python3
"""Wrap u-boot.bin in the GK710x boot ROM header.

    gk710x-image.py <soc>.rvs u-boot.bin <load address> <out> [<limit>]

With a limit (the environment's offset), an image that would reach it is
refused: header and U-Boot have to end before the environment sector.

The mask ROM reads the header from flash offset 0, in CPU words:

    0, 1   "GK71" magic, version      (0x474B3731, e.g. 0x30310801)
    2      U-Boot length in words
    3      load address, where it also starts U-Boot
    4      the load address in words
    5, 6   DDR test address and pattern
    7..    the .rvs table: <register, value> to write, 0xFFFFFFFE,
           <register, value> to wait for, or 0xFFFFFFFF, <count> to
           wait; 0 ends it

and U-Boot follows the 0. The .rvs file gives the version and the DDR test
words ("version:", "ddrtest:") and the table ("setup:", "rdcom:",
"delay:"), as the header of the vendor image it came from has them.

The output is in CPU order, like every other published file: tftp, mmc or
usb load it and sf write puts it at 0. The chip holds every 32-bit word
byte-swapped (drivers/mtd/spi/gk_sflash.c), so a programmer, or qemu's
flash-file, needs every word swapped (the openipc branch's README shows
how).
"""
import re
import struct
import sys

MAGIC = 0x474B3731
POLL = 0xFFFFFFFE
DELAY = 0xFFFFFFFF


def rvs(path):
    """(version, ddr test address, ddr test pattern, table words)"""
    version, ddrtest, words = None, None, []
    for line in open(path):
        line = line.split('#')[0].split('//')[0].strip()
        m = re.match(r'(setup|rdcom|ddrtest):\s*(0x[0-9a-fA-F]+)\s*=\s*'
                     r'(0x[0-9a-fA-F]+)$', line)
        if m:
            a, v = int(m.group(2), 16), int(m.group(3), 16)
            if m.group(1) == 'ddrtest':
                ddrtest = (a, v)
                continue
            if m.group(1) == 'rdcom':
                words.append(POLL)
            words += [a, v]
            continue
        m = re.match(r'(version|delay):\s*(0x[0-9a-fA-F]+)$', line)
        if m and m.group(1) == 'version':
            version = int(m.group(2), 16)
        elif m:
            words += [DELAY, int(m.group(2), 16)]
        elif line:
            sys.exit('%s: cannot parse "%s"' % (path, line))
    if version is None or ddrtest is None or not words:
        sys.exit('%s: needs version:, ddrtest: and setup lines' % path)
    return version, ddrtest[0], ddrtest[1], words + [0]


def main():
    path, src, load, dst = sys.argv[1], sys.argv[2], int(sys.argv[3], 0), \
        sys.argv[4]
    limit = int(sys.argv[5], 0) if len(sys.argv) > 5 else None
    version, test_addr, test_val, table = rvs(path)
    code = open(src, 'rb').read()
    code += b'\0' * (-len(code) % 4)
    head = [MAGIC, version, len(code) // 4, load, load // 4, test_addr,
            test_val] + table
    image = struct.pack('<%dI' % len(head), *head) + code
    if limit is not None and len(image) > limit:
        sys.exit('%s: %d bytes, %d over the %d bytes before the environment'
                 % (dst, len(image), len(image) - limit, limit))
    with open(dst, 'wb') as f:
        f.write(image)


if __name__ == '__main__':
    main()
