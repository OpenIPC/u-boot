/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#include <fmc_type.h>
#include "td_type.h"
#include "share_drivers.h"
#include "include/soc_errno.h"
#include "fmc100.h"

#define SPI_NOR_MANU_DEV_ID_MIN 2

int write_bp_area_check(struct fmc_host* const host, u_int to, size_t len)
{
#ifdef CONFIG_SPI_BLOCK_PROTECT
	if (host->level) {
		if ((host->cmp == BP_CMP_TOP) && ((to + len) > host->start_addr)) {
			puts("\n");
			db_msg("Reg write area[%#x => %#lx] was locked\n",
					host->start_addr, (to + len));
			return -EINVAL;
		}

		if ((host->cmp == BP_CMP_BOTTOM) && (to < host->end_addr)) {
			unsigned end = ((to + len) > host->end_addr) ?
				host->end_addr : (to + len);

			puts("\n");
			db_msg("Reg write area[%#x => %#x] was locked\n", to,
					end);
			return -EINVAL;
		}
	}
#endif
	return 0;
}

static void fmc100_dma_transfer(struct fmc_spi* const spi,
			 unsigned int spi_start_addr, unsigned char* const dma_buffer,
			 unsigned char rw_op, unsigned int size)
{
	unsigned char if_type = 0;
	unsigned char dummy = 0;
	unsigned char w_cmd = 0;
	unsigned char r_cmd = 0;
	unsigned int regval;
	struct fmc_host *host = spi->host;

	fmc_pr(DMA_DB, "\t\t *-Start dma transfer => [%#x], len[%#x], buf = %p\n",
	       spi_start_addr, size, dma_buffer);

	regval = FMC_INT_CLR_ALL;
	fmc_write(host, FMC_INT_CLR, regval);
	fmc_pr(DMA_DB, "\t\t   Set INT_CLR[%#x]%#x\n", FMC_INT_CLR, regval);

	regval = spi_start_addr;
	fmc_write(host, FMC_ADDRL, regval);
	fmc_pr(DMA_DB, "\t\t   Set ADDRL[%#x]%#x\n", FMC_ADDRL, regval);

	if (rw_op == RW_OP_WRITE) {
		if_type = spi->write->iftype;
		dummy = spi->write->dummy;
		w_cmd = spi->write->cmd;
	} else if (rw_op == RW_OP_READ) {
		if_type = spi->read->iftype;
		dummy = spi->read->dummy;
		r_cmd = spi->read->cmd;
	}

	regval = op_cfg_fm_cs(spi->chipselect) | OP_CFG_OEN_EN |
		 op_cfg_mem_if_type(if_type) | op_cfg_addr_num(spi->addrcycle) |
		 op_cfg_dummy_num(dummy);
	fmc_write(host, FMC_OP_CFG, regval);
	fmc_pr(DMA_DB, "\t\t   Set OP_CFG[%#x]%#x\n", FMC_OP_CFG, regval);

	regval = fmc_dma_len_set(size);
	fmc_write(host, FMC_DMA_LEN, regval);
	fmc_pr(DMA_DB, "\t\t   Set DMA_LEN[%#x]%#x\n", FMC_DMA_LEN, regval);
	/* get hight 32 bits */
	regval = (((uintptr_t)dma_buffer & FMC_DMA_SADDRH_MASK) >> 32);
	fmc_write(host, FMC_DMA_SADDRH_D0, regval);
	fmc_pr(DMA_DB, "\t\t   Set DMA_SADDRH_D0[%#x]%#x\n", FMC_DMA_SADDRH_D0,
	       regval);

	regval = (unsigned int)((uintptr_t)dma_buffer);
	fmc_write(host, FMC_DMA_SADDR_D0, regval);
	fmc_pr(DMA_DB, "\t\t   Set DMA_SADDR_D0[%#x]%#x\n", FMC_DMA_SADDR_D0,
	       regval);

	regval = op_ctrl_rd_opcode(r_cmd) | op_ctrl_wr_opcode(w_cmd) |
		 op_ctrl_rw_op(rw_op) | OP_CTRL_DMA_OP_READY;

	fmc_write(host, FMC_OP_CTRL, regval);
	fmc_pr(DMA_DB, "\t\t   Set OP_CTRL[%#x]%#x\n", FMC_OP_CTRL, regval);

	fmc_dma_wait_int_finish(host);

	fmc_pr(DMA_DB, "\t\t *-End dma transfer.\n");
}

