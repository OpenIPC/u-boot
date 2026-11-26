#!/bin/sh
# Point the vendor register accessors at the recorder (rec.h).
V=$1
U=$V/disp/hal/infinity2m/inc/hal_disp_util.h
# hal_disp_util.h: the display HAL's macros; u32Reg is bank << 8 | byte offset
sed -i 's/^#endif.*__HAL_DISP_UTIL_H__.*$//' "$U"
cat >> "$U" <<'EOT'
#include "rec.h"
#undef RBYTE
#undef R2BYTE
#undef R2BYTEMSK
#undef R4BYTE
#undef R4BYTEMSK
#undef WBYTE
#undef WBYTEMSK
#undef W2BYTE
#undef W2BYTEMSK
#undef W4BYTE
#undef W4BYTEMSK
#undef W3BYTE
#define RBYTE(r)            rec_r8(RIU_PHYS + ((r) << 1))
#define R2BYTE(r)           rec_r16(RIU_PHYS + ((r) << 1))
#define R2BYTEMSK(r, m)     (rec_r16(RIU_PHYS + ((r) << 1)) & (m))
#define R4BYTE(r)           rec_r32(RIU_PHYS + ((r) << 1))
#define R4BYTEMSK(r, m)     (rec_r32(RIU_PHYS + ((r) << 1)) & (m))
#define WBYTE(r, v)         rec_w8(RIU_PHYS + ((r) << 1), v)
#define WBYTEMSK(r, v, m)   rec_wmsk8(RIU_PHYS + ((r) << 1), v, m)
#define W2BYTE(r, v)        rec_w16(RIU_PHYS + ((r) << 1), v)
#define W2BYTEMSK(r, v, m)  rec_wmsk16(RIU_PHYS + ((r) << 1), v, m)
#define W4BYTE(r, v)        rec_w32(RIU_PHYS + ((r) << 1), v)
#define W4BYTEMSK(r, v, m)  do { rec_wmsk16(RIU_PHYS + ((r) << 1), (v) & 0xffff, (m) & 0xffff); \
                                 rec_wmsk16(RIU_PHYS + ((r) << 1) + 4, (v) >> 16, (m) >> 16); } while (0)
#define W3BYTE(r, v)        do { rec_w16(RIU_PHYS + ((r) << 1), (v) & 0xffff); \
                                 rec_wmsk8(RIU_PHYS + ((r) << 1) + 4, (v) >> 16, 0xff); } while (0)
#endif
EOT
# any direct READ_/WRITE_ (uboot OS header) goes to the recorder too
O=$V/disp/drv/disp/inc/uboot/drv_disp_os_header.h
sed -i 's/^#define READ_SCL_REG(addr, type).*$/#define READ_SCL_REG(addr, type) ((type)(sizeof(type) == 1 ? rec_r8(addr) : sizeof(type) == 2 ? rec_r16(addr) : rec_r32(addr)))/' "$O"
sed -i 's/^#define WRITE_SCL_REG(addr, type, data).*$/#define WRITE_SCL_REG(addr, type, data) (sizeof(type) == 1 ? rec_w8(addr, data) : sizeof(type) == 2 ? rec_w16(addr, data) : rec_w32(addr, data))/' "$O"
sed -i '1i #include "rec.h"' "$O"
# halHDMIUtilTx.c: its accessors are hdmi_rec.c's
H=$V/hdmitx/hal/infinity2m/hdmitx/halHDMIUtilTx.c
for f in MHal_HDMITx_Read MHal_HDMITx_ReadByte MHal_HDMITx_Write MHal_HDMITx_Mask_Write \
         MHal_HDMITx_Mask_WriteByte MHal_HDMITx_RegsTbl_Write MHal_HDMITxPM_Read \
         MHal_HDMITxPM_Write MHal_HDMITxPM_Mask_Write; do
    sed -i "s/^\(MS_U16\|MS_U8\|void\) $f(/static \1 vendor_$f(/" "$H"
done
