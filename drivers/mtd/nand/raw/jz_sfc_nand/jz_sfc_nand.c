// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NAND on the SFC of the Ingenic T40, as the vendor U-Boot drives it
 * (drivers/mtd/devices/jz_sfc_v1/jz_sfc_nand.c of OpenIPC/u-boot-t40):
 * the SFC at CONFIG_SFC_NAND_RATE through the vendor's transfer layer
 * (jz_sfc_common.c), the chip found by its ID among the vendor's chip
 * drivers (chips/, each registering its ID list, geometry and read
 * status handling), a reset, write protection off and the on-chip ECC on,
 * then page reads through the chip cache with the ECC status, page
 * programs, 128 KiB block erases, and the bad block marker in the first
 * byte of the first page's spare area.
 *
 * The chip is registered as nand0 of mainline's raw NAND layer with its
 * own mtd operations, as the vendor registered it, so the nand, mtdparts
 * and ubi commands, the environment in NAND and common/openipc.c work on
 * it. The partitions come from mtdparts, not from the vendor's burner
 * table in the boot partition.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 */

#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <mach/ingenic.h>
#include "jz_sfc_nand.h"
#include "jz_sfc_common.h"
#include "chips/nand_common.h"

static LIST_HEAD(nand_list);
static struct sfc_flash *flash;
static struct nand_chip jz_sfcnand_chip;
/* Per block: 0 unknown, 1 good, 2 bad (the vendor's RAM bad block table) */
static u8 *jz_sfcnand_bbt;

static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs);

