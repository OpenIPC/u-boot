/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include "types.h"
#include "soc_errno.h"
#include "cipher_romable.h"
#include "share_drivers.h"

#define SHARE_DRIVER_ENTRY	  0x7f00
/* sec cipher */
#define BOOTROM_DRV_REGISTER_FUNC (SHARE_DRIVER_ENTRY)
#define OTP_GET_DIE_ID		  (BOOTROM_DRV_REGISTER_FUNC + 4)
#define EXT_DRV_CIPHER_INIT	  (OTP_GET_DIE_ID + 4)
#define EXT_DRV_CIPHER_DEINIT	  (EXT_DRV_CIPHER_INIT + 4)
#define EXT_DRV_CIPHER_CREATE_KEYSLOT_BY_SET_EFFECTIVE_KEY \
	(EXT_DRV_CIPHER_DEINIT + 4)
#define EXT_DRV_CIPHER_DESTROY_KEYSLOT \
	(EXT_DRV_CIPHER_CREATE_KEYSLOT_BY_SET_EFFECTIVE_KEY + 4)
#define EXT_DRV_CIPHER_OTP_READ_WORD   (EXT_DRV_CIPHER_DESTROY_KEYSLOT + 4)
#define EXT_DRV_CIPHER_TRNG_GET_RANDOM (EXT_DRV_CIPHER_OTP_READ_WORD + 4)
#define EXT_DRV_CIPHER_DMA_COPY	       (EXT_DRV_CIPHER_TRNG_GET_RANDOM + 4)
#define EXT_DRV_CIPHER_SYMC_DECRYPT    (EXT_DRV_CIPHER_DMA_COPY + 4)
#define EXT_DRV_CIPHER_SHA256	       (EXT_DRV_CIPHER_SYMC_DECRYPT + 4)
#define EXT_DRV_CIPHER_PKE_RSA_VERIFY  (EXT_DRV_CIPHER_SHA256 + 4)
#define EXT_DRV_CIPHER_FUNC_REGISTER   (EXT_DRV_CIPHER_PKE_RSA_VERIFY + 4)
#define MMC_INIT		       (EXT_DRV_CIPHER_FUNC_REGISTER + 4)
#define MMC_READ		       (MMC_INIT + 4)
#define IS_BOOTMODE		       (MMC_READ + 4)
#define MMC_GET_CUR_MODE	       (IS_BOOTMODE + 4)
#define MMC_SET_BOOTUP_MODE	       (MMC_GET_CUR_MODE + 4)
#define COPY_FROM_SDIO		       (MMC_SET_BOOTUP_MODE + 4)
#define SET_SDIO_POS		       (COPY_FROM_SDIO + 4)
#define ENABLE_SDIO_DMA		       (SET_SDIO_POS + 4)
#define SELF_USB_CHECK		       (ENABLE_SDIO_DMA + 4)
#define COPY_FROM_USB		       (SELF_USB_CHECK + 4)
#define USB3_DRIVER_INIT	       (COPY_FROM_USB + 4)
#define SEND_TO_USB		       (USB3_DRIVER_INIT + 4)
#define PL011_PUTC		       (SEND_TO_USB + 4)
#define PL011_GETC		       (PL011_PUTC + 4)
#define SERIAL_INIT		       (PL011_GETC + 4)
#define LOG_SERIAL_PUTS		       (SERIAL_INIT + 4)
#define COPY_FROM_UART		       (LOG_SERIAL_PUTS + 4)
#define TIMER_INIT		       (COPY_FROM_UART + 4)
#define TIMER_DEINIT		       (TIMER_INIT + 4)
#define TIMER_START		       (TIMER_DEINIT + 4)
#define TIMER_GET_VAL		       (TIMER_START + 4)
#define UDELAY			       (TIMER_GET_VAL + 4)
#define MDELAY			       (UDELAY + 4)
#define WATCHDOG_ENABLE		       (MDELAY + 4)
#define WATCHDOG_DISABLE	       (WATCHDOG_ENABLE + 4)
#define WATCHDOG_FEED		       (WATCHDOG_DISABLE + 4)
#define MEMCMP_SS		       (WATCHDOG_FEED + 4)
#define MEMCPY_SS		       (MEMCMP_SS + 4)
#define MEMSET_SS		       (MEMCPY_SS + 4)
#define MEMCPY_S		       (MEMSET_SS + 4)
#define MEMSET_S		       (MEMCPY_S + 4)
#define SAVE_CUR_POINT_SYSCNT	       (MEMSET_S + 4)
#define SELF_SDIO_CHECK		       (SAVE_CUR_POINT_SYSCNT + 4)
#define SERIAL_TSTC		       (SELF_SDIO_CHECK + 4)

void bootrom_drv_register_func(const bootrom_drv_func_list *func_list)
{
	typedef __typeof__(bootrom_drv_register_func) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)BOOTROM_DRV_REGISTER_FUNC));
	return entry(func_list);
}