#ifdef FMC100_SPI_NOR_SUPPORT_REG_READ
static char *fmc100_reg_read_buf(struct fmc_host *host,
				struct fmc_spi *spi,
				unsigned int spi_start_addr,
				unsigned int size,
				unsigned char *buffer)
{
	unsigned int regval;

	fmc_pr(DMA_DB, "* Start reg read, from:%#x len:%#x\n", spi_start_addr,
	       size);

	if (size > FMC100_REG_RD_MAX_SIZE)
		db_bug("reg read out of reg range.\n");

	fmc_write(host, FMC_ADDRL, spi_start_addr);
	fmc_pr(DMA_DB, "  Set ADDRL[%#x]%#x\n", FMC_ADDRL, spi_start_addr);

	regval = fmc_data_num_cnt(size);
	fmc_write(host, FMC_DATA_NUM, regval);
	fmc_pr(DMA_DB, "  Set DATA_NUM[%#x]%#x\n", FMC_DATA_NUM, regval);

	regval = op_ctrl_rd_opcode(spi->read->cmd) |
		 op_ctrl_dma_op(OP_TYPE_REG) |
		 op_ctrl_rw_op(RW_OP_READ) |
		 OP_CTRL_DMA_OP_READY;
	fmc_write(host, FMC_OP_CTRL, regval);
	fmc_pr(DMA_DB, "  Set OP_CTRL[%#x]%#x\n", FMC_OP_CTRL, regval);

	fmc_cmd_wait_cpu_finish(host);
	if (!host->iobase)
		return NULL;
	if (memcpy_s(buffer, size, host->iobase, size))
		return NULL;

	fmc_pr(DMA_DB, "*-End reg page read.\n");

	return (char*)buffer;
}

static int read_to_buff(struct fmc_host *host,
			struct fmc_spi *spi,
			loff_t from,
			size_t len,
			u_char *buf)
{
	int num;
	int chip_idx = 0;
	unsigned char *ptr = buf;
	int ret = 0;

	while (len > 0) {
		while (from >= spi->chipsize) {
			from -= spi->chipsize;
			chip_idx++;
			spi++;
			if ((chip_idx == CONFIG_SPI_NOR_MAX_CHIP_NUM) || (!spi->name)) {
				db_bug("read memory out of range.\n");
				ret = -1;
				return ret;
			}
			if (spi->driver->wait_ready(spi)) {
				ret = -1;
				return ret;
			}
			host->set_system_clock(spi->read, ENABLE);
		}

		num = ((from + len) >= spi->chipsize) ? (spi->chipsize - from) : len;

		while (num >= FMC100_REG_RD_MAX_SIZE) {
			fmc100_reg_read_buf(host, spi,
					      from, FMC100_REG_RD_MAX_SIZE, ptr);
			ptr += FMC100_REG_RD_MAX_SIZE;
			from += FMC100_REG_RD_MAX_SIZE;
			len -= FMC100_REG_RD_MAX_SIZE;
			num -= FMC100_REG_RD_MAX_SIZE;
		}

		if (num) {
			fmc100_reg_read_buf(host, spi, from, num, ptr);
			from += num;
			ptr += num;
			len -= num;
		}
	}
	return ret;
}

static ssize_t  fmc100_reg_read(struct spi_flash *spiflash, loff_t from,
				size_t len, u_char *buf)
{
	int result = -EIO;
	struct fmc_host *host = spiflash_to_host(spiflash);
	struct fmc_spi *spi = host->spi;
	unsigned char *fmc_ip;
	int ret;

	if (((from + len) > spiflash->size) || (!len)) {
		db_msg("read area out of range or len is zero\n");
		return -EINVAL;
	}

	fmc_ip = get_fmc_ip();
	if (*fmc_ip) {
		fmc_print("Warning: Hifmc IP is busy, Please try again.\n");
		udelay(100); /* delay 100 us */
		return -EBUSY;
	} else {
		fmc_dev_type_switch(FLASH_TYPE_SPI_NOR);
		(*fmc_ip)++;
	}

	if (spi->driver->wait_ready(spi)) {
		db_msg("Error: Dma read wait ready fail!\n");
		result = -EBUSY;
		goto fail;
	}

	host->set_system_clock(spi->read, ENABLE);

	ret = read_to_buff(host, spi, from, len, buf);
	if (ret)
		goto fail;

	result = 0;

fail:
	(*fmc_ip)--;
	return result;
}
#endif

