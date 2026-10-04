/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: cmd_write_otp
 * Author: Hisilicon multimedia software group
 * Create: 2023/05/04
 */

#include <otp_maps.h>
#include <otp_cmd_utils.h>
#include <securec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OTP_VAL_PREFIX_LEN 2
#define CHARS_PER_BYTE 2
#define BIT_PER_BYTE 8
#define HALF_BYTE 4

static unsigned char hextochar(unsigned char bchar)
{
	if (bchar >= 0x30 && bchar <= 0x39) /* ascii : '0' :0x30   '9' :0x30 */
		bchar -= 0x30; /* ascii : '0' :0x30 */
	else if (bchar >= 0x41 && bchar <= 0x46) /* ascii : 'A' :0x41   'F' :0x46 */
		bchar -= 0x37; /* 0x37 : Converted to a hexadecimal number */
	else if (bchar >= 0x61 && bchar <= 0x66) /* ascii : 'a' :0x61   'f' :0x66 */
		bchar -= 0x57; /* 0x57 : Converted to a hexadecimal number */
	else
		bchar = 0xff; /* 0xff : Invalid val */
	return bchar;
}

static unsigned int check_arg(char *val, unsigned int bytes)
{
	if (val[0] != '0') {
		otp_trace_err("error: Invalid key! Please check.\n");
		return ERR_RET;
	}

	if (val[1] != 'x' && val[1] != 'X') {
		otp_trace_err("error: Invalid key! Please check.\n");
		return ERR_RET;
	}

	if (strlen(val) - OTP_VAL_PREFIX_LEN > bytes * CHARS_PER_BYTE) {
		otp_trace_err("error: Invalid key! Please check.\n");
		return ERR_RET;
	}

	for (int ind = 2; ind < strlen(val); ind++) {
		int n = hextochar(val[ind]);
		if (n == 0xff) {
			otp_trace_err("error: Invalid otp val! Please check.\n");
			return ERR_RET;
		}
	}

	return 0;
}

static unsigned int reached_first_charater_of_val(char *c)
{
	return *c == 'x' || *c == 'X';
}

static int populate_key_buf(char *val, unsigned char *raw_buf, unsigned int bytes)
{
	int raw_buf_ind = bytes - 1;
	int low_bits = 1;

	if (check_arg(val, bytes))
		return ERR_RET;

	for (char *cur = val + strlen(val) - 1; !reached_first_charater_of_val(cur) && raw_buf_ind >= 0;
	     cur--) {
		int n = hextochar(*cur);
		if (n == 0xff) {
			otp_trace_err("error: Invalid otp val! Please check.\n");
			return ERR_RET;
		}

		raw_buf[raw_buf_ind] += low_bits ? n : n << HALF_BYTE;
		if (!low_bits) {
			otp_trace_debug("raw_bud %x: %x\n", raw_buf_ind, raw_buf[raw_buf_ind]);
			raw_buf_ind--;
		}

		// if low_bits == 0 {low_bits = 1} else {low_bits = 0}
		low_bits = low_bits == 0;
	}

	return 0;
}

//  If there is any error occur, the val_to_raw function releases the raw.data buffer.
//  Otherwise, the caller take ownership of raw.data.
static int val_to_raw(otp_val *val, otp_raw *raw)
{
	int bytes = (val->bit_width - 1) / BIT_PER_BYTE + 1;

	raw->bit_width = val->bit_width;
	raw->lock_attr = val->lock_attr;
	raw->otp_addr = val->otp_addr;
	raw->bit_offset = val->bit_offset;
	raw->data = malloc(bytes);

	if (raw->data == NULL)
		return ERR_RET;

	if (memset_s(raw->data, bytes, 0, bytes)) {
		otp_trace_err("error: %s memset_s error.\n", __func__);
		free(raw->data);
		return ERR_RET;
	}

	if (populate_key_buf(val->val, raw->data, bytes) != 0) {
		free(raw->data);
		return ERR_RET;
	}

	return 0;
}

static int otp_pos_cmp(struct otp_pos l, struct otp_pos r)
{
	// reduce the possibility of overflow
	return (l.byte - r.byte) * BIT_PER_BYTE + l.bit - r.bit;
}

static int otp_pos_dist(struct otp_pos r, struct otp_pos l)
{
	return otp_pos_cmp(r, l);
}

static unsigned int range_width(struct otp_pos l, struct otp_pos r)
{
	otp_trace_debug("debug: range width cal: %d\n", otp_pos_dist(r, l) + 1);
	if (otp_pos_cmp(l, r) <= 0) {
		return otp_pos_dist(r, l) + 1;
	}
	return 0;
}

