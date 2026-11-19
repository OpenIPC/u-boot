#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Lay out the Axera AX620E NOR boot partition.

    ax620e-image.py <spl> <u-boot.bin> <text-base> <ax_gzip> <key.pem>
                    <env-offset> <out>

The boot ROM runs the vendor's first stage from flash 0; it sets up the
DDR, then loads the image at 0x10000, unpacks its payload with the SoC's
GZIPD engine to 0x5C000400, past where the 1 KiB header would be at
0x5C000000, and starts it there. So the boot partition is:

  0x00000  the first stage as it is (its own header and code), zero padded
  0x10000  1 KiB image header, then the payload packed by the SDK's ax_gzip
           (Axera's own LZ77/Huffman format, which only that tool writes)

U-Boot runs from its link address, which on arm64 has to be 4 KiB aligned
(adrp addresses pages, so the relocation offset must be whole pages): the
payload starts with a branch to it, then U-Boot at that address.

The first stage reads the packed image in 64 KiB tiles and queues them to
the engine. With fewer than three tiles it queues the first one before it
has read all of it, and the engine unpacks 16 stale bytes at its end
(boot/bl1/core/boot/boot.c, gzip_pipeline_flash_read(); the vendor's
U-Boot images are all larger). A smaller image is padded: incompressible
filler goes into the payload past U-Boot's end, into RAM U-Boot does not
use before it relocates, until the packed image spans three tiles.

The header is the SDK's sec_boot_AX620E_sign.py's: checksums, capability
0x54FAFE (that of the vendor's U-Boot image), the RSA-2048 public key and
a PKCS#1 v1.5 SHA-256 signature of the packed image. Parts with secure boot
fused check the key's hash and the signature; others only the checksums.
The key is the SDK's default test key, which signs the vendor's first
stage as well.
"""

import hashlib
import os
import shutil
import struct
import subprocess
import sys
import tempfile

HEADER_SIZE = 0x400
UBOOT_OFFSET = 0x10000
ENTRY = 0x5C000400
MAGIC = 0x55543322
CAPABILITY = 0x54FAFE
SHA256_PREFIX = bytes.fromhex('3031300d060960864801650304020105000420')
TILE = 0x10000
# Three tiles: the 16-byte stream header, two whole tiles and the 8 KiB
# the first stage needs to count a third (gzipd_dev_cfg())
MIN_PACKED = 16 + 2 * TILE + 0x2000


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
    import base64
    lines = open(path).read().split('\n')
    if lines[0].strip() != '-----BEGIN RSA PRIVATE KEY-----':
        raise ValueError(path + ': not a PKCS#1 RSA private key')
    b64 = ''.join(l.strip() for l in lines[1:] if l and not l.startswith('-----'))
    _, n, e, d = der_ints(base64.b64decode(b64))[:4]
    return n, e, d


def sign(data, n, d):
    k = (n.bit_length() + 7) // 8
    t = SHA256_PREFIX + hashlib.sha256(data).digest()
    em = b'\x00\x01' + b'\xff' * (k - len(t) - 3) + b'\x00' + t
    return pow(int.from_bytes(em, 'big'), d, n).to_bytes(k, 'big')


def word_sum(data):
    """Sum of the little-endian 32-bit words, a trailing partial word left out."""
    words = struct.unpack('<%dI' % (len(data) // 4), data[:len(data) // 4 * 4])
    return sum(words) & 0xffffffff


def header(image, n, e, d):
    if n.bit_length() != 2048:
        raise ValueError('the key must be RSA-2048')
    h = bytearray(HEADER_SIZE)
    struct.pack_into('<IIIII', h, 4, MAGIC, CAPABILITY, len(image), 0,
                     word_sum(image))
    struct.pack_into('<I', h, 44, 0x02000800)        # key n: 2048 bits
    h[48:48 + 256] = n.to_bytes(256, 'little')
    struct.pack_into('<I', h, 432, 0x02010020)       # key e: 32 bits
    h[436:440] = e.to_bytes(4, 'little')
    struct.pack_into('<I', h, 440, 0x01000800)       # signature: 2048 bits
    h[444:444 + 256] = sign(image, n, d)[::-1]
    struct.pack_into('<I', h, 0, word_sum(bytes(h[8:HEADER_SIZE - 8])))
    return bytes(h)


def payload(uboot, text_base):
    """U-Boot behind a branch from the entry point to its link address."""
    gap = text_base - ENTRY
    if gap < 0 or gap % 4 or gap >= 1 << 27:
        raise ValueError('text base 0x%x: not reachable from 0x%x' %
                         (text_base, ENTRY))
    if gap == 0:
        return open(uboot, 'rb').read()
    branch = struct.pack('<I', 0x14000000 | (gap // 4))      # b text_base
    return branch.ljust(gap, b'\0') + open(uboot, 'rb').read()


def filler(n):
    """n bytes of a SHA-256 counter stream: incompressible, reproducible."""
    out = b''.join(hashlib.sha256(struct.pack('<I', i)).digest()
                   for i in range((n + 31) // 32))
    return out[:n]


def ax_gzip(tool, data):
    with tempfile.TemporaryDirectory() as tmp:
        src = os.path.join(tmp, 'u-boot.bin')
        open(src, 'wb').write(data)
        subprocess.run([tool, '-9', src], cwd=tmp, check=True,
                       stdout=subprocess.DEVNULL)
        return open(os.path.join(tmp, 'u-boot_axgzip.bin'), 'rb').read()


def main():
    if len(sys.argv) != 8:
        sys.exit(__doc__.split('\n\n')[1])
    spl_path, uboot, text_base, tool, key, env_off, out = sys.argv[1:]
    env_off = int(env_off, 0)

    spl = open(spl_path, 'rb').read()
    if len(spl) > UBOOT_OFFSET:
        sys.exit('%s: %d bytes, more than its 64 KiB' % (spl_path, len(spl)))

    data = payload(uboot, int(text_base, 0))
    pad = 0
    packed = ax_gzip(tool, data)
    while len(packed) < MIN_PACKED:
        pad += MIN_PACKED - len(packed) + 256
        packed = ax_gzip(tool, data + filler(pad))
    n, e, d = load_key(key)
    image = spl.ljust(UBOOT_OFFSET, b'\0') + header(packed, n, e, d) + packed

    if len(image) > env_off:
        sys.exit('%s: %d bytes, past the environment at 0x%x (%d over)' %
                 (out, len(image), env_off, len(image) - env_off))
    open(out, 'wb').write(image)
    print('%s: %d bytes, U-Boot packed to %d%s, %d left before the '
          'environment' % (out, len(image), len(packed),
                           ' (%d bytes of filler)' % pad if pad else '',
                           env_off - len(image)))


if __name__ == '__main__':
    main()
