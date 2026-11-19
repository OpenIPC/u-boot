# AX620U first stage (SPL)

The code the AX620U boot ROM loads from SPI NAND offset 0x800 to on-chip
RAM 0x800 and runs in ARM state, as the cameras' own first stage does:
clocks, PLLs, core voltage and pads, LPDDR training, the secondary cores'
release, then U-Boot from flash 0x100000 (`boot`; its 1 KiB header to
0x40000000, the image to 0x40000400, entered there). `build.sh` builds it
with `make -C openipc/ax620u-spl O=<dir>`, and
`openipc/ax620u-spl-image.py` writes the boot ROM's header in front of it
and lays out the start of the `spl` partition.

## Provenance

No source for the AX620U first stage is published. This one is
reconstructed from the binary the AX620U Xiongmai cameras run (`stock.bin`
below), which is built at -O0 (Thumb-2, frame pointer in r7) and so
decompiles nearly function by function: Ghidra's output for the functions
the SPI NAND boot reaches (133 of its 382) was rewritten as C, register
for register, and the names and comments are ours.

- `driver/ddr/ddr_tables.c` is generated, not written:
  `tools/gen_ddr_tables.py` copies the DDR controller, PI and PHY tables
  (register/value pairs, the PI's 0x559 words, the bit-field fix-ups) and
  the four board configuration words out of `stock.bin`
  (`make tables` regenerates it; it is in the tree so the build needs no
  vendor binary). Values from one camera's first stage: 256 MiB LPDDR,
  the single training pass its configuration word (0x1) selects.
- `ddr_init.c` keeps the two-pass path (CA training results read in the
  first pass, written back with CA training off in the second) that the
  stock code has for configuration words with bits 0x300 set, though the
  cameras' word does not select it.

## What is left out

The stock first stage is 74736 bytes of code; this one is 34704. Not
here: USB and UART download (FDL), SD and FAT, eMMC and SPI NOR boot,
the crypto engine and secure boot (signature checks, the EIP-130
firmware, decryption). Only SPI NAND boot remains, with its quad reads
when the header asks for them, ECC status checks and the skip over bad
blocks from 0x100000 ("eba %d is bad").

## Deliberate differences

- A part with secure boot fused (`CHIP_SECURE_STATUS` bit 26) or an
  encrypted U-Boot image (capability bit 8) is refused: nothing to check
  a signature or decrypt with. The board resets, as the stock code does
  on a failed load. The cameras known have neither.
- The secondary cores' check-in words (`pen_release[4]`) are pinned at
  0x13298, where the stock code has them (`board/bl1.lds`): qemu-bench's
  `ax620u` machine acknowledges the cores' release at exactly those
  addresses. On silicon the cores write them themselves, so any address
  would do.
- Like the stock code, it checks U-Boot's header (magic, header word
  sum) but not the image's word sum (+0x14), and it goes on after a DDR
  training timeout (printed), a PLL that does not lock or an eFuse read
  error. The bad-block check reads the spare area into a buffer for the
  largest size the header's geometry word can give (512 bytes).
- -O0, as the stock build: the DDR PLL and PHY waits (`DELAY()` in
  `ddr_init.c`) are counted loops whose length is the -O0 code's. Built
  with Ubuntu's gcc 15, the loop is the stock one instruction for
  instruction (load, subtract, store, compare, branch on a stack
  counter); the other waits use the 24 MHz timer.

## Checked in QEMU only

In qemu-bench's `-M ax620u` with a camera's flash dump, this first stage
spliced in at 0 and 0x20000: the ordered MMIO writes from reset to
U-Boot's entry equal the stock first stage's (29736 writes with the
stock U-Boot, 15871 with OpenIPC's; also in the two-pass and quad-read
paths), except the DMA destination register when the bad-block check
reads into a stack buffer (the stack differs). It boots the stock U-Boot
and kernel to login and OpenIPC's U-Boot to its prompt, and from an
otherwise erased flash with just this first stage and OpenIPC's U-Boot,
the U-Boot prompt and its saved environment after `reset`. The machine
does not model the boot ROM (no header check: it copies flash 0x800 on
to RAM 0x800) nor the DDR training (a plain register bank), so only a
camera tests the header and the DDR.

## The stock first stage (`stock.bin`), the fallback

`stock.bin` is flash 0 to 0x12bf0 of the AX620U Xiongmai camera dump
`JFtech_IPC_AX620U_A8.Nat.dss.OnvifS.HIK_V5.00.M01.20241031_all` (128 MiB
SPI NAND): the header, its copy at 0x400 and 0x123f0 bytes of code at
0x800, signed with the Axera SDK test key. SHA-256
`faad7357d752cce79e278b2cc074ed5f5091b78a92b2d065f544b968aa06ed33`.
Its partition also holds the code again at 0x20000 (rebuilt from
`stock.bin`), 0x12ac0 bytes at 0x40000 and again at 0x60000 whose size
and word sum are the header's firmware fields (the EIP-130 firmware for
secure boot parts; not here and not written), and zeros to 1 MiB.

`build.sh` writes, besides U-Boot's `u-boot-ax620u-nand.bin`:

- `output/spl-ax620u-nand.bin`: this first stage
- `output/spl-ax620u-nand-stock.bin`: `stock.bin`, the same bytes as the
  camera's flash 0 to 0x40000

Both are the first 256 KiB (two 128 KiB blocks) of the `spl` partition,
written at flash 0: header at 0 and 0x400, code at 0x800, code at
0x20000. They leave the firmware at 0x40000 as it is.
`SPL=stock ./build.sh ax620u-nand` puts `stock.bin` in
`spl-ax620u-nand.bin` and builds no other. Write the stock one back when
the camera prints nothing, no `ddr init success!`, or U-Boot does not
report 256 MiB of DRAM.
