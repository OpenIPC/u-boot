#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Lay out the Axera AX620U image: OpenIPC's boot partition, from 0.

    ax620u-image.py <spl.bin> <key.pem> <u-boot.img> <limit> <out>
    ax620u-image.py --stock <stock.bin> <key.pem> <u-boot.img> <limit> <out>
    ax620u-image.py --check <flash dump> <key.pem>

One image, written at flash 0 as it is, as OpenIPC's other NAND images:
the boot partition of 768k(boot),256k(env),-(ubi), in 128 KiB blocks:

  0x00000  the boot ROM header, the same header again at 0x400, and the
           first stage's code at 0x800, which the boot ROM loads to
           on-chip RAM 0 as it is in flash and runs at 0x800
           (openipc/ax620u-spl)
  0x20000  the code again, without a header: the header's backup
  0x40000  U-Boot behind its 1 KiB Axera header (u-boot.img,
           axera-image.py's output), which the first stage loads,
           skipping bad blocks up to the environment at 0xc0000

U-Boot may take three of the four blocks from 0x40000: the fourth is the
one bad block the first stage and `nand write` can skip. <limit> is the
boot partition's size from the target's mtdparts, which must be 768 KiB.

The cameras keep the EIP-130 security engine's firmware at 0x40000 and
0x60000, which the stock first stage loads only on parts with secure boot
fused; neither first stage here boots those, so the image has none and its
header says so (size, sum and places zero).

The first stage's header, as the cameras' first stage has it:

  0x000 word sum of bytes 0x8-0x3f8      0x004 magic 0x55543322
  0x008 capability 0x7afe                0x00c code size
  0x010 firmware size (0x12ac0)          0x014 word sum of the code
  0x018 firmware word sum (0xdbb7c627)   0x01c SPI NAND geometry
  0x020 0x20000 (the code's backup), (0x40000 and 0x60000, the firmware's)
  0x02c 0x02000800, RSA-2048 modulus at 0x030 (little endian),
  0x130 0x02010020, exponent at 0x134,
  0x1b8 0x01000800, PKCS#1 v1.5 SHA-256 signature of the code at 0x1bc
        (little endian)

The boot ROM's format, unlike U-Boot's header: it is signed with the Axera
SDK's default test key, as the cameras' first stage is; given its code and
the firmware's fields, this writes its header byte for byte (--check, on a
camera's flash dump). Parts with secure boot fused check the key's hash
and the signature, the others the sums.

--stock takes the cameras' own first stage (openipc/ax620u-spl/stock.bin:
flash 0 up to the end of its code, its SHA-256 checked), which loads
U-Boot from 0x100000 (the cameras' 1 MiB spl partition): six instructions
are changed to load it from 0x40000, at most eight blocks (STOCK_PATCHES),
and its header is written again for the changed code, with the same key.
"""
import base64
import hashlib
import struct
import sys

HEADER_SIZE = 0x400
CODE = 0x800
BACKUP = 0x20000
UBOOT = 0x40000             # U-Boot, as the first stage loads it
BOOT_PART = 0xc0000         # the boot partition; the environment follows
BLOCK = 0x20000
UBOOT_MAX = BOOT_PART - UBOOT - BLOCK   # a block left for a bad one
MAGIC = 0x55543322
CAPABILITY = 0x7afe
NAND_CFG = 0x06010504       # 2 KiB page, 128 KiB block, 64 B OOB, 1 BBM byte
CODE_MAX = 0x13298 - CODE   # the secondaries' check-in words (bl1.lds)
# The cameras' EIP-130 firmware fields (size, word sum, places), for --check
CAMERA_FW = (0x12ac0, 0xdbb7c627, 0x40000, 0x60000)
NO_FW = (0, 0, 0, 0)
STOCK_SHA256 = 'faad7357d752cce79e278b2cc074ed5f5091b78a92b2d065f544b968aa06ed33'
# stock.bin offset (= its on-chip RAM address): the Thumb-2 instruction
# there, and what it becomes. U-Boot's flash address is 0x100000 in five
# places: the bad-block map's alignment check and its message, its first
# block, the address the NAND boot loads U-Boot's header from, and its
# image's (0x100400, behind the header); the map's 16 blocks become 8
# (0x80000 of 64 KiB blocks, which a header of zeros gives, as in QEMU).
STOCK_PATCHES = (
    (0x6bde, 'f4031380', 'f4032380'),   # and.w r3, r3, #0x100000 -> #0x40000
    (0x6bf8, 'f44f1180', 'f44f2180'),   # mov.w r1, #0x100000 -> #0x40000
    (0x6c18, 'f44f1280', 'f44f2280'),   # mov.w r2, #0x100000 -> #0x40000
    (0x6c2c, '3310', '3308'),           # adds r3, #16 -> #8
    (0x9eec, 'f44f1380', 'f44f2380'),   # mov.w r3, #0x100000 -> #0x40000
    (0xa086, 'f2c00310', 'f2c00304'),   # movt r3, #0x10 -> #0x4: 0x40400
)
SHA256_PREFIX = bytes.fromhex('3031300d060960864801650304020105000420')


def der_ints(der):
    """The INTEGERs of a DER SEQUENCE (an RSA PKCS#1 private key)."""
    def length(pos):
        n = der[pos]
        if n < 0x80:
            return n, pos + 1
        k = n & 0x7f
        return int.from_bytes(der[pos + 1:pos + 1 + k], 'big'), pos + 1 + k

    if der[0] != 0x30:
        raise ValueError('not a DER sequence')
    _, pos = length(1)
    ints = []
    while pos < len(der):
        if der[pos] != 0x02:
            raise ValueError('not an RSA private key')
        n, pos = length(pos + 1)
        ints.append(int.from_bytes(der[pos:pos + n], 'big'))
        pos += n
    return ints


def load_key(path):
    lines = open(path).read().split('\n')
    if lines[0].strip() != '-----BEGIN RSA PRIVATE KEY-----':
        raise ValueError(path + ': not a PKCS#1 RSA private key')
    b64 = ''.join(l.strip() for l in lines[1:] if l and not l.startswith('-----'))
    _, n, e, d = der_ints(base64.b64decode(b64))[:4]
    if n.bit_length() != 2048:
        raise ValueError(path + ': the key must be RSA-2048')
    return n, e, d


def word_sum(data):
    """Sum of the little-endian 32-bit words, a trailing partial word left out."""
    words = struct.unpack('<%dI' % (len(data) // 4), data[:len(data) // 4 * 4])
    return sum(words) & 0xffffffff


def sign(data, n, d):
    k = (n.bit_length() + 7) // 8
    t = SHA256_PREFIX + hashlib.sha256(data).digest()
    em = b'\x00\x01' + b'\xff' * (k - len(t) - 3) + b'\x00' + t
    return pow(int.from_bytes(em, 'big'), d, n).to_bytes(k, 'big')


def header(code, n, e, d, fw=NO_FW):
    fw_size, fw_sum, fw_a, fw_b = fw
    h = bytearray(HEADER_SIZE)
    struct.pack_into('<11I', h, 4, MAGIC, CAPABILITY, len(code), fw_size,
                     word_sum(code), fw_sum, NAND_CFG, BACKUP, fw_a, fw_b,
                     0x02000800)
    h[0x30:0x130] = n.to_bytes(256, 'little')
    struct.pack_into('<I', h, 0x130, 0x02010020)
    h[0x134:0x138] = e.to_bytes(4, 'little')
    struct.pack_into('<I', h, 0x1b8, 0x01000800)
    h[0x1bc:0x2bc] = sign(code, n, d)[::-1]
    struct.pack_into('<I', h, 0, word_sum(bytes(h[8:HEADER_SIZE - 8])))
    return bytes(h)


def check_signed(spl):
    """Flash 0 up to the code's end: two equal headers, magic, length, sums."""
    if len(spl) < CODE:
        raise ValueError('shorter than its headers')
    h = spl[:HEADER_SIZE]
    if h != spl[HEADER_SIZE:CODE]:
        raise ValueError('the header at 0x400 is not the one at 0')
    magic, length = struct.unpack_from('<I4xI', h, 4)
    if magic != MAGIC:
        raise ValueError('no boot ROM header (magic 0x%08x)' % magic)
    if length != len(spl) - CODE:
        raise ValueError('the header says %d bytes of code, %d follow' %
                         (length, len(spl) - CODE))
    if struct.unpack_from('<I', h, 0)[0] != word_sum(h[8:HEADER_SIZE - 8]):
        raise ValueError('bad header checksum')
    if struct.unpack_from('<I', h, 20)[0] != word_sum(spl[CODE:]):
        raise ValueError('bad code checksum')


def thumb(hexstr):
    """A Thumb instruction as written in the disassembly, as flash bytes."""
    b = bytes.fromhex(hexstr)
    return b''.join(b[i:i + 2][::-1] for i in range(0, len(b), 2))


def patch_stock(spl):
    """stock.bin's code, loading U-Boot from UBOOT."""
    if hashlib.sha256(spl).hexdigest() != STOCK_SHA256:
        raise ValueError('not the cameras\' first stage (SHA-256 differs)')
    check_signed(spl)
    img = bytearray(spl)
    for off, old, new in STOCK_PATCHES:
        old, new = thumb(old), thumb(new)
        if img[off:off + len(old)] != old:
            raise ValueError('0x%x: not the instruction to change' % off)
        img[off:off + len(new)] = new
    return bytes(img[CODE:])


def layout(hdr, code, uboot, limit):
    if limit != BOOT_PART:
        raise ValueError('a %d KiB boot partition, the image wants %d' %
                         (limit // 1024, BOOT_PART // 1024))
    if len(code) > CODE_MAX:
        raise ValueError('%d bytes of code, more than the %d the on-chip RAM '
                         'has for it' % (len(code), CODE_MAX))
    if len(code) + CODE > BACKUP:
        raise ValueError('%d bytes of code do not fit a block' % len(code))
    if struct.unpack_from('<I', uboot, 4)[0] != MAGIC:
        raise ValueError('U-Boot\'s image has no Axera header')
    if len(uboot) > UBOOT_MAX:
        raise ValueError('U-Boot\'s %d bytes, %d over the %d that leave a '
                         'block for a bad one' % (len(uboot), len(uboot) - UBOOT_MAX,
                                                  UBOOT_MAX))
    img = bytearray(UBOOT)
    img[:HEADER_SIZE] = hdr
    img[HEADER_SIZE:CODE] = hdr
    img[CODE:CODE + len(code)] = code
    img[BACKUP:BACKUP + len(code)] = code
    img += uboot
    return bytes(img)


def main():
    args = sys.argv[1:]
    try:
        if args[:1] == ['--check'] and len(args) == 3:
            flash = open(args[1], 'rb').read(BACKUP)
            size = struct.unpack_from('<I', flash, 0xc)[0]
            h = header(flash[CODE:CODE + size], *load_key(args[2]), fw=CAMERA_FW)
            same = h == flash[:HEADER_SIZE] and h == flash[HEADER_SIZE:CODE]
            print('header reproduced' if same else 'header differs')
            sys.exit(0 if same else 1)
        if args[:1] == ['--stock'] and len(args) == 6:
            code = patch_stock(open(args[1], 'rb').read())
            key, uboot, limit, out = args[2:]
            what = 'the cameras\' first stage, loading U-Boot from 0x%x' % UBOOT
        elif len(args) == 5 and not args[0].startswith('-'):
            code = open(args[0], 'rb').read()
            code += b'\0' * (-len(code) % 16)
            key, uboot, limit, out = args[1:]
            what = 'first stage'
        else:
            sys.exit(__doc__.split('\n\n')[1])
        hdr = header(code, *load_key(key))
        uboot = open(uboot, 'rb').read()
        img = layout(hdr, code, uboot, int(limit, 0))
    except ValueError as err:
        sys.exit('%s: %s' % (args[-1], err))
    open(out, 'wb').write(img)
    print('%s: %d bytes: %s (%d bytes of code), U-Boot at 0x%x, %d bytes; '
          '%d left in its three blocks, a fourth for a bad one' %
          (out, len(img), what, len(code), UBOOT, len(uboot),
           UBOOT_MAX - len(uboot)))


if __name__ == '__main__':
    main()