#if !defined(FMC100_SPI_NOR_SUPPORT_REG_READ) || !defined(FMC100_SPI_NOR_SUPPORT_REG_WRITE)
static int dma_cycle_op(const struct spi_flash *spiflash, unsigned char rw_op,
				loff_t from, loff_t len, const void *buf)
{
	int op_len;
	loff_t num;
	int chip_idx = 0;
	struct spi_op *op = NULL;
	struct fmc_host *host = spiflash_to_host(spiflash);
	struct fmc_spi *spi = host->spi;

	if (rw_op == RW_OP_READ) {
		op_len = FMC100_DMA_RD_MAX_SIZE;
		op = spi->read;
	} else {
		op_len = FMC100_DMA_WR_MAX_SIZE;
		op = spi->write;
	}

	if (!len || !op_len)
	    return -1;

	while (len) {
		num = ((len >= op_len) ? op_len : len);

		while (from >= spi->chipsize) {
			from -= spi->chipsize;
			chip_idx++;
			spi++;
			if ((chip_idx == CONFIG_SPI_NOR_MAX_CHIP_NUM) || (!spi->name)) {
				db_bug("read memory out of range.\n");
				return -1;
			}

			if (spi->driver->wait_ready(spi)) {
				db_msg("Error: Dma op wait ready fail!!\n");
				return -EBUSY;
			}

			host->set_system_clock(op, ENABLE);
		}

		if (from + num > spi->chipsize)
			num = spi->chipsize - from;

		fmc100_dma_transfer(spi, from, (u_char *)buf,
				rw_op, num);
		from += num;
		buf  += num;
		len  -= num;
	}
	return 0;
}
#endif

#ifndef FMC100_SPI_NOR_SUPPORT_REG_READ
static ssize_t  fmc100_dma_read(struct spi_flash* const spiflash, loff_t from, size_t len,
				unsigned char* const buf)
{
	int result = -EIO;
	struct fmc_host *host = spiflash_to_host(spiflash);
	struct fmc_spi *spi = host->spi;
	unsigned char *fmc_ip = NULL;

	fmc_pr(RD_DBG, "\t|*-Start dma read, from:%#x len:%#x buf:%#x\n", from, len, buf);

	if (((from + len) > spiflash->size) || (!len)) {
		db_msg("read area out of range[%#x].\n", spiflash->size);
		return -EINVAL;
	}

	fmc_ip = get_fmc_ip();
	if (*fmc_ip) {
		fmc_print("Warning: Hifmc IP is busy, Please try again.\n");
		udelay(100); /* delay 100 us */
		return -EBUSY;
	} else {
		fmc_dev_type_switch(FLASH_TYPE_SPI_NOR);
		(*fmc_ip)++;
	}

	if (spi->driver->wait_ready(spi)) {
		db_msg("Error: Dma read wait ready fail!\n");
		result = -EBUSY;
		goto fail;
	}

	host->set_system_clock(spi->read, ENABLE);

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range((uintptr_t)buf, (uintptr_t)(buf + len));
#endif
	if (dma_cycle_op(spiflash, RW_OP_READ, from, len, buf))
		goto fail;

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range((uintptr_t)buf, (uintptr_t)(buf + len));
#endif

	result = 0;
fail:
	(*fmc_ip)--;

	fmc_pr(RD_DBG, "\t|*-End dma read.\n");

	return result;
}
#endif

