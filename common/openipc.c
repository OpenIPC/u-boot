#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <common.h>
#include <env.h>
#include <fs.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <vsprintf.h>
#include <spi_flash.h>
#ifdef CONFIG_USB
#include <usb.h>
#endif
#ifdef CONFIG_MMC
#include <mmc.h>
#endif
#ifdef CONFIG_ENV_IS_IN_NAND
#include <nand.h>
#endif
#ifdef CONFIG_FMC_SPI_NAND
#include <command.h>
#include <linux/err.h>
#include <linux/mtd/ubi.h>
#include <mtd.h>
#include <mtd/ubi-user.h>
#include <ubi_uboot.h>
#endif

#define SCAN_FROM CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE
#define SCAN_STEP 0x10000
#define SCAN_LIMIT 0x400000

#define UBI_MAGIC_ASCII "UBI#"
#define FDT_MAGIC 0xd00dfeed

#define SQSH_SIZE_OFF 0x58
#define SQSH_MAGIC_BE 0x73717368
#define SQSH_MAGIC_LE 0x68737173

static int check_squashfs(void *buf) {
  uint32_t magic = get_unaligned_be32(buf);
  if (magic == SQSH_MAGIC_BE || magic == SQSH_MAGIC_LE)
    return 1;
  return 0;
}

#ifdef CONFIG_FMC_SPI_NAND
#define UBIFS_SB_MAGIC 0x06101831

static int check_ubifs(void *buf) {
  return get_unaligned_le32(buf) == UBIFS_SB_MAGIC;
}
#endif

static int check_kernel(void *buf) {
  struct image_header *hdr = (struct image_header *)buf;
  if (image_check_magic(hdr)) {
    return 1;
  }

  if (fdt_magic(buf) == FDT_MAGIC) {
    if (fdt_check_header(buf) == 0) {
      return 1;
    }
  }

  return 0;
}

#ifndef CONFIG_FMC_SPI_NAND

static uint64_t get_squashfs_size(void *buf) {
  uint32_t magic = get_unaligned_be32(buf);
  uint64_t size;

  if (magic == SQSH_MAGIC_BE) {
    size = get_unaligned_be64((u8 *)buf + SQSH_SIZE_OFF);
  } else {
    size = get_unaligned_le64((u8 *)buf + SQSH_SIZE_OFF);
  }

  return size;
}

static int check_rootfs(void *buf, uint64_t *rootfs_size) {
  if (check_squashfs(buf)) {
    *rootfs_size = get_squashfs_size(buf);
    return 1;
  }
  if (memcmp(buf, UBI_MAGIC_ASCII, 4) == 0) {
    *rootfs_size = 0;
    return 1;
  }
  return 0;
}

static int scan_spi_device(struct spi_flash *flash, ulong *kernel_off, ulong *rootfs_off, uint64_t *kernel_size,
                           uint64_t *rootfs_size) {
  u8 *buf;
  ulong offset;
  ulong erase_size;
  int ret;

  buf = malloc_cache_aligned(SCAN_STEP);
  if (!buf)
    return -1;

#ifdef CONFIG_SPI_FLASH
  erase_size = flash->sector_size;
#else
  erase_size = 0x10000;
#endif

  printf("Scanning 0x%x-0x%x ...\n", SCAN_FROM, SCAN_LIMIT);

  for (offset = SCAN_FROM; offset < SCAN_LIMIT && offset < flash->size; offset += SCAN_STEP) {
    ret = spi_flash_read(flash, offset, SCAN_STEP, buf);
    if (ret != 0)
      continue;

    /* Skip completely empty regions */
    if (buf[0] == 0x00 || buf[0] == 0xFF) {
      int i, uniform = 1;
      for (i = 1; i < 32; i++) {
        if (buf[i] != buf[0]) {
          uniform = 0;
          break;
        }
      }
      if (uniform)
        continue;
    }

    if (!*kernel_off && check_kernel(buf)) {
      *kernel_off = offset;
      printf("Found Kernel at 0x%08lx\n", offset);
    }

    if (!*rootfs_off && check_rootfs(buf, rootfs_size)) {
      *rootfs_off = offset;
      printf("Found RootFS at 0x%08lx (%lldk)\n", offset, *rootfs_size / 1024);
    }

    if (*kernel_off && *rootfs_off)
      break;
  }

  if (*kernel_off && *rootfs_off) {
    *kernel_size = *rootfs_off - *kernel_off;
  }

  /* Align rootfs size to erase block boundary */
  if (*rootfs_size > 0) {
    ulong aligned_size = (*rootfs_size + erase_size - 1) & ~(erase_size - 1);
    *rootfs_size = aligned_size;
  }

  free(buf);
  return 0;
}

