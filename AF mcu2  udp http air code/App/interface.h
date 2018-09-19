#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "stm32f4xx.h"

#define READ_CMD										0x0a
#define WRITE_CMD										0x05


/* interface register address*/
#define FREQ_ADDR 		             0x00
#define DOWNLINK_ADDR     	             0x01
#define RF_POWER_ADDR      	    0x02
#define CHANNEL_BW_ADDR              0x03
#define TDD_RATIO_ADDR               0x04
#define FREQ_HOP_ADDR                0x05
#define ANTENNA_SELET_ADDR     		 0x06
#define COM_BAUDRATE_ADDR      		 0x07
#define SET_COMPRESS_ADDR      		 0x08
#define SET_VIDEO_RATE_ADDR    		 0x09
#define BIND_MODE_ADDR      		 0x0a


#define COMMAND_STATUS_ADDR    		 0x80
#define AIR_SNR_ADDR		   		 0x81
#define GRD_SNR_ADDR	      		 0x82
#define AIR_RSSI_ADDR      		 	 0x83
#define GRD_RSSI_ADDR      		 	 0x84
#define AIR_VGA_ADDR		   		 		0x85
#define GRD_VGA_ADDR	      				0x86
#define AIR_LDPC_FAILED_ADDR   		 0x87
#define GRD_LDPC_FAILED_ADDR   		 0x88
#define AIR_LDPC_PASSED_ADDR   		 0x89
#define GRD_LDPC_PASSED_ADDR   		 0x8a
#define AIR_BB_STATUS_ADDR	   		 0x8b
#define GRD_BB_STATUS_ADDR	       0x8c
#define FPGA_VERSION_ADDR	   		 	 0x8d
#define MCU_VERSION_ADDR	   		 	 0x8e
#define BIND_STATUS_ADDR		       0x8f
#define SWEEP_FREQ0_ADDR		       0x90
#define SWEEP_FREQ1_ADDR		       0x91
#define SWEEP_FREQ2_ADDR		       0x92
#define SWEEP_FREQ3_ADDR		       0x93


#define UPGRADE_COMMAND_ADDR   		 0xc0
#define UPGRADE_DATA_ADDR	      	 0xc1
#define UPGRADE_FIFO_ADDR	   		 	 0xc2


struct  antenna_data_t{

 uint32_t A_Antenna_Rssi;
 uint32_t B_Antenna_Rssi;
 uint32_t A_Antenna_sum;
 uint32_t B_Antenna_sum;
 uint8_t  Current_Antenna;
 uint8_t  ant_status;
 uint32_t  Tx_mode;    

	
};


#ifdef AIR_MODULE
struct dev_status_t
{
	uint32_t	ldpc_pass;
	uint32_t	ldpc_failed;
	uint8_t		SNR;
	uint8_t		A_VGA;   
	uint8_t		A_RSSI;  
	uint8_t		B_VGA;		
	uint8_t		B_RSSI;		
//	uint8_t		sw_ant;		// new
  uint8_t		current_ant;   // 14 
 //unsigned char	 rsvd;
	uint32_t	rf_tx_freq;   //
	uint32_t	rf_rx_freq;  //22
	uint32_t	rf_power;
	uint32_t	tx_mode;     //   16    
	uint32_t	rx_mode;
	uint32_t	bb_bw;            //´ø¿í
	uint32_t	sw_version;
	uint32_t	sw_date;
	uint32_t	hw_version;
	uint32_t	hw_date;
	int32_t		fpga_temp;
	int32_t		ad9361_temp;
 // uint32_t   rx_lost;    //¶ª°üÂÊ
	
};
#else
struct dev_status_t
{
	unsigned int	ldpc_pass;
	unsigned int	ldpc_failed;
	unsigned char	SNR;
	unsigned char	A_VGA;
	unsigned char	A_RSSI;
	unsigned char	B_VGA;
	unsigned char	B_RSSI;
	unsigned char	ant_switch;
	unsigned char	 rsvd;
	unsigned int	rf_tx_freq;
	unsigned int	rf_rx_freq;
	unsigned int	rf_power;
	unsigned int	tx_mode;
	unsigned int	rx_mode;
	unsigned int	bb_bw;
	unsigned int	sw_version;
	unsigned int	sw_date;
	unsigned int	hw_version;
	unsigned int	hw_date;
	unsigned int	fpga_temp;
	unsigned int	ad9361_temp;
	// uint32_t   rx_lost;
};
#endif

int if_bus_init(void);


int if_set_freq(uint32_t val, int ch);
int if_set_rf_power(uint32_t val);
int if_set_downlink(uint32_t val, uint8_t ch);
int if_set_tdd_ratio(uint32_t val);
int if_set_channel_bw(uint32_t val);
int if_set_com_baudrate(uint32_t val);
int if_bind_mode(uint32_t val);
int if_get_sof_version(uint32_t *ver);
int if_get_bb_version(uint32_t *ver);
int if_get_freq_scan(uint8_t *buf,uint32_t len);
int if_get_dev_status(void *buf, uint32_t len);
int if_mcu_erase(void);
int if_fpga_erase(void);
int if_mcu_upgrade(unsigned int len, uint8_t *buf);
int if_fpga_upgrade(unsigned int len, uint8_t *buf);
void MCU1_upgrade_enable(uint8_t status);
void MCU1_reset(void);
int if_set_antenna_sw(uint32_t val);

#endif