static int32_t jz_sfc_nand_erase_blk(struct sfc_flash *flash, uint32_t pageaddr)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_erase *nand_erase_ops = &(nand_desc->ops->nand_erase_ops);
	struct sfc_transfer transfer[2];
	struct sfc_message message;
	struct cmd_info cmd[2];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = 0,
		.buffer = NULL,
		.len = 0,
	};
	int32_t ret;

	memset(transfer, 0, sizeof(transfer));
	memset(cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	/* 1. write enable */
	if (nand_erase_ops->write_enable)
		nand_erase_ops->write_enable(transfer, cmd, &op_info);
	else
		nand_write_enable(transfer, cmd, &op_info);
	sfc_message_add_tail(transfer, &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	/* 2. block erase */
	if (nand_erase_ops->block_erase)
		nand_erase_ops->block_erase(&transfer[1], &cmd[1], &op_info);
	else
		nand_block_erase(&transfer[1], &cmd[1], &op_info);
	sfc_message_add_tail(&transfer[1], &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	/* 3. get feature */
	if (nand_erase_ops->get_feature)
		ret = nand_erase_ops->get_feature(&op_info);
	else
		ret = nand_get_erase_feature(&op_info);

	if (ret)
		printf("Erase error,get state error ! %s %s %d\n", __FILE__,
		       __func__, __LINE__);
	return ret;
}

static int32_t jz_sfc_nand_write(struct sfc_flash *flash, const u_char *buffer,
				 uint32_t pageaddr, uint32_t columnaddr,
				 size_t len)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_write *nand_write_ops = &nand_desc->ops->nand_write_ops;
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = columnaddr,
		.buffer = (u_char *)buffer,
		.len = len,
	};
	int32_t ret = 0;

	memset(transfer, 0, sizeof(transfer));
	memset(cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	/* 1. write enable */
	if (nand_write_ops->write_enable)
		nand_write_ops->write_enable(transfer, cmd, &op_info);
	else
		nand_write_enable(transfer, cmd, &op_info);
	sfc_message_add_tail(transfer, &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	/* 2. write to cache */
	if (nand_desc->param.need_quad) {
		if (nand_write_ops->quad_load)
			nand_write_ops->quad_load(&transfer[1], &cmd[1], &op_info);
		else
			nand_quad_load(&transfer[1], &cmd[1], &op_info);
	} else {
		if (nand_write_ops->single_load)
			nand_write_ops->single_load(&transfer[1], &cmd[1], &op_info);
		else
			nand_single_load(&transfer[1], &cmd[1], &op_info);
	}
	sfc_message_add_tail(&transfer[1], &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	/* 3. program exec */
	if (nand_write_ops->program_exec)
		nand_write_ops->program_exec(&transfer[2], &cmd[2], &op_info);
	else
		nand_program_exec(&transfer[2], &cmd[2], &op_info);
	sfc_message_add_tail(&transfer[2], &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	/* 4. get status to be sure nand write completed */
	if (nand_write_ops->get_feature)
		ret = nand_write_ops->get_feature(&op_info);
	else
		ret = nand_get_program_feature(&op_info);
	return ret;
}

/* > 0: bitflips corrected, -EBADMSG: uncorrectable, -EIO: no answer */
static int32_t jz_sfc_nand_read(struct sfc_flash *flash, int32_t pageaddr,
				int32_t columnaddr, u_char *buffer, size_t len)
{
	struct jz_nand_descriptor *nand_desc = flash->flash_info;
	struct jz_nand_read *nand_read_ops = &nand_desc->ops->nand_read_ops;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = columnaddr,
		.buffer = buffer,
		.len = len,
	};
	int32_t ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	if (nand_read_ops->pageread_to_cache)
		nand_read_ops->pageread_to_cache(&transfer, &cmd, &op_info);
	else
		nand_pageread_to_cache(&transfer, &cmd, &op_info);
	sfc_message_add_tail(&transfer, &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}

	if (nand_read_ops->get_feature) {
		ret = nand_read_ops->get_feature(&op_info);
	} else {
		printf("ERROR: nand device must have get_read_feature function,id_manufactory= %02x, id_device=%02x\n",
		       nand_desc->id_manufactory, nand_desc->id_device);
		ret = -EIO;
	}

	if (ret == -EIO)
		return ret;

	if (nand_desc->param.need_quad) {
		if (nand_read_ops->quad_read)
			nand_read_ops->quad_read(&transfer, &cmd, &op_info);
		else
			/* addrlen = 2, data_dummy_bits = 8 */
			nand_quad_read(&transfer, &cmd, &op_info, 2);
	} else {
		if (nand_read_ops->single_read)
			nand_read_ops->single_read(&transfer, &cmd, &op_info);
		else
			/* addrlen = 2, data_dummy_bits = 8 */
			nand_single_read(&transfer, &cmd, &op_info, 2);
	}
	sfc_message_add_tail(&transfer, &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		ret = -EIO;
	}

	return ret;
}

static int jz_sfcnand_read(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t rlen;
	int32_t ret = 0, reterr = 0;
	unsigned int max_bitflips = 0;

	*retlen = 0;
	while (len) {
		pageaddr = (uint32_t)from / pagesize;
		columnaddr = (uint32_t)from % pagesize;
		rlen = min_t(uint32_t, len, pagesize - columnaddr);
		ret = jz_sfc_nand_read(flash, pageaddr, columnaddr, buf, rlen);
		/* as the MTD core's other NAND drivers count them */
		if (ret == -EBADMSG)
			mtd->ecc_stats.failed++;
		else if (ret > 0)
			mtd->ecc_stats.corrected += ret;
		if (ret < 0) {
			printf("%s: page %u read error %d\n", __func__, pageaddr,
			       ret);
			reterr = ret;
			if (ret == -EIO)
				break;
		} else if (ret > max_bitflips) {
			max_bitflips = ret;
		}
		len -= rlen;
		from += rlen;
		buf += rlen;
		*retlen += rlen;
	}

	return reterr ? reterr : max_bitflips;
}

static int jz_sfcnand_write(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, const u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t wlen;
	int32_t ret = 0;

	*retlen = 0;
	while (len) {
		pageaddr = (uint32_t)to / pagesize;
		columnaddr = (uint32_t)to % pagesize;
		wlen = min_t(uint32_t, pagesize - columnaddr, len);
		ret = jz_sfc_nand_write(flash, buf, pageaddr, columnaddr, wlen);
		if (ret) {
			printf("spi nand write fail %s %s %d\n", __FILE__,
			       __func__, __LINE__);
			break;
		}
		*retlen += wlen;
		len -= wlen;
		to += wlen;
		buf += wlen;
	}
	return ret;
}

/* The spare area of the page at addr, from ooboffs; and its data when asked */
static int jz_sfcnand_read_oob(struct mtd_info *mtd, loff_t from,
			       struct mtd_oob_ops *ops)
{
	uint32_t pageaddr = (uint32_t)from / mtd->writesize;
	int32_t ret = 0;

	ops->retlen = 0;
	ops->oobretlen = 0;
	/* ops->len of the page's data, not more: the buffer may be shorter */
	if (ops->datbuf && ops->len) {
		uint32_t len = min_t(uint32_t, ops->len, mtd->writesize);

		ret = jz_sfc_nand_read(flash, pageaddr, 0, ops->datbuf, len);
		if (ret < 0) {
			printf("spi nand read error %s %d, ret = %d\n",
			       __func__, __LINE__, ret);
			return ret;
		}
		ops->retlen = len;
	}
	if (ops->oobbuf && ops->ooblen) {
		ret = jz_sfc_nand_read(flash, pageaddr,
				       mtd->writesize + ops->ooboffs,
				       ops->oobbuf, ops->ooblen);
		if (ret < 0) {
			printf("spi nand read error %s %d, ret = %d\n",
			       __func__, __LINE__, ret);
			return ret;
		}
		ops->oobretlen = ops->ooblen;
	}
	return ret;
}

static int jz_sfcnand_write_oob(struct mtd_info *mtd, loff_t addr,
				struct mtd_oob_ops *ops)
{
	uint32_t oob_addr = (uint32_t)addr;
	int32_t ret;

	ops->retlen = 0;
	ops->oobretlen = 0;
	/*
	 * Spare area only: the vendor code programmed ops->oobbuf and
	 * ignored ops->datbuf, so a data and OOB write reported success
	 * having written only the spare bytes. Data goes through _write.
	 */
	if (ops->datbuf && ops->len)
		return -EOPNOTSUPP;
	if (!ops->oobbuf || !ops->ooblen)
		return 0;

	ret = jz_sfc_nand_write(flash, (const u_char *)ops->oobbuf,
				oob_addr / mtd->writesize,
				mtd->writesize + ops->ooboffs, ops->ooblen);
	if (ret) {
		printf("spi nand write oob error %s %d\n", __func__, __LINE__);
		return ret;
	}
	ops->oobretlen = ops->ooblen;
	return 0;
}

static int jz_sfc_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	uint32_t addr = (uint32_t)instr->addr;
	uint32_t end;
	int32_t ret = 0;

	if (addr % mtd->erasesize || (uint32_t)instr->len % mtd->erasesize) {
		printf("ERROR:%s line %d erase addr or len not aligned\n",
		       __func__, __LINE__);
		return -EINVAL;
	}
	end = addr + instr->len;
	instr->state = MTD_ERASING;

	while (addr < end) {
		ret = jz_sfc_nand_erase_blk(flash, addr / mtd->writesize);
		if (ret) {
			printf("spi nand erase error blk id  %d !\n",
			       addr / mtd->erasesize);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = addr;
			return ret;
		}
		addr += mtd->erasesize;
	}
	instr->state = MTD_ERASE_DONE;
	return 0;
}

static int jz_sfcnand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;

	ret = jz_sfc_nand_erase(mtd, instr);
	/* A request refused before erasing has no failed block to mark */
	if (ret && instr->state != MTD_ERASE_FAILED)
		return ret;
	if (ret) {
		printf("WARNING: block %d erase fail !\n",
		       (uint32_t)instr->fail_addr / mtd->erasesize);
		if (jz_sfcnand_block_markbad(mtd, instr->fail_addr))
			printf("mark bad block error\n");
		/*
		 * The vendor driver reported success here, so a caller went
		 * on writing into the block it had just marked bad; UBI and
		 * nand erase need the failure to handle it themselves.
		 */
		return -EIO;
	}
	return 0;
}

/*
 * A block is bad when the first byte of its first page's spare area is not
 * 0xff: the factory marker, and what jz_sfcnand_block_markbad() writes.
 * That is the check the vendor's RAM table (nand_default_bbt(), its pattern
 * at badblockpos, one byte on an 8-bit chip) was built with. The vendor
 * scanned the whole chip at the first look-up; this looks a block up the
 * first time it is asked about and keeps the answer.
 */
static int jz_sfcnand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	uint32_t blk = (uint32_t)ofs / mtd->erasesize;
	struct mtd_oob_ops ops;
	u8 marker = 0;

	if (jz_sfcnand_bbt[blk])
		return jz_sfcnand_bbt[blk] == 2;

	memset(&ops, 0, sizeof(ops));
	ops.mode = MTD_OPS_PLACE_OOB;
	ops.oobbuf = &marker;
	ops.ooblen = 1;
	ops.ooboffs = chip->badblockpos;
	if (jz_sfcnand_read_oob(mtd, ofs & ~(mtd->erasesize - 1), &ops) == -EIO)
		return -EIO;

	jz_sfcnand_bbt[blk] = marker == 0xff ? 1 : 2;
	return marker != 0xff;
}