#define bits(nr) ((1 << (nr)) - 1)
#define bits_mask(start, nr) ((0xff << (start)) & (bits(nr) << (start)))

static int otp_write_byte(unsigned int addr, unsigned char byte)
{
	otp_trace_debug("otp addr: 0x%x\n", addr);
	otp_trace_debug("otp byte: 0x%x\n", byte);
#ifdef DEBUG
	return 0;
#else /* ifdef DEBUG */
	return ot_mpi_otp_write_byte(addr, byte) == TD_SUCCESS ? 0 : ERR_RET;
#endif /* ifdef DEBUG */
}

static int otp_in_range(struct otp_range subregion, struct otp_range range)
{
	return otp_pos_cmp(range.start, subregion.start) <= 0 && otp_pos_cmp(range.end, subregion.end) >= 0;
}

static unsigned int uint_roundup(unsigned int num, unsigned int base)
{
	return (num + base - 1) / base * base;
}

static unsigned int uint_rounddown(unsigned int num, unsigned int base)
{
	return num / base * base;
}

static unsigned int next_pos(unsigned int pos, unsigned int offset)
{
	return uint_rounddown(pos + offset + BIT_PER_BYTE, BIT_PER_BYTE) - offset;
}

static unsigned char get_bit_in_array(unsigned char *arr, unsigned int pos)
{
	return (arr[pos / BIT_PER_BYTE] & bits_mask(pos % BIT_PER_BYTE, 1)) >> (pos % BIT_PER_BYTE);
}

static void set_bit_in_array(unsigned char *arr, unsigned int pos)
{
	otp_trace_debug("set bit in array: pos %d\n", pos);
	arr[pos / BIT_PER_BYTE] |= bits_mask(pos % BIT_PER_BYTE, 1);
}

// [start, end]: is closed interval
static int extrect_bits_from_bytes_signle_byte(unsigned char *arr, unsigned int start, unsigned int end)
{
	unsigned int width = end - start + 1;
	unsigned char byte = 0;

	if (width > BIT_PER_BYTE) {
		otp_trace_debug("debug: extrect from raw array: start: %d, end: %d;\n", start, end);
		otp_trace_err("error: extrect_bits_from_bytes params error!!!\n");
		return ERR_RET;
	}

	for (int off = 0; off < width; off++)
		byte |= get_bit_in_array(arr, off + start) << off;

	otp_trace_debug("debug: read from raw array:  start: %d, end: %d;\n", start, end);
	return byte;
}

static int burn_otp_raw(otp_raw *raw)
{
	unsigned int bit_off;
	for (bit_off = 0; bit_off < raw->bit_width; bit_off = next_pos(bit_off, raw->bit_offset)) {
		unsigned int width = next_pos(bit_off, raw->bit_offset) > raw->bit_width ?
					     raw->bit_width - bit_off :
					     next_pos(bit_off, raw->bit_offset) - bit_off;
		unsigned int byte_addr = raw->otp_addr + (bit_off + raw->bit_offset) / BIT_PER_BYTE;
		unsigned int burn_byte =
			extrect_bits_from_bytes_signle_byte(raw->data, bit_off, bit_off + width - 1);
		if (burn_byte == -1) {
			otp_trace_debug("debug %s:%d: error in burn otp.\n", __func__, __LINE__);
			return ERR_RET;
		}

		if (bit_off == 0)
			burn_byte = burn_byte << raw->bit_offset;

		if (otp_write_byte(byte_addr, burn_byte)) {
			otp_trace_debug("debug %s:%d: error in burn otp.\n", __func__, __LINE__);
			return ERR_RET;
		}
	}
	return 0;
}

static struct otp_rule *find_matching_rule(struct otp_range range)
{
	int rule_ind;
	for (rule_ind = 0; rule_ind < ARRAY_SIZE(g_locker_rule_list); rule_ind++) {
		struct otp_rule rule = g_locker_rule_list[rule_ind];

		if (otp_in_range(range, rule.otp_range)) {
			otp_trace_debug("debug: find locker range\n");
			return &g_locker_rule_list[rule_ind];
		}
	}
	return NULL;
}