#endif /* !CONFIG_FMC_SPI_NAND */

#if (defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)) && defined(CONFIG_FS_FAT)
#define RECOVERY_KERNEL 0
#define RECOVERY_ROOTFS 1

static int recovery_mount(const char *ifname) {
  if (!fs_set_blk_dev(ifname, "0:1", FS_TYPE_FAT))
    return 0;
  if (!fs_set_blk_dev(ifname, "0", FS_TYPE_FAT))
    return 0;
  return -1;
}

#ifdef CONFIG_FMC_SPI_NAND
static int recovery_ensure_volume(const char *name, const char *size) {
  struct ubi_volume_desc *desc;
  char cmd[64];

  desc = ubi_open_volume_nm(0, name, UBI_READONLY);
  if (!IS_ERR(desc)) {
    ubi_close_volume(desc);
    return 0;
  }

  printf("Recovery: creating volume %s\n", name);
  snprintf(cmd, sizeof(cmd), "ubi create %s %s d", name, size);

  return run_command(cmd, 0);
}

static int recovery_write(const char *name, int type) {
  struct image_header hdr;
  loff_t actread;
  ulong size;
  int ret;

  if (!fs_exists(name))
    return 0;

  if (type == RECOVERY_ROOTFS) {
    /* Raw ubifs image, written into the rootfs volume */
    loff_t fsize;

    ret = fs_size(name, &fsize);
    if (ret || fsize == 0) {
      printf("Recovery: failed to stat %s\n", name);
      return -1;
    }
    size = fsize;
  } else {
    /* Kernel: uImage or FIT, written to the kernel volume */
    ret = fs_read(name, (ulong)&hdr, 0, sizeof(hdr), &actread);
    if (ret || actread != sizeof(hdr)) {
      printf("Recovery: failed to read %s\n", name);
      return -1;
    }

    if (image_check_magic(&hdr)) {
      size = sizeof(hdr) + image_get_data_size(&hdr);
    } else if (fdt_magic(&hdr) == FDT_MAGIC && fdt_check_header(&hdr) == 0) {
      size = fdt_totalsize(&hdr);
    } else {
      printf("Recovery: no valid uImage/FIT header in %s\n", name);
      return -1;
    }
  }

  ret = fs_read(name, CONFIG_SYS_LOAD_ADDR, 0, size, &actread);
  if (ret || actread != size) {
    printf("Recovery: failed to read %s\n", name);
    return -1;
  }

  if (type == RECOVERY_ROOTFS) {
    ret = recovery_ensure_volume("rootfs", "0");
    if (ret) {
      printf("Recovery: cannot create rootfs volume (%d)\n", ret);
      return -1;
    }

    ret = ubi_volume_write("rootfs", (void *)CONFIG_SYS_LOAD_ADDR, size);
    if (ret) {
      printf("Recovery: failed to write rootfs volume (%d)\n", ret);
      return -1;
    }

    printf("Recovery: wrote %s (%lu bytes) to rootfs volume\n", name, size);
  } else {
    ret = recovery_ensure_volume("kernel", "0x400000");
    if (ret) {
      printf("Recovery: cannot create kernel volume (%d)\n", ret);
      return -1;
    }

    ret = ubi_volume_write("kernel", (void *)CONFIG_SYS_LOAD_ADDR, size);
    if (ret) {
      printf("Recovery: failed to write kernel volume (%d)\n", ret);
      return -1;
    }

    printf("Recovery: wrote %s (%lu bytes) to kernel volume\n", name, size);
  }

  return 1;
}
#else
static struct spi_flash *recovery_flash;

