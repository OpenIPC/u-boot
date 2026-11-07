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

On SPI NAND (--nand), the SNI (--gcis) in page 0 and the PNI (--pni) in page 1,
then make_boot_spinand.sh's 2 MiB: IPL at 0x40000 and 0x80000, IPL_CUST at
0xC0000 and 0x100000, U-Boot at 0x140000 and 0x1A0000. That is what OpenIPC
publishes, whatever the PNI says: infinity6's table has its stages further up
and its images boot regardless. Nothing is published for infinity2m, so
--pni-layout puts each stage where its SDK's PNI says.
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

NAND_BOOT_SIZE = 0x200000
NAND_IPL_OFFS = (0x40000, 0x80000)
NAND_CUST_OFFS = (0xC0000, 0x100000)
NAND_UBOOT_OFFS = (0x140000, 0x1A0000)

# The PNI's partition types
PNI_IPL_CUST, PNI_IPL, PNI_UBOOT, PNI_ENV = 1, 3, 6, 13


def pni_layout(pni):
    """Each partition's copies, in bytes, from the IPL's own table."""
    spare, page, pages, blocks, count, unit = struct.unpack_from("<6H", pni, 4)
    block = page * pages
    recs = [struct.unpack_from("<4H", pni, 16 + i * 8) for i in range(count)]
    parts = {}
    for start, cnt, kind, backup in recs:
        # a partition holds its piece once per cnt blocks, backups included
        for k in range((cnt + backup) // cnt if cnt else 1):
            parts.setdefault(kind, []).append((start + k * cnt) * block)
    return parts


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
    with open(args.out, "wb") as f:
        f.write(boot)
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
    with open(args.out, "wb") as f:
        f.write(boot)
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
    with open(args.out, "wb") as f:
        f.write(boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 0x1C000))


