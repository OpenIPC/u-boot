#include <string.h>
#include "constant.h"
#include "uart_upgrade.h"
#include "scsi_op.h"
#include "debug.h"
#include "debug.h"
#include "global.h"


#define _THUMB2 __attribute__((target("thumb")))

UART_Verify_CB guiUARTCheck_cb;
UART_Vendor_CB guiUARTVendor_cb;
static int uartrom_exit = 0;
static unsigned int chipid = 0;
static CONSOLE_OBJ *m_console = NULL;
static INT32U nvtuart_opened = 1;

_THUMB2 static INT32U calc_sum(INT32U dat0, INT32U dat1, INT32U dat2, INT32U dat3)
{
	INT32U sum = 0;

	sum = (dat0 & 0xFFFF) + ((dat0 >> 16)) + (dat1 & 0xFFFF) + ((dat1 >> 16)) +
		  (dat2 & 0xFFFF) + ((dat2 >> 16)) + (dat3 & 0xFFFF) + ((dat3 >> 16)) + 28;

	return  sum & 0xFFFF;
}

_THUMB2 static INT32U make_sum(INT32U dat0, INT32U dat1, INT32U dat2, INT32U dat3)
{
	INT32U sum = 0;

	sum = (dat0 & 0xFFFF) + ((dat0 >> 16)) + (dat1 & 0xFFFF) + ((dat1 >> 16)) +
		  (dat2 & 0xFFFF) + ((dat2 >> 16)) + (dat3 & 0xFFFF) + ((dat3 >> 16)) + 28;

	sum = (~sum + 1) & 0xFFFF;

	return  sum;
}

_THUMB2 static int recv_data(char *p, unsigned int len)
{
	while (len) {
		*p = m_console->getc();
		p++;
		len--;
	}
	return 0;
}

_THUMB2 static int send_data(char *p, unsigned int len)
{
	while (len) {
		m_console->putc(*p);
		p++;
		len--;
	}
	return 0;
}

_THUMB2 static void uart_send_csw(INT32U *cdb, int er)
{
	char *p_cdb = (char *)cdb;
	p_cdb[IDX_CDB_UART_STATUS_ER] = er;

	unsigned int sum = make_sum(cdb[0], cdb[1], cdb[2], (cdb[3] & 0xFFFF));
	cdb[3] |= sum << 16;

	send_data((char *)cdb, sizeof(INT32U) * 4);
}


_THUMB2 static int uart_cmd(INT32U *cdb)
{
	unsigned int cmd_id = ((cdb[0] >> 8) & 0xFF);
	unsigned int insize = ((cdb[1] >> 16) & 0xFFFF) + ((cdb[2] & 0xFFFF) << 16);
	unsigned int tmp1, tmp2;

	switch (cmd_id) {
	case SCSIOP_OUT_OPEN_DEVICE:
		nvtuart_opened = 1;
		uart_send_csw(cdb, 0);
		break;

	case SCSIOP_IN_WAIT_DONE:
		uartrom_exit = 1;
		uart_send_csw(cdb, 0);
		break;

	case SCSIOP_IN_IS_NVT:
		chipid = uart_read_reg(TOP_CTRL_VERSION_REG);
		uart_send_csw(cdb, 0);
		send_data((char *)&chipid, sizeof(chipid));
		break;

	case SCSIOP_OUT_ADDR_WRITE:
		tmp1 = ((cdb[0] >> 16) & 0xFFFF) + ((cdb[1] & 0xFFFF) << 16); //addr
		tmp2 = ((cdb[1] >> 16) & 0xFFFF) + ((cdb[2] & 0xFFFF) << 16); //data
		*(INT32U *)tmp1 = tmp2;
		__asm__ __volatile__("dsb\n\t");
		uart_send_csw(cdb, 0);
		break;

	case SCSIOP_IN_ADDR_READ:
		tmp1 = ((cdb[0] >> 16) & 0xFFFF) + ((cdb[1] & 0xFFFF) << 16); //addr
		tmp2 = *(INT32U *)tmp1;
		__asm__ __volatile__("dsb\n\t");
		uart_send_csw(cdb, 0);
		send_data((char *)&tmp1, sizeof(tmp2)); //repeat addr
		send_data((char *)&tmp2, sizeof(tmp2)); //reply data
		break;

	default:
		if (guiUARTCheck_cb != NULL) {
			NVT_SCSI_CBW cbw;
			unsigned int OutDataBuf = 0;
			utl_memset(&cbw, 0, sizeof(cbw));
			cbw.dCBWDataTransferLength = insize;
			utl_memcpy(cbw.CBWCB, cdb, 4 * sizeof(unsigned int));
			int er = guiUARTCheck_cb((unsigned int)&cbw, (unsigned int *)&OutDataBuf, (unsigned int *)&insize);
			utl_memcpy(cdb, cbw.CBWCB, 4 * sizeof(unsigned int));
			if (er != 0) {
				uart_send_csw(cdb, 1);
				return -1;
			} else {
				uart_send_csw(cdb, 0);
			}
			if ((cmd_id & 0x80)) {
				//dir-in
				guiUARTVendor_cb((unsigned int)&cbw);
				if (insize) {
					send_data((char *)OutDataBuf, insize);
				}
			} else {
				//dir-out
				if (insize) {
					recv_data((char *)OutDataBuf, insize);
				}
				guiUARTVendor_cb((unsigned int)&cbw);
			}
		}
		break;

	}
	return 0;
}

_THUMB2 static void uart_upgrade_routine(void)
{
	static int idx_cdb = 0;
	static INT32U cdb[4] = {0};

	char *p_cdb = (char *)cdb;

	p_cdb[idx_cdb++] = m_console->getc();

	// loop to cdb fill 16 bytes
	if (idx_cdb < 16) {
		return ;
	}

	idx_cdb = 0; // reset idx

	// Parse SCSI command
	if (!(((cdb[0] & 0xFF) == SCSI_OP_SET0) || ((cdb[0] & 0xFF) == SCSI_OP_SET1) || ((cdb[0] & 0xFF) == SCSI_OP_SET2))) {
		uart_send_csw(cdb, 1);
		debug_err("uart opset ng.\r\n");
	}

	if (calc_sum(cdb[0], cdb[1], cdb[2], cdb[3]) != 0) {
		uart_send_csw(cdb, 1);
		debug_err("uart cmd sum ng.\r\n");
		return;
	}

	uart_cmd(cdb);
}

_THUMB2 void uart_upgrade_procedure()
{
	//if(utl_get_chipversion() != CHIPVER_A) {
		m_console = get_uart_object(CONSOLE_UART0);
	//	} else {
	//		m_console = get_uart_object(CONSOLE_UART1);
	//}

	m_console->hook();

	while (1) {

		if (uartrom_exit > 0) {
			break;
		}

		uart_upgrade_routine();
	}

	uartrom_exit = 0;

}

_THUMB2 void loader_installUART_Verify_CB(UART_Verify_CB callback)
{
	guiUARTCheck_cb = callback;
}

_THUMB2 void loader_installUART_Vendor_CB(UART_Vendor_CB callback)
{
	guiUARTVendor_cb = callback;
}


