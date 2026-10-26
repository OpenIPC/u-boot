#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""
The Fullhan boot image: the mask ROM's header, its DDR scripts, the
vendor's first stage where the generation has one, and U-Boot.

The header at flash 0 is "2BL*OABD", then 64-byte records from 0x100,
parameter (DDR script) records first, then images:

  +0x00 name[16]   +0x10..0x1C four words (package ID, compression,
  unpadded size)   +0x20 size  +0x24 flash offset  +0x28 load
  +0x2C entry      +0x30 type  +0x34 ~crc32 of the data  +0x38 +0x3C

The ROM runs the DDR script whose package ID matches the chip, then loads
and enters the type-0x10 image: U-Boot itself, or a ramboot in SRAM that
loads the type-0x11 U-Boot. openipc/<generation>.boot holds a camera's or
SDK image's header as text (fullhan-image.py extract wrote it); the build
lays it out again with this U-Boot:

  fullhan-image.py <gen.boot> <u-boot.bin> <text base> <env offset> <out>
  fullhan-image.py extract <flash image> <out.boot> [first-stage file]
"""

import os
import struct
import sys
import zlib

SECTOR = 0x1000
REC = 0x100


def crc(data):
    return ~zlib.crc32(data) & 0xffffffff


def align(n, a):
    return (n + a - 1) & ~(a - 1)


def rec_fields(img, r):
    name = img[r:r + 16].split(b'\0')[0].decode()
    w = struct.unpack_from('<12I', img, r + 0x10)
    return name, w


def extract(flash_path, out_path, stage_path=None):
    """Write a .boot description of a flash image's header and scripts"""
    img = open(flash_path, 'rb').read()
    if img[:8] != b'2BL*OABD':
        sys.exit('%s: no Fullhan boot header' % flash_path)
    _, nimg, npar, hsz = struct.unpack_from('<4I', img, 8)
    out = ['# Fullhan boot header, from %s' % os.path.basename(flash_path),
           '# root: the first 0x100 bytes as they are; the record counts',
           '# and header size are recomputed']
    for i in range(0, REC, 32):
        out.append('root %s' % img[i:i + 32].hex())
    for k in range(npar + nimg):
        r = REC + 0x40 * k
        name, w = rec_fields(img, r)
        pid, comp, osz, w1c, size, off, load, entry, typ, ck, w38, w3c = w
        data = img[off:off + size]
        if crc(data) != ck:
            sys.exit('%s: record %s fails its CRC' % (flash_path, name))
        fields = ('name=%s pid=%#x comp=%#x osz=%#x w1c=%#x load=%#x '
                  'entry=%#x type=%#x w38=%#x w3c=%#x' %
                  (name, pid, comp, osz, w1c, load, entry, typ, w38, w3c))
        if k < npar:
            out.append('param %s size=%#x' % (fields, size))
            ents = [struct.unpack_from('<4I', data, i)
                    for i in range(0, size, 16)]
            while ents and ents[-1] == (0, 0, 0, 0):
                ents.pop()
            for e in ents:
                out.append('  %08x %08x %08x %x' % e)
        elif typ & 0xff == 0x10 and load < 0xA0000000:
            if not stage_path:
                sys.exit('%s: a first stage (%s): name a file to save it'
                         % (flash_path, name))
            n = osz or size
            open(stage_path, 'wb').write(data[:n])
            out.append('stage %s file=%s' %
                       (fields, os.path.basename(stage_path)))
        else:
            out.append('uboot %s' % fields)
    open(out_path, 'w').write('\n'.join(out) + '\n')


def parse(path):
    root = b''
    recs = []
    for line in open(path):
        line = line.split('#')[0].strip()
        if not line:
            continue
        tok = line.split()
        if tok[0] == 'root':
            root += bytes.fromhex(tok[1])
        elif tok[0] in ('param', 'stage', 'uboot'):
            kv = dict(t.split('=', 1) for t in tok[1:])
            rec = {'kind': tok[0], 'name': kv.pop('name'), 'ents': []}
            for k, v in kv.items():
                rec[k] = v if k == 'file' else int(v, 0)
            recs.append(rec)
        else:
            recs[-1]['ents'].append(tuple(int(t, 16) for t in tok))
    if len(root) != REC or root[:8] != b'2BL*OABD':
        sys.exit('%s: bad root header' % path)
    return root, recs


def build(desc, uboot_path, text, env, out_path):
    root, recs = parse(desc)
    uboot = open(uboot_path, 'rb').read()
    npar = sum(r['kind'] == 'param' for r in recs)
    nimg = len(recs) - npar
    if [r['kind'] for r in recs[:npar]] != ['param'] * npar or \
            recs[-1]['kind'] != 'uboot':
        sys.exit('%s: records out of order' % desc)
    if recs[-1]['load'] != text or recs[-1]['entry'] != text:
        sys.exit('%s: U-Boot is loaded at %#x, linked at %#x' %
                 (desc, recs[-1]['load'], text))

    hdr = bytearray(root)
    struct.pack_into('<3I', hdr, 0x0C, nimg, npar, REC + 0x40 * len(recs))
    img = bytearray(b'\xff' * SECTOR)
    img[:REC] = hdr

    pos = SECTOR
    for k, r in enumerate(recs):
        if r['kind'] == 'param':
            data = b''.join(struct.pack('<4I', *e) for e in r['ents'])
            data += bytes(r['size'] - len(data))
            osz = r['osz']
        else:
            if r['kind'] == 'stage':
                raw = open(os.path.join(os.path.dirname(desc),
                                        r['file']), 'rb').read()
            else:
                raw = uboot
            data = raw + bytes(-len(raw) % 0x80)
            # Recorded only where the source image had it
            osz = len(raw) if r['osz'] else 0
        if len(img) < pos:
            img += b'\xff' * (pos - len(img))
        img[pos:pos + len(data)] = data
        start = pos
        rec = bytearray(b'\0' * 0x40)
        name = r['name'].encode()
        rec[:len(name)] = name
        struct.pack_into('<12I', rec, 0x10, r['pid'], r['comp'], osz,
                         r['w1c'], len(data), pos, r['load'], r['entry'],
                         r['type'], crc(data), r['w38'], r['w3c'])
        img[REC + 0x40 * k:REC + 0x40 * (k + 1)] = rec
        end = pos + len(data)
        pos = align(end, SECTOR)

    if end > env:
        sys.exit('%s: the boot image ends at %#x, past the environment at '
                 '%#x' % (out_path, end, env))
    open(out_path, 'wb').write(img[:end])
    print('%s: %d bytes, U-Boot at %#x, %d bytes to spare' %
          (out_path, end, start, env - end))


def main():
    if len(sys.argv) >= 4 and sys.argv[1] == 'extract':
        extract(*sys.argv[2:5])
    elif len(sys.argv) == 6:
        build(sys.argv[1], sys.argv[2], int(sys.argv[3], 0),
              int(sys.argv[4], 0), sys.argv[5])
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main()
