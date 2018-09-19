#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdio.h"
#include "string.h"
#include "stm32f4xx.h"
#include "ymodem.h"

typedef  void(*pFunction)(void);

#define CMD_STRING_SIZE		128
#define ApplicationAddress	0x801f000
#define FLASH_SIZE			(0x100000) /* 1 MByte */
#define FLASH_IMAGE_SIZE	(uint32_t) (FLASH_SIZE - (ApplicationAddress - 0x08000000))

#define IS_AF(c)			((c >= 'A') && (c <= 'F'))
#define IS_af(c)			((c >= 'a') && (c <= 'f'))
#define IS_09(c)			((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)		IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)		IS_09(c)
#define CONVERTDEC(c)		(c - '0')
#define CONVERTHEX_alpha(c)	(IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)		(IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))
#define SerialPutString(x)	Serial_PutString((uint8_t*)(x))

void Int2Str(uint8_t* str, int32_t intnum);
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum);
uint32_t GetIntegerInput(int32_t * num);
uint8_t GetKey(void);
void SerialPutChar(uint8_t c);
uint32_t FLASH_PagesMask(__IO uint32_t Size);
void FLASH_DisableWriteProtectionPages(void);
void Main_Menu(void);
void SerialDownload(void);
void SerialUpload(void);

#endif
