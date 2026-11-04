#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Start U-Boot on a Novatek board through its UART loader, from a recovery
# set (build.sh, output/recovery-<soc>-<flash>/): wrloader.bin, already
# running, takes FW98xxxT.BIN and boots it. Nothing is written to flash.
#
# The protocol is the loader's (Project/Model/Src/UART/uart_upgrade.c,
# Include/scsi_op.h): 16-byte commands, byte 0 the opset, 1 the command,
# 2-5 an address and 6-9 a size, 14-15 a checksum making the 16-bit words
# plus 28 sum to 0. Each is answered by its echo, byte 13 the status (0
# good); the loader's own messages may come in between on the same UART.
#
# Getting wrloader.bin to the board is the mask ROM's part, which no SDK
# document describes: this tool does not do it yet.
#
# usage: nvt-uart-boot.py [--baud N] PORT|HOST:PORT FILE

import select
import socket
import struct
import sys
import termios
import time

OPSET = 0xC0
OP_OPEN_DEVICE = 0x01
OP_MEM_WRITE = 0x04
OP_IS_NVT = 0x81
OP_WAIT_DONE = 0x82
OP_GET_TMP_ADDR = 0x85
OP_UBOOT = 0x87

TIMEOUT = 30
CHUNK = 0x1000
# What goes out between looks at what came in: the loader's messages must
# not back up behind a blocked write (a TCP link has flow control)
STEP = 256


class Link:
    """A serial port, or a TCP socket such as QEMU's -serial tcp:..."""

    def __init__(self, port, baud):
        if ':' in port and not port.startswith('/'):
            host, tcp = port.rsplit(':', 1)
            self.sock = socket.create_connection((host, int(tcp)), TIMEOUT)
            self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            self.fd = None
            return
        self.sock = None
        self.fd = open(port, 'r+b', buffering=0)
        attrs = termios.tcgetattr(self.fd)
        speed = getattr(termios, 'B%d' % baud)
        attrs[0] = attrs[1] = attrs[3] = 0
        attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        attrs[4] = attrs[5] = speed
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 10
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)

    def write(self, data):
        if self.sock:
            self.sock.sendall(data)
        else:
            self.fd.write(data)

    def read(self):
        if self.sock:
            return self.sock.recv(4096)
        return self.fd.read(4096)

    def pending(self):
        src = self.sock or self.fd
        return bool(select.select([src], [], [], 0)[0])


class Loader:
    def __init__(self, link):
        self.link = link
        self.rx = b''

    def fill(self, what):
        deadline = time.monotonic() + TIMEOUT
        while True:
            try:
                data = self.link.read()
            except socket.timeout:
                data = b''
            if data:
                self.rx += data
                return
            if time.monotonic() > deadline:
                sys.exit('no answer to %s (got %r)' % (what, self.rx[-64:]))

    def command(self, op, addr=0, size=0, data=b'', reply=0):
        cdb = bytearray(struct.pack('<BBIIxxxxxx', OPSET, op, addr, size))
        total = sum(struct.unpack('<8H', cdb)) + 28
        struct.pack_into('<H', cdb, 14, -total & 0xffff)
        self.link.write(cdb)
        # The echo, past whatever the loader printed
        while True:
            i = self.rx.find(bytes(cdb[:13]))
            if i >= 0 and len(self.rx) >= i + 16:
                break
            self.fill('command %#x' % op)
        status = self.rx[i + 13]
        self.rx = self.rx[i + 16:]
        if status:
            sys.exit('command %#x refused: status %d' % (op, status))
        for off in range(0, len(data), STEP):
            self.link.write(data[off:off + STEP])
            while self.link.pending():
                self.rx += self.link.read()
        while len(self.rx) < reply:
            self.fill('command %#x data' % op)
        out = self.rx[:reply]
        self.rx = self.rx[reply:]
        return out


def main():
    args = sys.argv[1:]
    baud = 115200
    if args[:1] == ['--baud']:
        baud = int(args[1])
        args = args[2:]
    if len(args) != 2:
        sys.exit('usage: nvt-uart-boot.py [--baud N] PORT|HOST:PORT FILE')
    with open(args[1], 'rb') as f:
        image = f.read()

    loader = Loader(Link(args[0], baud))
    chip = struct.unpack('<I', loader.command(OP_IS_NVT, reply=4))[0]
    print('chip id %04x' % (chip >> 16))
    loader.command(OP_OPEN_DEVICE)
    tmp = struct.unpack('<I', loader.command(OP_GET_TMP_ADDR, size=4,
                                             reply=4))[0]
    print('sending %d bytes to %#x' % (len(image), tmp))
    start = time.monotonic()
    for off in range(0, len(image), CHUNK):
        part = image[off:off + CHUNK]
        loader.command(OP_MEM_WRITE, tmp + off, len(part), part)
        print('\r%d%%' % ((off + len(part)) * 100 // len(image)), end='',
              flush=True)
    print(' in %.0f s' % (time.monotonic() - start))
    status = struct.unpack('<i', loader.command(OP_UBOOT, size=4, reply=4))[0]
    if status:
        sys.exit('the loader refuses the image: %d' % status)
    loader.command(OP_WAIT_DONE)
    print('U-Boot is starting: open a terminal on the same port')


if __name__ == '__main__':
    main()
