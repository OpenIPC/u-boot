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
#include <spi_flash.h>
#ifdef CONFIG_USB
#include <usb.h>
#endif
#ifdef CONFIG_MMC
#include <mmc.h>
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

#if (defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)) && defined(CONFIG_FS_FAT)
static struct spi_flash *recovery_flash;

static int recovery_write(const char *name) {
  struct image_header hdr;
  loff_t actread;
  ulong addr, size, erase_len;
  int ret;

  if (!fs_exists(name))
    return 0;

  ret = fs_read(name, (ulong)&hdr, 0, sizeof(hdr), &actread);
  if (ret || actread != sizeof(hdr) || !image_check_magic(&hdr)) {
    printf("Recovery: no valid uImage header in %s\n", name);
    return -1;
  }

  addr = image_get_load(&hdr);
  size = sizeof(hdr) + image_get_data_size(&hdr);

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

static int recovery_mount(const char *ifname) {
  if (!fs_set_blk_dev(ifname, "0:1", FS_TYPE_FAT))
    return 0;
  if (!fs_set_blk_dev(ifname, "0", FS_TYPE_FAT))
    return 0;
  return -1;
}

int firmware_recovery(void) {
  char name[64];
  int i, ret, found = 0;
  static const char *fmt[] = { "uImage.%s.img", "rootfs.squashfs.%s.img" };

  recovery_flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                                   CONFIG_SF_DEFAULT_SPEED,
                                   CONFIG_SF_DEFAULT_MODE);
  if (!recovery_flash)
    return -1;

#ifdef CONFIG_MMC
	{
		struct mmc *mmc = find_mmc_device(0);

		if (mmc && !mmc_init(mmc)) {
			if (!recovery_mount("mmc")) {
				for (i = 0; i < 2; i++) {
					sprintf(name, fmt[i], CONFIG_PRODUCT_SOC);
					ret = recovery_write(name);
					if (ret > 0)
						found = 1;
				}
			}
		}
	}
#endif

#ifdef CONFIG_USB_STORAGE
	if (!usb_init() && usb_stor_scan(1) == 0) {
		if (!recovery_mount("usb")) {
			for (i = 0; i < 2; i++) {
				sprintf(name, fmt[i], CONFIG_PRODUCT_SOC);
				ret = recovery_write(name);
				if (ret > 0)
					found = 1;
			}
		}
	}
#endif

  fs_close();
  spi_flash_free(recovery_flash);

  return found ? 0 : -1;
}
#else
int firmware_recovery(void) {
  return -1;
}
#endif

int firmware_scan(void) {
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

    char mtdparts[128];
    sprintf(mtdparts, "%dk(u-boot),%dk(env),%lldk(kernel),%lldk(rootfs),%lldk@0x%lx(firmware),-(rootfs_data)",
            CONFIG_ENV_OFFSET / 1024, CONFIG_ENV_SIZE / 1024, kernel_size / 1024, rootfs_size / 1024,
            (kernel_size + rootfs_size) / 1024, kernel_off);
    env_set("mtdparts", mtdparts);
  }

  if (kernel_off && rootfs_off) {
    env_set("bootfail", NULL);
  } else {
    printf("Firmware is absent/corrupt (kernel=0x%lx, rootfs=0x%lx)\n", kernel_off, rootfs_off);
    env_set("bootfail", "1");
  }

  return 0;
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

int openipc_helper() {
  char msize[16];
  sprintf(msize, "%ldM", gd->ram_size / 1024 / 1024);
  env_set("totalmem", msize);
  firmware_recovery();
  firmware_scan();

  return 0;
}
