/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "dht11.h"
#include "oled.h"
#include "light_sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define DEVICE_ID "zzzgy"   // 定义设备ID，用来唯一的标识当前设备
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile char cmd;  // 用于存放移动端发送过来的控制命令
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern char ying[][16];
extern unsigned char Picture1[];
extern unsigned char Picture2[];
extern unsigned char Picture3[];
extern unsigned char Picture4[];
extern char Temp[][16];
extern char Hum[][16];
extern char T[][16];

// 重写 fputc 函数，将它重定向到 UART1
int fputc(int ch, FILE* fp)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 1000);
	return ch;
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
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	
// 用中断的方式接收一个字节的数据
	HAL_UART_Receive_IT(&huart2, (uint8_t*)&cmd, sizeof(cmd));

	HAL_Delay(1000);
	
	OLED_Init();
	OLED_Clear();  // 清屏
	
	
	OLED_ShowPicture(Picture4);
	HAL_Delay(1000);
	OLED_Clear();  // 清屏
	
	OLED_ShowChinese(40, 6, ying, 3);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	char text[] = "                Welcome to our company! Hello,China!                ";
	char line[17] = {0};
	uint16_t i = 0, cnt = 0;
	char th[17];
	
	char s[100];
	float temp;
	uint8_t humi;
	uint32_t light,smoke;
  while (1)
  {
		strncpy(line, text + i, 16);
		OLED_ShowString(0, 0, (uint8_t*)line, 16);
		i++;
		
		if(strlen(text + i) < 16) i = 0;
		
		if(cnt % 120 == 0)
		{
			cnt = 0;
			
			if(DHT11_Get(&temp, &humi))
			{
				// 采集失败
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
				HAL_Delay(50);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
			}
			else
			{
				// 采集成功
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
				HAL_Delay(50);
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
				
				if(temp > 35 || temp < 0)
				{
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
				}
			}
			
			OLED_ShowChinese(0, 2, Temp, 3);
			
			sprintf(th, "%.1f", temp);
			OLED_ShowString(45, 2, (uint8_t*)th, 16);
			
			OLED_ShowChinese(85, 2, T, 1);
			
			OLED_ShowChinese(0, 4, Hum, 3);
			
			sprintf(th, "%u %%RH", humi);
			OLED_ShowString(45, 4, (uint8_t*)th, 16);	
			
			sprintf(s, "%s/sensor/dht11 %.1f_%u\n", DEVICE_ID, temp, humi);
		
			// 将采集到的温湿度数据上传到云端平台
			HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 1000);
			
			// 上传光照传感器数据(ADC)
			light = Light_Sensor_Get();
			sprintf(s, "%s/state/light %u\n", DEVICE_ID, light);
			HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 1000);
			
			
			// 上传烟雾传感器数据(ADC)
			smoke = Smoke_Sensor_Get();
			sprintf(s, "%s/sensor/smoke %u\n", DEVICE_ID, smoke);
			HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 10);
		}			
		
		HAL_Delay(10);
		cnt += 10;
		
		  // 上传有害气体传感器数据
//		sprintf(s, "%s/sensor/qiti %d\n", DEVICE_ID, !(int)HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5));
//		HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 1000);
//		
  						
		
		  // 上传 LED 的状态
//		sprintf(s, "%s/state/led %d\n", DEVICE_ID, (int)HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
//		HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 1000);		

		
   		// 上传蜂鸣器的状态
//		sprintf(s, "%s/state/beep %d\n", DEVICE_ID, (int)HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7));
//		HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 1000);		

		
//		HAL_Delay(1200);
		
		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