static int recovery_write(const char *name, int type) {
  struct image_header hdr;
  loff_t actread;
  ulong addr, size, erase_len;
  int ret;

  if (!fs_exists(name))
    return 0;

  ret = fs_read(name, (ulong)&hdr, 0, sizeof(hdr), &actread);
  if (ret || actread != sizeof(hdr)) {
    printf("Recovery: failed to read %s\n", name);
    return -1;
  }

  if (image_check_magic(&hdr)) {
    addr = image_get_load(&hdr);
    size = sizeof(hdr) + image_get_data_size(&hdr);
  } else if (type == RECOVERY_KERNEL && fdt_magic(&hdr) == FDT_MAGIC &&
             fdt_check_header(&hdr) == 0) {
    const char *str = env_get("kernaddr");

    addr = str ? hextoul(str, NULL) : CONFIG_ENV_KERNADDR;
    size = fdt_totalsize(&hdr);
  } else {
    printf("Recovery: no valid uImage/FIT header in %s\n", name);
    return -1;
  }

  ret = fs_read(name, CONFIG_SYS_LOAD_ADDR, 0, size, &actread);
  if (ret || actread != size) {
    printf("Recovery: failed to read %s\n", name);
    return -1;
  }

  erase_len = (size + recovery_flash->sector_size - 1) &
              ~(recovery_flash->sector_size - 1);
  spi_flash_erase(recovery_flash, addr, erase_len);
  spi_flash_write(recovery_flash, addr, size, (void *)CONFIG_SYS_LOAD_ADDR);

  printf("Recovery: wrote %s (%lu bytes) to flash at 0x%lx\n", name, size,
         addr);

  return 1;
}
#endif

static int recovery_try(void) {
  char name[64];
  int ret, found = 0;

  /* Kernel: uImage preferred, FIT fallback (creates the kernel volume) */
  sprintf(name, "uImage.%s.img", CONFIG_PRODUCT_SOC);
  ret = recovery_write(name, RECOVERY_KERNEL);
  if (ret > 0) {
    found = 1;
  } else if (ret == 0) {
    sprintf(name, "fitImage.%s.img", CONFIG_PRODUCT_SOC);
    ret = recovery_write(name, RECOVERY_KERNEL);
    if (ret > 0)
      found = 1;
  }

  /* Rootfs (creates its volume with the remaining space) */
#ifdef CONFIG_FMC_SPI_NAND
  sprintf(name, "rootfs.ubifs.%s.img", CONFIG_PRODUCT_SOC);
#else
  sprintf(name, "rootfs.squashfs.%s.img", CONFIG_PRODUCT_SOC);
#endif
  ret = recovery_write(name, RECOVERY_ROOTFS);
  if (ret > 0)
    found = 1;

  return found;
}

int firmware_recovery(void) {
  int found = 0;

#ifndef CONFIG_FMC_SPI_NAND
  recovery_flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                                   CONFIG_SF_DEFAULT_SPEED,
                                   CONFIG_SF_DEFAULT_MODE);
  if (!recovery_flash)
    return -1;
#else
  if (ubi_part("ubi", NULL)) {
    printf("Recovery: cannot attach UBI\n");
    return -1;
  }
#endif

#ifdef CONFIG_MMC
	{
		struct mmc *mmc = find_mmc_device(0);

		if (mmc && !mmc_init(mmc) && !recovery_mount("mmc"))
			found |= recovery_try();
	}
#endif

#ifdef CONFIG_USB_STORAGE
	if (!usb_init() && usb_stor_scan(1) == 0 && !recovery_mount("usb"))
		found |= recovery_try();
#endif

  fs_close();
#ifndef CONFIG_FMC_SPI_NAND
  spi_flash_free(recovery_flash);
#endif

  return found ? 0 : -1;
}
#else
int firmware_recovery(void) {
  return -1;
}
#endif

