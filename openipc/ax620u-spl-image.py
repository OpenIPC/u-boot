#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Lay out the start of the Axera AX620U SPI NAND `spl` partition.

    ax620u-spl-image.py <spl.bin> <key.pem> <out>
    ax620u-spl-image.py --signed <stock.bin> <out>
    ax620u-spl-image.py --check <flash dump> <key.pem>

The boot ROM reads flash 0: a 1 KiB header, the same header again at
0x400, and the first stage's code at 0x800, which it loads to on-chip RAM
0 as it is in flash and runs at 0x800 (openipc/ax620u-spl). The cameras'
`spl` partition (1 MiB at 0) has the code once more at 0x20000, the
header's first backup address, without a header. 0x40000 holds 0x12ac0
bytes whose size and word sum are the header's firmware fields (the
EIP-130 firmware, which only parts with secure boot fused load, as on the
AX620E), 0x60000 the same again, zeros after.
The output is the first two 128 KiB blocks, as the cameras have them
(zeros between): header, header, code, then the code at 0x20000. It
leaves the firmware blocks alone, so writing it at flash 0 changes only
what this first stage replaces.

The header, as the cameras' first stage has it:

  0x000 word sum of bytes 0x8-0x3f8      0x004 magic 0x55543322
  0x008 capability 0x7afe                0x00c code size
  0x010 firmware size 0x12ac0            0x014 word sum of the code
  0x018 firmware word sum 0xdbb7c627     0x01c SPI NAND geometry
  0x020 0x20000 (the code's backup), 0x40000 and 0x60000 (the firmware's)
  0x02c 0x02000800, RSA-2048 modulus at 0x030 (little endian),
  0x130 0x02010020, exponent at 0x134,
  0x1b8 0x01000800, PKCS#1 v1.5 SHA-256 signature of the code at 0x1bc
        (little endian)

The firmware fields describe the cameras' firmware, which stays in their
flash; this first stage does not boot secure boot parts anyway. The key
is the Axera SDK's default test key, which signs the cameras' first stage
too: given its code, this writes its header byte for byte (--check).
Parts with secure boot fused check the key's hash and the signature, the
others the sums.

--signed takes a first stage that has its headers already, flash 0 up to
the end of its code as the cameras have it (openipc/ax620u-spl/stock.bin),
checks them and lays it out the same way.
"""
import base64
import hashlib
import struct
import sys

HEADER_SIZE = 0x400
CODE = 0x800
BACKUP = 0x20000
OUT_SIZE = 0x40000          # two 128 KiB blocks: up to the firmware
MAGIC = 0x55543322
CAPABILITY = 0x7afe
FW_SIZE = 0x12ac0
FW_SUM = 0xdbb7c627
NAND_CFG = 0x06010504       # 2 KiB page, 128 KiB block, 64 B OOB, 1 BBM byte
BACKUPS = (BACKUP, 0x40000, 0x60000)
CODE_MAX = 0x13298 - CODE   # the secondaries' check-in words (bl1.lds)
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


def header(code, n, e, d):
    h = bytearray(HEADER_SIZE)
    struct.pack_into('<11I', h, 4, MAGIC, CAPABILITY, len(code), FW_SIZE,
                     word_sum(code), FW_SUM, NAND_CFG, *BACKUPS, 0x02000800)
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


def layout(hdr, code):
    if len(code) > CODE_MAX:
        raise ValueError('%d bytes of code, more than the %d the on-chip RAM '
                         'has for it' % (len(code), CODE_MAX))
    img = bytearray(OUT_SIZE)
    img[:HEADER_SIZE] = hdr
    img[HEADER_SIZE:CODE] = hdr
    img[CODE:CODE + len(code)] = code
    img[BACKUP:BACKUP + len(code)] = code
    return bytes(img)


def main():
    args = sys.argv[1:]
    if len(args) != 3:
        sys.exit(__doc__.split('\n\n')[1])
    try:
        if args[:1] == ['--check'] and len(args) == 3:
            flash = open(args[1], 'rb').read(BACKUP)
            size = struct.unpack_from('<I', flash, 0xc)[0]
            h = header(flash[CODE:CODE + size], *load_key(args[2]))
            same = h == flash[:HEADER_SIZE] and h == flash[HEADER_SIZE:CODE]
            print('header reproduced' if same else 'header differs')
            sys.exit(0 if same else 1)
        if args[:1] == ['--signed'] and len(args) == 3:
            spl = open(args[1], 'rb').read()
            check_signed(spl)
            img = layout(spl[:HEADER_SIZE], spl[CODE:])
            what = 'signed first stage'
        elif len(args) == 3 and not args[0].startswith('-'):
            code = open(args[0], 'rb').read()
            code += b'\0' * (-len(code) % 16)
            img = layout(header(code, *load_key(args[1])), code)
            what = 'first stage'
        else:
            sys.exit(__doc__.split('\n\n')[1])
    except ValueError as err:
        sys.exit('%s: %s' % (args[1] if args[0].startswith('-') else args[0], err))
    out = args[2]
    open(out, 'wb').write(img)
    size = struct.unpack_from('<I', img, 0xc)[0]
    print('%s: %d bytes, the %s (%d bytes of code) at 0x%x and 0x%x' %
          (out, len(img), what, size, CODE, BACKUP))


if __name__ == '__main__':
    main()
