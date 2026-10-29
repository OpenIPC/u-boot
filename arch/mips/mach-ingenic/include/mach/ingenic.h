/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T-series: the hardware set-up the vendor U-Boot keeps in its
 * board headers (OpenIPC/u-boot-ingenic include/configs/isvp*.h), for the
 * code ported from it. The SoC and its package variant are Kconfig
 * options; what follows from them is in the per-SoC header.
 */

#ifndef __MACH_INGENIC_H
#define __MACH_INGENIC_H

#if defined(CONFIG_T10)
#include <mach/t10-hw.h>
#elif defined(CONFIG_T20)
#include <mach/t20-hw.h>
#elif defined(CONFIG_T21)
#include <mach/t21-hw.h>
#elif defined(CONFIG_T23)
#include <mach/t23-hw.h>
#elif defined(CONFIG_T30)
#include <mach/t30-hw.h>
#elif defined(CONFIG_T31)
#include <mach/t31-hw.h>
#elif defined(CONFIG_T40)
#include <mach/t40-hw.h>
#endif

#if defined(CONFIG_CPU_XBURST2) && !defined(USE_HOSTCC) && \
    !defined(__ASSEMBLY__) && !defined(flush_l2cache_all)
/* XBurst2: write the L2 cache back and invalidate it (xburst2/cpu.c) */
void flush_l2cache_all(void);
#endif

#ifndef USE_HOSTCC
/* The vendor U-Boot's error() (include/common.h there) */
#define error(fmt, args...) do {					\
	printf("ERROR: " fmt "\nat %s:%d/%s()\n",			\
		##args, __FILE__, __LINE__, __func__);		\
} while (0)
#endif

#ifndef CONFIG_CPU_XBURST2
/* XBurst: what the isvp headers share; the XBurst2 SoCs carry theirs */

/* MSC0 on port B, the SD card slot */
#define CONFIG_JZ_MMC_SPLMSC		0
#define CONFIG_JZ_MMC_MSC0_PB		1

/* SFC: SPI NOR, dual I/O reads */
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_PA
#define CONFIG_SPI_DUAL
#define CONFIG_SFC_MIN_ALIGN		0x10000

/* GMAC, as the isvp boards have it */
#define GMAC_PHY_MII			1
#define GMAC_PHY_RMII			2
#define GMAC_PHY_GMII			3
#define GMAC_PHY_RGMII			4

#define PHY_TYPE_DM9161			1
#define PHY_TYPE_88E1111		2
#define PHY_TYPE_8710A			3
#define PHY_TYPE_IP101G			4
#define PHY_TYPE_OMNI			5

#if defined(CONFIG_T21)
/* The internal PHY, on MII */
#define GMAC_PHY_INNER
#define CONFIG_NET_GMAC_PHY_MODE	GMAC_PHY_MII
#define CONFIG_NET_PHY_TYPE		PHY_TYPE_OMNI
#else
#define CONFIG_NET_GMAC_PHY_MODE	GMAC_PHY_RMII
#define CONFIG_NET_PHY_TYPE		PHY_TYPE_IP101G
#endif

#if defined(CONFIG_T23)
#define CONFIG_GMAC_PHY_RATE		25000000
#if (CONFIG_SYS_MPLL_FREQ % CONFIG_GMAC_PHY_RATE)
#define CONFIG_MACPHY_SEL_PLL		APLL
#else
#define CONFIG_MACPHY_SEL_PLL		MPLL
#endif
#endif

#if defined(CONFIG_T23) || defined(CONFIG_T31)
#define CONFIG_GPIO_IP101G_RESET	GPIO_PB(28)
#define CONFIG_GPIO_IP101G_RESET_ENLEVEL	0
#endif

#endif /* !CONFIG_CPU_XBURST2 */

#endif /* __MACH_INGENIC_H */