void fmc100_read_ids(const struct fmc_spi *spi, u_char cs, u_char* const id)
{
	unsigned int reg;
	struct fmc_host *host = NULL;

	if (!spi || !spi->host || !id)
		return;
	host = spi->host;
	if (!host->iobase)
		return;
	fmc_pr(BT_DBG, "\t|||*-Start Read SPI(cs:%d) ID.\n", cs);

	reg = fmc_cmd_cmd1(SPI_CMD_RDID);
	fmc_write(host, FMC_CMD, reg);
	fmc_pr(BT_DBG, "\t||||-Set CMD[%#x]%#x\n", FMC_CMD, reg);

	reg = op_cfg_fm_cs(cs) | OP_CFG_OEN_EN;
	fmc_write(host, FMC_OP_CFG, reg);
	fmc_pr(BT_DBG, "\t||||-Set OP_CFG[%#x]%#x\n", FMC_OP_CFG, reg);

	reg = fmc_data_num_cnt(MAX_SPI_NOR_ID_LEN);
	fmc_write(host, FMC_DATA_NUM, reg);
	fmc_pr(BT_DBG, "\t||||-Set DATA_NUM[%#x]%#x\n", FMC_DATA_NUM, reg);

	reg = fmc_op_cmd1_en(ENABLE) |
	      fmc_op_read_data_en(ENABLE) |
	      FMC_OP_REG_OP_START;
	fmc_write(host, FMC_OP, reg);
	fmc_pr(BT_DBG, "\t||||-Set OP[%#x]%#x\n", FMC_OP, reg);

	fmc_cmd_wait_cpu_finish(host);

	if (memcpy_s(id, MAX_SPI_NOR_ID_LEN, host->iobase, MAX_SPI_NOR_ID_LEN))
		return;
	fmc_pr(BT_DBG, "\t|||*-Read CS: %d ID: %#X %#X %#X %#X %#X %#X\n",
	       cs, id[0], id[1], id[2], id[3], id[4], id[5]);
}

static void spi_nor_func_hook(struct fmc_host* const host)
{
	struct spi_flash *spi_nor_flash = host->spi_nor_flash;

	fmc_pr(BT_DBG, "\t||-Initial spi_flash structure\n");
	spi_nor_flash->name = "fmc";

#ifdef FMC100_SPI_NOR_SUPPORT_REG_READ
	spi_nor_flash->read = fmc100_reg_read;
#else
	spi_nor_flash->read = fmc100_dma_read;
#endif
}

struct spi_flash *fmc100_spi_nor_scan(struct fmc_host *host)
{
	unsigned char cs;
	if (!host || !host->set_host_addr_mode)
		return NULL;

	struct spi_flash *spi_nor_flash = host->spi_nor_flash;
	struct fmc_spi *spi = host->spi;
	struct mtd_info_ex *spi_nor_info = host->spi_nor_info;
	if (!spi_nor_flash || !spi || !spi_nor_info)
		return NULL;
	fmc_pr(BT_DBG, "\t|*-Start Scan SPI nor flash\n");

	spi_nor_flash->size = 0;

	for (cs = 0; cs < CONFIG_SPI_NOR_MAX_CHIP_NUM; cs++)
		host->spi[cs].host = host;

	fmc_pr(BT_DBG, "\t||-Initial mtd_info_ex structure\n");
	if (memset_s(spi_nor_info, sizeof(struct mtd_info_ex), 0, sizeof(struct mtd_info_ex)))
		return NULL;

	if (!fmc_spi_nor_probe(spi_nor_info, spi)) {
#ifndef CONFIG_SPI_NOR_QUIET_TEST
		fmc_print("Can't find a valid spi nor flash chip.\n");
#endif
		return NULL;
	}

	if (spi->addrcycle == SPI_NOR_4BYTE_ADDR_LEN) {
		host->set_host_addr_mode(host, ENABLE);
		fmc_pr(AC_DBG, "\t* Controller entry 4-byte mode.\n");
	}

	spi_nor_func_hook(host);

#ifdef CONFIG_DTR_MODE_SUPPORT
	if (spi->dtr_mode_support) {
		int ret;

		host->dtr_training_flag = 0;
		/* setting DTR mode dummy and traning */
		ret = spi_dtr_dummy_training_set(host, ENABLE);
		/* If training setting fail, must reset back to SDR mode,
		 * Note: logic auto turn on DTR when read
		 *       auto turn off DTR when write and erase */
		if (ret) {
			fmc_print("Reset to STR mode.\n");
			/* switch DTR mode to SDR mode */
			fmc_dtr_mode_ctrl(spi, DISABLE);
			spi_dtr_to_sdr_switch(spi);
			spi_dtr_dummy_training_set(host, DISABLE);
		}
	}
#endif