static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	uint32_t blk = (uint32_t)ofs / mtd->erasesize;
	uint8_t buf[2] = { 0, 0 };
	struct mtd_oob_ops ops;
	int ret;

	ret = jz_sfcnand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	/* Write the bad block marker to the first page's OOB */
	memset(&ops, 0, sizeof(ops));
	ops.oobbuf = buf;
	ops.ooboffs = chip->badblockpos;
	ops.len = ops.ooblen = 1;
	ops.mode = MTD_OPS_PLACE_OOB;
	ret = jz_sfcnand_write_oob(mtd, ofs & ~(mtd->erasesize - 1), &ops);

	/* The vendor driver left its RAM table saying good */
	jz_sfcnand_bbt[blk] = 2;
	if (!ret)
		mtd->ecc_stats.badblocks++;
	return ret;
}

static void sfc_nand_reset(void)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	transfer.sfc_mode = TM_STD_SPI;
	cmd.cmd = 0xff;
	transfer.direction = 1;
	transfer.cmd_info = &cmd;
	transfer.ops_mode = CPU_OPS;

	sfc_message_add_tail(&transfer, &message);
	sfc_sync(flash->sfc, &message);
}

static int32_t jz_sfc_nand_try_id(struct sfc_flash *flash,
				  struct jz_nand_descriptor *nand_desc)
{
	struct jz_nand_device *nand_device;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	uint8_t addr_len[2] = {0, 1};
	uint8_t id_buf[2] = {0};
	uint8_t i = 0;

