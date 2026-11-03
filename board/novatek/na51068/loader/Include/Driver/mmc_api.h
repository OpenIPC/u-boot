#ifndef __MMC_API_H__
#define __MMC_API_H__

int add_sdhci (void);
int mmc_bread(unsigned long start, unsigned long blkcnt, unsigned long dst);
//int mmc_loader(unsigned int load_addr, UINT8 * p_run_filename, UINT8 * p_update_filename, UINT32 * uiLoaderFunc);

INT32 card_open(void);
void card_close(void);



#endif /* __MMC_API_H__ */
