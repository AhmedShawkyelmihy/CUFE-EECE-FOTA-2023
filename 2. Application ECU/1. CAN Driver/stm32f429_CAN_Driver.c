/*
 * stm32f103c8_CAN_Driver.c
 *
 *  Created on: Feb 9, 2023
 *      Author: Ahmed
 */


// =============================================
// ================== Includes =================
// =============================================

#include <stm32f429_CAN_Driver.h>
#include "Platform_Types.h"

/*
 * =====================================================================================
 * ================================= Generic Macros ====================================
 * =====================================================================================
 */



#define CAN_SEG1_QUANTA_MASK				0x000F0000
#define CAN_SEG2_QUANTA_MASK				0x00700000
#define CAN_SJW_QUANTA_MASK					0x03000000
#define CAN_MODE_MASK						0x30000000
#define CAN_STID_MASK						0xFFE00000
#define CAN_EXID_MASK						0xFFFFFFF8
#define CAN_DLC_MASK						0x0000000F
#define CAN_FMP_MASK						0x00000003


/* CAN_MCR */
#define ABOM		6
#define AWUM		5
#define NART		4
#define RFLM		3
#define TXFP		2
#define SLEEP		1
#define INRQ		0

/* CAN_MSR */
#define RX			11
#define SAMP		10
#define RXM			9
#define TXM			8
#define SLAKI		4
#define WKUI		3
#define ERRI		2
#define SLAK		1
#define INAK		0

/* CAN_RFxR */
#define RFOMx		5
#define FOVRx		4
#define FULLx		3
#define FMPx		0

/* CAN_IER */
#define SLKIE		17
#define WKUIE		16
#define ERRIE		15
#define LECIE		11
#define BOFIE		10
#define EPVIE		9
#define EWGIE		8
#define FOVIE1		6
#define FFIE1		5
#define FMPIE1		4
#define FOVIE0		3
#define FFIE0		2
#define FMPIE0		1
#define TMEIE		0


/* CAN_TSR */
#define LOW2		31
#define LOW1		30
#define LOW0		29

#define TME2		28
#define TME1		27
#define TME0		26

#define CODE		24

#define ABRQ2		23
#define TERR2		19
#define ALST2		18
#define TXOK2		17
#define RQCP2		16

#define ABRQ1		15
#define TERR1		11
#define ALST1		10
#define TXOK1		9
#define RQCP1		8

#define ABRQ0		7
#define TERR0		3
#define ALST0		2
#define TXOK0		1
#define RQCP0		0



/* CAN_BTR */
#define SLIM		31
#define LBKM		30
#define SJW			24
#define TS2			20
#define TS1			16
#define BPR			0


/* CAN_TIxR */
#define STID		21
#define EXID		3
#define IDE			2
#define RTR			1
#define TXRQ		0

/* CAN_TDTxR */
#define TGT			8
#define DLC			0

/* CAN_TDLxR */
#define DATA3		24
#define DATA2		16
#define DATA1		8
#define DATA0		0

/* CAN_TDHxR */
#define DATA7		24
#define DATA6		16
#define DATA5		8
#define DATA4		0






/* CAN_FMR */
#define FINIT		0


/*
 * =====================================================================================
 * ================================= APIs Function Definition ==========================
 * =====================================================================================
 */