	for (i = 0; i < sizeof(addr_len); i++) {
		memset(id_buf, 0, 2);
		memset(&transfer, 0, sizeof(transfer));
		memset(&cmd, 0, sizeof(cmd));
		sfc_message_init(&message);

		transfer.sfc_mode = TM_STD_SPI;
		cmd.cmd = SPINAND_CMD_RDID;
		transfer.addr = 0;
		transfer.addr_len = addr_len[i];
		cmd.dataen = ENABLE;
		transfer.data = (unsigned char *)id_buf;
		transfer.len = sizeof(id_buf);
		transfer.direction = GLB_TRAN_DIR_READ;
		transfer.data_dummy_bits = 0;
		transfer.cmd_info = &cmd;
		transfer.ops_mode = CPU_OPS;

		sfc_message_add_tail(&transfer, &message);
		if (sfc_sync(flash->sfc, &message)) {
			printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__,
			       __LINE__);
			return -EIO;
		}

		list_for_each_entry(nand_device, &nand_list, nand) {
			if (nand_device->id_manufactory == id_buf[0]) {
				nand_desc->id_manufactory = id_buf[0];
				nand_desc->id_device = id_buf[1];
				break;
			}
		}
		if (nand_desc->id_manufactory && nand_desc->id_device)
			break;
		udelay(500);
	}

	if (!nand_desc->id_manufactory && !nand_desc->id_device) {
		printf("ERROR!: don`t support this nand manufactory, please add nand driver, id_buf[0]= %x\n",
		       id_buf[0]);
		return -ENODEV;
	} else {
		struct device_id_struct *device_id = nand_device->id_device_list;
		int32_t id_count = nand_device->id_device_count;

		while (id_count--) {
			if (device_id->id_device == nand_desc->id_device) {
				nand_desc->param = *device_id->param;
				break;
			}
			device_id++;
		}
		if (id_count < 0) {
			printf("ERROR: do support this device, id_manufactory = 0x%02x, id_device = 0x%02x\n",
			       nand_desc->id_manufactory, nand_desc->id_device);
			return -ENODEV;
		}
		printf("SFC NAND: %s (%02x %02x)\n", device_id->name,
		       nand_desc->id_manufactory, nand_desc->id_device);
	}
	nand_desc->ops = &nand_device->ops;
	return 0;
}

int jz_spinand_register(struct jz_nand_device *flash)
{
	if (!flash)
		return -EINVAL;
	list_add_tail(&flash->nand, &nand_list);
	return 0;
}

