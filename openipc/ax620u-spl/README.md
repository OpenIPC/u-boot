# AX620U first stage (SPL)

The code the AX620U boot ROM loads from SPI NAND offset 0x800 to on-chip
RAM 0x800 and runs in ARM state, as the cameras' own first stage does:
clocks, PLLs, core voltage and pads, LPDDR training, the secondary cores'
release, then U-Boot from flash 0x40000 (its 1 KiB header to
0x40000000, the image to 0x40000400, entered there). `build.sh` builds it
with `make -C openipc/ax620u-spl O=<dir>`, and `openipc/ax620u-image.py`
writes the boot ROM's header in front of it and lays out the image: the
`boot` partition of OpenIPC's NAND layout, `768k(boot),256k(env),-(ubi)`,
from flash 0. The cameras' own first stage loads U-Boot from 0x100000,
behind its 1 MiB `spl` partition; that layout is not used.

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

The stock first stage is 74736 bytes of code; this one is 34688. Not
here: USB and UART download (FDL), SD and FAT, eMMC and SPI NOR boot,
the crypto engine and secure boot (signature checks, the EIP-130
firmware, decryption). Only SPI NAND boot remains, with its quad reads
when the header asks for them, ECC status checks and the skip over bad
blocks from 0x40000 ("eba %d is bad").

## Deliberate differences

- U-Boot is at 0x40000, not 0x100000, and its area ends at the
  environment (0xc0000): the bad-block skip stops there, where the stock
  code's goes on to its sixteenth good block. The map is sized for 64 KiB
  blocks, the smallest the header's geometry word gives (and what a
  header of zeros gives, as in QEMU, which does not load it).

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

Those comparisons were made with U-Boot at 0x100000, before the move to
OpenIPC's layout, which changed U-Boot's address and the end of its
bad-block skip only. Since then, in the same machine from an otherwise
erased flash, both images (this first stage, and `stock.bin` with its
six changes) reach the U-Boot prompt and its saved environment after
`reset`, and `run ubwrite` of the image rewrites both from U-Boot.

## The stock first stage (`stock.bin`), the fallback

`stock.bin` is flash 0 to 0x12bf0 of the AX620U Xiongmai camera dump
`JFtech_IPC_AX620U_A8.Nat.dss.OnvifS.HIK_V5.00.M01.20241031_all` (128 MiB
SPI NAND): the header, its copy at 0x400 and 0x123f0 bytes of code at
0x800, signed with the Axera SDK test key. SHA-256
`faad7357d752cce79e278b2cc074ed5f5091b78a92b2d065f544b968aa06ed33`.
The cameras' `spl` partition also holds the code again at 0x20000, the
EIP-130 firmware at 0x40000 and again at 0x60000 (below), and zeros to
1 MiB; U-Boot follows at 0x100000.

`build.sh` writes two images, each the 768 KiB `boot` partition from
flash 0 (the first stage at 0 and 0x20000, U-Boot at 0x40000), U-Boot the
same in both:

- `output/u-boot-ax620u-nand.bin`: this first stage
- `output/u-boot-ax620u-nand-stockspl.bin`: `stock.bin` with six
  instructions changed so it loads U-Boot from 0x40000 instead of
  0x100000 (`STOCK_PATCHES` in `ax620u-image.py`: the bad-block map's
  alignment check, message, first block and block count, the header's
  and the image's flash addresses), its header written again for the
  changed code with the same key

`SPL=stock ./build.sh ax620u-nand` puts `stock.bin` in
`u-boot-ax620u-nand.bin` and builds no other. Write the stock one back
when the camera prints nothing, no `ddr init success!`, or U-Boot does
not report 256 MiB of DRAM.

## The EIP-130 firmware

The cameras keep 0x12ac0 (76480) bytes at flash 0x40000 and 0x60000,
SHA-256
`4ba8368c2cc413e4e4b00d84947287318835a22664062cc295f4eb5ee6777f48`:
the firmware of the SoC's EIP-130 security engine, the same bytes as the
AX620 SDK's `tools/imgsign/eip.bin` that Sipeed publishes
(<https://github.com/sipeed/maix_ax620e_sdk>, commit `fe7545cf`), so not
the camera's own. The first stage's header gives its size and word sum
(+0x10, +0x18) and its places (+0x24, +0x28); the stock first stage
loads it into the engine only on parts with secure boot fused, and
neither first stage here boots those. Nothing in OpenIPC uses it, and
OpenIPC's layout has U-Boot there: the image has none, and its header
says so (size, sum and places zero). Whether the boot ROM reads it on a
part without secure boot is not known; only a camera can tell.
