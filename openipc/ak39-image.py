#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Put the Anyka AK39 boot ROM's header in front of u-boot.bin.

  ak39-image.py ev200|ev300|ev300l|av100 <script.rvs> <u-boot.bin> <text_base> <out>
  ak39-image.py ev330 <boot.bin> <u-boot.bin> <text_base> <out>

EV200, EV300, EV300L and AV100: the ROM reads the 512-byte header at flash
offset 0, replays its register script (PLLs, DDR controller), copies the
payload that follows the header to the load address and starts it there.

  +0x000  ARM branch to the payload, for a ROM that enters at the header
  +0x004  magic: AKSH_240 (EV200), ANYKA_H3 (EV300), AK_H322L (EV300L),
          ANYKA322 (AV100)
  +0x00c  payload length
  +0x010  EV300L, AV100: 5 (the SDK's "spi divide")
  +0x018  EV300L, AV100: 1 (the SDK's "single read mode")
  +0x01c  6, as in every header seen ("use outside RAM"; the AV100 SDK's
          ak_secureboot writes 0x84000000 into its signed images)
  +0x020  load address
  +0x024  EV300L, AV100: 1 (the SDK's "use CPU mode", not DMA)
  +0x02c  {address, value} pairs; 0x66668888 = delay, 0x66668881 = poll
          {address, value}, 0x88888888 = end
  +0x18c  SPIP: the SPI NOR descriptor (JEDEC ID, size, page, erase size);
          just the tag on the EV300L flash seen and in the AV100 SDK's
          vectors.S

AV100: U-Boot starts at 0x600, not 0x200. The 0x400 bytes between are the
secure-boot area (ak_secureboot: public key at 0x200, signature at 0x300),
left zero here as the SDK's vectors.S leaves them in an unsigned image. The
ROM still copies from 0x200, so the header covers that area too: load
address <text_base> - 0x400 and length from 0x200, and U-Boot lands at
<text_base>, the ROM entering through the zeroes (andeq). That is the
vendor's header on an AK3918AV100N's flash (load 0x80DFFC00, owner report
r-s9hgnp86 on openipc.org) and on the AK3918AV130's. Load <text_base> with
the length from 0x600, as this script wrote before, put U-Boot 0x400 too
high on the chip and left its last 0x400 bytes uncopied: no output at all.

EV330: the ROM loads a 4 KiB first stage into L2 memory, which runs the DDR
script of its own header and copies 0x40000 bytes from flash offset 0x1200
to 0x80E01000. <boot.bin> is those first 0x1200 bytes, header and first
stage, as on the vendor's flash; U-Boot follows it.
"""

import struct
import sys

HEADER_LEN = 0x200
SCRIPT_OFF, SCRIPT_END = 0x2C, 0x18C
SPIP_OFF = 0x18C

DELAY, POLL, END = 0x66668888, 0x66668881, 0x88888888

# EV330: where U-Boot goes in flash, and how much of it the first stage loads
EV330_PREFIX = 0x1200
EV330_LOAD_LEN = 0x40000

FAMILY = {
    # The descriptors of the flash on the camera each script was read from
    "ev200": (b"AKSH_240", bytes.fromhex(
        "53504950ef4017000000800000010000000100000010000040787d0100000035"
        "77323571363400")),
    "ev300": (b"ANYKA_H3", bytes.fromhex(
        "53504950c840170000008000000100000001000000100000005a6202f8000036"
        "67643235713634")),
    # The gzhongshi HVT3610S3 camera's flash: the tag, nothing after it
    "ev300l": (b"AK_H322L", b"SPIP"),
    # The AV100 SDK's vectors.S: the tag, nothing after it
    "av100": (b"ANYKA322", b"SPIP"),
}

# Header words other than length and load address, by family
EXTRA = {
    "ev300l": {0x10: 5, 0x18: 1, 0x24: 1},
    "av100": {0x10: 5, 0x18: 1, 0x24: 1},
}

# Where U-Boot starts in flash, if not right after the header. The ROM
# copies from HEADER_LEN on, whatever lies between.
PAYLOAD_OFF = {
    "av100": 0x600,
}


def script(path):
    out = b""
    for line in open(path):
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        words = line.split()
        if words[0] == "end":
            last = int(words[1], 0) if len(words) > 1 else END
            out += struct.pack("<II", END, last)
            break
        if words[0] == "poll":
            out += struct.pack("<III", POLL, int(words[1], 0),
                               int(words[2], 0))
            continue
        if words[0] == "raw":
            out += struct.pack("<II", int(words[1], 0), int(words[2], 0))
            continue
        if line.startswith("delay"):
            out += struct.pack("<II", DELAY, int(line.split()[1], 0))
            continue
        addr, value = line.split("=")
        out += struct.pack("<II", int(addr, 0), int(value, 0))
    else:
        sys.exit(f"{path}: no 'end'")
    if SCRIPT_OFF + len(out) > SCRIPT_END:
        sys.exit(f"{path}: script runs into the SPIP descriptor")
    return out


def main():
    if len(sys.argv) != 6:
        sys.exit(__doc__)
    family, data, payload, text, out = sys.argv[1:]
    text = int(text, 0)
    uboot = open(payload, "rb").read()

    if family == "ev330":
        prefix = open(data, "rb").read()
        if len(prefix) != EV330_PREFIX or prefix[0x204:0x20C] != b"ANYKAH3B":
            sys.exit(f"{data}: not an EV330 header and first stage")
        if text != 0x80E01000:
            sys.exit(f"the first stage loads to 0x80E01000, not {text:#x}")
        if len(uboot) > EV330_LOAD_LEN:
            sys.exit(f"{payload}: larger than the first stage loads")
        image = prefix + uboot
    else:
        magic, spip = FAMILY[family]
        start = PAYLOAD_OFF.get(family, HEADER_LEN)
        hdr = bytearray(start)
        # b <payload>
        struct.pack_into("<I", hdr, 0x00, 0xEA000000 | (start - 8) >> 2)
        hdr[0x04:0x0C] = magic
        gap = start - HEADER_LEN
        struct.pack_into("<I", hdr, 0x0C, gap + len(uboot))
        struct.pack_into("<I", hdr, 0x1C, 6)
        struct.pack_into("<I", hdr, 0x20, text - gap)
        for off, val in EXTRA.get(family, {}).items():
            struct.pack_into("<I", hdr, off, val)
        body = script(data)
        hdr[SCRIPT_OFF:SCRIPT_OFF + len(body)] = body
        hdr[SPIP_OFF:SPIP_OFF + len(spip)] = spip
        image = bytes(hdr) + uboot

    open(out, "wb").write(image)
    print(f"{out}: {len(image)} bytes, U-Boot at {text:#x}")


if __name__ == "__main__":
    main()