int32_t ext_drv_cipher_func_register(const ext_drv_func *func)
{
	typedef __typeof__(ext_drv_cipher_func_register) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_FUNC_REGISTER));
	return entry(func);
}

td_s32 otp_get_die_id(td_u8 *die_id, const td_u32 len)
{
	typedef __typeof__(otp_get_die_id) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)OTP_GET_DIE_ID));
	return entry(die_id, len);
}

int32_t ext_drv_cipher_init(void)
{
	typedef __typeof__(ext_drv_cipher_init) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_INIT));
	return entry();
}

int32_t ext_drv_cipher_deinit(void)
{
	typedef __typeof__(ext_drv_cipher_deinit) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_DEINIT));
	return entry();
}

int32_t ext_drv_cipher_create_keyslot_by_set_effective_key(
	unsigned int *keyslot_handle, ext_km_rootkey_type type,
	const uint8_t *salt, uint32_t salt_len)
{
	typedef __typeof__(ext_drv_cipher_create_keyslot_by_set_effective_key)
		function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_CREATE_KEYSLOT_BY_SET_EFFECTIVE_KEY));
	return entry(keyslot_handle, type, salt, salt_len);
}

int32_t ext_drv_cipher_destroy_keyslot(unsigned int keyslot_handle)
{
	typedef __typeof__(ext_drv_cipher_destroy_keyslot) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_DESTROY_KEYSLOT));
	return entry(keyslot_handle);
}

int32_t ext_drv_cipher_otp_read_word(const uint32_t addr, uint32_t *data)
{
	typedef __typeof__(ext_drv_cipher_otp_read_word) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_OTP_READ_WORD));
	return entry(addr, data);
}

int32_t ext_drv_cipher_trng_get_random(uint32_t *random)
{
	typedef __typeof__(ext_drv_cipher_trng_get_random) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_TRNG_GET_RANDOM));
	return entry(random);
}

int32_t ext_drv_cipher_dma_copy(uint64_t src, uint64_t dst, uint32_t length)
{
	typedef __typeof__(ext_drv_cipher_dma_copy) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_DMA_COPY));
	if (length > MAX_DMA_COPY_LENGTH)
		return EXT_SEC_FAILURE;
	return entry(src, dst, length);
}

int32_t ext_drv_cipher_symc_decrypt(ext_symc_alg symc_alg,
				    uint32_t keyslot_chn_num, uint8_t *iv,
				    uint32_t iv_length, uint64_t src,
				    uint64_t dst, uint32_t length)
{
	typedef __typeof__(ext_drv_cipher_symc_decrypt) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_SYMC_DECRYPT));
	if ((iv == NULL) | (length % SYCC_DATA_ALIGN_LEN != 0))
		return EXT_SEC_FAILURE;
	return entry(symc_alg, keyslot_chn_num, iv, iv_length, src, dst,
		     length);
}

int32_t ext_drv_cipher_sha256(const uint8_t *data, uint32_t data_len,
			      uint8_t *hash, uint32_t hash_len)
{
	typedef __typeof__(ext_drv_cipher_sha256) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_SHA256));
	if (hash_len != HASH_LEN)
		return EXT_SEC_FAILURE;
	return entry(data, data_len, hash, hash_len);
}

int32_t ext_drv_cipher_pke_rsa_verify(const ext_pke_rsa_pub_key *pub_key,
				      ext_pke_rsa_scheme scheme,
				      const ext_pke_hash_type hash_type,
				      const ext_pke_data *input_hash,
				      const ext_pke_data *sign)
{
	typedef __typeof__(ext_drv_cipher_pke_rsa_verify) function_type;
	function_type *entry =
		(function_type *)(*((td_u32 *)(uintptr_t)EXT_DRV_CIPHER_PKE_RSA_VERIFY));
	if (entry(pub_key, scheme, hash_type, input_hash, sign) == EXT_SEC_SUCCESS)
		return EXT_SEC_SUCCESS;
	else
		return EXT_SEC_FAILURE;
}

/* mmc */
int mmc_init(void)
{
	typedef __typeof__(mmc_init) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MMC_INIT));
	return entry();
}

int mmc_read(void *ptr, size_t src, size_t size, size_t read_type)
{
	typedef __typeof__(mmc_read) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MMC_READ));
	return entry(ptr, src, size, read_type);
}

unsigned int is_bootmode(void)
{
	typedef __typeof__(is_bootmode) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)IS_BOOTMODE));
	return entry();
}

emmc_mode_u mmc_get_cur_mode(void)
{
	typedef __typeof__(mmc_get_cur_mode) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MMC_GET_CUR_MODE));
	return entry();
}

void mmc_set_bootup_mode(sys_boot_u mode)
{
	typedef __typeof__(mmc_set_bootup_mode) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MMC_SET_BOOTUP_MODE));
	return entry(mode);
}

/* sdio */
int copy_from_sdio(void *buffer, unsigned long maxsize)
{
	typedef __typeof__(copy_from_sdio) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)COPY_FROM_SDIO));
	return entry(buffer, maxsize);
}

