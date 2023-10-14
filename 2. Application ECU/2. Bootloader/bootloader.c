/*
 * @file       bootloader.c
 * @version    1.0.0
 * @brief      Bootloader Source File.
 * @author     Ahmed Shawky Hamed
 */

/* -------------------------- Includes -------------------------- */
#include "bootloader.h"

#ifndef FILTER_FLAG
//#define FILTER_FLAG
#endif

/* --------------- Software Interfaces Definitions -------------- */
void BL_Print_Message(char* format , ...);
BL_Status BL_UART_Fetch_Host_Command(void);
void BL_CAN_Fetch_Host_Command(void);

/* ------------- Static Helper Functions Prototypes ------------- */
static uint8_t Bootloader_CRC_Verify( uint8_t* pData , uint32_t Data_Len , uint32_t Host_CRC );
static void Bootloader_Send_ACK( uint8_t Replay_Len );
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t* Data_Buffer , uint32_t Data_Len );
static uint8_t CBL_STM32F429_Get_RDP_Level(void);
static uint8_t Host_Jump_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number , uint8_t Number_of_Sectors );
static uint8_t Flash_Memory_Write_Payload( uint8_t* Host_Payload , uint32_t Payload_Start_Address , uint16_t Payload_Len );
static void Bootloader_Jump_To_User_App(void);
static uint8_t Configure_Flash_Sector_RW_Protection(uint8_t Sector_Details , uint8_t Protection_Mode , uint8_t Disable);
static uint8_t Change_ROP_Level(uint32_t ROP_Level);

static void Bootloader_Send_Data_To_Host_CAN(uint16_t ID ,uint8_t* Data_Buffer , uint32_t Data_Len );


