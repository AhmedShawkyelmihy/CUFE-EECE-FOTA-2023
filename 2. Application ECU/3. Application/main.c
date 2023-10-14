/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "can_test.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* void pointer to function */
typedef void (*pMainApp)(void);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BOOTLOADER_BASE_ADDRESS 			0x08000000U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

void BL_Print_Message(char* format , ...);
void Jump_To_Bootloader(void);
void Fan_Task(void* pv);
void Servo_Task(void* pv);
void Bootloader_Task(void* pv);

SemaphoreHandle_t Sempahore ;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef pHeader ;
	uint8_t Buffer[8] = {0} ;

	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &pHeader, Buffer);
	if(Buffer[0] == 0x23)
	{
		BaseType_t pxHigherPriorityTaskWoken = pdFALSE ;
		xSemaphoreGiveFromISR(Sempahore,&pxHigherPriorityTaskWoken);
		
		if(pxHigherPriorityTaskWoken)
		{
			/* Toggling LED for debugging */
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
	
	/* Initialize bxCAN Filter */
   CAN_Rx_Filter_Init(0x3FF);

   if( HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK )
        {
      	  Error_Handler();
        }

   /* Start bxCAN { Running State } */
   if( HAL_CAN_Start(&hcan1) != HAL_OK )
     {
   	  Error_Handler();
     }
	
	xTaskCreate(Fan_Task,"Fan_Task",1000,NULL,1,NULL);
	xTaskCreate(Servo_Task,"Servo_Task",1000,NULL,1,NULL);
	xTaskCreate(Bootloader_Task,"Bootloader_Task",1000,NULL,2,NULL);
		 
	/* Creating the binary semaphore and storing the semaphore handle in this variable */
  Sempahore = xSemaphoreCreateBinary();
	
	BL_Print_Message("Starting Application\n\r");
	
	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
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
	HAL_UART_Transmit( &huart1 , (uint8_t *)Message , sizeof(Message) , HAL_MAX_DELAY ) ;

#elif ( BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MESSAGE )
	/* Transmit the formatted data through the defined SPI */


#elif ( BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE )
	/* Transmit the formatted data through the defined CAN */

#endif

	/* Performs cleanup for an ap object initialized by a call to va_start */
	va_end( args ) ;
}

void Jump_To_Bootloader(void)
{
	/* Value of the main stack pointer of our bootloader */
	/* MSP Value is stored in the first 4 bytes of the bootloader sector */
	uint32_t MSP_VALUE = *((volatile uint32_t*)BOOTLOADER_BASE_ADDRESS) ;

	/* Reset Handler definition function of our bootloader */
	uint32_t MainAppAddr = *((volatile uint32_t*)(BOOTLOADER_BASE_ADDRESS + 4)) ;

	/* We need pointer to function to save the reset handler address in it for jumping to the
	 * bootloader by calling it + we make casting to overcome the error */
	pMainApp ResetHandler_Address = (pMainApp)MainAppAddr;

	/* Assigns the given value (MSP Value of the bootloader) to the main stack pointer of the application
	 * to achieve jumping */
	__set_MSP(MSP_VALUE);

	/* Deinitialization of modules to initialize it as the bootloader program require */
	HAL_RCC_DeInit();   /* Resets the RCC clock configuration to the default reset state. */
	/* we can call any other deinit function for (GPIO , UART , ....) to reset it if required */
	HAL_CAN_DeInit(&hcan1);
	HAL_TIM_Base_DeInit(&htim1);
	HAL_TIM_Base_DeInit(&htim2);
	HAL_ADC_Stop(&hadc3);

	/* Jumping to bootloader's reset handler */
	ResetHandler_Address();
}	

void Fan_Task(void* pv)
{
	/* Initializations */
	uint16_t PWM = 0 ;
	uint16_t readValue = 0;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	
	while(1)
	{
		HAL_ADC_Start(&hadc3);
		HAL_ADC_PollForConversion(&hadc3,1000);
		readValue = HAL_ADC_GetValue(&hadc3);
		BL_Print_Message("The Value read from adc is %d. \n\r", readValue);
		PWM = 250 + readValue/4.1;
		
		if((readValue >= 0 ) && (readValue < 500))
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 0);
		}
		else if((readValue >= 500) && (readValue < 2000))
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 4096);
		}
		else if(readValue >= 2000)
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 65355);
		}
		
		//__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, PWM);
	}
}

void Servo_Task(void* pv)
{
	/* Initializations */
	uint8_t flag = 0;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 395);
	while(1)
	{
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
		{
			HAL_Delay(100);
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
			{
				if(flag == 0)
				{
					flag = 1;
					__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 909);
				}
				else
				{
					flag = 0;
					__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 395);
				}
			}
		}
	}
}

void Bootloader_Task(void* pv)
{
	uint8_t Check ;
	while(1)
	{
		Check = xSemaphoreTake(Sempahore,4000);
		if(Check)
		{
			Jump_To_Bootloader();
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
