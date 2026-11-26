/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "rec.h"

#define NREG (0x400000 / 4)            /* 4 MiB of RIU, one u16 per 4 bytes */
static uint16_t shadow[NREG];
static unsigned long now_ms;
FILE *rec_out;

static unsigned idx(uint32_t addr)
{
    if (addr < RIU_PHYS || addr >= RIU_PHYS + 0x400000) {
        fprintf(stderr, "regdump: access outside the RIU: 0x%08x\n", addr);
        abort();
    }
    return (addr - RIU_PHYS) >> 2;
}

static void name(uint32_t addr, unsigned *bank, unsigned *reg)
{
    unsigned i = idx(addr);

    *bank = i >> 7;
    *reg = i & 0x7f;
}

void rec_wmsk16(uint32_t addr, uint16_t val, uint16_t mask)
{
    unsigned b, r, i = idx(addr);

    name(addr, &b, &r);
    shadow[i] = (shadow[i] & ~mask) | (val & mask);
    if (mask == 0xffff)
        fprintf(rec_out, "W %04x %02x %04x\n", b, r, val);
    else
        fprintf(rec_out, "M %04x %02x %04x %04x\n", b, r, val & mask, mask);
}

void rec_w16(uint32_t addr, uint16_t val) { rec_wmsk16(addr, val, 0xffff); }

void rec_wmsk8(uint32_t addr, uint8_t val, uint8_t mask)
{
    int hi = (addr & 3) != 0;          /* the high byte: +1 (or +2) */

    rec_wmsk16(addr & ~3u, (uint16_t)val << (hi ? 8 : 0),
               (uint16_t)mask << (hi ? 8 : 0));
}

void rec_w8(uint32_t addr, uint8_t val) { rec_wmsk8(addr, val, 0xff); }

void rec_w32(uint32_t addr, uint32_t val)
{
    rec_w16(addr, val & 0xffff);
    rec_w16(addr + 4, val >> 16);
}

uint16_t rec_r16(uint32_t addr)
{
    unsigned b, r;
    uint16_t v = shadow[idx(addr)];

    name(addr, &b, &r);
    fprintf(rec_out, "R %04x %02x %04x\n", b, r, v);
    return v;
}

uint8_t rec_r8(uint32_t addr)
{
    uint16_t v = rec_r16(addr & ~3u);

    return (addr & 3) ? v >> 8 : v & 0xff;
}

uint32_t rec_r32(uint32_t addr)
{
    return rec_r16(addr) | (uint32_t)rec_r16(addr + 4) << 16;
}

void rec_delay_us(uint32_t us)
{
    now_ms += us / 1000;
    if (us)
        fprintf(rec_out, "D %u\n", us);
}

unsigned long rec_time_ms(void) { return now_ms; }

void rec_mark(const char *what) { fprintf(rec_out, "# %s\n", what); }
