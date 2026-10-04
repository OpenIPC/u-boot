/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify Command
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */

#include <common.h>
#include <command.h>
#include "security_subsys_init.h"

#include "verify_boot.h"
#define CMD_VERIFY_BOOT_ARGV_NUM	        3
#define CMD_VERIFY_BOOT_PARTITION_NAME_IDX  1
#define CMD_VERIFY_BOOT_PARAMS_ADDR_IDX     2
#define HEX_BASE                            16

static int do_verify_boot(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    unsigned long long image_ddr_address = 0;
    const image_key_area_t *key_area = NULL;
    const image_code_info_t *code_info = NULL;
    const unsigned char *app_code = NULL;
    const char *partition_name = NULL;
    uint64_t start_time = 0;
    uint64_t cost_time = 0;

    (void)crypto_cipher_init();

    if (argc != CMD_VERIFY_BOOT_ARGV_NUM) {
        printf("invalid cmd nums\n");
        goto error_usage;
    }
    partition_name = argv[CMD_VERIFY_BOOT_PARTITION_NAME_IDX];
    image_ddr_address = simple_strtoul(argv[CMD_VERIFY_BOOT_PARAMS_ADDR_IDX], NULL, HEX_BASE);
    key_area = (const image_key_area_t *)(uintptr_t)image_ddr_address;
    code_info = (const image_code_info_t *)(uintptr_t)(image_ddr_address + sizeof(image_key_area_t));
    app_code = (const unsigned char *)(uintptr_t)(image_ddr_address +
        sizeof(image_key_area_t) + sizeof(image_code_info_t));

    printf("Start verify partition %s\n", partition_name);
    start_time = get_timer_us(0);
    ret = secure_boot_verify(key_area, code_info, app_code);
    if (ret != ERRCODE_SUCC) {
        printf("Verify partition %s failed!\n", partition_name);
        do_reset(NULL, 0, 0, NULL);
    }
    cost_time = get_timer_us(start_time);
    printf("Cost %llu us, Verify partition %s success!\n", cost_time, partition_name);

    (void)crypto_cipher_deinit();
    return 0;
error_usage:
    (void)crypto_cipher_deinit();
    cmd_usage(cmdtp);
    return -1;
}

U_BOOT_CMD(verify_boot, CMD_VERIFY_BOOT_ARGV_NUM, 0, do_verify_boot,
    "Verify one image by passing the start ddr address",
    "Usage: verify_boot <partition_name> <ddr address>\n"
);