/**================================================================
 * @Fn				- MCAL_CAN_Init
 * @brief			- This is used to configure and initialize bxCAN module.
 * @param [in] 		- config: This specifies the configuration required to initialize the can module.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_Init(CAN_InitConfig_t *config)
{
	/* Configure CAN pins */
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_CAN1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**CAN1 GPIO Configuration
	    PA11     ------> CAN1_RX
	    PA12     ------> CAN1_TX
	 */
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Request to Enter Initialization Mode */
	SET__BIT(CAN_CSR->MCR, INRQ);
	while(READ__BIT(CAN_CSR->MSR, INAK) == 0);

	CLEAR__BIT(CAN_CSR->MCR, SLEEP);

	/* Wait for confirmation of entering Initialization Mode */
	while(READ__BIT(CAN_CSR->MSR, SLAK) != 0);

	//while((READ__BIT(CAN_CSR->MSR, SLAK) != 0) || (READ__BIT(CAN_CSR->MSR, INAK) == 0));

	/* Setting the BuadRate */
	CAN_CSR->BTR |= config->CAN_buadRate_preScaler;

	CAN_CSR->BTR &= ~(CAN_SEG1_QUANTA_MASK);
	CAN_CSR->BTR |=  (config->CAN_segment1_quantum << TS1);

	CAN_CSR->BTR &= ~(CAN_SEG2_QUANTA_MASK);
	CAN_CSR->BTR |=  (config->CAN_segment2_quantum << TS2);

	CAN_CSR->BTR &= ~(CAN_SJW_QUANTA_MASK);
	CAN_CSR->BTR |=  (config->CAN_reSyncJumpWidth << SJW);

	/* Determine CAN Mode */
	CAN_CSR->BTR |= (config->CAN_Mode << LBKM);

	/* Automatic Bus Off Management */
	switch(config->CAN_automaticBusOffManagement)
	{
	case DISABLE:
		CLEAR__BIT(CAN_CSR->MCR, ABOM);
		break;

	case ENABLE:
		SET__BIT(CAN_CSR->MCR, ABOM);
		break;
	}


	/* Automatic Wake Up */
	switch(config->CAN_automaticWakeUp)
	{
	case DISABLE:
		CLEAR__BIT(CAN_CSR->MCR, AWUM);
		break;

	case ENABLE:
		SET__BIT(CAN_CSR->MCR, AWUM);
		break;
	}

	/* No Automatic Retransmission */
	switch(config->CAN_noAutomaticRetransmission)
	{
	case DISABLE:
		CLEAR__BIT(CAN_CSR->MCR, NART);
		break;

	case ENABLE:
		SET__BIT(CAN_CSR->MCR, NART);
		break;
	}

	/* Received FIFO Locked Mode */
	switch(config->CAN_receiveFIFOLockedMode)
	{
	case CAN_RX_FIFO_LOCK_MODE_NOT_LOCKED:
		CLEAR__BIT(CAN_CSR->MCR, RFLM);
		break;

	case CAN_RX_FIFO_LOCK_MODE_LOCKED:
		SET__BIT(CAN_CSR->MCR, RFLM);
		break;
	}

	/* Transmit FIFO Priority */
	switch(config->CAN_transmitFIFOPriority)
	{
	case CAN_TX_FIFO_PRIORITY_IDENTIFIER:
		CLEAR__BIT(CAN_CSR->MCR, TXFP);
		break;

	case CAN_TX_FIFO_PRIORITY_REQUEST_ORDER:
		SET__BIT(CAN_CSR->MCR, TXFP);
		break;
	}


}

