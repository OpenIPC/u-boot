#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Put the Anyka AK39 boot ROM's header in front of u-boot.bin.

  ak39-image.py ev200|ev300 <script.rvs> <u-boot.bin> <text_base> <out>
  ak39-image.py ev330 <boot.bin> <u-boot.bin> <text_base> <out>

EV200 and EV300: the ROM reads the 512-byte header at flash offset 0,
replays its register script (PLLs, DDR controller), copies the payload that
follows the header to the load address and starts it there.

  +0x000  ARM branch to the payload, for a ROM that enters at the header
  +0x004  magic: AKSH_240 (EV200), ANYKA_H3 (EV300)
  +0x00c  payload length
  +0x01c  6, as in every header seen
  +0x020  load address
  +0x02c  {address, value} pairs; 0x66668888 = delay, 0x88888888 = end
  +0x18c  SPIP: the SPI NOR descriptor (JEDEC ID, size, page, erase size)

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

DELAY, END = 0x66668888, 0x88888888

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
}


def script(path):
    out = b""
    for line in open(path):
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        if line == "end":
            out += struct.pack("<II", END, END)
            break
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
        hdr = bytearray(HEADER_LEN)
        # b <header + 0x200>, i.e. the payload
        struct.pack_into("<I", hdr, 0x00, 0xEA000000 | (HEADER_LEN - 8) >> 2)
        hdr[0x04:0x0C] = magic
        struct.pack_into("<I", hdr, 0x0C, len(uboot))
        struct.pack_into("<I", hdr, 0x1C, 6)
        struct.pack_into("<I", hdr, 0x20, text)
        body = script(data)
        hdr[SCRIPT_OFF:SCRIPT_OFF + len(body)] = body
        hdr[SPIP_OFF:SPIP_OFF + len(spip)] = spip
        image = bytes(hdr) + uboot

    open(out, "wb").write(image)
    print(f"{out}: {len(image)} bytes, U-Boot at {text:#x}")


if __name__ == "__main__":
    main()