static int lock_otp_range(struct otp_range range)
{
	static struct otp_rule *rule;
	int locked_width, bytes;
	otp_raw locker_raw = {
		.lock_attr = ONEWAY,
	};
	unsigned int off;

	rule = find_matching_rule(range);
	if (rule == NULL) {
		otp_trace_err("error: can not find matching rule!!!!!\n");
		return ERR_RET;
	}

	otp_trace_debug("debug: rule of otp lock range start 0x%x byte, 0x%x bit.\n",
			rule->otp_range.start.byte, rule->otp_range.start.bit);

	locker_raw.bit_width = range_width(rule->locker_range.start, rule->locker_range.end);
	locker_raw.otp_addr = rule->locker_range.start.byte;
	locker_raw.bit_offset = rule->locker_range.start.bit;
	bytes = uint_roundup(locker_raw.bit_width, BIT_PER_BYTE) / BIT_PER_BYTE;
	locker_raw.data = malloc(bytes);

	if (locker_raw.data == NULL) {
		otp_trace_debug("debug: %s:%d malloc error.\n", __func__, __LINE__);
		otp_trace_err("error: otp range 0x%x byte, 0x%x bit will not be lock.\n", range.start.byte,
			      range.start.bit);
		return ERR_RET;
	}

	if (memset_s(locker_raw.data, bytes, 0, bytes)) {
		otp_trace_debug("error: %s memset_s error.\n", __func__);
		goto exit_with_error;
	}

	off = otp_pos_dist(range.start, rule->otp_range.start) / rule->otp_corresponding_per_bit_locker;

	otp_trace_debug("debug: locker bit offset start 0x%x.\n", off);

	for (locked_width = 0; locked_width < range_width(range.start, range.end);
	     locked_width += rule->otp_corresponding_per_bit_locker) {
		unsigned int off_in_locker_range =
			off + locked_width / rule->otp_corresponding_per_bit_locker;

		otp_trace_debug("debug: locker bit offset 0x%x.\n", off_in_locker_range);

		if (off_in_locker_range > locker_raw.bit_width) {
			otp_trace_debug("bug: otp out of locker range.\n");
			goto exit_with_error;
		}

		set_bit_in_array(locker_raw.data, off_in_locker_range);
	}

	if (burn_otp_raw(&locker_raw))
		goto exit_with_error;

	free(locker_raw.data);
	return 0;

exit_with_error:
	free(locker_raw.data);
	otp_trace_err("error: otp range 0x%x byte, 0x%x bit will not be lock.\n", range.start.byte,
		      range.start.bit);
	return ERR_RET;
}

static int burn_otp_raw_and_lock(otp_raw *raw)
{
	struct otp_range range = {
		.start = { raw->otp_addr, raw->bit_offset },
		.end = { raw->otp_addr + (raw->bit_offset + raw->bit_width - 1) / BIT_PER_BYTE,
			 (raw->bit_offset + raw->bit_width - 1) % BIT_PER_BYTE },
	};

	if (burn_otp_raw(raw))
		return ERR_RET;

	otp_trace_debug("\n======== burn locker otp ============\n");
	if (raw->lock_attr == LOCKABLE)
		return lock_otp_range(range);

	return 0;
}

static int burn_otp_val_and_lock(otp_val *val)
{
	otp_raw raw;
	int res;

	otp_trace_info("\n====================\n");
	otp_trace_info("burn otp name: %s\n", val->field_name);
	otp_trace_debug("burn otp val: %s\n", val->val);
	res = val_to_raw(val, &raw);
	if (res != 0) {
		otp_trace_err("error: otp burn error.\n");
		return res;
	}

	res = burn_otp_raw_and_lock(&raw);
	otp_trace_info("\n====================\n");

	// free data alloc from val_to_raw
	free(raw.data);
	return res;
}

int do_write_otp_main(int argc, char *const argv[])
{
	ot_mpi_otp_init();
	otp_trace_info("\n write otp start\n");

	for (int ind = 0; ind < ARRAY_SIZE(g_otp_fields_big_endian_num); ind++) {
		if (burn_otp_val_and_lock(&g_otp_fields_big_endian_num[ind]) != 0) {
			ot_mpi_otp_deinit();
			otp_trace_err("otp burn stop with error!!!!!\n");
			return ERR_RET;
		}
	}

	// use burn_otp_raw_and_lock, if otp sould be locked
	for (int ind = 0; ind < ARRAY_SIZE(g_otp_fields_raw); ind++) {
		if (burn_otp_raw_and_lock(&g_otp_fields_raw[ind]) == -1) {
			ot_mpi_otp_deinit();
			otp_trace_err("otp burn stop with error!!!!!\n");
			return ERR_RET;
		}
	}

	ot_mpi_otp_deinit();
	otp_trace_info("\n write otp end\n");
	return 0;
}