void set_sdio_pos(unsigned long pos)
{
	typedef __typeof__(set_sdio_pos) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SET_SDIO_POS));
	return entry(pos);
}

void enable_sdio_dma(void)
{
	typedef __typeof__(enable_sdio_dma) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)ENABLE_SDIO_DMA));
	entry();
}
/* usb */
int self_usb_check(void)
{
	typedef __typeof__(self_usb_check) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SELF_USB_CHECK));
	return entry();
}

int copy_from_usb(void *dest, size_t *count, u32 max_length)
{
	typedef __typeof__(copy_from_usb) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)COPY_FROM_USB));
	return entry(dest, count, max_length);
}

void usb3_driver_init(void)
{
	typedef __typeof__(usb3_driver_init) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)USB3_DRIVER_INIT));
	entry();
}

int send_to_usb(char *addr, size_t count)
{
	typedef __typeof__(send_to_usb) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SEND_TO_USB));
	return entry(addr, count);
}

/* uart */
void pl011_putc(uart_num uart_base, s8 c)
{
	typedef __typeof__(pl011_putc) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)PL011_PUTC));
	return entry(uart_base, c);
}

td_s32 pl011_getc(uart_num uart_base)
{
	typedef __typeof__(pl011_getc) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)PL011_GETC));
	return entry(uart_base);
}

td_s32 serial_init(uart_num uart_base, const uart_cfg *cfg)
{
	typedef __typeof__(serial_init) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SERIAL_INIT));
	return entry(uart_base, cfg);
}

void log_serial_puts(const s8 *s)
{
	typedef __typeof__(log_serial_puts) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)LOG_SERIAL_PUTS));
	return entry(s);
}

td_s32 copy_from_uart(void *dest, size_t *count, td_u32 max_length)
{
	typedef __typeof__(copy_from_uart) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)COPY_FROM_UART));
	return entry(dest, count, max_length);
}

/* timer */
void timer_init(void)
{
	typedef __typeof__(timer_init) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)TIMER_INIT));
	entry();
}

void timer_deinit(void)
{
	typedef __typeof__(timer_deinit) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)TIMER_DEINIT));
	entry();
}

void timer_start(void)
{
	typedef __typeof__(timer_start) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)TIMER_START));
	entry();
}

unsigned long timer_get_val(void)
{
	typedef __typeof__(timer_get_val) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)TIMER_GET_VAL));
	return entry();
}

void udelay(unsigned long usec)
{
	typedef __typeof__(udelay) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)UDELAY));
	entry(usec);
}

void mdelay(unsigned long msec)
{
	typedef __typeof__(mdelay) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MDELAY));
	((void (*)(unsigned long msec))((uintptr_t)entry))(msec);
}

/* watchdog */
td_s32 watchdog_enable(td_u32 n, td_u32 timeout, td_u32 wdg_freq)
{
	typedef __typeof__(watchdog_enable) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)WATCHDOG_ENABLE));
	return entry(n, timeout, wdg_freq);
}

td_s32 watchdog_disable(td_u32 n)
{
	typedef __typeof__(watchdog_disable) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)WATCHDOG_DISABLE));
	return entry(n);
}
td_s32 watchdog_feed(td_u32 n, td_u32 timeout)
{
	typedef __typeof__(watchdog_feed) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)WATCHDOG_FEED));
	return entry(n, timeout);
}
/* sec mem opt */
td_s32 memcmp_ss(const td_void *cs, const td_void *ct, td_size_t count,
		 uintptr_t check_word)
{
	typedef __typeof__(memcmp_ss) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MEMCMP_SS));
	return entry(cs, ct, count, check_word);
}

td_s32 memcpy_ss(td_void *dest, td_size_t dest_max, const td_void *src,
		 td_size_t count, uintptr_t check_word)
{
	typedef __typeof__(memcpy_ss) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MEMCPY_SS));
	return entry(dest, dest_max, src, count, check_word);
}

td_s32 memset_ss(td_void *dest, td_size_t dest_max, td_u8 c, td_size_t count,
		 uintptr_t check_word)
{
	typedef __typeof__(memset_ss) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MEMSET_SS));
	return entry(dest, dest_max, c, count, check_word);
}

errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count)
{
	typedef __typeof__(memcpy_s) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MEMCPY_S));
	return entry(dest, dest_max, src, count);
}

errno_t memset_s(void *dest, size_t dest_max, int c, size_t count)
{
	typedef __typeof__(memset_s) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)MEMSET_S));
	return entry(dest, dest_max, c, count);
}

void save_cur_point_syscnt(unsigned int index)
{
	typedef __typeof__(save_cur_point_syscnt) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SAVE_CUR_POINT_SYSCNT));
	entry(index);
}

int self_sdio_check(void)
{
	typedef __typeof__(self_sdio_check) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SELF_SDIO_CHECK));
	return entry();
}

s32 serial_tstc(uart_num uart_base)
{
	typedef __typeof__(serial_tstc) function_type;
	function_type *entry = (function_type *)(*((td_u32 *)(uintptr_t)SERIAL_TSTC));
	return entry(uart_base);
}
