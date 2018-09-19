#ifndef _PUBLIC_H_
#define _PUBLIC_H_
#include "sys/types.h"

#define SET_TX_FREQ					0xC0
#define SET_RX_FREQ					0xC1
#define SET_RF_POWER				0xC2
#define GET_RF_POWER				0xC3
#define SET_BB_DOWNLINK				0xC4
#define GET_BB_DOWNLINK				0xC5
#define SET_BB_BW					 0xC6
#define GET_BB_BW					 0xC7
#define GET_TYPE					 0xC8
#define SET_ANTENNA_SW		 0xC9


#define GET_DEV_STATUS				0xE6
#define GET_COMMAND_STATUS			0xF8
#define GET_SCAN_FREQ				0xF9

#define MCU_ERASE_BIT				0
#define MCU_PROG_BIT				1
#define FPGA_ERASE_BIT				2
#define FPGA_PROG_BIT				3
#define UBOOT_ERASE_BIT				4
#define UBOOT_PROG_BIT				5
#define KERNEL_ERASE_BIT			6
#define KERNEL_PROG_BIT				7
#define ROOTFS_ERASE_BIT			8
#define ROOTFS_PROG_BIT				9

#define MCU_FLASH_ERASE				0xB0
#define MCU_FLASH_PROG				0xB1
#define FPGA_FLASH_ERASE			0xB2
#define FPGA_FLASH_PROG				0xB3
#define HI3516_UBOOT_ERASE			0xB4
#define HI3516_UBOOT_PROG			0xB5
#define HI3516_IMAGE_ERASE			0xB6
#define HI3516_IMAGE_PROG			0xB7
#define HI3516_ROOTFS_ERASE			0xB8
#define HI3516_ROOTFS_PROG			0xB9
#define ERASE_PROG_DONE				0xBA
#define SET_PROG_DONE				0xBC

#define MCU2_BOOT_MODE				0xD2
#define NORMAL_MODE					0x0
#define UPGRADE_MODE				0x5aa5

typedef enum
{
	UP_MCU1 = 0,
	UP_FPGA,
	UP_BOOT,
	UP_UBOOT,
	UP_KERNEL,
	UP_ROOTFS,
}dev_type;

struct upgrade_status
{
	unsigned int	flag;
	dev_type  		type;
};

struct rf_bb_configure
{
	unsigned int	rf_freq;
	unsigned int	rf_power;
	unsigned char	downlink;
	unsigned char	bb_bw;
	unsigned char	freq_hop;
	unsigned char	bind_mode;
	unsigned char	tdd_ratio;
	unsigned char	com_baudrate;
	unsigned char	prog_done;
};

struct rf_bb_status
{
	unsigned int	rf_freq;
	unsigned int	g_ldpc_failed;
	unsigned int	rf_power;
	unsigned int	a_ldpc_failed;
	unsigned char	downlink;
	unsigned char	bb_bw;
	unsigned char	freq_hop;
	unsigned char	bind_mode;
	unsigned char	tdd_ratio;
	unsigned char	com_baudrate;
	unsigned char	g_SNR;
	unsigned char	g_VGA;
	unsigned char	g_RSSI;
	unsigned char	a_SNR;
	unsigned char	a_VGA;
	unsigned char	a_RSSI;
	unsigned char	grd_link_status;
	unsigned char	air_link_status;
	unsigned char	cmd_status;
	unsigned int	bb_version;
	unsigned int	sof_version;
};

struct firmware_upgrade_status
{
	unsigned int current_upgrade_item;
	unsigned int current_upgrade_offset;
	unsigned int current_upgrade_done;
};

struct boot_status
{
	unsigned char ID[8];
	unsigned int  into_boot;
};

#define	UBOOT_UPGRADE	1
#define IMAGE_UPGRADE	2
#define ROOTFS_UPGRADE	3
#define MCU_UGRADE		4
#define FPGA_UPGRADE	5

#endif