	fmc_pr(BT_DBG, "\t|*-Found SPI nor flash: %s\n", spi_nor_info->name);

	return spi_nor_flash;
}

static void fmc100_set_host_addr_mode(struct fmc_host* const host, int enable)
{
	unsigned int regval = fmc_read(host, FMC_CFG);

	/* 1: SPI_NOR_ADDR_MODE_4_BYTES 0: SPI_NOR_ADDR_MODE_3_BYTES */
	if (enable)
		regval |= SPI_NOR_ADDR_MODE_MASK;
	else
		regval &= ~SPI_NOR_ADDR_MODE_MASK;

	fmc_write(host, FMC_CFG, regval);
}

static int fmc100_host_init(struct fmc_host *host)
{
	unsigned int reg, flash_type;

	fmc_pr(BT_DBG, "\t|||*-Start SPI Nor host init\n");

	host->regbase = (void *)CONFIG_HIFMC_REG_BASE;
	host->iobase = (void *)CONFIG_HIFMC_BUFFER_BASE;

	reg = fmc_read(host, FMC_CFG);
	fmc_pr(BT_DBG, "\t||||-Get CFG[%#x]%#x\n", FMC_CFG, reg);
	flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
	if (flash_type != FLASH_TYPE_SPI_NOR) {
		db_msg("Error: Flash type isn't SPI Nor. reg: %#x\n", reg);
		return -ENODEV;
	}

	if ((reg & OP_MODE_MASK) == OP_MODE_BOOT) {
		reg |= fmc_cfg_op_mode(OP_MODE_NORMAL);
		fmc_pr(BT_DBG, "\t||||-Controller enter normal mode\n");
		fmc_write(host, FMC_CFG, reg);
		fmc_pr(BT_DBG, "\t||||-Set CFG[%#x]%#x\n", FMC_CFG, reg);
	}

	host->set_system_clock = fmc_set_fmc_system_clock;
	host->set_host_addr_mode = fmc100_set_host_addr_mode;

	reg = timing_cfg_tcsh(CS_HOLD_TIME) |
	      timing_cfg_tcss(CS_SETUP_TIME) |
	      timing_cfg_tshsl(CS_DESELECT_TIME);
	fmc_write(host, FMC_SPI_TIMING_CFG, reg);
	fmc_pr(BT_DBG, "\t||||-Set TIMING[%#x]%#x\n", FMC_SPI_TIMING_CFG, reg);

	fmc_pr(BT_DBG, "\t|||*-End SPI Nor host init\n");

	return 0;
}

int fmc100_spi_nor_init(struct fmc_host *host)
{
	int ret;
	if (!host) {
		db_msg("Error: host is NULL, please check input parameter\n");
		return -1;
	}

#ifdef CONFIG_DTR_MODE_SUPPORT
	if (memcpy_s((void *)DTR_TRAINING_CMP_ADDR_S, DTR_TRAINING_CMP_LEN,
		(void *)FMC_MEM_BASE, DTR_TRAINING_CMP_LEN)) {
		return -1;
	}
#endif
	fmc_pr(BT_DBG, "\t||*-Start fmc100 SPI Nor init\n");

	fmc_pr(BT_DBG, "\t|||-Hifmc100 host structure init\n");
	ret = fmc100_host_init(host);
	if (ret) {
		db_msg("Error: SPI Nor host init failed, result: %d\n", ret);
		return ret;
	}

	fmc_pr(BT_DBG, "\t|||-Set default system clock, Enable controller\n");
	if (host->set_system_clock)
		host->set_system_clock(NULL, ENABLE);

	fmc_pr(BT_DBG, "\t||*-End fmc100 SPI Nor init\n");

	return ret;
}


