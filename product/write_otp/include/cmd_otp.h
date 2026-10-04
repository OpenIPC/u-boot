/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: otp_cmd_h
 * Author: Hisilicon multimedia software group
 * Create: 2023-05-08
 */
#ifndef CMD_OTP_H
#define CMD_OTP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_RET (-1)

enum lock_attr { LOCKABLE, ONEWAY };

/* val: otp val, hex with prefix 0x or 0X */
typedef struct {
	char *field_name;
	unsigned int bit_width;
	enum lock_attr lock_attr;
	unsigned int otp_addr;
	unsigned int bit_offset;
	char *val;
} otp_val;

typedef struct {
	char *field_name;
	unsigned int bit_width;
	enum lock_attr lock_attr;
	unsigned int otp_addr;
	unsigned int bit_offset;
	unsigned char *data;
} otp_raw;

struct otp_pos {
	unsigned int byte;
	unsigned char bit;
};

// otp_range: is a closed set
struct otp_range {
	struct otp_pos start;
	struct otp_pos end;
};

struct otp_rule {
	struct otp_range otp_range;
	struct otp_range locker_range;
	unsigned int otp_corresponding_per_bit_locker;
};

#ifdef __cplusplus
}
#endif

#endif /* CMD_OTP_H */
