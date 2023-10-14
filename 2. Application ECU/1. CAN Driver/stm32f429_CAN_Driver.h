/*
 * stm32f103c8_CAN_Driver.h
 *
 *  Created on: Feb 9, 2023
 *      Author: Ahmed
 */

#ifndef INC_STM32F429_CAN_DRIVER_H_
#define INC_STM32F429_CAN_DRIVER_H_

// =============================================
// ================== Includes =================
// =============================================

#include "Platform_Types.h"
#include "stm32f429_device_header.h"
#include "main.h"

#include "Utils.h"


// ==========================================================
// ================== User Type Definitions =================
// ==========================================================

typedef struct
{
	uint8_t		CAN_buadRate_preScaler;		/* This is used to define the length of a time quanta.
												tq = (BRP[9:0]+1) x tPCLK */

	uint8_t		CAN_segment1_quantum;		/* This is used to specify the number of quantum of segment 1
		 	 	 	 	 	 	 	 	 	 	propagation segment + phase 1 segment
		 	 	 	 	 	 	 	 	 	 	@ref CAN_SEG1_QUANTA_define. */

	uint8_t 		CAN_segment2_quantum;		/* This is used to specify the number of quantum of segment 1
		 	 	 	 	 	 	 	 	 	 	phase 2 segment only
		 	 	 	 	 	 	 	 	 	 	@ref CAN_SEG2_QUANTA_define. */

	uint8_t		CAN_reSyncJumpWidth;			/* This is used to specify the maximum number of time quanta
												the CAN hardware is allowed to
												lengthen or shorten a bit to perform
												the resynchronization.
												@ref CAN_SJW_QUANTA_define.  */

	uint8_t 		CAN_Mode;					/* This is used to specify the test mode of the can for
	 	 	 	 	 	 	 	 	 	 	 	 debugging
	 	 	 	 	 	 	 	 	 	 	 	 @ref CAN_MODE_define. */

	uint8_t	CAN_automaticBusOffManagement;	/* This is used to controls the behavior
													of the CAN hardware on leaving the Bus-Off state.
													this must be a value of ENABLE or DISABLE */

	uint8_t	CAN_automaticWakeUp;			/* This is used to controls the behavior
													of the CAN hardware on leaving the Bus-Off state.
													this must be a value of ENABLE or DISABLE */

	uint8_t	CAN_noAutomaticRetransmission;	/* This is used to determine if CAN hardware
	 	 	 	 	 	 	 	 	 	 	 	 	 will automatically retransmit the message until
	 	 	 	 	 	 	 	 	 	 	 	 	it has been successfully transmitted
	 	 	 	 	 	 	 	 	 	 	 	 	according to the CAN standard.
	 	 	 	 	 	 	 	 	 	 	 	 	This must be a value of ENABLE or DISABLE. */

	uint8_t		CAN_receiveFIFOLockedMode;		/*	This is used to lock the receiving fifo from
	 	 	 	 	 	 	 	 	 	 	 	 	 data overrun.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_RX_FIFO_LOCK_MODE_define. */

	uint8_t 		CAN_transmitFIFOPriority;		/*	This is used to control the transmission order
													when several mailboxes are pending at the same
													time.
													This must be a value of @ref CAN_TX_FIFO_PRIORITY_define. */

}	CAN_InitConfig_t;

typedef struct
{
	uint32		CAN_ID;							/* This is used to put the identifier extended or standard. */

	uint8_t		CAN_IDE;						/* This is used to determine the identifier is
	 	 	 	 	 	 	 	 	 	 	 	 	 extended or standard.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_IDE_define. */

	uint8_t		CAN_RTR;						/* This is used to determine the transmission frame is
	 	 	 	 	 	 	 	 	 	 	 	 	 data frame or remote frame.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_RTR_define. */

	uint8_t		CAN_DLC;						/* This is used to determine the length of the data
	 	 	 	 	 	 	 	 	 	 	 	 	 in case of data frame or zero byte in case of
	 	 	 	 	 	 	 	 	 	 	 	 	 remote frame.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_DLC_define. */

//	uint8_t		CAN_dataPacket[8];				/* This is a pointer that points to an array of
//	 	 	 	 	 	 	 	 	 	 	 	 	 the data need to be sent.
//	 	 	 	 	 	 	 	 	 	 	 	 	 The maximum length is 8 bytes*/

}	CAN_FrameHeader_t;

