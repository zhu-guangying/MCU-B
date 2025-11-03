#include "main.h"
#include "usart.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)
	{
		if(cmd == 'a')
		{
			HAL_GPIO_WritePin(GPIOA , GPIO_PIN_1, GPIO_PIN_RESET);
		}
		else if(cmd == 'b')
		{
			HAL_GPIO_WritePin(GPIOA , GPIO_PIN_1, GPIO_PIN_SET);
		}
		else if(cmd == 'c')
		{
			HAL_GPIO_WritePin(GPIOB , GPIO_PIN_10, GPIO_PIN_SET);
		}
		else if(cmd == 'd')
		{
			HAL_GPIO_WritePin(GPIOB , GPIO_PIN_10, GPIO_PIN_RESET);
		}
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&cmd, sizeof(cmd));
	}
}