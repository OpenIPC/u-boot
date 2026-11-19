/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __SPI_H__
#define __SPI_H__

#define SPI_MEM_DATA_IN		1
#define SPI_MEM_DATA_OUT	2

/* Linux's struct spi_mem_op, as the vendor code lays it out. */
struct spi_mem_op {
	struct {
		u8 buswidth;
		u8 opcode;
	} cmd;
	struct {
		u8 nbytes;
		u8 buswidth;
		u32 val;
	} addr;
	struct {
		u8 nbytes;
		u8 buswidth;
	} dummy;
	struct {
		u8 buswidth;
		u32 dir;
		u32 nbytes;
		void *buf;
	} data;
};

void spi_init(u32 hz);
int spi_mem_adjust_op_size(struct spi_mem_op *op);
int spi_mem_exec_op(struct spi_mem_op *op);
#endif
