/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: otp_cmd_utils
 * Author: Hisilicon multimedia software group
 * Create: 2023-05-08
 */
#ifndef OTP_CMD_UTILS_H
#define OTP_CMD_UTILS_H

#ifdef __KERNEL__
/*
 * For UBOOT
 */
#include <ot_mpi_otp.h>
#include <ot_type.h>
#include <linux/kernel.h>
#else
/*
 * For User Mode
 */
#include <hi_mpi_otp.h>
#include <ot_type.h>
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __KERNEL__
#define otp_trace_err pr_err
#define otp_trace_info pr_info
#define otp_trace_debug pr_debug
#else
#ifndef CONFIG_LOGLEVEL
#define CONFIG_LOGLEVEL 3
#endif // !CONFIG_LOGLEVEL

#define __OTP_CMD_PRINT      printf

/*
 * Keep the definition of CONFIG_LOGLEVEL consistent with that in the Linux kernel header file <linux/printk.h>.
 */
#define otp_trace_err(fmt, ...)						\
({									\
	CONFIG_LOGLEVEL > 3 ? __OTP_CMD_PRINT(fmt, ##__VA_ARGS__) : 0;		\
})

#define otp_trace_info(fmt, ...)						\
({									\
	CONFIG_LOGLEVEL > 6 ? __OTP_CMD_PRINT(fmt, ##__VA_ARGS__) : 0;		\
})

#define otp_trace_debug(fmt, ...)						\
({									\
	CONFIG_LOGLEVEL > 7 ? __OTP_CMD_PRINT(fmt, ##__VA_ARGS__) : 0;	\
})

#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifdef __cplusplus
}
#endif

#endif /* OTP_CMD_UTILS_H */