def iford_nand(args):
    """
    Iford's SPI NAND, as the SDK's OnebinnandBurnImgConfig.cfg writes it: its
    cis.bin in each of the ten CIS blocks (page 0 SNIB with the part's SNIF,
    page 1 the PNI), then its boot.bin from 0x140000, IPL, IPL_CUST and
    OP-TEE three blocks each, with our uImage in UBOOT at 0x260000. The
    second boot copy stays empty, as the SDK leaves it.
    """
    cis = read(args.iford_nand_cis)
    stages = read(args.iford_nand_boot)
    payload = lzma.compress(read(args.uboot), format=lzma.FORMAT_XZ,
                            check=lzma.CHECK_CRC64, preset=6)
    tag = ("MVX4##%-4s" % args.tag).replace(" ", "#") + "openIPC#CM_UBT2110#XVM"
    img = uimage(payload, tag[:32], 0x23E00000)
    if len(img) > 0x100000 or len(stages) > 0x120000:
        sys.exit("Iford NAND: a stage does not fit")
    boot = bytearray(b"\xff" * 0x580000)
    for blk in range(10):
        boot[blk * 0x20000:blk * 0x20000 + len(cis)] = cis
    boot[0x140000:0x140000 + len(stages)] = stages
    boot[0x260000:0x260000 + len(img)] = img
    with open(args.out, "wb") as f:
        f.write(boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), 0x100000))


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
    ap.add_argument("--iford-nand-cis", help="Iford NAND: the SDK's cis.bin")
    ap.add_argument("--iford-nand-boot",
                    help="Iford NAND: IPL, IPL_CUST, OP-TEE from 0x140000")
    ap.add_argument("--nand", action="store_true")
    ap.add_argument("--pni", help="partition table, with --nand")
    ap.add_argument("--pni-layout", action="store_true")
    ap.add_argument("--sni-list", help="flash table for infinity6c, page 2")
    ap.add_argument("--nand-6c", action="store_true")
    ap.add_argument("--nand-i2m", action="store_true")
    ap.add_argument("--sni-part", type=int, default=1,
                    help="infinity6c: the flash list's record for page 0")
    ap.add_argument("--tag", required=True, help="die in the MVX tag, 4 chars")
    ap.add_argument("--uboot", required=True)
    ap.add_argument("--soc", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    if args.iford_nand_cis:
        return iford_nand(args)
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

    if args.nand:
        pni = read(args.pni)
        parts = {PNI_IPL: NAND_IPL_OFFS, PNI_IPL_CUST: NAND_CUST_OFFS,
                 PNI_UBOOT: NAND_UBOOT_OFFS, PNI_ENV: (NAND_BOOT_SIZE,)}
        if args.pni_layout:
            parts = pni_layout(pni)
        if args.nand_6c:
            # image/configs/general/spinand.*: three copies of each stage,
            # one block apart, then U-Boot, and the same again for BOOT_BAK
            ipls = [0x140000 + i * 0x20000 for i in range(3)]
            custs = [0x1A0000 + i * 0x20000 for i in range(3)]
            parts = {PNI_IPL: ipls + [o + 0x1C0000 for o in ipls],
                     PNI_IPL_CUST: custs + [o + 0x1C0000 for o in custs],
                     PNI_UBOOT: (0x200000, 0x3C0000), PNI_ENV: (0x4C0000,)}
        size = parts[PNI_ENV][0]
        uboot = parts[PNI_UBOOT]
        limit = min(uboot[1] - uboot[0], 0x100000)
        for what, have, room in (("SNI", len(tab), 0x800),
                                 ("PNI", len(pni), 0x800),
                                 ("UBOOT", len(img), limit)):
            if have > room:
                sys.exit("%s: %u bytes, it has %u" % (what, have, room))
        boot = bytearray(b"\xff" * size)
        boot[:len(tab)] = tab
        boot[0x800:0x800 + len(pni)] = pni
        if args.sni_list and args.nand_6c:
            sni_list = read(args.sni_list)
            boot[0x1000:0x1000 + len(sni_list)] = sni_list
        if args.nand_6c:
            # infinity6c's IPL wants a part's own SNI in page 0 (the SDK's
            # "nand probe" writes it from the list), with its boot block
            # fields and CRC32, and the CIS in each of the ten CIS blocks
            rec = bytearray(sni_list[args.sni_part * 512:
                                     (args.sni_part + 1) * 512])
            rec[0x2F:0x34] = tab[0x2F:0x34]
            struct.pack_into("<I", rec, 0x1DC, zlib.crc32(bytes(rec[:0x1DC])))
            boot[0:0x800] = bytes(rec).ljust(0x800, b"\xff")
            cis = bytes(boot[:0x1000 + len(sni_list)])
            for blk in range(1, 10):
                boot[blk * 0x20000:blk * 0x20000 + len(cis)] = cis
        for kind, data in ((PNI_IPL, ipl), (PNI_IPL_CUST, cust),
                           (PNI_UBOOT, img)):
            for off in parts[kind]:
                boot[off:off + len(data)] = data
        if args.nand_i2m:
            # the i2m SDK's "writecis ... 10 0 0 5": the part's record with
            # BL0PBA 10, BL1PBA and UBOOTPBA 0, and the PNI, in blocks 0-8
            sni_list = read(args.sni_list)
            rec = bytearray(sni_list[args.sni_part * 512:
                                     (args.sni_part + 1) * 512])
            rec[0x2E], rec[0x2F], rec[0x30] = 0, 10, 0
            struct.pack_into("<I", rec, 0x1DC, zlib.crc32(bytes(rec[:0x1DC])))
            for blk in range(0, 10, 2):
                o = blk * 0x20000
                boot[o:o + 0x800] = bytes(rec).ljust(0x800, b"\xff")
                boot[o + 0x800:o + 0x800 + len(pni)] = pni
        with open(args.out, "wb") as f:
            f.write(boot)
        print("%s: UBOOT %u of %u bytes" % (args.out, len(img), limit))
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
    with open(args.out, "wb") as f:
        f.write(boot)
    print("%s: UBOOT %u of %u bytes" % (args.out, len(img), UBOOT_SIZE))


if __name__ == "__main__":
    main()
