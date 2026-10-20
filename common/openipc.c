#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <command.h>
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
#include <u-boot/crc.h>
#include <linux/sizes.h>
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
#include <linux/err.h>
#include <linux/mtd/ubi.h>
#include <ubifs_uboot.h>
#include <mtd.h>
#include <mtd/ubi-user.h>
#include <ubi_uboot.h>
#endif

#define SCAN_FROM (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define SCAN_STEP 0x10000
/* How far to look for the kernel, and for the rootfs past the kernel's end */
#define SCAN_LIMIT 0x400000

#define UBI_MAGIC_ASCII "UBI#"
#define FDT_MAGIC 0xd00dfeed

#define SQSH_SIZE_OFF 0x58
#define SQSH_MAGIC_BE 0x73717368
#define SQSH_MAGIC_LE 0x68737173

static int __maybe_unused check_squashfs(void *buf) {
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

/* Check for the UBI marker without attaching: attaching an empty MTD
 * would create a fresh UBI on flash. */
static int ubi_marker_present(void) {
  struct mtd_info *mtd;
  u8 buf[4];
  size_t retlen;
  int ret;

  /* Register the mtdparts partitions before looking one up by name */
  mtd_probe_devices();
  mtd = get_mtd_device_nm("ubi");
  if (IS_ERR(mtd))
    return 0;

  ret = mtd_read(mtd, 0, sizeof(buf), &retlen, buf);
  put_mtd_device(mtd);

  return !ret && retlen == sizeof(buf) &&
         get_unaligned_be32(buf) == UBI_EC_HDR_MAGIC;
}
#endif

static int __maybe_unused check_kernel(void *buf) {
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

/* The kernel image's size from its header, or 0 when it has none */
static ulong kernel_image_size(void *buf) {
  if (image_check_magic((struct image_header *)buf))
    return image_get_image_size((struct image_header *)buf);
  if (fdt_magic(buf) == FDT_MAGIC && fdt_check_header(buf) == 0)
    return fdt_totalsize(buf);
  return 0;
}

static int scan_spi_device(struct spi_flash *flash, ulong *kernel_off, ulong *rootfs_off, uint64_t *kernel_size,
                           uint64_t *rootfs_size) {
  u8 *buf;
  ulong offset, limit;
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

  limit = min_t(ulong, SCAN_FROM + SCAN_LIMIT, flash->size);
  printf("Scanning 0x%x-0x%lx ...\n", SCAN_FROM, limit);

  for (offset = SCAN_FROM; offset < limit; offset += SCAN_STEP) {
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
      ulong end = offset + ALIGN(kernel_image_size(buf), SCAN_STEP);

      *kernel_off = offset;
      printf("Found Kernel at 0x%08lx\n", offset);
      /*
       * The rootfs is past the kernel: skip the image, whose contents could
       * pass for one, and look as far past its end as before it. A large
       * kernel (arm64) ends beyond the first window.
       */
      if (end > offset) {
        limit = min_t(ulong, max_t(ulong, limit, end + SCAN_LIMIT), flash->size);
        offset = end - SCAN_STEP;
        continue;
      }
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

static const char *recovery_if, *recovery_part;

/* The first partition, else the whole medium (a card formatted without a
 * partition table); the one that worked is remembered for the calls after. */
static int recovery_mount(const char *ifname) {
  if (recovery_if != ifname) {
    recovery_if = ifname;
    recovery_part = NULL;
  }
  if (recovery_part)
    return fs_set_blk_dev(ifname, recovery_part, FS_TYPE_FAT) ? -1 : 0;
  if (!fs_set_blk_dev(ifname, "0:1", FS_TYPE_FAT)) {
    recovery_part = "0:1";
    return 0;
  }
  if (!fs_set_blk_dev(ifname, "0", FS_TYPE_FAT)) {
    recovery_part = "0";
    return 0;
  }
  return -1;
}

/*
 * Every fs_* call ends in fs_close(), which forgets the device: the next one
 * would read through no filesystem at all ("failed to read" right after
 * fs_exists found the file). So each call selects the medium again.
 */
static int rfs_exists(const char *name) {
  return !recovery_mount(recovery_if) && fs_exists(name);
}

static int rfs_size(const char *name, loff_t *size) {
  return recovery_mount(recovery_if) ? -1 : fs_size(name, size);
}

static int rfs_read(const char *name, ulong addr, loff_t offset, loff_t len,
                    loff_t *actread) {
  return recovery_mount(recovery_if) ? -1 : fs_read(name, addr, offset, len, actread);
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

/* Recreate a system volume so it always has the expected size; the old
 * one is removed first because UBI volumes cannot be resized. With
 * `liberate' set, a create that runs out of space removes rootfs_data
 * and retries: growing the rootfs is the only way to need its PEBs. */
static int recovery_recreate_volume(const char *name, const char *size,
                                    bool liberate) {
  struct ubi_volume_desc *desc;
  char cmd[64];
  int ret;

  desc = ubi_open_volume_nm(0, name, UBI_READONLY);
  if (!IS_ERR(desc)) {
    ubi_close_volume(desc);
    printf("Recovery: recreating volume %s\n", name);
    snprintf(cmd, sizeof(cmd), "ubi remove %s", name);
    run_command(cmd, 0);
  } else {
    printf("Recovery: creating volume %s\n", name);
  }

  snprintf(cmd, sizeof(cmd), "ubi create %s %s d", name, size);
  ret = run_command(cmd, 0);

  if (ret && liberate) {
    printf("Recovery: no room for %s, dropping rootfs_data\n", name);
    run_command("ubi remove rootfs_data", 0);
    ret = run_command(cmd, 0);
  }

  return ret;
}

static int recovery_write(const char *name, int type) {
  struct image_header hdr;
  loff_t actread;
  ulong size;
  int ret;

  if (!rfs_exists(name))
    return 0;

  if (type == RECOVERY_ROOTFS) {
    /* Raw ubifs image, written into the rootfs volume */
    loff_t fsize;

    ret = rfs_size(name, &fsize);
    if (ret || fsize == 0) {
      printf("Recovery: failed to stat %s\n", name);
      return -1;
    }
    size = fsize;
  } else {
    /* Kernel: uImage or FIT, written to the kernel volume */
    ret = rfs_read(name, (ulong)&hdr, 0, sizeof(hdr), &actread);
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

  ret = rfs_read(name, CONFIG_SYS_LOAD_ADDR, 0, size, &actread);
  if (ret || actread != size) {
    printf("Recovery: failed to read %s\n", name);
    return -1;
  }

  /* Attach only now that there is something to write */
  if (ubi_part("ubi", NULL)) {
    printf("Recovery: cannot attach UBI\n");
    return -1;
  }

  if (type == RECOVERY_ROOTFS) {
    char size_str[24];

    /* The volume is sized to the image; if it does not fit, the data
     * volume is dropped (UBI volumes cannot be resized) */
    snprintf(size_str, sizeof(size_str), "0x%lx", size);
    ret = recovery_recreate_volume("rootfs", size_str, true);
    if (ret) {
      printf("Recovery: cannot recreate rootfs volume (%d)\n", ret);
      return -1;
    }

    ret = ubi_volume_write("rootfs", (void *)CONFIG_SYS_LOAD_ADDR, size);
    if (ret) {
      printf("Recovery: failed to write rootfs volume (%d)\n", ret);
      return -1;
    }

    /* The data volume takes the remaining space; keep it if present */
    recovery_ensure_volume("rootfs_data", "0");

    printf("Recovery: wrote %s (%lu bytes) to rootfs volume\n", name, size);
  } else {
    ret = recovery_recreate_volume("kernel", "0x300000", false);
    if (ret) {
      printf("Recovery: cannot recreate kernel volume (%d)\n", ret);
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

static int recovery_flash_holds(ulong addr, const void *data, ulong len) {
  const ulong chunk = 0x10000;
  u8 *buf = malloc(chunk);
  ulong off, n;
  int same = 1;

  if (!buf)
    return 0;
  for (off = 0; same && off < len; off += n) {
    n = min(len - off, chunk);
    same = !spi_flash_read(recovery_flash, addr + off, n, buf) &&
           !memcmp(buf, (const u8 *)data + off, n);
  }
  free(buf);
  return same;
}

/*
 * One recovery image: a uImage wrapper whose load address is where in the
 * flash its payload goes. Whatever the payload is -- a kernel, a rootfs, a
 * whole firmware, U-Boot itself -- it is written there; the wrapper header
 * never is. Returns 1 when it wrote, 0 when it skipped, -1 on a failed write.
 */
/* What a file read to CONFIG_SYS_LOAD_ADDR may take: the RAM below the
 * stack and the relocated U-Boot, with room */
static ulong recovery_ram_room(void) {
  return gd->start_addr_sp - SZ_1M - CONFIG_SYS_LOAD_ADDR;
}

static int recovery_write_img(const char *name, loff_t size) {
  struct image_header hdr;
  loff_t actread;
  ulong addr, len, sect = recovery_flash->sector_size;

  if (size <= (loff_t)sizeof(hdr) ||
      rfs_read(name, (ulong)&hdr, 0, sizeof(hdr), &actread) ||
      actread != sizeof(hdr) || !image_check_magic(&hdr) ||
      !image_check_hcrc(&hdr)) {
    printf("Recovery: %s is not a uImage, skipped\n", name);
    return 0;
  }
  addr = image_get_load(&hdr);
  len = image_get_data_size(&hdr);
  /* a read of length 0 would be a read of the whole file */
  if (!len) {
    printf("Recovery: %s has no data, skipped\n", name);
    return 0;
  }
  /* in 64 bits: a size near 4 GiB would wrap a 32-bit sum */
  if ((loff_t)len > size - (loff_t)sizeof(hdr)) {
    printf("Recovery: %s is shorter than its header says, skipped\n", name);
    return 0;
  }
  if (addr >= recovery_flash->size || len > recovery_flash->size - addr) {
    printf("Recovery: %s (0x%lx + %lu bytes) is outside the flash, skipped\n",
           name, addr, len);
    return 0;
  }
  if (addr % sect) {
    printf("Recovery: %s: 0x%lx is not on a %lu-byte erase boundary, skipped\n",
           name, addr, sect);
    return 0;
  }
  if (len > recovery_ram_room()) {
    printf("Recovery: %s (%lu bytes) does not fit the free RAM, skipped\n",
           name, len);
    return 0;
  }
  if (rfs_read(name, CONFIG_SYS_LOAD_ADDR, sizeof(hdr), len, &actread) ||
      actread != len) {
    printf("Recovery: failed to read %s, skipped\n", name);
    return 0;
  }
  if (crc32(0, (const u8 *)CONFIG_SYS_LOAD_ADDR, len) != image_get_dcrc(&hdr)) {
    printf("Recovery: %s: the data CRC does not match, skipped\n", name);
    return 0;
  }
  /* A card left in the slot: nothing to do, the system boots */
  if (recovery_flash_holds(addr, (void *)CONFIG_SYS_LOAD_ADDR, len)) {
    printf("Recovery: %s is already on the flash at 0x%lx\n", name, addr);
    return 0;
  }

  printf("Recovery: writing %s (%lu bytes) at 0x%lx\n", name, len, addr);
  if (spi_flash_erase(recovery_flash, addr, ALIGN(len, sect)) ||
      spi_flash_write(recovery_flash, addr, len, (void *)CONFIG_SYS_LOAD_ADDR)) {
    printf("Recovery: writing %s failed\n", name);
    return -1;
  }
  printf("Recovery: wrote %s\n", name);
  return 1;
}

struct recovery_file {
  char name[256];
  loff_t size;
};

/* Every *.img in the root of the medium, in the order the directory lists
 * them. The names are collected first: reading a file closes the
 * filesystem under an open directory stream. */
static int recovery_images(void) {
  struct recovery_file *files = NULL, *more;
  struct fs_dir_stream *dirs;
  struct fs_dirent *d;
  int n = 0, i, written = 0;
  size_t l;

  if (recovery_mount(recovery_if))
    return 0;
  dirs = fs_opendir("/");
  if (!dirs)
    return 0;
  while ((d = fs_readdir(dirs))) {
    l = strlen(d->name);
    if (d->type != FS_DT_REG || l < 5 || l >= sizeof(files->name) ||
        strcasecmp(d->name + l - 4, ".img"))
      continue;
    more = realloc(files, (n + 1) * sizeof(*files));
    if (!more)
      break;
    files = more;
    strcpy(files[n].name, d->name);
    files[n++].size = d->size;
  }
  fs_closedir(dirs);

  for (i = 0; i < n; i++)
    if (recovery_write_img(files[i].name, files[i].size) > 0)
      written++;
  free(files);
  return written;
}
#endif

/* If a script.txt is present on the medium, execute it (uImage script or
 * plain text) instead of searching for kernel/rootfs images. */
static int recovery_run_script(const char *name) {
  loff_t size, actread;
  int ret;
  char cmd[32];

  if (!rfs_exists(name))
    return 0;

  ret = rfs_size(name, &size);
  if (ret || size == 0) {
    printf("Recovery: failed to stat %s\n", name);
    return 1;
  }
  if ((u64)size > recovery_ram_room()) {
    printf("Recovery: %s (%lld bytes) does not fit the free RAM\n", name, size);
    return 1;
  }

  ret = rfs_read(name, CONFIG_SYS_LOAD_ADDR, 0, size, &actread);
  if (ret || actread != size) {
    printf("Recovery: failed to read %s\n", name);
    return 1;
  }

  if (image_check_magic((void *)CONFIG_SYS_LOAD_ADDR)) {
    /* uImage-wrapped script (mkimage -A arm -T script -d ...) */
    sprintf(cmd, "source 0x%lx", (ulong)CONFIG_SYS_LOAD_ADDR);
    ret = run_command(cmd, 0);
  } else {
    /* Plain text script */
    ret = run_command_list((char *)CONFIG_SYS_LOAD_ADDR, size, 0);
  }

  /* The script took recovery over: its result is reported, not retried
   * with the images */
  printf("Recovery: executed %s (%lld bytes)%s\n", name, size,
         ret ? ", it failed" : "");

  return 1;
}

static int recovery_try(void) {
#ifdef CONFIG_FMC_SPI_NAND
  char name[64];
#endif

  /* script.txt takes precedence over the image search */
  if (recovery_run_script("script.txt"))
    return 1;

#ifdef CONFIG_FMC_SPI_NAND
  /* Rootfs. On NAND it carries the kernel (/boot), its volume is sized
   * to the image and rootfs_data takes the rest. */
  sprintf(name, "rootfs.ubifs.%s.img", CONFIG_PRODUCT_SOC);
  return recovery_write(name, RECOVERY_ROOTFS) > 0;
#else
  return recovery_images() > 0;
#endif
}

int firmware_recovery(void) {
  int found = 0;

#ifndef CONFIG_FMC_SPI_NAND
  recovery_flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                                   CONFIG_SF_DEFAULT_SPEED,
                                   CONFIG_SF_DEFAULT_MODE);
  if (!recovery_flash)
    return -1;
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

#if !defined(CONFIG_FMC_SPI_NAND) && defined(CONFIG_OPENIPC_NOR_SPLIT)
/*
 * The split layout needs nothing from bootargs: the kernel's device tree has
 * the partitions and the kernel takes the root its firmware split finds. A
 * saved environment still spells out the fixed slots (mtdparts=..., root=
 * /dev/mtdblock3), and an older build of this bootloader added
 * openipc_nor_fit=1; drop those, in RAM only, so the command line says only
 * what is true.
 */
static void nor_split_bootargs(void)
{
  static const char * const drop[] = { "mtdparts=", "root=", "openipc_nor_fit=" };
  const char *cur = env_get("bootargs");
  char buf[1024], *out = buf;
  const char *p;

  if (!cur || strlen(cur) >= sizeof(buf))
    return;
  for (p = cur; *p;) {
    const char *end = strchr(p, ' ');
    size_t len = end ? (size_t)(end - p) : strlen(p);
    int keep = len > 0;
    int i;

    for (i = 0; keep && i < ARRAY_SIZE(drop); i++)
      if (!strncmp(p, drop[i], strlen(drop[i])))
        keep = 0;
    if (keep) {
      if (out != buf)
        *out++ = ' ';
      memcpy(out, p, len);
      out += len;
    }
    p += len;
    while (*p == ' ')
      p++;
  }
  *out = '\0';
  if (strcmp(buf, cur))
    env_set("bootargs", buf);
}

/*
 * The split layout: a uImage at the kernel offset and the root squashfs at the
 * next 64 KiB block after it, in one "firmware" partition the kernel divides
 * at boot. The fixed-slot layout also starts with a uImage, but its squashfs
 * sits at the slot boundary.
 */
static int nor_split_layout(struct spi_flash *flash, ulong kernel_off, ulong rootfs_off)
{
  image_header_t hdr;

  /*
   * A squashfs at a fixed rootfs slot is fixed slots, even when the uImage
   * before it ends in the kernel slot's last block and so looks split: a
   * kernel from before the split layout needs its mtdparts and root=. A
   * split image of that size loses nothing, as the kernel finds the split
   * itself and ignores both.
   */
  if (rootfs_off - kernel_off == 0x200000 || rootfs_off - kernel_off == 0x300000)
    return 0;
  if (!kernel_off || !rootfs_off ||
      spi_flash_read(flash, kernel_off, sizeof(hdr), &hdr) ||
      !image_check_magic(&hdr))
    return 0;
  return rootfs_off == kernel_off + ALIGN(image_get_image_size(&hdr), 0x10000);
}
#endif

int firmware_scan(void) {
#ifdef CONFIG_FMC_SPI_NAND
  u8 buf[64];
  int ret;

  printf("Checking UBI firmware...\n");

  /* Only attach when the UBI actually exists, so we never create one */
  if (!ubi_marker_present()) {
    printf("Firmware is absent/corrupt (no UBI)\n");
    env_set("bootfail", "1");
    return 0;
  }

  if (ubi_part("ubi", NULL)) {
    printf("Firmware is absent/corrupt (cannot attach UBI)\n");
    env_set("bootfail", "1");
    return 0;
  }

  /* The kernel is a file in the UBIFS rootfs (/boot/fitImage or
   * /boot/uImage): there is no kernel volume to check. */
  ret = ubi_volume_read("rootfs", (char *)buf, sizeof(buf));
  if (ret || !check_ubifs(buf)) {
    printf("Firmware is absent/corrupt (rootfs volume)\n");
    env_set("bootfail", "1");
    return 0;
  }

  /* A rootfs without a kernel in it boots nothing: hand it to recovery
   * like a missing one. loadkernel mounts the volume again itself. */
  if (cmd_ubifs_mount("ubi0:rootfs")) {
    printf("Firmware is absent/corrupt (cannot mount rootfs)\n");
    env_set("bootfail", "1");
    return 0;
  }
  ret = ubifs_exists("/boot/fitImage") || ubifs_exists("/boot/uImage");
  cmd_ubifs_umount();
  if (!ret) {
    printf("Firmware is absent/corrupt (no /boot/fitImage or /boot/uImage)\n");
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

#ifdef CONFIG_OPENIPC_NOR_SPLIT
  {
    /* fwwrite erases from the firmware start to the end of the chip, as
     * recovery does, so no old overlay is left behind a shorter image */
    char str[32];
    const char *ka = env_get("kernaddr");
    ulong start = kernel_off ? kernel_off :
                  (ka ? hextoul(ka, NULL) : CONFIG_ENV_KERNADDR);

    if (start < flash->size) {
      sprintf(str, "0x%lx", (ulong)(flash->size - start));
      env_set("fwsize", str);
      /* RAM only, like the rest of the scan: a saved environment keeps an
       * older fwwrite that erased only the file's length */
      env_set("fwwrite", "sf probe 0; sf erase ${kernaddr} ${fwsize}; "
                         "sf write ${baseaddr} ${kernaddr} ${filesize}");
    }
  }
#endif

  /*
   * Fixed slots -- the rootfs at the 2 or 3 MiB slot boundary after the
   * kernel, or nothing found to say otherwise -- get the slots' sizes, so the
   * rootfs partition is its whole slot and rootfs_data starts after it. A
   * rootfs packed right after the kernel (a FIT and its squashfs written as
   * one image) keeps the sizes found: a slot there would put the rootfs
   * partition past the squashfs.
   *
   * The 10 MiB rootfs slot is the 3 MiB kernel slot's (the ultimate pair,
   * rootfs at 0x350000; ur moves a large rootfs there before it writes). A
   * rootfs over 5 MiB behind the 2 MiB kernel slot is a packed image that
   * outgrew the lite slot -- hi3516cv6xx ultimate on 16 MiB, 7104k at
   * 0x250000 -- and keeps the size found: rounding it up to 10 MiB moved
   * rootfs_data from 0x940000 to 0xc50000, away from the settings there.
   */
  if (!kernel_size || kernel_size == 0x200000 || kernel_size == 0x300000) {
    if (kernel_size <= 0x200000)
      kernel_size = 0x200000;
    if (rootfs_size < 0x500000)
      rootfs_size = 0x500000;
    else if (kernel_size == 0x300000 && rootfs_size < 0xa00000 && flash->size > 0x800000)
      rootfs_size = 0xa00000;
  }

#ifdef CONFIG_OPENIPC_NOR_SPLIT
  if (nor_split_layout(flash, kernel_off, rootfs_off)) {
    char str[32], mtds[128], mtdparts[160];
    const char *mtdids = env_get("mtdids");
    ulong kern_slot, root_slot;

    /*
     * kernsize is what loadkernel reads, and also what uk erases; rootaddr
     * and rootsize are where ur writes. Those commands install a uImage and a
     * squashfs, i.e. fixed slots, so they get the fixed slots: the kernel
     * slot whole (the split squashfs starts inside it, and a head left there
     * would be found by the next scan before the new rootfs), the rootfs slot
     * after it. The kernel slot holds the whole uImage either way.
     */
    if (rootfs_off - kernel_off <= 0x200000) {
      kern_slot = 0x200000;
      root_slot = 0x500000;
    } else if (rootfs_off - kernel_off <= 0x300000 && flash->size > 0x800000) {
      kern_slot = 0x300000;
      root_slot = 0xa00000;
    } else {
      /* No fixed-slot layout holds this kernel. */
      kern_slot = rootfs_off - kernel_off;
      root_slot = flash->size - rootfs_off;
    }
    spi_flash_free(flash);
    sprintf(str, "0x%08lx", kernel_off);
    env_set("kernaddr", str);
    sprintf(str, "0x%lx", kern_slot);
    env_set("kernsize", str);
    sprintf(str, "0x%08lx", kernel_off + kern_slot);
    env_set("rootaddr", str);
    sprintf(str, "0x%lx", root_slot);
    env_set("rootsize", str);
#ifdef SFC
    if (!mtdids || !*mtdids)
      mtdids = SFC;
#endif
    /* U-Boot's own idea of the flash; the kernel is not told. */
    sprintf(mtds, "%dk(u-boot),%dk(env),-(firmware)",
            CONFIG_ENV_OFFSET / 1024, CONFIG_ENV_SIZE / 1024);
    env_set("mtds", mtds);
    sprintf(mtdparts, "%s:%s", mtdids ? mtdids : "", mtds);
    env_set("mtdparts", mtdparts);
    /* bootm checks the uImage's data CRC; a verify=n saved by an older
     * bootloader must not skip it. RAM only, like the rest of the scan. */
    env_set("verify", NULL);
    nor_split_bootargs();
    env_set("bootfail", NULL);
    return 0;
  }
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
    /*
     * An environment saved by an earlier OpenIPC U-Boot has no mtdids: its
     * bootargs carried "mtdparts=${mtdparts}" with the controller name
     * spelled out. Without one the kernel gets "mtdparts=:...", matches no
     * flash, and cannot mount /dev/mtdblock3. Name the controller this
     * build drives, as its own default environment does.
     */
#ifdef SFC
    if (!mtdids || !*mtdids)
      mtdids = SFC;
#endif
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
  u8 *buf;

  flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                          CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
  if (!flash)
    return 0;

  /* Cache-aligned: the DMA path invalidates cache lines around the
   * buffer and a plain malloc buffer can share lines with heap metadata */
  buf = malloc_cache_aligned(CONFIG_ENV_SIZE);
  if (!buf) {
    spi_flash_free(flash);
    return 0;
  }

  if (spi_flash_read(flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, buf)) {
    free(buf);
    spi_flash_free(flash);
    return 0;
  }

  memcpy(&crc, buf, sizeof(crc));
  free(buf);
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

/* An environment saved by an earlier build of this tree carries the retired
 * boot-limit command, which reflashes instead of entering failsafe (see
 * OPENIPC_ALTBOOTCMD). Replace exactly that one, once; a command anyone
 * edited is left alone. */
static void altbootcmd_migrate(void) {
  const char *s = env_get("altbootcmd");

  if (!s || strcmp(s, OPENIPC_ALTBOOTCMD_RETIRED))
    return;
  printf("altbootcmd: retired reflash command replaced with failsafe\n");
  env_set("altbootcmd", OPENIPC_ALTBOOTCMD);
  env_save();
}

int openipc_helper(void) {
  char msize[16];
  sprintf(msize, "%ldM", gd->ram_size / 1024 / 1024);
  env_set("totalmem", msize);
  firmware_recovery();
  firmware_scan();
  altbootcmd_migrate();

  /* Persist the default environment on first start (or after a bad env);
   * only if the device env area is still blank so we never overwrite
   * foreign data (e.g. a RAM-booted U-Boot on another device). */
  if ((gd->flags & GD_FLG_ENV_DEFAULT) && env_crc_is_blank())
    env_save();

  return 0;
}