typedef struct
{
	uint8_t		CAN_filterMode;						/* This is used to determine Mode of
														the registers of Filter x.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_FILTER_MODE_define. */

	uint8_t		CAN_filterScale;				/* This is used to determine the scale of
													the filter
													+dual 16-bit scale configuration
													+single 32-bit scale configuration
													This must be a value of @ref CAN_FILTER_SCALE_define. */

	uint8_t		CAN_filterAssign;				/* This is used to determine which FIFO is used to be
													assigned by the filter.
	 	 	 	 	 	 	 	 	 	 	 	 	 This must be a value of @ref CAN_FILTER_ASSIGN_define. */

	uint16_t		CAN_filterId;					/* This is used to determine which filter from 14 filters
		 	 	 	 	 	 	 	 	 	 	 	 	is used for filtering the messages.
		 	 	 	 	 	 	 	 	 	 	 	 	This parameter must be a value of @ref CAN_FILTER_ID_define.*/

	uint16_t		CAN_filterIdHigh;				/* This is used to specify the high bytes for the ID.*/


	uint16_t		CAN_filterIdLow;				/* This is used to specify the LOW bytes for the ID.*/


	uint16_t		CAN_filterMaskHigh;				/* This is used to specify the high bytes for the ID mask
													in the mask mode or the ID in the id list mode.*/

	uint16_t		CAN_filterMaskLow;				/* This is used to specify the high bytes for the ID mask
													in the mask mode or the ID in the id list mode.*/

}	CAN_Filter_Config_t;

// ===================================================================
// ================== Macros Configuration Reference =================
// ===================================================================

// @ref SEG1_QUANTA_define.
#define CAN_SEG1_QUANTA_1tq					0x0
#define CAN_SEG1_QUANTA_2tq					0x1
#define CAN_SEG1_QUANTA_3tq					0x2
#define CAN_SEG1_QUANTA_4tq					0x3
#define CAN_SEG1_QUANTA_5tq					0x4
#define CAN_SEG1_QUANTA_6tq					0x5
#define CAN_SEG1_QUANTA_7tq					0x6
#define CAN_SEG1_QUANTA_8tq					0x7
#define CAN_SEG1_QUANTA_9tq					0x8
#define CAN_SEG1_QUANTA_10tq				0x9
#define CAN_SEG1_QUANTA_11tq				0xA
#define CAN_SEG1_QUANTA_12tq				0xB
#define CAN_SEG1_QUANTA_13tq				0xC
#define CAN_SEG1_QUANTA_14tq				0xD
#define CAN_SEG1_QUANTA_15tq				0xE
#define CAN_SEG1_QUANTA_16tq				0xF

// @ref CAN_SEG2_QUANTA_define.
#define CAN_SEG2_QUANTA_1tq					0x0
#define CAN_SEG2_QUANTA_2tq					0x1
#define CAN_SEG2_QUANTA_3tq					0x2
#define CAN_SEG2_QUANTA_4tq					0x3
#define CAN_SEG2_QUANTA_5tq					0x4
#define CAN_SEG2_QUANTA_6tq					0x5
#define CAN_SEG2_QUANTA_7tq					0x6
#define CAN_SEG2_QUANTA_8tq					0x7

// @ref CAN_SJW_QUANTA_define.
#define CAN_SJW_QUANTA_1tq					0x0
#define CAN_SJW_QUANTA_2tq					0x1
#define CAN_SJW_QUANTA_3tq					0x2
#define CAN_SJW_QUANTA_4tq					0x3

// @ref CAN_MODE_define.
#define CAN_MODE_NORMAL						0x0
#define CAN_MODE_SILENT						0x2
#define CAN_MODE_LOOPBACK					0x1
#define CAN_MODE_SILENT_LOOPBACK			0x3

// @ref CAN_RX_FIFO_LOCK_MODE_define.
#define CAN_RX_FIFO_LOCK_MODE_NOT_LOCKED	0x0
#define CAN_RX_FIFO_LOCK_MODE_LOCKED		0x4

// @ref CAN_TX_FIFO_PRIORITY_define.
#define CAN_TX_FIFO_PRIORITY_IDENTIFIER		0x0
#define CAN_TX_FIFO_PRIORITY_REQUEST_ORDER	0x2


#define ENABLE								0x1
#define DISABLE								0x0


// @ref CAN_IDE_define.
#define CAN_IDE_STD							0x0
#define CAN_IDE_EXTD						0x1

// @ref CAN_RTR_define.
#define CAN_RTR_DATA						0x0
#define CAN_RTR_REMOTE						0x1

// @ref CAN_DLC_define.
#define CAN_DLC_ZERO_BYTE					0x0
#define CAN_DLC_ONE_BYTE					0x1
#define CAN_DLC_TWO_BYTE					0x2
#define CAN_DLC_THREE_BYTE					0x3
#define CAN_DLC_FOUR_BYTE					0x4
#define CAN_DLC_FIVE_BYTE					0x5
#define CAN_DLC_SIX_BYTE					0x6
#define CAN_DLC_SEVEN_BYTE					0x7
#define CAN_DLC_EIGHT_BYTE					0x8

// @ref CAN_FILTER_ID_define.
#define CAN_FILTER_ID_0						0
#define CAN_FILTER_ID_1						1
#define CAN_FILTER_ID_2						2
#define CAN_FILTER_ID_3						3
#define CAN_FILTER_ID_4						4
#define CAN_FILTER_ID_5						5
#define CAN_FILTER_ID_6						6
#define CAN_FILTER_ID_7						7
#define CAN_FILTER_ID_8						8
#define CAN_FILTER_ID_9						9
#define CAN_FILTER_ID_10					10
#define CAN_FILTER_ID_11					11
#define CAN_FILTER_ID_12					12
#define CAN_FILTER_ID_13					13

