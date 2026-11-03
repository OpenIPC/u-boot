#ifndef _AT80C51_H
#define _AT80C51_H

typedef enum _MCU_CC_TYPE {
	MCU_CC_TYPE_UNKNOWN,
	MCU_CC_TYPE_SYSTEM,
	MCU_CC_TYPE_USER,
} MCU_CC_TYPE;

// if modified, wq_cc.h at80c51.h (sdk, loader) must sync
typedef enum _MCU_CC_SYSCMD_ID {
	MCU_CC_SYSCMD_ID_UNKNOWN,
	MCU_CC_SYSCMD_ID_SHUTDOWN,
	MCU_CC_SYSCMD_ID_SHUTDOWN_AT80C51_PD1,
	MCU_CC_SYSCMD_ID_SHUTDOWN_AT80C51_PD2,
	MCU_CC_SYSCMD_ID_PERIODIC_MANUAL_PD1,
	MCU_CC_SYSCMD_ID_PERIODIC_MANUAL_PD2,
	MCU_CC_SYSCMD_ID_PERIODIC_VD_PD1,
	MCU_CC_SYSCMD_ID_PERIODIC_VD_PD2,
	MCU_CC_SYSCMD_ID_SIMULATE_YDCC_ALARM,
	MCU_CC_SYSCMD_ID_MODE_PREROLL, // shutdown main-part for preroll
	MCU_CC_SYSCMD_ID_MODE_SENTRY,  // shutdown main-part for sentry
	MCU_CC_SYSCMD_ID_SET_LOADER2_OFS, // rtos notify flash offset of loader2 (aov) to 8051
	MCU_CC_SYSCMD_ID_REBOOT_TO_MAIN, // loader2 (aov) reboot to loader1
} MCU_CC_SYSCMD_ID;

extern int at80c51_ipc(MCU_CC_SYSCMD_ID cmd_id, UINT32 param);
extern int at80c51_is_opened(void);

#endif