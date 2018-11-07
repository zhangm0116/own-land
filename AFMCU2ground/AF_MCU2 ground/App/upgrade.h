#ifndef __UPGRADE_H__
#define __UPGRADE_H__

#include "sys/types.h"

int32_t flash_probe(void);
int32_t macrnix_uboot_erase(void);
int32_t macrnix_kernel_erase(void);
int32_t macrnix_rootfs_erase(void);
int32_t macrnix_uboot_upgrade(uint32_t offset, size_t len, void *buf);
int32_t macrnix_kernel_upgrade(uint32_t offset, size_t len, void *buf);
int32_t macrnix_rootfs_upgrade(uint32_t offset, size_t len, void *buf);
#endif
