/* SPDX-License-Identifier: MIT
 * The HDMI TX HAL's register accessors (halHDMIUtilTx.c), recorded: its
 * banks are RIU byte addresses >> 1 (0x112500), its addresses 16-bit
 * register indices; byte addresses keep the byte in bit 0. */
#include "rec.h"

typedef unsigned int MS_U32;
typedef unsigned short MS_U16;
typedef unsigned char MS_U8;
typedef struct { MS_U32 bank; MS_U16 address; MS_U16 mask; MS_U16 value; } RegEntry;
extern unsigned char g_bDisableRegWrite;

#define ABS(bank, a) (RIU_PHYS + ((bank) << 1) + ((MS_U32)(a) << 2))
#define ABS8(bank, a) (RIU_PHYS + ((bank) << 1) + (((MS_U32)(a) & ~1u) << 1) + ((a) & 1))

MS_U16 MHal_HDMITx_Read(MS_U32 bank, MS_U16 a) { return rec_r16(ABS(bank, a)); }
MS_U8 MHal_HDMITx_ReadByte(MS_U32 bank, MS_U16 a) { return rec_r8(ABS8(bank, a)); }
void MHal_HDMITx_Write(MS_U32 bank, MS_U16 a, MS_U16 v)
{
    if (!g_bDisableRegWrite)
        rec_w16(ABS(bank, a), v);
}
void MHal_HDMITx_Mask_Write(MS_U32 bank, MS_U16 a, MS_U16 m, MS_U16 v)
{
    if (!g_bDisableRegWrite)
        rec_wmsk16(ABS(bank, a), v, m);
}
void MHal_HDMITx_Mask_WriteByte(MS_U32 bank, MS_U16 a, MS_U8 m, MS_U8 v)
{
    if (!g_bDisableRegWrite)
        rec_wmsk8(ABS8(bank, a), v, m);
}
void MHal_HDMITx_RegsTbl_Write(RegEntry *t, MS_U8 n)
{
    for (; n; n--, t++)
        MHal_HDMITx_Mask_Write(t->bank, t->address, t->mask, t->value);
}
MS_U16 MHal_HDMITxPM_Read(MS_U32 bank, MS_U16 a) { return rec_r16(ABS(bank, a)); }
void MHal_HDMITxPM_Write(MS_U32 bank, MS_U16 a, MS_U16 v)
{
    if (!g_bDisableRegWrite)
        rec_w16(ABS(bank, a), v);
}
void MHal_HDMITxPM_Mask_Write(MS_U32 bank, MS_U16 a, MS_U16 m, MS_U16 v)
{
    if (!g_bDisableRegWrite)
        rec_wmsk16(ABS(bank, a), v, m);
}
