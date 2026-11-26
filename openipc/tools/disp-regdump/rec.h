/* The register recorder: every access the vendor code makes to the RIU,
 * logged in order; reads come from a shadow of what was written. */
#ifndef REGDUMP_REC_H
#define REGDUMP_REC_H
#include <stdint.h>
void rec_w16(uint32_t addr, uint16_t val);
void rec_w8(uint32_t addr, uint8_t val);
void rec_w32(uint32_t addr, uint32_t val);
void rec_wmsk16(uint32_t addr, uint16_t val, uint16_t mask);
void rec_wmsk8(uint32_t addr, uint8_t val, uint8_t mask);
uint16_t rec_r16(uint32_t addr);
uint8_t rec_r8(uint32_t addr);
uint32_t rec_r32(uint32_t addr);
void rec_delay_us(uint32_t us);
unsigned long rec_time_ms(void);
void rec_mark(const char *what);
/* the RIU: byte address of a 16-bit register = 0x1F000000 + (bank << 9) + (reg << 2) */
#define RIU_PHYS 0x1F000000u
#endif