/* ----------------- UART Static Functions Prototypes ---------------- */
static void Bootloader_Get_Version(uint8_t* Host_Buffer);
static void Bootloader_Get_Chip_Indentification_Nmuber(uint8_t* Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8_t* Host_Buffer);
static void Bootloader_Jump_To_Address(uint8_t* Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t* Host_Buffer);
static void Bootloader_Memory_Write(uint8_t* Host_Buffer);
static void Bootloader_Enable_RW_Protection(uint8_t* Host_Buffer);
static void Bootloader_Disable_RW_Protection(uint8_t* Host_Buffer);
static void Bootloader_Change_Read_Protection_Level(uint8_t* Host_Buffer);
static void Bootloader_Memory_Read(uint8_t* Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status(uint8_t* Host_Buffer);
static void Bootloader_Read_OTP(uint8_t* Host_Buffer);

/* ----------------- CAN Static Functions Prototypes ---------------- */
static void Bootloader_Get_Version_CAN(uint8_t* Host_Buffer);
static void Bootloader_Get_Chip_Indentification_Nmuber_CAN(uint8_t* Host_Buffer);
static void Bootloader_Read_Protection_Level_CAN(uint8_t* Host_Buffer);
static void Bootloader_Jump_To_Address_CAN(uint8_t* Host_Buffer);
static void Bootloader_Erase_Flash_CAN(uint8_t* Host_Buffer);
static void Bootloader_Memory_Write_CAN(uint8_t* Host_Buffer);

/* ------------- Global Variables Definitions ------------- */

/* to store the data sent by the host to the bootloader*/
static uint8_t BL_Host_Buffer[BL_HOST_BUFFER_RX_LENGTH];
//static uint8_t BL_Test_Buffer[BL_HOST_BUFFER_RX_LENGTH] = {0};

static uint32_t RxID ;
static uint8_t  RxDLC ;


#ifdef FILTER_FLAG
int filter_flag = 0;
#endif

/* ------------- Software Interfaces Definitions ------------- */

void BL_Print_Message(char* format , ...)
{
	char Message[1000] = {0} ;
	va_list args ;

	/* Enable access to the variable arguments */
	va_start( args , format ) ;

	/* Write formatted data from variable argument list to string */
	vsprintf( Message , format , args ) ;

#if ( BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MESSAGE )
	/* Transmit the formatted data through the defined UART */
	HAL_UART_Transmit( BL_DEBUG_UART , (uint8_t *)Message , sizeof(Message) , HAL_MAX_DELAY ) ;

#elif ( BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MESSAGE )
	/* Transmit the formatted data through the defined SPI */


#elif ( BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE )
	/* Transmit the formatted data through the defined CAN */

#endif

	/* Performs cleanup for an ap object initialized by a call to va_start */
	va_end( args ) ;
}

BL_Status BL_UART_Fetch_Host_Command(void)
{
	BL_Status Status = BL_NACK ;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;

	/* Local var. to determine the size of the data sent by the HOST */
	uint8_t Data_Length = 0 ;

	/* To clear the buffer every time while(1) call the function so
	 * we can write new data came from the host in it
	 */
	memset( BL_Host_Buffer , 0 , BL_HOST_BUFFER_RX_LENGTH ) ;

	/* Receiving the data from the host by BL_HOST_COMMUNICATION_UART and
	 * storing it in BL_Host_Buffer , receiving 1 byte (length of the packet sent) with timeout = HAL_MAX_DELAY
	 */
	// can receive
	HAL_Status = HAL_UART_Receive( BL_HOST_COMMUNICATION_UART , BL_Host_Buffer , 1 , HAL_MAX_DELAY ) ;

	if( HAL_Status != HAL_OK )   /* to check that there is no error in receiving */
	{
		/* There is an error in receiving */
		Status = BL_NACK ;
	}

	else
	{
		/* We did it to save in this var. the length of the packet to use this var.
		 * in a function receives the complete packet fro the host */
		Data_Length = BL_Host_Buffer[0] ;

		/* Receiving the rest of the packet sent by the HOST */
		HAL_Status = HAL_UART_Receive( BL_HOST_COMMUNICATION_UART , &BL_Host_Buffer[1] , Data_Length , HAL_MAX_DELAY ) ;
		if( HAL_Status != HAL_OK )  /* to check that there is no error in receiving */
		{
			/* There is an error in receiving */
			Status = BL_NACK ;
		}

		else
		{
			switch( BL_Host_Buffer[1] )  /* To parse the command sent by the host */
			{
			case CBL_GET_VER_CMD :
				Bootloader_Get_Version( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_GET_CID_CMD :
				Bootloader_Get_Chip_Indentification_Nmuber( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_GET_RDP_STATUS_CMD :
				Bootloader_Read_Protection_Level( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_GO_TO_ADDR_CMD :
				Bootloader_Jump_To_Address( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_FLASH_ERASE_CMD :
				Bootloader_Erase_Flash( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_MEM_WRITE_CMD :
				Bootloader_Memory_Write( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_EN_R_W_PROTECT_CMD :
				Bootloader_Enable_RW_Protection( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_DIS_R_W_PROTECT_CMD :
				Bootloader_Disable_RW_Protection( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_CHANGE_ROP_Level_CMD :
				Bootloader_Change_Read_Protection_Level( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_MEM_READ_CMD :
				BL_Print_Message("Read data from different memories of the MCU \r\n") ;
				Bootloader_Memory_Read( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_READ_SECTOR_STATUS_CMD :
				BL_Print_Message("Read all the sector protection status \r\n") ;
				Bootloader_Get_Sector_Protection_Status( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			case CBL_OTP_READ_CMD :
				BL_Print_Message("Read the OTP contents \r\n") ;
				Bootloader_Read_OTP( BL_Host_Buffer ) ;
				Status = BL_OK ;
				break ;
			default :
				BL_Print_Message("Invalid Command Code From Host !! \r\n") ;
				break ;
			}
		}
	}

	return Status ;
}

void BL_CAN_Fetch_Host_Command(void)
{

	/* Local var. to determine the size of the data sent by the HOST */
	//uint8_t Data_Length = 0 ;

	/* To clear the buffer every time while(1) call the function so
	 * we can write new data came from the host in it
	 */
	memset( BL_Host_Buffer , 0 , BL_HOST_BUFFER_RX_LENGTH ) ;

	/* Receiving the data from the host by BL_HOST_COMMUNICATION_UART and
	 * storing it in BL_Host_Buffer , receiving 1 byte (length of the packet sent) with timeout = HAL_MAX_DELAY
	 */
	// can receive
	CAN_Rx( &RxID , &RxDLC , BL_Host_Buffer );

	/* We did it to save in this var. the length of the packet to use this var.
	 * in a function receives the complete packet fro the host */
	//Data_Length = BL_Host_Buffer[0] ;

	switch( BL_Host_Buffer[0] )  /* To parse the command sent by the host */
	{
	case CBL_GET_VER_CMD :
		Bootloader_Get_Version_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_GET_CID_CMD :
		Bootloader_Get_Chip_Indentification_Nmuber_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_GET_RDP_STATUS_CMD :
		Bootloader_Read_Protection_Level_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_GO_TO_ADDR_CMD :
		Bootloader_Jump_To_Address_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_FLASH_ERASE_CMD :
		Bootloader_Erase_Flash_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_MEM_WRITE_CMD :
		//		Bootloader_Memory_Write_CAN( BL_Host_Buffer ) ;
		Bootloader_Memory_Write_CAN( BL_Host_Buffer ) ;
		break ;
	case CBL_EN_R_W_PROTECT_CMD :
		Bootloader_Enable_RW_Protection( BL_Host_Buffer ) ;
		break ;
	case CBL_DIS_R_W_PROTECT_CMD :
		Bootloader_Disable_RW_Protection( BL_Host_Buffer ) ;
		break ;
	case CBL_CHANGE_ROP_Level_CMD :
		Bootloader_Change_Read_Protection_Level( BL_Host_Buffer ) ;
		break ;
	case CBL_MEM_READ_CMD :
		BL_Print_Message("Read data from different memories of the MCU \r\n") ;
		Bootloader_Memory_Read( BL_Host_Buffer ) ;
		break ;
	case CBL_READ_SECTOR_STATUS_CMD :
		BL_Print_Message("Read all the sector protection status \r\n") ;
		Bootloader_Get_Sector_Protection_Status( BL_Host_Buffer ) ;
		break ;
	case CBL_OTP_READ_CMD :
		BL_Print_Message("Read the OTP contents \r\n") ;
		Bootloader_Read_OTP( BL_Host_Buffer ) ;
		break ;
	case CBL_JUMP_TO_USER_APP_CMD :
		Bootloader_Jump_To_User_App();
		break ;
	case CBL_JUMP_TO_BOOTLOADER_CMD :
		BL_Print_Message("MCU is already in bootloader \r\n") ;
		break ;
	default :
		BL_Print_Message("Invalid Command Code From Host !! \r\n") ;
		break ;
	}

}

/* ------------- Static Helper Functions Definitions ------------- */

/* Funtion to verify that the CRC sent by the HOST is equal to the CRC calculated by the MCU(Bootloader). */
static uint8_t Bootloader_CRC_Verify( uint8_t* pData , uint32_t Data_Len , uint32_t Host_CRC )
{
	/* Local var. to be returned for determining if the CRC verification process is passed or failed. */
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED ;

	/* Local var. to store the CRC value calculated by the MCU(Bootloader). */
	uint32_t MCU_CRC_CALCULATED = 0 ;

	/* As HAL_CRC_Accumulate wants a 32-bit buffer as argument to it */
	uint32_t Data_Buffer = 0 ;

	/* Calculate CRC32 */
	uint8_t Data_Counter = 0 ;
	for( Data_Counter = 0 ; Data_Counter < Data_Len ; Data_Counter++ )
	{
		Data_Buffer = (uint32_t)pData[Data_Counter] ;
		MCU_CRC_CALCULATED = HAL_CRC_Accumulate( CRC_ENGINE_OBJ , &Data_Buffer , 1 );
	}

	/* Reset the CRC calculation unit */
	__HAL_CRC_DR_RESET( CRC_ENGINE_OBJ ) ;

	/* Comparing the host CRC with calculated CRC */
	if( MCU_CRC_CALCULATED == Host_CRC )
	{
		/* Two CRCs have the same value */
		CRC_Status = CRC_VERIFICATION_PASSED ;
	}

	else
	{
		/* Two CRCs have different values */
		CRC_Status = CRC_VERIFICATION_FAILED ;
	}

	return CRC_Status ;
}

/* Function to send ACK to the host */
static void Bootloader_Send_ACK( uint8_t Replay_Len )
{
	/* Define a buffer to be sent to the HOST includes the ACK value and the length of the reply. */
	uint8_t Ack_Value[2] = {0} ;

	/* First element of the buffer is the ACK value. */
	Ack_Value[0] = CBL_SEND_ACK ;

	/* Second element of the buffer is the reply length */
	Ack_Value[1] = Replay_Len ;

#if(BL_COMM_METHOD == BL_ENABLE_UART_COMM_MESSAGE)
	/* Sending the ACK Buffer to the HOST via UART */
	HAL_UART_Transmit( BL_HOST_COMMUNICATION_UART , (uint8_t*)Ack_Value , 2 , HAL_MAX_DELAY ) ;
#elif(BL_COMM_METHOD == BL_ENABLE_CAN_COMM_MESSAGE)
	/* Sending the ACK Buffer to the HOST via CAN */
	//CAN_Tx();
#endif
}

/* Function to send NACK to the host */
static void Bootloader_Send_NACK(void)
{
	/* Local var. to determine the NACK value to be sent to the HOST. */
	uint8_t Ack_Value = CBL_SEND_NACK ;

#if(BL_COMM_METHOD == BL_ENABLE_UART_COMM_MESSAGE)
	/* Sending NACK to the HOST via UART */
	HAL_UART_Transmit( BL_HOST_COMMUNICATION_UART , &Ack_Value , 1 , HAL_MAX_DELAY ) ;
#elif(BL_COMM_METHOD == BL_ENABLE_CAN_COMM_MESSAGE)
	/* Sending NACK to the HOST via CAN */
	//CAN_Tx();
#endif
}

/* Function to send the required data from the MCU(Bootloader) to the HOST. */
static void Bootloader_Send_Data_To_Host(uint8_t* Data_Buffer , uint32_t Data_Len )
{
	HAL_UART_Transmit( BL_HOST_COMMUNICATION_UART , Data_Buffer , Data_Len , HAL_MAX_DELAY ) ;
}

/* Function to send the required data from the MCU(Bootloader) to the HOST. */
static void Bootloader_Send_Data_To_Host_CAN(uint16_t ID ,uint8_t* Data_Buffer , uint32_t Data_Len )
{
	CAN_Tx( ID , Data_Len , Data_Buffer);
}

/* Function to get the reading protection level. */
static uint8_t CBL_STM32F429_Get_RDP_Level(void)
{
	FLASH_OBProgramInitTypeDef FLASH_OBProgram ;

	/* Getting the option byte configuration */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram) ;

	return (uint8_t)(FLASH_OBProgram.RDPLevel) ;
}

/* Function to verify the jump address */
static uint8_t Host_Jump_Address_Verification(uint32_t Jump_Address)
{
	/* Local var. to store if the jump address is valid or invalid */
	uint8_t Address_Verification = ADDRESS_IS_INVALID ;

	if(( Jump_Address >= SRAM1_BASE ) && ( Jump_Address <= STM32F429_SRAM1_END ))
	{
		Address_Verification = ADDRESS_IS_VALID ;
	}

	else if(( Jump_Address >= SRAM2_BASE ) && ( Jump_Address <= STM32F429_SRAM2_END ))
	{
		Address_Verification = ADDRESS_IS_VALID ;
	}

	else if(( Jump_Address >= SRAM3_BASE ) && ( Jump_Address <= STM32F429_SRAM3_END ))
	{
		Address_Verification = ADDRESS_IS_VALID ;
	}

	else if(( Jump_Address >= FLASH_BASE ) && ( Jump_Address <= STM32F429_FLASH_END ))
	{
		Address_Verification = ADDRESS_IS_VALID ;
	}

	else
	{
		Address_Verification = ADDRESS_IS_INVALID ;
	}

	return Address_Verification ;
}

/* Function is requiring modification on choosing the bank to make the mass erase on it */
/* Function to perform the flash erase */
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number , uint8_t Number_of_Sectors )
{
	/* Local var. to store the validity of the required sector/s to erase */
	uint8_t Sector_Validity_Status = INVALID_SECTOR_NUMBER ;

	/* Struct to initialize the configurations of the erase process */
	FLASH_EraseInitTypeDef pEraseInit ;

	/* Local var. to store the remaining sectors if host requested erase
	 * sector number + number of sectors > CBL_FLASH_MAX_SECTOR_NUMBER
	 * so we will force the erase process to be limited with the number of sectors in the flash */
	uint8_t Remaining_Sectors = 0 ;

	uint32_t SectorError = 0 ;

	if( Number_of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER )
	{
		/* Number of sectors is out of range */
		Sector_Validity_Status = INVALID_SECTOR_NUMBER ;
	}

	else
	{
		if(( Sector_Number <= (CBL_FLASH_MAX_SECTOR_NUMBER - 1)) || ( CBL_FLASH_MASS_ERASE == Sector_Number ))
		{
			if( CBL_FLASH_MASS_ERASE == Sector_Number )
			{
				/* Flash mass erase activation */
				pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
				BL_Print_Message("Flash mass erase activation \r\n") ;
#endif
			}

			else
			{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
				BL_Print_Message("HOST needs sector erase \r\n") ;
#endif

				Remaining_Sectors = CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Number ;

				if( Number_of_Sectors > Remaining_Sectors )
				{
					Number_of_Sectors = Remaining_Sectors ;             /* Forcing operation as we mentioned */
				}
				else{ /* Nothing */ }

				pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS ; 			/* Sectors erase only */
				pEraseInit.Sector = Sector_Number ;										/* Initial FLASH sector to erase when Mass erase is disabled */
				pEraseInit.NbSectors = Number_of_Sectors ;						/* Number of sectors to be erased */
			}

			pEraseInit.Banks = FLASH_BANK_1 ;   										/* needs modification to choose between bank 1 & 2 */
			pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3 ;				/* Device operating range: 2.7V to 3.6V */

			/* Unlock the FLASH control register access */
			HAL_FLASH_Unlock() ;

			/* Performing a mass erase or erase the specified FLASH memory sectors */
			HAL_FLASHEx_Erase( &pEraseInit , &SectorError );
			if( HAL_SUCCESSFUL_ERASE == SectorError )
			{
				Sector_Validity_Status = SUCCESSFUL_ERASE ;
			}
			else
			{
				Sector_Validity_Status = UNSUCCESSFUL_ERASE ;
			}

			/* Lock the FLASH control register access */
			HAL_FLASH_Lock() ;
		}

		else
		{
			Sector_Validity_Status = UNSUCCESSFUL_ERASE ;
		}
	}

	return Sector_Validity_Status ;
}

/* Function to perform memory write. */
static uint8_t Flash_Memory_Write_Payload( uint8_t* Host_Payload , uint32_t Payload_Start_Address , uint16_t Payload_Len )
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;

	/* Local var. to store the value of a counter looping on the file which is required to be written */
	uint16_t Payload_Counter = 0 ;

	/* Local var. to store the status of writing process if it is passed pr failed */
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;

	/* Unlocking the Flash control register access & check if it passed or failed */
	HAL_Status = HAL_FLASH_Unlock() ;
	if( HAL_Status != HAL_OK )
	{
		/* Sending to the HOST that the writing process is failed and stop the process */
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;
	}
	else
	{
		for( Payload_Counter = 0 ; Payload_Counter < Payload_Len ; Payload_Counter++)
		{
			HAL_Status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE , (Payload_Start_Address + Payload_Counter) , Host_Payload[Payload_Counter] ) ;
			//			HAL_Status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD , (Payload_Start_Address + Payload_Counter) , *(uint32_t*)(Host_Payload+Payload_Counter) ) ;
			if( HAL_Status != HAL_OK )
			{
				/* Sending to the HOST that the writing process is failed and stop the process */
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;
				break ;
			}
			else
			{
				/* Sending to the HOST that the writing process is passed */
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED ;
			}
		}

	}

	if(( FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status ) && ( HAL_OK == HAL_Status ))
	{
		/* Everything is good now we can lock */
		/* Locking the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock() ;

		/* Checking that the locking process is passed or failed */
		if( HAL_Status != HAL_OK )
		{
			/* Sending to the HOST that the writing process is failed and stop the process */
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;
		}
		else
		{
			/* Sending to the HOST that the writing process is passed */
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED ;
		}
	}
	else
	{
		/* Sending to the HOST that the writing process is failed and stop the process */
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;
	}

	return Flash_Payload_Write_Status ;
}

/* Function to jump to user application. */
static void Bootloader_Jump_To_User_App(void)
{
	//	uint8_t BL_Jump[4] = {0};
	//	BL_Jump[0] = BL_OK;
	//
	//	Bootloader_Send_Data_To_Host_CAN(0x3FF, BL_Jump, 1);

	/* First of all we should know the start address of the application */

	/* Value of the main stack pointer of our main application */
	/* MSP Value is stored in the first 4 bytes of the application sector */
	uint32_t MSP_VALUE = *((volatile uint32_t*)FLASH_SECTOR2_BASE_ADDRESS) ;

	/* Reset Handler definition function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FLASH_SECTOR2_BASE_ADDRESS + 4)) ;

	/* We need pointer to function to save the reset handler address in it for jumping to the
	 * application by calling it + we make casting to overcome the error */
	pMainApp ResetHandler_Address = (pMainApp)MainAppAddr;

	/* Assigns the given value (MSP Value of the application) to the main stack pointer of the bootloader
	 * to acheive jumping */
	__set_MSP(MSP_VALUE);

	/* Deinitialization of modules to initialize it as the application program require */
	HAL_RCC_DeInit();   /* Resets the RCC clock configuration to the default reset state. */
	/* we can call any other deinit function for (GPIO , UART , ....) to reset it if required */
	HAL_UART_DeInit(&huart1);
	HAL_UART_DeInit(&huart3);

	/* Jumping to application's reset handler */
	ResetHandler_Address();



}

/* Function to configure flash sector R/W protection. */
static uint8_t Configure_Flash_Sector_RW_Protection(uint8_t Sector_Details , uint8_t Protection_Mode , uint8_t Disable)
{
	/* Local var. to store the status of enabling or disabling process if it is passed or failed */
	uint8_t Status = UNSUCCESSFUL_DISABLING ;

	/* Flash option control register (OPTCR) */
	volatile uint32_t * pOPTCR_Register = (uint32_t*)OPTCR_REGISTER_BASE_ADDRESS ;

	if(Disable)
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Disable Read/Write Protection On Sectors \r\n");
#endif

		/* There is a procedure to disable read/write protection */
		/* 1.Option byte configuration unlocking */
		HAL_FLASH_OB_Unlock();

		/* 2.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 3.Clearing SPRMOD bit (31st bit) to enable write protection */
		*pOPTCR_Register &= ~(1<<31) ;

		/* 4.Clearing the protection : make all bits belonging to sectors as 1 */
		*pOPTCR_Register |= (0xFF << 16) ;

		/* 5.Setting the option start bit (OPTSTRT) in the FLASH_OPTCR register */
		*pOPTCR_Register |= (1<<1) ;

		/* 6.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 7.Option byte configuration locking */
		HAL_FLASH_OB_Lock();

		Status = SUCCESSFUL_DISABLING ;
	}

	if( Protection_Mode == 1 )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Write Protection Is Setting On The Sectors \r\n");
#endif

		/* There is a procedure to enable write protection */
		/* 1.Option byte configuration unlocking */
		HAL_FLASH_OB_Unlock();

		/* 2.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 3.Clearing SPRMOD bit (31st bit) to enable write protection */
		*pOPTCR_Register &= ~(1<<31) ;

		/* 4.Putting write protection on specefied sectors */
		*pOPTCR_Register &= ~(Sector_Details << 16) ;

		/* 5.Setting the option start bit (OPTSTRT) in the FLASH_OPTCR register */
		*pOPTCR_Register |= (1<<1) ;

		/* 6.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 7.Option byte configuration locking */
		HAL_FLASH_OB_Lock();

		Status = SUCCESSFUL_PROTECTION ;
	}

	else if( Protection_Mode == 2 )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Read/Write Protection Is Setting On The Sectors \r\n");
#endif

		/* There is a procedure to enable read/write protection */
		/* 1.Option byte configuration unlocking */
		HAL_FLASH_OB_Unlock();

		/* 2.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 3.Setting SPRMOD bit (31st bit) to enable read/write protection */
		*pOPTCR_Register |= (1<<31) ;

		/* 4.Putting read/write protection on specefied sectors */
		*pOPTCR_Register &= ~(0xFF << 16) ;
		*pOPTCR_Register |= (Sector_Details << 16) ;

		/* 5.Setting the option start bit (OPTSTRT) in the FLASH_OPTCR register */
		*pOPTCR_Register |= (1<<1) ;

		/* 6.Wait til no active operation on flash */
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET ) ;

		/* 7.Option byte configuration locking */
		HAL_FLASH_OB_Lock();

		Status = SUCCESSFUL_PROTECTION ;
	}

	return Status ;
}

/* Function to change ROP level. */
static uint8_t Change_ROP_Level(uint32_t ROP_Level)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;

	FLASH_OBProgramInitTypeDef FLASH_OBProgramInit ;

	/* Local var. to store if the changing ROP level process is passed or failed */
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;

	/* Unlocking the FLASH option control registers access */
	HAL_Status = HAL_FLASH_OB_Unlock() ;
	if( HAL_Status != HAL_OK )
	{
		/* Reporting that unlocking process is failed */
		ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Unlocking the FLASH option control registers is failed \r\n");
#endif
	}
	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Unlocking the FLASH option control registers is passed \r\n");
#endif

		/* Initializing option bytes configurations structure */
		FLASH_OBProgramInit.OptionType = OPTIONBYTE_RDP ;

		/* Skip WRPState & WRPSector as they're not used */

		/* Do this configurations on bank 1 */
		FLASH_OBProgramInit.Banks = FLASH_BANK_1 ;

		/* Set the read protection level */
		FLASH_OBProgramInit.RDPLevel = (uint32_t)ROP_Level ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Changing the ROP level to be level : %i \r\n",FLASH_OBProgramInit.RDPLevel);
#endif

		/* Skip BORLevel & USERConfig as they're not used */

		/* Program option bytes */
		HAL_Status = HAL_FLASHEx_OBProgram(&FLASH_OBProgramInit) ;
		if( HAL_Status != HAL_OK )
		{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Programming the option bytes is failed \r\n");
#endif

			/* Lock the FLASH option control registers access */
			HAL_Status = HAL_FLASH_OB_Lock() ;

			/* Reporting that programming process is failed */
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
		}
		else
		{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Programming the option bytes is passed \r\n");
#endif

			/* Launch the option byte loading */
			HAL_Status = HAL_FLASH_OB_Launch() ;
			if( HAL_Status != HAL_OK )
			{
				/* Reporting that launching process is failed */
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
			}
			else
			{
				/* Lock the FLASH option control registers access */
				HAL_Status = HAL_FLASH_OB_Lock() ;
				if( HAL_Status != HAL_OK )
				{
					/* Reporting that locking process is failed */
					ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
				}
				else
				{
					/* Reporting that ROP Level changing process is passed */
					ROP_Level_Status = ROP_LEVEL_CHANGE_VALID ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
					BL_Print_Message("ROP level changed to be level : %i \r\n",FLASH_OBProgramInit.RDPLevel);
#endif
				}
			}
		}
	}
	return ROP_Level_Status ;
}

/* ------------- Static Functions Definitions ------------- */

static void Bootloader_Get_Version(uint8_t* Host_Buffer)
{
	/* Array to store in it all information about the BL version to send it to the host */
	uint8_t BL_Version[4] = { CBL_VENDOR_ID , CBL_SW_MAJOR_VERSION , CBL_SW_MINOR_VERSION , CBL_SW_PATCH_VERSION } ;

	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the bootloader version from the MCU \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing)
	 */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(4);
		Bootloader_Send_Data_To_Host( (uint8_t*)(&BL_Version[0]) , 4 );

		//CAN_Rx(&RxID , &RxDLC , BL_Test_Buffer );
		//#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		//BL_Print_Message("Bootloader running now is Ver. %d.%d.%d \r\n", BL_Test_Buffer[1] , BL_Test_Buffer[2] , BL_Test_Buffer[3] );
		//#endif

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Bootloader Ver. %d.%d.%d \r\n", BL_Version[1] , BL_Version[2] , BL_Version[3] );
#endif
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Get_Chip_Indentification_Nmuber(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to store the MCU chip indentification number */
	uint16_t MCU_Identification_Number = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the MCU chip indentification number \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Get the MCU chip indentification number */
		/* DBGMCU is a struct ST company defined it & we will access the element
		 * has the chip id (CID) and take only the first 12 bits in the IDCODE reg. by using a mask */
		/* The device ID for STM32F42xxx is = 0x419 */
		MCU_Identification_Number = (uint16_t)(( DBGMCU -> IDCODE ) & DBGMCU_IDCODE_DEV_ID_MASK) ;

		/* Report the MCU chip indentification number to Host */
		Bootloader_Send_ACK(2);
		Bootloader_Send_Data_To_Host( (uint8_t*)(&MCU_Identification_Number) , 2 );
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Read_Protection_Level(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to store the Read Protection Level */
	uint8_t RDP_Level = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the FLASH Read Protection Out level \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		Bootloader_Send_ACK(1);

		/* Reading Protection Level */
		RDP_Level = CBL_STM32F429_Get_RDP_Level() ;

		/* Sending to the HOST the Read Protection Level */
		Bootloader_Send_Data_To_Host( (uint8_t*)(&RDP_Level) , 1 );
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Jump_To_Address(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to store the required memory address to jump into */
	uint32_t Host_Jump_Address = 0 ;

	/* Local var. to store if the jump address is valid or invalid */
	uint8_t Address_Verification = ADDRESS_IS_INVALID ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Jump bootloader to specified address \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* First ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		/* Extract the address from HOST packet */
		/* This address is stored from element #2 to element #5 */
		/* To access the 4 bytes together we made casting to the address + dereferencing */
		Host_Jump_Address = *((uint32_t*)&Host_Buffer[2]) ;

		/* Verfiy the extracted address to be valid address */
		Address_Verification = Host_Jump_Address_Verification(Host_Jump_Address);
		if( ADDRESS_IS_VALID == Address_Verification )
		{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Address verifiaction succeeded \r\n");
#endif

			/* Report address verifiaction succeeded */
			Bootloader_Send_Data_To_Host((uint8_t*)&Address_Verification , 1 );

			/* Prepare the address to jump */
			/* why +1 ? -> because of t-bit in ARM architecture (thumb instruction) */
			Jump_Ptr Jump_Address = (Jump_Ptr)( Host_Jump_Address + 1 ) ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Jumping to : 0x%X \r\n" , Jump_Address );
#endif
			Jump_Address();
		}

		else
		{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Address verifiaction failed \r\n");
#endif

			/* Report address verifiaction failed */
			Bootloader_Send_Data_To_Host((uint8_t*)&Address_Verification , 1 );
		}

	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}

}

static void Bootloader_Erase_Flash(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to indicate that the erase process is succeeded or failed */
	uint8_t Erase_Status = UNSUCCESSFUL_ERASE ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Mass erase or sector erase of the user flash \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Send ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		/* Third element is the sector number & fourth element is # of sectors */
		Erase_Status = Perform_Flash_Erase( Host_Buffer[2] , Host_Buffer[3] );

		if( SUCCESSFUL_ERASE == Erase_Status )
		{
			/* Report erase process is succeeded */
			Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Successful Erase \r\n");
#endif
		}

		else
		{
			/* Report erase process is failed */
			Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status , 1 );
		}
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Memory_Write(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to store the base memory address */
	uint32_t HOST_Address = 0 ;

	/* Local var. to store the payload length */
	uint8_t Payload_Len = 0 ;

	/* Local var. to store that the base memory address is valid or invalid */
	uint8_t Address_Verification = ADDRESS_IS_INVALID ;

	/* Local var. to store the status of writing process if it is passed or failed */
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Write data into different memories of the MCU \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Send ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		/* Extract the base memory address from sent packet */
		HOST_Address = *((uint32_t*)(&Host_Buffer[2])) ;

		/* Extract the payload length from sent packet */
		Payload_Len = Host_Buffer[6] ;

		/* Base memory address verifiaction */
		Address_Verification = Host_Jump_Address_Verification( HOST_Address ) ;
		if( ADDRESS_IS_VALID == Address_Verification )
		{
			Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t*)(&Host_Buffer[7]) , HOST_Address , Payload_Len ) ;

			/* Checking that the writing process is passed or failed */
			if( FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status )
			{
				/* Sending to the HOST that the writing process is passed */
				Bootloader_Send_Data_To_Host( (uint8_t*)&Flash_Payload_Write_Status , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
				BL_Print_Message("Payload Valid \r\n");
#endif
			}
			else
			{
				/* Sending to the HOST that the writing process is failed */
				Bootloader_Send_Data_To_Host( (uint8_t*)&Flash_Payload_Write_Status , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
				BL_Print_Message("Payload Invalid \r\n");
#endif
			}
		}
		else
		{
			/* Sending to the HOST that the address is invalid */
			Bootloader_Send_Data_To_Host( (uint8_t*)&Address_Verification , 1 );
		}

	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}

}

static void Bootloader_Enable_RW_Protection(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to indicate that the protection process is succeeded or failed */
	uint8_t Protection_Status = UNSUCCESSFUL_PROTECTION ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Enable read/write protection on different sectors of the user flash \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Send ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		Protection_Status = Configure_Flash_Sector_RW_Protection( Host_Buffer[2] , Host_Buffer[3] , 0 ) ;

		Bootloader_Send_Data_To_Host((uint8_t*)&Protection_Status , 1) ;

	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Disable_RW_Protection(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to indicate that the disabling protection process is succeeded or failed */
	uint8_t Disabling_Status = UNSUCCESSFUL_DISABLING ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Disable Active Protection On All The Sectors \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Send ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		Disabling_Status = Configure_Flash_Sector_RW_Protection( 0 , 0 , 1 ) ;

		Bootloader_Send_Data_To_Host((uint8_t*)&Disabling_Status , 1) ;

	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Change_Read_Protection_Level(uint8_t* Host_Buffer)
{
	/* Local var. to store the all length of the packet (sent by the host) */
	uint16_t Host_CMD_Packet_Len = 0 ;

	/* Local var. to store the CRC value to compare it with the calculated one by the BL (sent by the host) */
	uint32_t Host_CRC32 = 0 ;

	/* Local var. to store if the changing ROP level process is passed or failed */
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;

	/* Local var. to store the ROP level sent by HOST */
	uint8_t HOST_ROP_Level = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Change read protection level of the user flash \r\n") ;
#endif

	/* Extract the packet length sent by the host */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1 ;

	/* Extract the CRC32 sent by the host as it is stored in
	 * the last 4 bytes of the sent packet (casting + dereferencing) */
	Host_CRC32 = *((uint32_t*)(( Host_Buffer + Host_CMD_Packet_Len ) - CRC_TYPE_SIZE_BYTE )) ;

	/* CRC Verification / Calculation (calculated by the BL) */
	if( CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify( (uint8_t*)&Host_Buffer[0] , Host_CMD_Packet_Len - 4 , Host_CRC32 ) )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Passed \r\n");
#endif

		/* Send ACK to indicate the CRC is verified */
		Bootloader_Send_ACK(1);

		/* Checking that the ROP Level sent by the HOST is not level 2 because it's dangerous */
		HOST_ROP_Level = Host_Buffer[2] ;
		if( 2 == HOST_ROP_Level )
		{
			/* Do Nothing! */
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
		}
		else
		{
			if( 0 == HOST_ROP_Level )
			{
				HOST_ROP_Level = 0xAA ;

				/* Request changing the Read Out Protection level */
				ROP_Level_Status = Change_ROP_Level( (uint32_t)HOST_ROP_Level ) ;
			}
			else if( 1 == HOST_ROP_Level )
			{
				HOST_ROP_Level = 0x55 ;

				/* Request changing the Read Out Protection level */
				ROP_Level_Status = Change_ROP_Level( (uint32_t)HOST_ROP_Level ) ;
			}
		}

		Bootloader_Send_Data_To_Host( (uint8_t*)&ROP_Level_Status , 1 );
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("CRC Verification Failed \r\n");
#endif

		Bootloader_Send_NACK();
	}
}

static void Bootloader_Memory_Read(uint8_t* Host_Buffer)
{

}

static void Bootloader_Get_Sector_Protection_Status(uint8_t* Host_Buffer)
{

}

static void Bootloader_Read_OTP(uint8_t* Host_Buffer)
{

}

static void Bootloader_Get_Version_CAN(uint8_t* Host_Buffer)
{
	/* Array to store in it all information about the BL version to send it to the host */
	uint8_t BL_Version[4] = { CBL_VENDOR_ID , CBL_SW_MAJOR_VERSION , CBL_SW_MINOR_VERSION , CBL_SW_PATCH_VERSION } ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the bootloader version from the MCU \r\n") ;
#endif

	Bootloader_Send_Data_To_Host_CAN( 0x3FF , BL_Version , 4 );

	//CAN_Rx(&RxID , &RxDLC , BL_Test_Buffer);

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Bootloader running now is Ver. %d.%d.%d \r\n", BL_Version[1] , BL_Version[2] , BL_Version[3] );
#endif
}

static void Bootloader_Get_Chip_Indentification_Nmuber_CAN(uint8_t* Host_Buffer)
{
	/* Local var. to store the MCU chip indentification number */
	uint16_t MCU_Identification_Number = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the MCU chip indentification number \r\n") ;
#endif

	/* Get the MCU chip indentification number */
	/* DBGMCU is a struct ST company defined it & we will access the element
	 * has the chip id (CID) and take only the first 12 bits in the IDCODE reg. by using a mask */
	/* The device ID for STM32F42xxx is = 0x419 */
	MCU_Identification_Number = (uint16_t)(( DBGMCU -> IDCODE ) & DBGMCU_IDCODE_DEV_ID_MASK) ;

	/* Report the MCU chip indentification number to Host */
	//Bootloader_Send_ACK(2);
	//Bootloader_Send_Data_To_Host( (uint8_t*)(&MCU_Identification_Number) , 2 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("MCU chip indentification number = 0x%x \r\n",MCU_Identification_Number) ;
#endif
}

static void Bootloader_Read_Protection_Level_CAN(uint8_t* Host_Buffer)
{
	/* Local var. to store the Read Protection Level */
	uint8_t RDP_Level = 0 ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Read the FLASH Read Protection Out level \r\n") ;
#endif

	/* Reading Protection Level */
	RDP_Level = CBL_STM32F429_Get_RDP_Level() ;

	/* Sending to the HOST the Read Protection Level */
	//Bootloader_Send_Data_To_Host( (uint8_t*)(&RDP_Level) , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("FLASH Read Protection Out level = level %d \r\n",RDP_Level) ;
#endif
}

static void Bootloader_Jump_To_Address_CAN(uint8_t* Host_Buffer)
{
	/* Local var. to store the required memory address to jump into */
	uint32_t Host_Jump_Address = 0 ;

	/* Local var. to store if the jump address is valid or invalid */
	uint8_t Address_Verification = ADDRESS_IS_INVALID ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Jump bootloader to specified address \r\n") ;
#endif

	/* Extract the address from HOST packet */
	/* This address is stored from element #2 to element #5 */
	/* To access the 4 bytes together we made casting to the address + dereferencing */
	Host_Jump_Address = *((uint32_t*)&Host_Buffer[1]) ;

	/* Verfiy the extracted address to be valid address */
	Address_Verification = Host_Jump_Address_Verification(Host_Jump_Address);
	if( ADDRESS_IS_VALID == Address_Verification )
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Address verifiaction succeeded \r\n");
#endif

		/* Report address verifiaction succeeded */
		Bootloader_Send_Data_To_Host((uint8_t*)&Address_Verification , 1 );

		/* Prepare the address to jump */
		/* why +1 ? -> because of t-bit in ARM architecture (thumb instruction) */
		Jump_Ptr Jump_Address = (Jump_Ptr)( Host_Jump_Address + 1 ) ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Jumping to : 0x%X \r\n" , Host_Jump_Address );
#endif
		Jump_Address();
	}

	else
	{
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Address verifiaction failed \r\n");
#endif

		/* Report address verifiaction failed */
		Bootloader_Send_Data_To_Host((uint8_t*)&Address_Verification , 1 );
	}
}

static void Bootloader_Erase_Flash_CAN(uint8_t* Host_Buffer)
{
	/* Local var. to indicate that the erase process is succeeded or failed */
	uint8_t Erase_Status = UNSUCCESSFUL_ERASE ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Mass erase or sector erase of the user flash \r\n") ;
#endif

	/* Second element is the sector number & third element is # of sectors */
	Erase_Status = Perform_Flash_Erase( Host_Buffer[1] , Host_Buffer[2] );

	/* Array to store in it all information about the BL version to send it to the host */
	//	uint8_t BL_Erase[4] = {0} ;

	if( SUCCESSFUL_ERASE == Erase_Status )
	{
		/* Report erase process is succeeded */
		//Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Successful Erase \r\n");
#endif

		//		BL_Erase[0] = BL_OK;

	}

	else
	{
		/* Report erase process is failed */
		//Bootloader_Send_Data_To_Host((uint8_t*)&Erase_Status , 1 );

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Failed Erase \r\n");
#endif

		//		BL_Erase[0] = BL_NACK;
	}


	//	Bootloader_Send_Data_To_Host_CAN( 0x3FF , BL_Erase , 1 );
}

#define CREATE(word, x, type)	number_of_##word##_##x##_##type
#define length_of_bytes_to_write 256

static void Bootloader_Memory_Write_CAN(uint8_t* Host_Buffer)
{
	/* Local var. to store the base memory address */
	uint32_t payload_length = 0;

	/* Local var. to store the base memory address */
	uint32_t HOST_Address = 0 ;

	/* Local var. to store that the base memory address is valid or invalid */
	uint8_t Address_Verification = ADDRESS_IS_INVALID ;

	/* Local var. to store the status of writing process if it is passed or failed */
	//	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
	BL_Print_Message("Write data into different memories of the MCU \r\n") ;
#endif


	/* Extract the base memory address from sent packet */
	HOST_Address = *((uint32_t*)(&Host_Buffer[1])) ;

	/* Extract the payload length */
	payload_length = *((uint32_t*)(&Host_Buffer[5])) ;

	/* Base memory address verifiaction */
	Address_Verification = Host_Jump_Address_Verification( HOST_Address ) ;

	if( ADDRESS_IS_VALID == Address_Verification )
	{
		/* Base memory address verifiaction */
		Address_Verification = Host_Jump_Address_Verification( HOST_Address + payload_length ) ;

		if( ADDRESS_IS_VALID == Address_Verification )
		{

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Address is valid. \r\n") ;
#endif

			uint32_t ID;
			uint8_t  DLC;
			uint8_t  Payload[length_of_bytes_to_write] = {0};

			uint32_t CREATE(completed, length_of_bytes_to_write, frame)	= payload_length / length_of_bytes_to_write;
			uint32_t CREATE(remaining, length_of_bytes_to_write, bytes) = payload_length % length_of_bytes_to_write;
			uint32_t CREATE(completed, 8, frame) 						= CREATE(remaining, length_of_bytes_to_write, bytes) / 8;
			uint32_t CREATE(remaining, 8, frame) 						= CREATE(remaining, length_of_bytes_to_write, bytes) % 8;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("c256= %d, r256 = %d, c8 = %d, r8 = %d. \r\n",
					CREATE(completed, length_of_bytes_to_write, frame),
					CREATE(remaining, length_of_bytes_to_write, bytes),
					CREATE(completed, 8, frame),
					CREATE(remaining, 8, frame)) ;
#endif

			uint32_t i, j;
			for(i = 0; i < CREATE(completed, length_of_bytes_to_write, frame); i++)
			{
				for(j = 0; j < length_of_bytes_to_write / 8; j++)
				{
					CAN_Rx(&ID, &DLC, &Payload[j*8]);
				}

				Flash_Memory_Write_Payload((uint8_t*)(&Payload[0]) , HOST_Address , length_of_bytes_to_write ) ;
				HOST_Address += length_of_bytes_to_write;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
				BL_Print_Message("payload number %d done. \r\n", i+1) ;
#endif
			}

			for(i = 0; i < CREATE(completed, 8, frame); i++)
			{
				CAN_Rx(&ID, &DLC, &Payload[i*8]);
			}

			if(CREATE(remaining, 8, frame) != 0)
			{
				CAN_Rx(&ID, &DLC, &Payload[i*8]);
			}

			Flash_Memory_Write_Payload((uint8_t*)(&Payload[0]) , HOST_Address , CREATE(completed, 8, frame) * 8 + CREATE(remaining, 8, frame)) ;

#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("finally. \r\n") ;
#endif

			//			/* Array to store in it all information about the BL version to send it to the host */
			//			uint8_t BL_Write[4] = {0} ;
			//
			//			BL_Write[0] = BL_OK;
			//
			//			Bootloader_Send_Data_To_Host_CAN( 0x3FF , BL_Write , 1 );

		}
		else
		{
			/* Sending to the HOST that the address is invalid */
			//Bootloader_Send_Data_To_Host( (uint8_t*)&Address_Verification , 1 );
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
			BL_Print_Message("Invalid Address. \r\n") ;
#endif
		}


	}
	else
	{
		/* Sending to the HOST that the address is invalid */
		//Bootloader_Send_Data_To_Host( (uint8_t*)&Address_Verification , 1 );
#if( BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE )
		BL_Print_Message("Invalid Address. \r\n") ;
#endif
	}

}
