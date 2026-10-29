// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL: read from SPI NAND on the T40's SFC, as the vendor SPL does
 * (common/spl/spl_sfc_nand.c of OpenIPC/u-boot-t40): the SFC at
 * CONFIG_SFC_NAND_RATE with the timing ddr-style computed on the build host
 * (sfc_timing_val.h), the chip found by its ID in the table the host makes
 * from the vendor's chip list (sfc_nand_params.h), write protection off,
 * ECC and the buffer mode on, pages read through the cache with the ECC
 * status checked, and a block whose first page's spare starts with
 * anything but 0xff skipped as bad.
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <mach/ingenic.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <asm/arch/sfc_params.h>
#include <asm/arch/spi_nand.h>
#include "sfc_timing_val.h"
#include "sfc_nand_params.h"

static struct spl_nand_param *curr_device;

static inline void sfc_writel(unsigned int value, unsigned short offset)
{
	writel(value, (void __iomem *)SFC_BASE + offset);
}

static inline unsigned int sfc_readl(unsigned short offset)
{
	return readl((void __iomem *)SFC_BASE + offset);
}

static inline void sfc_transfer_direction(unsigned int value)
{
	unsigned int val;

	val = sfc_readl(SFC_GLB);
	val &= ~(1 << GLB_TRAN_DIR_OFFSET);
	val |= value << GLB_TRAN_DIR_OFFSET;
	sfc_writel(val, SFC_GLB);
}

static inline void sfc_set_length(unsigned int value)
{
	sfc_writel(value, SFC_TRAN_LEN);
}

static inline void sfc_dev_addr(unsigned int channel, unsigned int value)
{
	sfc_writel(value, SFC_DEV_ADDR(channel));
}

static inline void set_flash_timing(void)
{
	sfc_writel(DEF_TIM_VAL, SFC_DEV_CONF);
}

static inline void sfc_tranconf_init(struct jz_sfc *sfc, unsigned int channel)
{
	sfc_writel(sfc->tranconf.d32, SFC_TRAN_CONF(channel));
}

static void sfc_set_transfer(struct jz_sfc *sfc, unsigned int dir)
{
	sfc_transfer_direction(dir);
	sfc_tranconf_init(sfc, 0);
	sfc_set_length(sfc->len);
	sfc_dev_addr(0, sfc->addr);
}

static void clear_end(void)
{
	while (!(sfc_readl(SFC_SR) & END))
		;
	sfc_writel(CLR_END, SFC_SCR);
}

static void sfc_tranconf1_init(unsigned int tran_mode)
{
	unsigned int tmp;

	tmp = sfc_readl(SFC_TRAN_CONF1(0));
	tmp &= ~(TRAN_CONF1_TRAN_MODE_MSK);
	tmp |= (tran_mode << TRAN_CONF1_TRAN_MODE_OFFSET);
	sfc_writel(tmp, SFC_TRAN_CONF1(0));
}

static void sfc_send_cmd(struct jz_sfc *sfc, unsigned char dir)
{
	sfc_writel(1 << 1, SFC_TRIG);
	sfc_set_transfer(sfc, dir);
	sfc_writel(1 << 2, SFC_TRIG);
	sfc_writel(TRIG_START, SFC_TRIG);
}

static void sfc_write_data(unsigned int *data, unsigned int length)
{
	while (!(sfc_readl(SFC_SR) & TRAN_REQ))
		;
	sfc_writel(CLR_TREQ, SFC_SCR);
	sfc_writel(*data, SFC_RM_DR);
	clear_end();
}

static int sfc_read_data(unsigned int *data, unsigned int length)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int reg_tmp = 0;
	unsigned int len = (length + 3) / 4;
	int i;

	while (1) {
		reg_tmp = sfc_readl(SFC_SR);
		if (reg_tmp & RECE_REQ) {
			sfc_writel(CLR_RREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else
				fifo_num = len - tmp_len;
			for (i = 0; i < fifo_num; i++) {
				*data++ = sfc_readl(SFC_RM_DR);
				tmp_len++;
			}
		}
		if (tmp_len == len)
			break;
	}
	clear_end();
	return 0;
}

static void sfc_controler_init(void)
{
	unsigned int tmp;

	clk_set_rate(SFC_CLK, CONFIG_SFC_NAND_RATE);

	tmp = sfc_readl(SFC_GLB);
	tmp &= ~(GLB_THRESHOLD_MSK);
	tmp |= (THRESHOLD << GLB_THRESHOLD_OFFSET);
	sfc_writel(tmp, SFC_GLB);

	set_flash_timing();
}

static int spinand_bad_block_check(int len, unsigned char *check_buf)
{
	int j;

	for (j = 0; j < len; j++)
		if (check_buf[j] != 0xff)
			return 1;
	return 0;
}