/**================================================================
 * @Fn				- MCAL_CAN_Start
 * @brief			- This is used to start CAN hardware to sent and receive messages.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_Start(void)
{
	CLEAR__BIT(CAN_CSR->MCR, INRQ);
	while(READ__BIT(CAN_CSR->MSR, INAK) != 0);
}


/**================================================================
 * @Fn				- MCAL_CAN_TxRequest
 * @brief			- This is used to configure and initialize Transmission.
 * @param [in] 		- pTxFrameHeader: This specifies the configuration required to transmit a frame.
 * @param [in]		- pTxMailBox: This is a pointer to empty mail box.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_TxRequest(CAN_FrameHeader_t *pTxFrameHeader, uint8_t payload[], CAN_TXMBRx_TypeDef *pTxMailBox)
{

	/* Set Identifier */
	switch(pTxFrameHeader->CAN_IDE)
	{
	case CAN_IDE_STD:
		CLEAR__BIT(pTxMailBox->TIxR, IDE);
		pTxMailBox->TIxR &=	~(CAN_STID_MASK);
		pTxMailBox->TIxR |= ((pTxFrameHeader->CAN_ID & 0x7FF) << STID);
		break;

	case CAN_IDE_EXTD:
		SET__BIT(pTxMailBox->TIxR, IDE);
		pTxMailBox->TIxR	&= ~(CAN_EXID_MASK);
		pTxMailBox->TIxR |= ((pTxFrameHeader->CAN_ID & 0x1FFFFFFF) << EXID);
		break;
	}

	/* set the frame type remote or data */
	switch(pTxFrameHeader->CAN_RTR)
	{
	case CAN_RTR_DATA:
		CLEAR__BIT(pTxMailBox->TIxR, RTR);
		break;

	case CAN_RTR_REMOTE:
		SET__BIT(pTxMailBox->TIxR, RTR);
		break;
	}

	/* set Data Length Counter */
	CLEAR__BIT(pTxMailBox->TDTxR, TGT);
	pTxMailBox->TDTxR &=	~(0xF << DLC);
	pTxMailBox->TDTxR |=	(pTxFrameHeader->CAN_DLC << DLC);

	/* set The Data */
	uint32 *pPayload = (uint32*)payload;
	pTxMailBox->TDLxR = pPayload[0];

	if(pTxFrameHeader->CAN_DLC > 4)
	{
		pTxMailBox->TDHxR = pPayload[1];
	}

	/* Start Transmission Request */
	pTxMailBox->TIxR |= (1 << TXRQ);
}


/**================================================================
 * @Fn				- MCAL_CAN_TxMailBoxEmpty
 * @brief			- This is used to return the address of the empty mail box.
 * @param [in]		- ppTxMailBox: This is a pointer to a pointer to return the
 * 						address of the empty mail box.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_TxMailBoxEmpty(CAN_TXMBRx_TypeDef **ppTxMailBox)
{
	if((READ__BIT(CAN_CSR->TSR, TME0)) == 1)
	{
		*ppTxMailBox = CAN_TXMBR0;
	}
	else if((READ__BIT(CAN_CSR->TSR, TME1)) == 1)
	{
		*ppTxMailBox = CAN_TXMBR1;
	}
	else if((READ__BIT(CAN_CSR->TSR, TME2)) == 1)
	{
		*ppTxMailBox = CAN_TXMBR2;
	}
	else
	{
		*ppTxMailBox = (void*)0;
	}
}

/**================================================================
 * @Fn				- MCAL_CAN_FilterInit
 * @brief			- This is used to initialize the filter hardware.
 * @param [in]		- filterConfig: This is a pointer to the configuration of the filter.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_FilterInit(CAN_Filter_Config_t *filterConfig)
{
	/* Enter The Initialization filter mode */
	SET__BIT(CAN_FLTR_CONFIG->FMR, FINIT);

	/* set the filter mode */
	switch(filterConfig->CAN_filterMode)
	{
	case CAN_FILTER_MODE_MASK:
		CLEAR__BIT(CAN_FLTR_CONFIG->FM1R, filterConfig->CAN_filterId);
		break;

	case CAN_FILTER_MODE_LIST:
		SET__BIT(CAN_FLTR_CONFIG->FM1R, filterConfig->CAN_filterId);
		break;
	}

	/* set the filter scale */
	switch(filterConfig->CAN_filterScale)
	{
	case CAN_FILTER_SCALE_DUAL_16:
		CLEAR__BIT(CAN_FLTR_CONFIG->FS1R, filterConfig->CAN_filterId);
		break;

	case CAN_FILTER_SCALE_SINGLE_32:
		SET__BIT(CAN_FLTR_CONFIG->FS1R, filterConfig->CAN_filterId);
		break;
	}

	/* set the filter assignment */
	switch(filterConfig->CAN_filterAssign)
	{
	case CAN_FILTER_ASSIGN_FIFO0:
		CLEAR__BIT(CAN_FLTR_CONFIG->FFA1R, filterConfig->CAN_filterId);
		break;

	case CAN_FILTER_ASSIGN_FIFO1:
		SET__BIT(CAN_FLTR_CONFIG->FFA1R, filterConfig->CAN_filterId);
		break;
	}

	/* set the filter Id List and Mask */
	CAN_FILTERS->FiRx[2 * filterConfig->CAN_filterId] = ((uint32)filterConfig->CAN_filterIdHigh << 16)
																									| filterConfig->CAN_filterIdLow;

	CAN_FILTERS->FiRx[2 * filterConfig->CAN_filterId + 1] = ((uint32)filterConfig->CAN_filterMaskHigh << 16)
																									| filterConfig->CAN_filterMaskLow;

	/* Exit from The Initialization filter mode */
	CLEAR__BIT(CAN_FLTR_CONFIG->FMR, FINIT);
}

