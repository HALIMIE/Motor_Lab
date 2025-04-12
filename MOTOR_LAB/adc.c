#include "device_driver.h"

void Adc_Cds_Init(void)
{
	Macro_Set_Bit(RCC->APB2ENR, 3); 				// PB POWER ON
	Macro_Write_Block(GPIOB->CRL, 0xf, 0x0, 4); 	// PB1(ADC-IN9) = Analog Input

	Macro_Set_Bit(RCC->APB2ENR, 9); 				// ADC1 POWER ON
	Macro_Write_Block(RCC->CFGR, 0x3, 0x2, 14); 	// ADC1 CLOCK = 12MHz(PCLK2/6)

	Macro_Write_Block(ADC1->SMPR2, 0x7, 0x7, 27); 	// Clock Configuration of CH5 = 239.5 Cycles
	Macro_Write_Block(ADC1->SQR1, 0xF, 0x0, 20); 	// Conversion Sequence No = 1
	Macro_Write_Block(ADC1->SQR3, 0x1F, 9, 0); 		// Sequence Channel of No 1 = CH9
	Macro_Write_Block(ADC1->CR2, 0x7, 0x7, 17); 	// EXT Trigger = SW Trigger
	Macro_Set_Bit(ADC1->CR2, 0); 					// ADC ON
}

void Adc_Start(void)
{
	Macro_Set_Bit(ADC1->CR2, 20); 					// EXT Trigger Start
	Macro_Set_Bit(ADC1->CR2, 22); 					// ADC Start
}

void Adc_Stop(void)
{
	Macro_Clear_Bit(ADC1->CR2, 22); 				// ADC Stop
	Macro_Clear_Bit(ADC1->CR2, 0); 					// ADC OFF
}

int Adc_Get_Status(void)
{
	int timeout = 1000000;

	while (!Macro_Check_Bit_Set(ADC1->SR, 1))
	{
		if (--timeout == 0)
		{
			Uart1_Printf("ADC conversion timeout!\n");
			return 0;
		}
	}

	Macro_Clear_Bit(ADC1->SR, 1);
	Macro_Clear_Bit(ADC1->SR, 4);

	return 1;
}

int Adc_Get_Data(void)
{
	return Macro_Extract_Area(ADC1->DR, 0xFFF, 0);
}


