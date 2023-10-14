/*
 * stm32f10xxx_device_header.h
 *
 *  Created on: Nov 4, 2022
 *      Author: Ahmed
 */

#ifndef STM32F10XXX_DEVICE_HEADER_H_
#define STM32F10XXX_DEVICE_HEADER_H_

// =============================================
// ================== Includes =================
// =============================================

#include "Platform_Types.h"
#include "Utils.h"


// ==============================================================
// ================== Peripheral Base Addresses =================
// ==============================================================



// *-*-*-*-*-*-*-*-*-*-*-*-*-
// -*-*-*-*-* bxCAN -*-*-*-*-*
// *-*-*-*-*-*-*-*-*-*-*-*-*-
#define CAN_CSR_BASE					0x40006400
#define CAN_TXMBR0_BASE					(0x40006400 + 0x180)
#define CAN_TXMBR1_BASE					(0x40006400 + 0x190)
#define CAN_TXMBR2_BASE					(0x40006400 + 0x1A0)
#define CAN_RXFIFO0_BASE				(0x40006400 + 0x1B0)
#define CAN_RXFIFO1_BASE				(0x40006400 + 0x1C0)
#define CAN_FLTR_CONFIG_BASE			(0x40006400 + 0x200)
#define CAN_FILTERS_BASE				(0x40006400 + 0x240)



// =========================================================
// ================== Peripheral Registers =================
// =========================================================



// *-*-*-*-*-*-*-*-*-*-*-*-*-
// -*-*-*-*-* bxCAN -*-*-*-*-* APB1
// *-*-*-*-*-*-*-*-*-*-*-*-*-
typedef struct
{
	vuint32_t		MCR;
	vuint32_t 		MSR;
	vuint32_t		TSR;
	vuint32_t		RF0R;
	vuint32_t		RF1R;
	vuint32_t		IER;
	vuint32_t 		ESR;
	vuint32_t 		BTR;

} CAN_CSR_TypeDef;

typedef struct
{
	vuint32_t		TIxR;
	vuint32_t 		TDTxR;
	vuint32_t		TDLxR;
	vuint32_t		TDHxR;

} CAN_TXMBRx_TypeDef;

typedef struct
{
	vuint32_t		RIxR;
	vuint32_t		RDTxR;
	vuint32_t		RDLxR;
	vuint32_t		RDHxR;

} CAN_RXFIFOx_TypeDef;

typedef struct
{
	vuint32_t		FMR;
	vuint32_t 		FM1R;
	vuint32_t		reserved1;
	vuint32_t		FS1R;
	vuint32_t		reserved2;
	vuint32_t		FFA1R;
	vuint32_t		reserved3;
	vuint32_t		FA1R;

} CAN_FLTR_Config_TypeDef;

typedef struct
{
	vuint32_t		FiRx[14];
	//vuint32_t 	F0R2;

	/*vuint32_t		F1R1;
	vuint32_t		F1R2;

	vuint32_t		F2R1;
	vuint32_t		F2R2;

	vuint32_t		F3R1;
	vuint32_t		F3R2;

	vuint32_t		F4R1;
	vuint32_t 	F4R2;

	vuint32_t		F5R1;
	vuint32_t		F5R2;

	vuint32_t		F6R1;
	vuint32_t		F6R2;

	vuint32_t		F7R1;
	vuint32_t		F7R2;*/

} CAN_Filters_TypeDef;






// =========================================================
// ================== Peripheral Instances =================
// =========================================================



// *-*-*-*-*-*-*-*-*-*-*-*-*-
// -*-*-*-*-* bxCAN -*-*-*-*-*
// *-*-*-*-*-*-*-*-*-*-*-*-*-
#define CAN_CSR					((CAN_CSR_TypeDef*)CAN_CSR_BASE)
#define CAN_TXMBR0				((CAN_TXMBRx_TypeDef*)CAN_TXMBR0_BASE)
#define CAN_TXMBR1				((CAN_TXMBRx_TypeDef*)CAN_TXMBR1_BASE)
#define CAN_TXMBR2				((CAN_TXMBRx_TypeDef*)CAN_TXMBR2_BASE)
#define CAN_RXFIFO0				((CAN_RXFIFOx_TypeDef*)CAN_RXFIFO0_BASE)
#define CAN_RXFIFO1				((CAN_RXFIFOx_TypeDef*)CAN_RXFIFO1_BASE)
#define CAN_FLTR_CONFIG			((CAN_FLTR_Config_TypeDef*)CAN_FLTR_CONFIG_BASE)
#define CAN_FILTERS				((CAN_Filters_TypeDef*)CAN_FILTERS_BASE)



#endif /* STM32F10XXX_DEVICE_HEADER_H_ */