int firmware_scan(void) {
#ifdef CONFIG_FMC_SPI_NAND
  u8 buf[64];
  int ret;

  printf("Checking UBI firmware...\n");

  if (ubi_part("ubi", NULL)) {
    printf("Firmware is absent/corrupt (cannot attach UBI)\n");
    env_set("bootfail", "1");
    return 0;
  }

  ret = ubi_volume_read("kernel", (char *)buf, sizeof(buf));
  if (ret || !check_kernel(buf)) {
    printf("Firmware is absent/corrupt (kernel volume)\n");
    env_set("bootfail", "1");
    return 0;
  }

  ret = ubi_volume_read("rootfs", (char *)buf, sizeof(buf));
  if (ret || !(check_ubifs(buf) || check_squashfs(buf))) {
    printf("Firmware is absent/corrupt (rootfs volume)\n");
    env_set("bootfail", "1");
    return 0;
  }

  env_set("bootfail", NULL);

  return 0;
#else
  struct spi_flash *flash;
  ulong kernel_off = 0;
  ulong rootfs_off = 0;
  uint64_t kernel_size = 0;
  uint64_t rootfs_size = 0;

  printf("Automatic flash layout detection...\n");

  flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
  if (!flash) {
    printf("Failed to probe SPI flash at %u:%u\n", CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS);
    return -1;
  }

  scan_spi_device(flash, &kernel_off, &rootfs_off, &kernel_size, &rootfs_size);

#ifndef CONFIG_TARGET_HI3516CV610_FAMILY
  if (kernel_size <= 0x200000)
    kernel_size = 0x200000;
  if (rootfs_size < 0x500000)
    rootfs_size = 0x500000;
  else if (rootfs_size > 0x500000 && flash->size > 0x800000)
    rootfs_size = 0xa00000;
#endif

  spi_flash_free(flash);

  if (kernel_off) {
    char str[32];
    sprintf(str, "0x%08lx", kernel_off);
    env_set("kernaddr", str);

    sprintf(str, "0x%llx", kernel_size);
    env_set("kernsize", str);
  }

  if (rootfs_off) {
    char str[32];
    sprintf(str, "0x%08lx", rootfs_off);
    env_set("rootaddr", str);

    sprintf(str, "0x%llx", rootfs_size);
    env_set("rootsize", str);

    char mtds[128], mtdparts[160];
    const char *mtdids = env_get("mtdids");
    sprintf(mtds, "%dk(u-boot),%dk(env),%lldk(kernel),%lldk(rootfs),%lldk@0x%lx(firmware),-(rootfs_data)",
            CONFIG_ENV_OFFSET / 1024, CONFIG_ENV_SIZE / 1024, kernel_size / 1024, rootfs_size / 1024,
            (kernel_size + rootfs_size) / 1024, kernel_off);
    env_set("mtds", mtds);
    sprintf(mtdparts, "%s:%s", mtdids ? mtdids : "", mtds);
    env_set("mtdparts", mtdparts);
  }

  if (kernel_off && rootfs_off) {
    env_set("bootfail", NULL);
  } else {
    printf("Firmware is absent/corrupt (kernel=0x%lx, rootfs=0x%lx)\n", kernel_off, rootfs_off);
    env_set("bootfail", "1");
  }

  return 0;
#endif
}

#ifndef OPENIPC_RAM_MAX_SIZE
#ifdef PHYS_SDRAM_1_SIZE
#define OPENIPC_RAM_MAX_SIZE PHYS_SDRAM_1_SIZE
#endif
#endif

#if defined(OPENIPC_RAM_MAX_SIZE) && defined(CONFIG_SYS_SDRAM_BASE)
ulong openipc_ram_size(void)
{
  return get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, OPENIPC_RAM_MAX_SIZE);
}
#endif

/* Check whether the device environment area is still blank (donor-style
 * CRC check): only then it is safe to write the default environment. */
static int env_crc_is_blank(void) {
  u32 crc;

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
  struct spi_flash *flash;

  flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                          CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
  if (!flash)
    return 0;

  if (spi_flash_read(flash, CONFIG_ENV_OFFSET, sizeof(crc), &crc)) {
    spi_flash_free(flash);
    return 0;
  }

  spi_flash_free(flash);
#elif defined(CONFIG_ENV_IS_IN_NAND)
  struct mtd_info *mtd = get_nand_dev_by_index(0);
  size_t len = sizeof(crc);

  if (!mtd)
    return 0;

  if (nand_read_skip_bad(mtd, CONFIG_ENV_OFFSET, &len, NULL, mtd->size,
                         (u_char *)&crc))
    return 0;

  if (len != sizeof(crc))
    return 0;
#else
  return 0;
#endif

  return crc == 0xffffffff;
}

int openipc_helper(void) {
  char msize[16];
  sprintf(msize, "%ldM", gd->ram_size / 1024 / 1024);
  env_set("totalmem", msize);
  firmware_recovery();
  firmware_scan();

  /* Persist the default environment on first start (or after a bad env);
   * only if the device env area is still blank so we never overwrite
   * foreign data (e.g. a RAM-booted U-Boot on another device). */
  if ((gd->flags & GD_FLG_ENV_DEFAULT) && env_crc_is_blank())
    env_save();

  return 0;
}
