#include "main.h"
#include "adc.h"
#include "light_sensor.h"


uint32_t Light_Sensor_Get(void)
{
	// 均值滤波
		uint32_t data = 0, i;
	
		for(i = 0; i < 50; i++)
		{
			HAL_ADC_Start(&hadc1);  // 启动 ADC，开始转换
			
			HAL_ADC_PollForConversion(&hadc1, 100);  // 等待转换完成
			
			data += HAL_ADC_GetValue(&hadc1);  // 读取转换成功的数值
		}
		
		data /= 50;
		
		return data;
}
uint32_t Smoke_Sensor_Get(void)
{
	// 均值滤波
		uint32_t data = 0, i;
	
		for(i = 0; i < 50; i++)
		{
			HAL_ADC_Start(&hadc1);  // 启动 ADC，开始转换
			
			HAL_ADC_PollForConversion(&hadc1, 100);  // 等待转换完成
			
			data += HAL_ADC_GetValue(&hadc1);  // 读取转换成功的数值
		}
		
		data /= 100;
		
		return data;
}