#include "main.h"
#include "delay.h"
#include "dht11.h"


// 定义 DHT11 连接的 GPIO 端口和引脚，方便移植
#define DHT11_PORT  GPIOA
#define DHT11_PIN   GPIO_PIN_4


/*
** @Brief 通过 DHT11 传感器采集一次温度和湿度数据
** 
** @Params
** 		temp 用于接收温度数据，如果对温度数据不感兴趣，可以传 NULL
**    humi 用于接收湿度数据，如果对湿度数据不感兴趣，可以传 NULL
** 
** @Retval
**		采集成功返回 0，否则返回非零（错误码），具体如下：
**    1 表示 DHT11 未响应（DHT11 故障或连接异常），2 表示校验失败（可能存在干扰）
** 
** @Example
** 		float t, h;
** 
**    if(DHT11_Get(&t, &h) == 0)
**    {
** 			// 处理温湿度数据
**			// ......
**    }
*/
int8_t DHT11_Get(float* temp, uint8_t* humi)
{
	uint8_t i, j, data[5];
	GPIO_InitTypeDef conf;
	
	// 将 DHT11 引脚设置为开漏输出模式
	conf.Pin = DHT11_PIN;
	conf.Mode = GPIO_MODE_OUTPUT_OD;
	conf.Pull = GPIO_NOPULL;
	conf.Speed = GPIO_SPEED_FREQ_LOW;
	
	HAL_GPIO_Init(DHT11_PORT, &conf);
	
	// 向 DHT11 发出开始信号
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);
	
	// 将 DHT11 引脚切换到输入模式，释放总线
	conf.Mode = GPIO_MODE_INPUT;
	HAL_GPIO_Init(DHT11_PORT, &conf);
	
	// 延时 60us
	Delay_US(60);
	
	// 读取 DHT11 的响应信号
	// 如果 DHT11 未响应，就返回 1，说明 DHT11 传感器故障或连接异常
	if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
		return 1;
	
	// 等待 DHT11 的低电平响应信号结束
	while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET);
	
	// 延时 120us
	Delay_US(120);
	
	// 逐位接收温湿度数据（共 40 位，高位先出）
	
	for(i = 0; i < 5; i++)
	{
		data[i] = 0;
		
		for(j = 0; j < 8; j++)
		{
			// 等待 0 或 1 信号的前导低电平结束
			while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET);
			
			// 延时 50us
			Delay_US(50);
			
			data[i] <<= 1;
			
			if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
			{
				// 接收到 1
				data[i] += 1;
				
				// 延时 50us
				Delay_US(40);
			}
		}
	}
	
	// 检查校验和，如果校验失败，就返回 2，说明通信存在干扰 
	if(data[0] + data[1] + data[2] + data[3] != data[4])
		return 2;
	
	// 计算温度和湿度
	if(humi != NULL) *humi = data[0];
	
	if(temp != NULL)
	{
		*temp = data[2] + (float)(data[3] & 0x7F) / 10;
		
		if(data[3] & 0x80) *temp = -*temp;  // 判断符号
	}
	
	return 0;
}