static int sfc_nand_get_feature(struct sfc_flash *flash, unsigned char addr,
				char *val)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	cmd.cmd = 0x0f;
	cmd.dataen = ENABLE;
	transfer.addr_len = 1;
	transfer.addr = addr;
	transfer.len = 1;
	transfer.data = (unsigned char *)val;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);
	if (sfc_sync(flash->sfc, &message)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}
	return 0;
}

static int sfc_nand_set_feature(struct sfc_flash *flash, int addr, int val)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;

	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));
	sfc_message_init(&message);

	cmd.cmd = 0x1f;
	cmd.dataen = ENABLE;
	transfer.addr_len = 1;
	transfer.addr = addr;
	transfer.len = 1;
	transfer.data = (unsigned char *)&val;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_WRITE;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);
	ret = sfc_sync(flash->sfc, &message);
	if (ret) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		ret = -EIO;
	}
	return ret;
}

static int sfc_nand_clear_write_protect(struct sfc_flash *flash)
{
	return sfc_nand_set_feature(flash, 0xa0, 0);
}

static int sfc_nand_enable_ecc(struct sfc_flash *flash)
{
	char val = 0;

	sfc_nand_get_feature(flash, 0xb0, &val);
	val |= 0x10;
	return sfc_nand_set_feature(flash, 0xb0, val);
}

static int32_t sfc_nand_special_init(struct sfc_flash *flash)
{
	sfc_nand_clear_write_protect(flash);
	sfc_nand_enable_ecc(flash);
	return 0;
}

static int jz_sfc_nand_init(void)
{
	struct nand_chip *chip = &jz_sfcnand_chip;
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct jz_nand_descriptor *nand_desc;
	struct sfc *sfc;

	flash = calloc(1, sizeof(struct sfc_flash));
	nand_desc = calloc(1, sizeof(struct jz_nand_descriptor));
	if (!flash || !nand_desc)
		return -ENOMEM;

	sfc = sfc_res_init(CONFIG_SFC_NAND_RATE);
	if (IS_ERR(sfc))
		return PTR_ERR(sfc);
	flash->sfc = sfc;
	flash->flash_info = nand_desc;
	flash->mtd = mtd;

	if (spinand_moudle_init())
		return -EINVAL;

	sfc_nand_reset();
	if (jz_sfc_nand_try_id(flash, nand_desc))
		return -ENODEV;

	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->size = nand_desc->param.flashsize;
	mtd->erasesize = nand_desc->param.blocksize;
	mtd->writesize = nand_desc->param.pagesize;
	mtd->writebufsize = mtd->writesize;
	mtd->oobsize = nand_desc->param.oobsize;
	/* the vendor's layout: no free spare bytes for a caller */
	mtd->oobavail = 0;
	mtd->erasesize_shift = ffs(mtd->erasesize) - 1;
	mtd->writesize_shift = ffs(mtd->writesize) - 1;
	mtd->erasesize_mask = (1 << mtd->erasesize_shift) - 1;
	mtd->writesize_mask = (1 << mtd->writesize_shift) - 1;
	mtd->ecc_strength = nand_desc->param.ecc_max;
	mtd->bitflip_threshold = mtd->ecc_strength;

	chip->numchips = 1;
	chip->chipsize = mtd->size;
	chip->page_shift = mtd->writesize_shift;
	chip->bbt_erase_shift = chip->phys_erase_shift = mtd->erasesize_shift;
	chip->badblockbits = 8;
	/* Set the bad block position */
	if (mtd->writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	jz_sfcnand_bbt = calloc(1, mtd->size >> mtd->erasesize_shift);
	if (!jz_sfcnand_bbt)
		return -ENOMEM;

	mtd->_erase = jz_sfcnand_erase;
	mtd->_read = jz_sfcnand_read;
	mtd->_write = jz_sfcnand_write;
	mtd->_read_oob = jz_sfcnand_read_oob;
	mtd->_write_oob = jz_sfcnand_write_oob;
	mtd->_block_isbad = jz_sfcnand_block_isbad;
	mtd->_block_markbad = jz_sfcnand_block_markbad;

	sfc_nand_special_init(flash);

	return nand_register(0, mtd);
}

/* CONFIG_SYS_NAND_SELF_INIT: from nand_init() */
void board_nand_init(void)
{
	int ret = jz_sfc_nand_init();

	if (ret)
		printf("SFC NAND: init failed (%d)\n", ret);
}
