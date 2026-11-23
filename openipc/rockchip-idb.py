#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Pack a Rockchip ID block, new format (RKNS, header v2), from the DDR-init
blob and the SPL, as rkbin's boot_merger does for the RV11xx (NEWIDB=true,
RC4 off) and mainline mkimage's rkcommon.c (header0_info_v2) for the
RK3568/RK3588.

  rockchip-idb.py DDR_BLOB SPL OUT

The mask ROM reads the 2 KiB header, runs the first image (the DDR blob)
from SRAM, returns, then loads the second (the SPL) into DRAM and starts
it. Layout:

  0x000 magic "RKNS"       0x008 images << 16 | hash offset / 4 (384)
  0x00c boot flag: hash type (1: SHA-256)
  0x078 4 entries of 88 bytes: size << 16 | offset (512-byte sectors),
        load address (0xffffffff: the ROM's choice), flag, counter,
        8 reserved bytes, the image's hash (64 bytes)
  0x600 the header's hash over bytes 0..0x5ff

Each image starts on a sector and is padded to 2 KiB, its hash taken over
the padded size.
"""
import hashlib
import struct
import sys

SECTOR = 512
ALIGN = 2048
HDR = 2048
HASH_OFF = 0x600
ENTRY_OFF = 0x78
ENTRY_SIZE = 88


def pad(data):
    return data + b"\0" * (-len(data) % ALIGN)


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    images = [pad(open(p, "rb").read()) for p in sys.argv[1:3]]
    hdr = bytearray(HDR)
    struct.pack_into("<4s4xII", hdr, 0, b"RKNS",
                     len(images) << 16 | HASH_OFF // 4, 1)
    sector = HDR // SECTOR
    for i, img in enumerate(images):
        count = len(img) // SECTOR
        if count > 0xffff or sector > 0xffff:
            sys.exit("rockchip-idb: image %d too big" % i)
        struct.pack_into("<IIII8x64s", hdr, ENTRY_OFF + i * ENTRY_SIZE,
                         count << 16 | sector, 0xffffffff, 0, i + 1,
                         hashlib.sha256(img).digest())
        sector += count
    hdr[HASH_OFF:HASH_OFF + 32] = hashlib.sha256(hdr[:HASH_OFF]).digest()
    with open(sys.argv[3], "wb") as f:
        f.write(bytes(hdr) + b"".join(images))


if __name__ == "__main__":
    main()