#ifdef CONFIG_DTR_MODE_SUPPORT
int spi_dtr_dummy_training_set(struct fmc_host *host, int dtr_en)
{
	struct fmc_spi *spi = host->spi;
	int ret;

	switch (spi->dtr_cookie) {
	case DTR_MODE_SET_ODS:
		if (spi->driver->dtr_set_device)
			spi->driver->dtr_set_device(spi, dtr_en);
		break;
	case DTR_MODE_SET_NONE:
	default:
		break;
	}

	/* disable DTR mode without training */
	/* dtr dummy training is done, return it */
	if ((host->dtr_training_flag == 1) || (dtr_en == DISABLE))
		return 0;

	/* enable DTR mode and set sample point */
	fmc_dtr_mode_ctrl(spi, ENABLE);

	/* set training */
	ret = spi_dtr_training(host);
	if (ret) {
		fmc_pr(DTR_DB, " * Set dtr training fail.\n");
		return 1;
	}
	fmc_pr(DTR_DB, "* Set dtr and dummy end.\n");
	return 0;
}

static int select_sample_point(unsigned char *status, int len)
{
	int ix;
	unsigned int p_count = 0;
	unsigned int p_temp = 0;
	unsigned int reg = 0;

	if (len <= 0)
		return 0;

	/* select the best smaple point */
	for (ix = 0; ix < len;) {
		if (status[ix] == 1) {
			p_count++;
			ix++;
			if ((status[ix] == 0) && (p_count > p_temp)) {
				p_temp = p_count;
				p_count = 0;
				reg = ix - ((p_temp + 1) >> 1);
				fmc_pr(DTR_DB, "the sample point choice: %#x\n", reg);
				break;
			}
			continue;
		}
		ix++;
	}
	return reg;
}

static int training_read_process(struct fmc_spi *spi,
					unsigned char *buf)
{
	int ret;

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range((uintptr_t)buf,
				(uintptr_t)(buf + DTR_TRAINING_CMP_LEN));
#endif

	fmc100_dma_transfer(spi, DTR_TRAINING_CMP_ADDR_SHIFT, buf,
				RW_OP_READ, DTR_TRAINING_CMP_LEN);

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range((uintptr_t)buf,
				(uintptr_t)(buf + DTR_TRAINING_CMP_LEN));
#endif

	ret = memcmp_ss((const void *)buf,
	    (const void *)DTR_TRAINING_CMP_ADDR_S,
		DTR_TRAINING_CMP_LEN, NO_CHECK_WORD);

	return ret;
}


static int dtr_training_handle(struct fmc_host *host)
{
	int ret = 0;
	int ix;
	unsigned int reg;
	unsigned int regval;
	unsigned char *buf = NULL;
	unsigned char status[DTR_TRAINING_POINT_NUM] = {0};
	struct fmc_spi *spi = host->spi;

	spi->driver->wait_ready(spi);

	/* set div 4 clock */
	host->set_system_clock(spi->read, ENABLE);

	buf = malloc(DTR_TRAINING_CMP_LEN);
	if (!buf)
		return -1;

	/* start training to check every sample point */
	regval = fmc_read(host, FMC_GLOBAL_CFG);
	for (ix = 0; ix < DTR_TRAINING_POINT_NUM; ix++) {
		regval = dtr_training_point_clr(regval);
		regval |= (ix << DTR_TRAINING_POINT_MASK);
		fmc_pr(DTR_DB, " setting the dtr training point:%d\n", ix);
		fmc_write(host, FMC_GLOBAL_CFG, regval);
		fmc_pr(DTR_DB, " Set dtr_training[%#x]%#x\n",
		       FMC_GLOBAL_CFG, regval);
		/* read */
		ret = training_read_process(spi, buf);
		if (ret == EXT_SEC_SUCCESS) {
			/* Just to reduce the use of variables, no other reasion */
			reg = 1;
			status[ix] = 1; /* like */
			fmc_pr(DTR_DB, " status[%d] = 1\n", ix);
		}
		if (!reg && (ix == DTR_TRAINING_POINT_NUM - 1))
			goto fail_training;
	}

	free(buf);

	/* select the best smaple point */
	reg = 0;
	reg = select_sample_point(status, DTR_TRAINING_POINT_NUM);

	/* to set the best sample point */
	regval = dtr_training_point_clr(regval);
	regval |= (reg << DTR_TRAINING_POINT_MASK);
	fmc_pr(DTR_DB, " set the sample point[%#x]%#x\n",
	       FMC_GLOBAL_CFG, regval);
	fmc_write(host, FMC_GLOBAL_CFG, regval);

	/* training handle end */
	return regval;

fail_training:
	fmc_print("Cannot find an useful sample point.\n");
	free(buf);
	return -1;
}

