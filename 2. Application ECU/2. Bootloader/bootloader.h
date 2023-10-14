/*
 * @file       Bootloader.h
 * @version    1.0.0
 * @brief      Bootloader Header File.
 * @author     Ahmed Shawky Hamed
 */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/* ------------- Includes ------------- */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "can_test.h"

/* ------------- Macro Declerations ------------- */

extern CRC_HandleTypeDef hcrc;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* UART pins used in debugging and communication. */
#define BL_DEBUG_UART 									&huart1          /* Tx:PA9 , Rx:PA10 */
#define BL_HOST_COMMUNICATION_UART 			&huart3					 /* Tx:PB10 , Rx:PB11 */

/* Communication module used in debugging. */
#define BL_ENABLE_UART_DEBUG_MESSAGE  	0x00
#define BL_ENABLE_SPI_DEBUG_MESSAGE   	0x01
#define BL_ENABLE_CAN_DEBUG_MESSAGE   	0x02
#define BL_DEBUG_METHOD 							  ( BL_ENABLE_UART_DEBUG_MESSAGE )

/* Communication module used in sending commands. */
#define BL_ENABLE_UART_COMM_MESSAGE  		0x00
#define BL_ENABLE_SPI_COMM_MESSAGE   		0x01
#define BL_ENABLE_CAN_COMM_MESSAGE   		0x02
#define BL_COMM_METHOD 							  	( BL_ENABLE_UART_COMM_MESSAGE )

/* Size of the buffer to store the data sent by the host to the bootloader. */
#define BL_HOST_BUFFER_RX_LENGTH 				200

/* Command codes for supported bootloader commands. */
#define CBL_GET_VER_CMD 						0x10
#define CBL_GET_CID_CMD 						0x12
#define CBL_GET_RDP_STATUS_CMD 					0x13
#define CBL_GO_TO_ADDR_CMD 						0x14
#define CBL_FLASH_ERASE_CMD 					0x15
#define CBL_MEM_WRITE_CMD 						0x16
#define CBL_EN_R_W_PROTECT_CMD					0x17
#define CBL_DIS_R_W_PROTECT_CMD 				0x55    /* needs modification */
#define CBL_MEM_READ_CMD 						0x18
#define CBL_READ_SECTOR_STATUS_CMD				0x19
#define CBL_OTP_READ_CMD 						0x20
#define CBL_CHANGE_ROP_Level_CMD			  	0x21
#define CBL_JUMP_TO_USER_APP_CMD 				0x22
#define CBL_JUMP_TO_BOOTLOADER_CMD 				0x23

/* Defines for vendor and software version.
 * Must be modified after any kind of update.
 */
#define CBL_VENDOR_ID                   100
#define CBL_SW_MAJOR_VERSION           	1
#define CBL_SW_MINOR_VERSION            2
#define CBL_SW_PATCH_VERSION            3

/* Defines the number of bytes used in CRC. */
#define CRC_TYPE_SIZE_BYTE              4

/* Defines used to determine if the CRC verification is
 * passed or failed.
 */
#define CRC_VERIFICATION_FAILED         0x00
#define CRC_VERIFICATION_PASSED         0x01

/* Defines the CRC engine used in the bootloader. */
#define CRC_ENGINE_OBJ                	&hcrc

/* Defines the values of ACK and NACK to be sent to the HOST. */
#define CBL_SEND_NACK                   0xAB
#define CBL_SEND_ACK                    0xCD

/* Enable/Disable sending debug messages. */
#define DEBUG_INFO_ENABLE             1
#define DEBUG_INFO_DISABLE            0
#define BL_DEBUG_ENABLE               DEBUG_INFO_ENABLE

/*********************************************************/
#define DBGMCU_IDCODE_DEV_ID_MASK       0x00000FFF

/* Start Address of Sector 2*/
#define FLASH_SECTOR2_BASE_ADDRESS      0x08008000U

/* Macros to determine if the jump address is valid or invalid */
#define ADDRESS_IS_VALID                0x01
#define ADDRESS_IS_INVALID              0x00

/* To determine the accessable memories ( accessable addresses ) */
#define STM32F429_SRAM1_SIZE            ( 112 * 1024 )  	/* 112 KB */
#define STM32F429_SRAM2_SIZE						( 16 * 1024 )  		/* 16 KB */
#define STM32F429_SRAM3_SIZE						( 64 * 1024 )  		/* 64 KB */
#define STM32F429_FLASH_SIZE						( 2048 * 1024 )  	/* 2 MB */

/* End addresses of different kinds of memories */
#define STM32F429_SRAM1_END							( SRAM1_BASE + STM32F429_SRAM1_SIZE )
#define STM32F429_SRAM2_END             ( SRAM2_BASE + STM32F429_SRAM2_SIZE )
#define STM32F429_SRAM3_END							( SRAM3_BASE + STM32F429_SRAM3_SIZE )
#define STM32F429_FLASH_END							( FLASH_BASE + STM32F429_FLASH_SIZE )

#define SUCCESSFUL_ERASE                0x03
#define UNSUCCESSFUL_ERASE              0x02

#define CBL_FLASH_MAX_SECTOR_NUMBER     12    /* needs modification to be 24 ( two banks ) */
#define CBL_FLASH_MASS_ERASE            0xFF  /* Performing mass erase regardless the value of # of sectors */

#define VALID_SECTOR_NUMBER             0x01
#define INVALID_SECTOR_NUMBER           0x00

#define HAL_SUCCESSFUL_ERASE            0xFFFFFFFFU

/* CBL_MEM_WRITE_CMD Macros */
#define FLASH_PAYLOAD_WRITE_PASSED      0x01
#define FLASH_PAYLOAD_WRITE_FAILED      0x00

#define FLASH_LOCK_WRITE_PASSED     		0x01
#define FLASH_LOCK_WRITE_FAILED      		0x00

/* CBL_CHANGE_ROP_Level_CMD Macros */
#define ROP_LEVEL_CHANGE_INVALID        0x00
#define ROP_LEVEL_CHANGE_VALID          0x01

/* CBL_EN_R_W_PROTECT_CMD Macros*/
#define SUCCESSFUL_PROTECTION 					0x01
#define UNSUCCESSFUL_PROTECTION 				0x00
#define OPTCR_REGISTER_BASE_ADDRESS 		0x40023C14U

/* CBL_DIS_R_W_PROTECT_CMD Macros*/
#define SUCCESSFUL_DISABLING 						0x01
#define UNSUCCESSFUL_DISABLING 					0x00

/* ------------- Macro Functions Declerations ------------- */

/* ------------- Data Type Declerations ------------- */

typedef enum
{
	BL_NACK = 0 ,
	BL_OK
}BL_Status;

/* void pointer to function */
typedef void (*pMainApp)(void);

/* void pointer to jump to specific address */
typedef void (*Jump_Ptr)(void);

/* ------------- Software Interfaces Declerations ------------- */
void BL_Print_Message(char* format , ...);

BL_Status BL_UART_Fetch_Host_Command(void);

void BL_CAN_Fetch_Host_Command(void);

void Print_Name_Fun(void);

#endif /* BOOTLOADER_H */
