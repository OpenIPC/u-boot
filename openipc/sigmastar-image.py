#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""
Build the boot partition of a SigmaStar NOR flash from infinity2m on, the
layout of OpenIPC's vendor U-Boot (make_boot_spinor.sh):

  0x00000  IPL       the vendor's: trains the DDR, loads IPL_CUST
  0x09000  GCIS      the flash table, infinity6c only (--gcis)
  0x0F000  MXP       partition table, the others (--mxp)
  0x10000  IPL_CUST  the vendor's: finds UBOOT, unpacks it
  0x20000  UBOOT     U-Boot, xz-compressed in a legacy uImage
  ...      0xFF up to 0x40000, where the environment starts

IPL_CUST takes the uImage as the vendor's create_img.sh makes it: `xz -z`
data (preset 6, CRC64) under compression type "lzma", load and entry 0,
named by the 32-byte MVX tag. It unpacks it to where U-Boot is linked,
0x23E00000.

  sigmastar-image.py --ipl F --ipl-cust F (--mxp F | --gcis F)
                     --tag TAG --uboot u-boot.bin --soc SOC --out F

On SPI NAND (--nand), OpenIPC's 768k(boot),256k(env),-(ubi): the SNI (--gcis,
or for infinity2m --sni-list's record) in page 0 and the PNI (--pni, its
records rewritten) in page 1 of block 0, IPL in block 1, IPL_CUST in block 2,
U-Boot in blocks 3-5, one copy each; the environment at 0xC0000.
"""

import argparse
import lzma
import struct
import sys
import time
import zlib

BOOT_SIZE = 0x40000
GCIS_OFF, MXP_OFF = 0x9000, 0xF000
CUST_OFF, CUST_SIZE = 0x10000, 0xF000
UBOOT_OFF, UBOOT_SIZE = 0x20000, 0x20000

# OpenIPC's SPI NAND: 768k(boot),256k(env),-(ubi)
NAND_ENV_OFF, NAND_UBI_OFF = 0xC0000, 0x100000

# The PNI's partition types
PNI_IPL_CUST, PNI_IPL, PNI_UBOOT, PNI_ENV = 1, 3, 6, 13


def uimage(data, name, load=0):
    """A legacy uImage: ARM, OS U-Boot, type kernel, "lzma" (xz data)."""
    hdr = struct.pack(">IIIIIIIBBBB32s", 0x27051956, 0, int(time.time()),
                      len(data), load, load, zlib.crc32(data), 17, 2, 2, 3,
                      name.encode()[:32])
    hdr = hdr[:4] + struct.pack(">I", zlib.crc32(hdr)) + hdr[8:]
    return hdr + data


def iford(args, ipl, cust):
    """
    Iford's NOR, as its SDK's boot.bin: IPL at 0, the CIS area (flash.nri,
    the PNI, the flash list) at 0xE000, IPL_CUST at 0x18000, UBOOT at 0x1F000,
    up to OpenIPC's environment at 0x40000. Its IPL_CUST unpacks the uImage
    to the load address the header gives.
    """
    cis = read(args.iford_cis)
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    for what, size, room in (("IPL", len(ipl), 0xE000),
                             ("CIS", len(cis), 0xA000),
                             ("IPL_CUST", len(cust), 0x7000),
                             ("UBOOT", len(img), 0x21000)):
        if size > room:
            sys.exit("%s: %u bytes, it has %u" % (what, size, room))
    boot = bytearray(b"\xff" * BOOT_SIZE)
    boot[:len(ipl)] = ipl
    boot[0xE000:0xE000 + len(cis)] = cis
    boot[0x18000:0x18000 + len(cust)] = cust
    boot[0x1F000:0x1F000 + len(img)] = img
    write_image(args.out, boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 0x21000))


def ifado(args, ipl, cust):
    """
    Ifado's NOR, as Dahua's (IPC-HDW1433V-A-IL2): IPL at 0, the CIS area
    (the NRI records, the PNI at 0x9000, NRIB) at 0x8000, IPL_CUST at
    0x10000, and UBOOT, the PNI's, at 0x20000 up to the environment at
    0x40000. Its IPL_CUST unpacks the uImage to the load address it gives.
    """
    cis = read(args.ifado_cis)
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    for what, size, room in (("IPL", len(ipl), 0x8000),
                             ("CIS", len(cis), 0x8000),
                             ("IPL_CUST", len(cust), 0x10000),
                             ("UBOOT", len(img), 0x20000)):
        if size > room:
            sys.exit("%s: %u bytes, it has %u" % (what, size, room))
    boot = bytearray(b"\xff" * BOOT_SIZE)
    boot[:len(ipl)] = ipl
    boot[0x8000:0x8000 + len(cis)] = cis
    boot[0x10000:0x10000 + len(cust)] = cust
    boot[0x20000:0x20000 + len(img)] = img
    write_image(args.out, boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 0x20000))


def ifliegen(args, ipl, cust):
    """
    Ifliegen's NOR, as its SDK's image.mk and nor.squashfs.tfa.partition.config
    lay it out, at offsets that fit both packages' IPLs: IPL at 0, the CIS
    area at 0xC000 (flash.nri, the PNI at +0x1000, the flash list with the
    CRC32 nrigenerator adds at +0x2000), IPL_CUST at 0x16000, TF-A's SP_MIN
    at 0x1D000, UBOOT at 0x24000 up to OpenIPC's environment at 0x40000.
    The IPL starts TF-A, which runs IPL_CUST; that unpacks the uImage to the
    load address its header gives.
    """
    tfa = read(args.ifliegen_tfa)
    nri = read(args.ifliegen_nri)
    flist = bytearray(read(args.ifliegen_list))
    for i in range(0, len(flist), 1024):
        if flist[i + 0x100:i + 0x104] == b"NRIF":
            struct.pack_into("<I", flist, i + 0x104,
                             zlib.crc32(bytes(flist[i + 0x108:i + 0x400])))
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    # name, offset, room, data; flags trunk, backup trunk, active, group
    parts = (("IPL", 0x0, 0xC000, ipl), ("CIS", 0xC000, 0xA000, None),
             ("IPL_CUST", 0x16000, 0x7000, cust), ("TF_A", 0x1D000, 0x7000, tfa),
             ("UBOOT", 0x24000, 0x1C000, img))
    recs = b""
    for name, off, room, data in parts:
        recs += name.encode().ljust(16, b"\0") + bytes((0, 0, 1, 1)) + \
            struct.pack("<III", off, room, 0)
    recs += b"ENV".ljust(16, b"\0") + bytes((0, 0, 1, 0)) + \
        struct.pack("<III", BOOT_SIZE, 0x10000, 0)
    body = struct.pack("<I", len(recs)) + recs
    pni = b"SSTARSEMICIS0001" + struct.pack("<I", sum(body)) + body
    cis = bytearray(b"\xff" * 0xA000)
    if len(nri) > 0x1000 or len(pni) > 0x1000 or 0x2000 + len(flist) > len(cis):
        sys.exit("Ifliegen: the CIS does not fit")
    cis[0:len(nri)] = nri
    cis[0x1000:0x1000 + len(pni)] = pni
    cis[0x2000:0x2000 + len(flist)] = flist
    boot = bytearray(b"\xff" * BOOT_SIZE)
    for name, off, room, data in parts:
        data = bytes(cis) if data is None else data
        if len(data) > room:
            sys.exit("%s: %u bytes, it has %u" % (name, len(data), room))
        boot[off:off + len(data)] = data
    write_image(args.out, boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 0x1C000))


def bdma_crc32(buf, crc=0xFFFFFFFF, poly=0xEDB88320):
    """snigenerator's FLASH_IMPL_BDMA_SW_CRC32, the SNIB's own."""
    for b in buf:
        for i in range(8):
            if ((crc >> 31) ^ (b >> i)) & 1:
                crc = (((crc << 1) & 0xFFFFFFFF) >> 1) ^ (poly >> 1)
                crc = ((crc << 1) | 1) & 0xFFFFFFFF
            else:
                crc = (crc << 1) & 0xFFFFFFFF
    return crc


def snigen(data, bl0, bl1, page=0, spare=0):
    """What the SDK's snigenerator does to each 1 KiB SNI record: the boot
    blocks BL0PBA and BL1PBA (and one block per image, three backups one
    apart) in the SNIB and every SNIF, then their CRCs."""
    boot = bytes([bl0, bl1, 0, 3, 1])
    out = bytearray()
    for i in range(0, len(data) - 1023, 1024):
        r = bytearray(data[i:i + 1024])
        if r[0x100:0x104] != b"SNIF":
            break
        snib = r[0:4] == b"SNIB"
        if snib:
            r[0x3F:0x44] = boot
            if page:
                struct.pack_into("<H", r, 0x36, page)
            if spare:
                struct.pack_into("<H", r, 0x34, spare)
        r[0x127:0x12C] = boot
        if page:
            struct.pack_into("<H", r, 0x11A, page)
        if spare:
            struct.pack_into("<H", r, 0x118, spare)
        struct.pack_into("<I", r, 0x104, zlib.crc32(bytes(r[0x108:0x400])))
        if snib:
            struct.pack_into("<I", r, 4, bdma_crc32(bytes(r[8:0x80])))
        out += r
    return bytes(out)


def ifliegen_nand(args):
    """
    Ifliegen's SPI NAND (2048+64, 64 pages a block) in OpenIPC's 768 KiB
    boot area, one copy of everything: the CIS in block 0 (page 0 the SNIB
    and the part's SNIF, page 1 the PNI, then the flash list, with the boot
    block fields as snigenerator sets them: BL0PBA 1, no backup IPL), IPL in
    block 1, IPL_CUST in 2, TF-A's SP_MIN in 3, our uImage in 4 and 5; the
    environment at 0xC0000, UBI from 0x100000. The IPL takes its timeouts
    from the SNIF, so the CIS carries a real part's: the flash list's record
    --sni-part (57, a W25N01GV).
    """
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    blk = 0x20000
    stages = (("IPL", blk, blk, read(args.ipl)),
              ("IPL_CUST", 2 * blk, blk, read(args.ipl_cust)),
              ("TF_A", 3 * blk, blk, read(args.ifliegen_tfa)),
              ("UBOOT", 4 * blk, 2 * blk, img))
    flist = snigen(read(args.ifliegen_sni_list), 1, 0)
    sni = bytearray(snigen(read(args.ifliegen_sni), 1, 0, 2048, 64))
    rec = args.sni_part * 1024
    sni[0x100:0x400] = flist[rec + 0x100:rec + 0x400]
    sni = snib_fix(sni[:0x400], 1, 0)
    write_image(args.out, nand_cis_boot(args, sni, flist, stages))


def snib_fix(r, bl0, bl1):
    """A SNIB or SNIF record with one IPL at block bl0 and its copy at bl1,
    no further backups, and its CRCs again."""
    r = bytearray(r)
    boot = bytes([bl0, bl1, 0, 1, 1])
    if r[0:4] == b"SNIB":
        r[0x3F:0x44] = boot
    r[0x127:0x12C] = boot
    struct.pack_into("<I", r, 0x104, zlib.crc32(bytes(r[0x108:0x400])))
    if r[0:4] == b"SNIB":
        struct.pack_into("<I", r, 4, bdma_crc32(bytes(r[8:0x80])))
    return bytes(r)


def nand_cis_boot(args, sni, tail, stages, cis_copies=()):
    """
    The 768 KiB boot area of the dies whose PNI names its partitions
    (pnigenerator's SSTARSEMICIS0001): the CIS in block 0, page 0 the SNI,
    page 1 the PNI, the flash list after, and a copy of that block at each
    of cis_copies; each stage (name, offset, PNI size or None for its own
    rounded to a 2 KiB page, data) where its IPL looks it up by name, ahead
    of the environment at 0xC0000.
    """
    blk = 0x20000
    boot = bytearray(b"\xff" * NAND_ENV_OFF)
    parts = [("CIS", 0, blk, (0, 1, 1, 0))]
    spans = [(0, blk)] + [(o, o + blk) for o in cis_copies]
    for name, off, size, data in stages:
        size = size or (len(data) + 0x7FF) // 0x800 * 0x800
        if len(data) > size or off + size > NAND_ENV_OFF:
            sys.exit("%s: %u bytes at 0x%x do not fit" % (name, len(data), off))
        parts.append((name, off, size, (0, 1, 1, 1)))
        spans.append((off, off + size))
        boot[off:off + len(data)] = data
    spans.sort()
    for (_, end), (start, _) in zip(spans, spans[1:]):
        if end > start:
            sys.exit("NAND: the boot stages overlap at 0x%x" % start)
    parts += [("ENV", NAND_ENV_OFF, 0x40000, (0, 1, 1, 0)),
              ("UBI", NAND_UBI_OFF, 0x8000000 - NAND_UBI_OFF, (0, 1, 1, 0))]
    recs = b"".join(name.encode().ljust(16, b"\0") + bytes(flags) +
                    struct.pack("<III", o, size, 0)
                    for name, o, size, flags in parts)
    body = struct.pack("<I", len(recs)) + recs
    pni = b"SSTARSEMICIS0001" + struct.pack("<I", sum(body)) + body
    cis = bytearray(b"\xff" * 0x1000)
    cis[0:len(sni)] = sni
    cis[0x800:0x800 + len(pni)] = pni
    cis = bytes(cis) + bytes(tail)
    if len(cis) > blk:
        sys.exit("NAND: the CIS does not fit its block")
    for o in (0,) + tuple(cis_copies):
        boot[o:o + len(cis)] = cis
    name, off, size, _ = parts[len(stages)]
    print("%s: %s %u of %u bytes" % (args.out, name, len(stages[-1][3]),
                                     size))
    return boot


def infinity6c_nand(args, ipl, cust, img):
    """
    infinity6c's SPI NAND in OpenIPC's 768 KiB: the CIS in blocks 0 and 1
    (its IPL reads a copy past block 0; the SNI the flash list's record
    --sni-part, with BL0PBA 2, no backup IPL, and its CRC32), IPL in block 2,
    IPL_CUST in 3, U-Boot in 4-5.
    """
    sni_list = read(args.sni_list)
    sni = bytearray(sni_list[args.sni_part * 512:(args.sni_part + 1) * 512])
    sni[0x2F:0x34] = bytes((2, 0, 0, 1, 1))
    struct.pack_into("<I", sni, 0x1DC, zlib.crc32(bytes(sni[:0x1DC])))
    blk = 0x20000
    stages = (("IPL", 2 * blk, blk, ipl), ("IPL_CUST", 3 * blk, blk, cust),
              ("UBOOT", 4 * blk, 2 * blk, img))
    write_image(args.out, nand_cis_boot(args, sni, sni_list, stages, (blk,)))


def iford_nand(args):
    """
    Iford's SPI NAND in OpenIPC's 768 KiB: its IPL takes every stage by
    name from the PNI, at any 2 KiB page, which is what lets OP-TEE (338 KB)
    in. The CIS in block 0 (the SDK cis.bin's SNIB and SNIF with BL0PBA 1, no
    backup IPL, their CRCs again; the PNI; its flash list), then IPL from
    0x20000, IPL_CUST, OP-TEE and our uImage each from the next page after
    the last, U-Boot ending near 0xB8000. The stages are the SDK boot.bin's
    first copies: IPL at 0, IPL_CUST at 0x60000, OP-TEE over 0xC0000-0x120000.
    """
    cis = read(args.iford_nand_cis)
    stages_bin = read(args.iford_nand_boot)
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    pieces = (("IPL", stages_bin[0:0x20000]),
              ("IPL_CUST", stages_bin[0x60000:0x80000]),
              ("OPTEE", stages_bin[0xC0000:0x120000]), ("UBOOT", img))
    stages, off = [], 0x20000
    for name, data in pieces:
        data = data.rstrip(b"\xff") if name != "UBOOT" else data
        stages.append((name, off, None, data))
        off += (len(data) + 0x7FF) // 0x800 * 0x800
    sni = snib_fix(cis[:0x400], 1, 0)
    write_image(args.out, nand_cis_boot(args, sni, cis[0x1000:], stages))


def write_image(path, boot):
    """The boot area as far as it holds data: what follows is erased flash,
    and the image is no bigger than what it carries, as OpenIPC's others."""
    with open(path, "wb") as f:
        f.write(bytes(boot).rstrip(b"\xff"))


def read(path):
    with open(path, "rb") as f:
        return f.read()


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("--ipl")
    ap.add_argument("--ipl-cust")
    table = ap.add_mutually_exclusive_group(required=False)
    table.add_argument("--mxp")
    table.add_argument("--gcis")
    ap.add_argument("--iford-cis", help="Iford: CIS area, 0xE000-0x18000")
    ap.add_argument("--ifado-cis", help="Ifado: CIS area, 0x8000-0x10000")
    ap.add_argument("--ifliegen-tfa", help="Ifliegen: TF-A's u-bl32.bin")
    ap.add_argument("--ifliegen-nri", help="Ifliegen: flash.nri")
    ap.add_argument("--ifliegen-list", help="Ifliegen: flash_list.nri")
    ap.add_argument("--ifliegen-sni", help="Ifliegen NAND: flash.sni")
    ap.add_argument("--ifliegen-sni-list", help="Ifliegen NAND: flash_list.sni")
    ap.add_argument("--iford-nand-cis", help="Iford NAND: the SDK's cis.bin")
    ap.add_argument("--iford-nand-boot",
                    help="Iford NAND: IPL, IPL_CUST, OP-TEE from 0x140000")
    ap.add_argument("--nand", action="store_true")
    ap.add_argument("--pni", help="partition table, with --nand")
    ap.add_argument("--sni-list", help="flash table for infinity6c, page 2")
    ap.add_argument("--nand-i2m", action="store_true")
    ap.add_argument("--nand-6c", action="store_true")
    ap.add_argument("--sni-part", type=int, default=1,
                    help="infinity6c: the flash list's record for page 0")
    ap.add_argument("--tag", required=True, help="die in the MVX tag, 4 chars")
    ap.add_argument("--uboot", required=True)
    ap.add_argument("--soc", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    if args.iford_nand_cis:
        return iford_nand(args)
    if args.ifliegen_sni:
        return ifliegen_nand(args)
    ipl, cust = read(args.ipl), read(args.ipl_cust)
    if args.iford_cis:
        return iford(args, ipl, cust)
    if args.ifado_cis:
        return ifado(args, ipl, cust)
    if args.ifliegen_tfa:
        return ifliegen(args, ipl, cust)
    tab = read(args.gcis or args.mxp)
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT1501#XVM"
    img = uimage(payload, tag[:32])

    if args.nand and args.nand_6c:
        return infinity6c_nand(args, ipl, cust, img)
    if args.nand:
        # OpenIPC's 768 KiB boot area, one copy of everything: the SNI in
        # page 0 of block 0 and the PNI in page 1, IPL in block 1 (BL0PBA 1,
        # no backup IPL), IPL_CUST in 2, U-Boot in 3-5 up to the environment
        # at 0xC0000. SNI byte 0x2E is where infinity6's IPL_CUST starts its
        # search for U-Boot, the block after it; the later IPLs take the
        # PNI's records.
        blk = 0x20000
        if args.nand_i2m:
            # the part's record of the i2m flash list, with its CRC32
            sni = bytearray(read(args.sni_list)[args.sni_part * 512:
                                                (args.sni_part + 1) * 512])
        else:
            sni = bytearray(tab[:0x200])
        sni[0x2E], sni[0x2F], sni[0x30] = 2, 1, 0
        if args.nand_i2m:
            struct.pack_into("<I", sni, 0x1DC,
                             zlib.crc32(bytes(sni[:0x1DC])))
        # (start block, blocks, type, backups), the PNI header the SDK's
        recs = ((0, 1, 32, 0), (1, 1, PNI_IPL, 0), (2, 1, PNI_IPL_CUST, 0),
                (3, 3, PNI_UBOOT, 0), (6, 2, PNI_ENV, 0))
        pni = bytearray(read(args.pni)[:16])
        struct.pack_into("<H", pni, 12, len(recs))
        for r in recs:
            pni += struct.pack("<4H", *r)
        pni = pni.ljust(0x200, b"\0")
        struct.pack_into("<I", pni, 0, sum(pni[4:]))
        boot = bytearray(b"\xff" * NAND_ENV_OFF)
        boot[0:len(sni)] = sni
        boot[0x800:0x800 + len(pni)] = pni
        for start, data in ((1, ipl), (2, cust), (3, img)):
            room = (NAND_ENV_OFF if start == 3 else (start + 1) * blk) - \
                start * blk
            if len(data) > room:
                sys.exit("NAND: %u bytes at block %u, it has %u" %
                         (len(data), start, room))
            boot[start * blk:start * blk + len(data)] = data
        write_image(args.out, boot)
        print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 3 * blk))
        return

    tab_off = GCIS_OFF if args.gcis else MXP_OFF
    for what, size, limit in (("IPL", len(ipl), tab_off),
                              ("table", tab_off + len(tab), CUST_OFF),
                              ("IPL_CUST", len(cust), CUST_SIZE),
                              ("UBOOT", len(img), UBOOT_SIZE)):
        if size > limit:
            sys.exit("%s: %u bytes, it has %u" % (what, size, limit))

    boot = bytearray(b"\xff" * BOOT_SIZE)
    boot[:len(ipl)] = ipl
    boot[tab_off:tab_off + len(tab)] = tab
    boot[CUST_OFF:CUST_OFF + len(cust)] = cust
    boot[UBOOT_OFF:UBOOT_OFF + len(img)] = img
    write_image(args.out, boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), UBOOT_SIZE))


if __name__ == "__main__":
    main()