unsigned int spi_dtr_training(struct fmc_host *host)
{
	int reg, cur_reg;

	fmc_pr(DTR_DB, "DTR traiining start ...\n");
	/* DTR traiining start */
	reg = dtr_training_handle(host);
	if (reg == -1) {
		host->dtr_training_flag = 0;
		fmc_print("DTR traiining fail.\n");
		return 1;
	}
	fmc_pr(DTR_DB, "* DTR traiining end.\n");
	cur_reg = fmc_read(host, FMC_GLOBAL_CFG);
	/* to check whether training is done */
	if (cur_reg == reg) {
		host->dtr_training_flag = 1;
		fmc_pr(DTR_DB, "* Set dtr_training seccess.\n");
		return 0;
	}
	return 1;
}

void fmc_dtr_mode_ctrl(struct fmc_spi *spi, int dtr_en)
{
	unsigned int regval;
	struct fmc_host *host = (struct fmc_host *)spi->host;

	host->dtr_mode_en = dtr_en;
	regval = fmc_read(host, FMC_GLOBAL_CFG);
	if (dtr_en == ENABLE) {
		/* enable DTR mode and set the DC value */
		regval |= (1 << DTR_MODE_REQUEST_SHIFT);
		fmc_write(host, FMC_GLOBAL_CFG, regval);
		fmc_pr(DTR_DB, " enable dtr mode[%#x]%#x\n",
		       FMC_GLOBAL_CFG, regval);
	} else {
		/* disable DTR mode */
		regval &= (~(1 << DTR_MODE_REQUEST_SHIFT));
		fmc_write(host, FMC_GLOBAL_CFG, regval);
		fmc_pr(DTR_DB, " disable dtr mode[%#x]%#x\n",
		       FMC_GLOBAL_CFG, regval);
	}
}

void fmc_check_spi_dtr_support(struct fmc_spi *spi, u_char *ids, int len)
{
	unsigned char manu_id;
	unsigned char dev_id;

	if (len < SPI_NOR_MANU_DEV_ID_MIN)
		return;

	manu_id = ids[0];
	dev_id = ids[1];

	spi->dtr_mode_support = 0;
	spi->dtr_cookie = DTR_MODE_SET_NONE;

	switch (manu_id) {
	case MID_MXIC:
		if (spi_mxic_check_spi_dtr_support(spi)) {
			spi->dtr_cookie = DTR_MODE_SET_ODS;
			goto dtr_on;
		}
		break;
	case MID_WINBOND:
		/* Device ID: 0x70 means support DTR Mode for Winbond */
		if (dev_id == DEVICE_ID_SUPPORT_DTR_WINBOND) {
			spi->dtr_mode_support = 1;
			goto dtr_on;
		}
		break;
	default:
		break;
	}

	fmc_pr(DTR_DB, "The Double Transfer Rate Read Mode isn't supported.\n");
	return;

dtr_on:
	fmc_pr(FMC_INFO, "The Double Transfer Rate Read Mode is supported.\n");
}
#endif /* CONFIG_DTR_MODE_SUPPORT */

void fmc100_op_reg(struct fmc_spi* const spi, unsigned char opcode,
				unsigned int len, unsigned char optype)
{
	struct fmc_host *host = NULL;
	u32 regval;

	if (!spi || !spi->host)
		return;

	host = (struct fmc_host *)spi->host;
	regval = fmc_cmd_cmd1(opcode);
	fmc_write(host, FMC_CMD, regval);

	regval = fmc_data_num_cnt(len);
	fmc_write(host, FMC_DATA_NUM, regval);

	regval = op_cfg_fm_cs(spi->chipselect) | OP_CFG_OEN_EN;
	fmc_write(host, FMC_OP_CFG, regval);

	regval = fmc_op_cmd1_en(ENABLE) | FMC_OP_REG_OP_START | optype;
	fmc_write(host, FMC_OP, regval);

	fmc_cmd_wait_cpu_finish(host);
}