// @ref CAN_FILTER_MODE_define.
#define CAN_FILTER_MODE_MASK				0x0
#define CAN_FILTER_MODE_LIST				0x1

// @ref CAN_FILTER_SCALE_define.
#define CAN_FILTER_SCALE_DUAL_16			0x0
#define CAN_FILTER_SCALE_SINGLE_32			0x1

 // @ref CAN_FILTER_ASSIGN_define.
#define CAN_FILTER_ASSIGN_FIFO0				0x0
#define CAN_FILTER_ASSIGN_FIFO1				0x1

// @ref FIFOx
#define FIFO0								CAN_RXFIFO0
#define FIFO1								CAN_RXFIFO1

// @ref FIFO_NUMBER_define.
#define FIFO_NUMBER_0						0x0
#define FIFO_NUMBER_1						0x1


/*
 * 			Filter bank scale configuration - register organization
 * 			=======================================================
 *
 * 	One 32-Bit Filter - Identifier Mask ---> CAN_filterMode = CAN_FILTER_MODE_MASK, CAN_filterScale = CAN_FILTER_SCALE_SINGLE_32
 *
 * 	bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * 	ID	   |			CAN_filterIdHigh			  	   				   |					CAN_filterIdLow						  	 	|
 * 	Mask   |			CAN_filterMaskHigh			  	   				   |					CAN_filterMaskLow					  		|
 * 	Map    |  				STID				   	   |			   							EXTID						   |IDE|RTR | 0	|
 *
 * 	===========================================================================================================================================
 * 	Two 32-Bit Filters - Identifier List ---> CAN_filterMode = CAN_FILTER_MODE_LIST, CAN_filterScale = CAN_FILTER_SCALE_SINGLE_32
 *
 * 	bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * 	ID	   |			CAN_filterIdHigh			  	   				   |					CAN_filterIdLow						  	 	|
 * 	ID	   |			CAN_filterMaskHigh			  	   				   |					CAN_filterMaskLow					  		|
 * 	Map    |  				STID				   	   |			   							EXTID						   |IDE|RTR | 0	|
 *
 * ============================================================================================================================================
 * Two 16-Bit Filters - Identifier Mask	---> CAN_filterMode = CAN_FILTER_MODE_MASK, CAN_filterScale = CAN_FILTER_SCALE_DUAL_16
 *
 * bit #	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * ID	   |					CAN_filterIdLow						  	 	|
 * Map	   |					STID				  |RTR|IDE|    EXID		|
 * Mask	   |					CAN_filterIdHigh	  	   				    |
 * bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16
 *
 * bit #	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * ID	   |					CAN_filterMaskLow					  	 	|
 * Map	   |					STID				  |RTR|IDE|    EXID		|
 * Mask	   |					CAN_filterMaskHigh	  	   				    |
 * bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16
 *
 * ===========================================================================================================================
 * Two 16-Bit Filters - Identifier List	---> CAN_filterMode = CAN_FILTER_MODE_LIST, CAN_filterScale = CAN_FILTER_SCALE_DUAL_16
 *
 * bit #	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * ID	   |					CAN_filterIdLow						  	 	|
 * Map	   |					STID				  |RTR|IDE|    EXID		|
 * ID	   |					CAN_filterIdHigh	  	   				    |
 * bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16
 *
 * bit #	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
 * ID	   |					CAN_filterMaskLow					  	 	|
 * Map	   |					STID				  |RTR|IDE|    EXID		|
 * ID	   |					CAN_filterMaskHigh	  	   				    |
 * bit #	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16
 *
 *
 *
 *
 * */


// ===================================================
// ================== APIs Functions =================
// ===================================================
void MCAL_CAN_Init(CAN_InitConfig_t *config);

void MCAL_CAN_Start(void);

void MCAL_CAN_TxRequest(CAN_FrameHeader_t *pTxFrameHeader, uint8_t payload[],CAN_TXMBRx_TypeDef *pTxMailBox);
void MCAL_CAN_TxMailBoxEmpty(CAN_TXMBRx_TypeDef **ppTxMailBox);

void MCAL_CAN_FilterInit(CAN_Filter_Config_t *filterConfig);
void MCAL_CAN_FilterActivate(uint8_t filterId);
void MCAL_CAN_FilterDeActivate(uint8_t filterId);

void MCAL_CAN_RxReadFIFO(CAN_RXFIFOx_TypeDef *FIFOx, uint8_t payload[], CAN_FrameHeader_t *pRxFrameHeader);
void MCAL_CAN_pendingMessages(CAN_RXFIFOx_TypeDef *FIFOx, uint8_t *numberOfPendingMessages);


#endif /* INC_STM32F429_CAN_DRIVER_H_ */