static int spinand_read_page(unsigned int page, unsigned int column,
			     unsigned char *dst_addr, unsigned int len,
			     unsigned int pagesize)
{
	struct jz_sfc sfc;
	unsigned int read_buf = 0;
	int oob_flag = 0;
	unsigned char i;
	unsigned char checklen = 1;

read_oob:
	if (oob_flag) {
		column = pagesize;
		len = 4;
		dst_addr = (unsigned char *)&read_buf;
	}

	SFC_SEND_COMMAND(&sfc, CMD_PARD, 0, page, 3, 0, 0, 0);
	clear_end();

	do {
		SFC_SEND_COMMAND(&sfc, CMD_GET_FEATURE, 1, FEATURE_REG_STATUS1,
				 1, 0, 1, 0);
		read_buf = 0;
		sfc_read_data(&read_buf, 1);
	} while (read_buf & 0x1);

	/* ecc check */
	for (i = 0; i < curr_device->eccstat_count; i++) {
		if (((read_buf >> curr_device->ecc_bit) &
		     (~(0xff << curr_device->bit_counts))) ==
		    curr_device->eccerrstatus[i])
			return -1;
	}

	/*
	 * Parity block plane select
	 * MXIC  MX35LF2GE4AB    DID : 0x22
	 * XTX   XT26G02E        DID : 0x24
	 * ESMT  F50L2G41XA      DID : 0x24
	 */
	if (curr_device->device_id == 0x20 || curr_device->device_id == 0x22 ||
	    curr_device->device_id == 0x24)
		column |= (((page >> 6) & 1) << 12);

#ifndef CONFIG_SPI_STANDARD
	SFC_SEND_COMMAND(&sfc, CMD_FR_CACHE_QUAD, len, column,
			 curr_device->addrlen, 8, 1, 0);
#else
	SFC_SEND_COMMAND(&sfc, CMD_FR_CACHE, len, column,
			 curr_device->addrlen, 8, 1, 0);
#endif
	sfc_read_data((unsigned int *)dst_addr, len);

	if (!oob_flag && !(page % CONFIG_SPI_NAND_PPB)) {
		oob_flag = 1;
		goto read_oob;
	} else if (oob_flag) {
#if NAND_BUSWIDTH == NAND_BUSWIDTH_16
		checklen = 2;
#endif
		if (spinand_bad_block_check(checklen,
					    (unsigned char *)&read_buf))
			return 1;
	}
	return 0;
}

static int probe_id_list(unsigned char *id)
{
	unsigned char i;

	for (i = 0; i < ARRAY_SIZE(nand_param); i++) {
		if (nand_param[i].id_manufactory == id[0] &&
		    nand_param[i].device_id == id[1]) {
			curr_device = &nand_param[i];
			break;
		}
	}
	if (i == ARRAY_SIZE(nand_param))
		return -ENODEV;
	return 0;
}

static int spinand_probe_id(struct jz_sfc *sfc)
{
	/*
	 * cmd-->addr-->pid
	 */
	unsigned char addrlen[] = {0, 1};
	unsigned char id[4] = {0};
	unsigned char i;

	for (i = 0; i < sizeof(addrlen); i++) {
		SFC_SEND_COMMAND(sfc, CMD_RDID, 2, 0, addrlen[i], 0, 1, 0);
		sfc_read_data((unsigned int *)id, 2);
		if (!probe_id_list(id))
			break;
	}
	if (i == sizeof(addrlen))
		return -ENODEV;
	return 0;
}

static int spinand_init(void)
{
	struct jz_sfc sfc;
	unsigned int x;
	unsigned int tmp = 0;
	unsigned int sfc_dcr = 0;

	/*
	 * Probe nand vid/pid, moving the sample point on after every miss
	 */
	while (spinand_probe_id(&sfc) < 0) {
		sfc_dcr = sfc_readl(SFC_DEV_CONF);
		sfc_dcr &= ~DEV_CONF_SMP_DELAY_MSK;
		tmp++;
		sfc_dcr |= tmp << DEV_CONF_SMP_DELAY_OFFSET;
		sfc_writel(sfc_dcr, SFC_DEV_CONF);
		if (tmp > 10) {
			printf("ERR: don`t support this kind of nand device,please add it\n");
			return -ENODEV;
		}
	}

	/* disable write protect */
	x = 0;
	SFC_SEND_COMMAND(&sfc, CMD_SET_FEATURE, 1, FEATURE_REG_PROTECT, 1, 0,
			 1, 1);
	sfc_write_data(&x, 1);

	x = BITS_QUAD_EN | BITS_ECC_EN | BITS_BUF_EN;
	SFC_SEND_COMMAND(&sfc, CMD_SET_FEATURE, 1, FEATURE_REG_FEATURE1, 1, 0,
			 1, 1);
	sfc_write_data(&x, 1);

	return 0;
}

/* Read count bytes from src_addr, the bad blocks skipped */
int sfc_nand_load(unsigned int src_addr, unsigned int count,
		  unsigned int dst_addr)
{
	unsigned int pageaddr, columnaddr, rlen;
	int ret;
	unsigned char *buf = (unsigned char *)dst_addr;
	unsigned int pagesize = curr_device->pagesize;

	while (count) {
		pageaddr = src_addr / pagesize;
		columnaddr = src_addr % pagesize;
		rlen = (pagesize - columnaddr) < count ?
			(pagesize - columnaddr) : count;
		ret = spinand_read_page(pageaddr, columnaddr, buf, rlen,
					pagesize);
		if (ret > 0) {
			printf("bad block %d\n", pageaddr / CONFIG_SPI_NAND_PPB);
			src_addr += CONFIG_SPI_NAND_PPB * pagesize;
			continue;
		}
		buf += rlen;
		src_addr += rlen;
		count -= rlen;
	}
	return 0;
}

int sfc_nand_init(void)
{
	sfc_controler_init();
	return spinand_init();
}