/**================================================================
 * @Fn				- MCAL_CAN_FilterActivate
 * @brief			- This is used to activate the specified filter.
 * @param [in]		- filterId: This specifies which filter to be activated @ref CAN_FILTER_ID_define.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_FilterActivate(uint8_t filterId)
{
	SET__BIT(CAN_FLTR_CONFIG->FA1R, filterId);
}

/**================================================================
 * @Fn				- MCAL_CAN_FilterDeActivate
 * @brief			- This is used to deactivate the specified filter.
 * @param [in]		- filterId: This specifies which filter to be deactivated @ref CAN_FILTER_ID_define.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_FilterDeActivate(uint8_t filterId)
{
	CLEAR__BIT(CAN_FLTR_CONFIG->FA1R, filterId);
}

/**================================================================
 * @Fn				- MCAL_CAN_RxReadFIFO
 * @brief			- This is used to read the received messages from the mail box.
 * @param [in]		- RxMessage: This is used to return the message from the mail box.
 * @param [in]		- FIFOx: This a pointer to FIFO which has the message @ref FIFOx.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_RxReadFIFO(CAN_RXFIFOx_TypeDef *FIFOx, uint8_t payload[], CAN_FrameHeader_t *pRxFrameHeader)
{
	/* Read standard or extended ID bit --> IDE */
	pRxFrameHeader->CAN_IDE	=	READ__BIT(FIFOx->RIxR, IDE);

	/* Read The ID */
	switch(pRxFrameHeader->CAN_IDE)
	{
	case CAN_IDE_STD:
		pRxFrameHeader->CAN_ID	=	(FIFOx->RIxR >> STID);
		break;

	case CAN_IDE_EXTD:
		pRxFrameHeader->CAN_ID	=	(FIFOx->RIxR >> EXID);
		break;
	}

	/* Read The Frame Type Remote or Data RTR BIT */
	pRxFrameHeader->CAN_RTR	=	READ__BIT(FIFOx->RIxR, RTR);

	/* Read Data Length Counter */
	pRxFrameHeader->CAN_DLC	 =	FIFOx->RDTxR & CAN_DLC_MASK;

	/* Read Data */
	uint32 *pPayload = (uint32*)payload;
	pPayload[0] = FIFOx->RDLxR;

	if(pRxFrameHeader->CAN_DLC > 4)
	{
		pPayload[1] = FIFOx->RDHxR;
	}

	if(FIFOx == CAN_RXFIFO0)
	{
		SET__BIT(CAN_CSR->RF0R, RFOMx);
	}
	else
	{
		SET__BIT(CAN_CSR->RF1R, RFOMx);
	}
}

/**================================================================
 * @Fn				- MCAL_CAN_pendingMessages
 * @brief			- This is used to return the number of pending messages in a certain FIFO.
 * @param [in]		- FIFOx: This specifies the FIFO number @ref FIFOx.
 * @param [in]		- numberOfPendingMessages: This is used return the number of pending the messages.
 * @retval 			- None.
 * Note				- None.
 */
void MCAL_CAN_pendingMessages(CAN_RXFIFOx_TypeDef *FIFOx, uint8_t *numberOfPendingMessages)
{
	if(FIFOx == CAN_RXFIFO0)
	{
		*numberOfPendingMessages	=	CAN_CSR->RF0R & CAN_FMP_MASK;
	}
	else
	{
		*numberOfPendingMessages	=	CAN_CSR->RF1R & CAN_FMP_MASK;
	}

}
