#include "device_driver.h"

#define TIM2_TICK (20)					// usec
#define TIM2_FREQ (1000000 / TIM2_TICK) // Hz
#define TIME2_PLS_OF_1ms (1000 / TIM2_TICK)
#define TIM2_MAX (0xffffu)

#define TIM4_TICK (20)					// usec
#define TIM4_FREQ (1000000 / TIM4_TICK) // Hz
#define TIME4_PLS_OF_1ms (1000 / TIM4_TICK)
#define TIM4_MAX (0xffffu)

void TIM4_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);

	TIM4->CR1 = (1 << 4) | (1 << 3);
	TIM4->PSC = (unsigned int)(TIMXCLK / (double)TIM4_FREQ + 0.5) - 1;
	TIM4->ARR = TIME4_PLS_OF_1ms * time;

	Macro_Set_Bit(TIM4->EGR, 0);
	Macro_Clear_Bit(TIM4->SR, 0);
	Macro_Set_Bit(TIM4->DIER, 0);
	Macro_Set_Bit(TIM4->CR1, 0);

	while (Macro_Check_Bit_Clear(TIM4->SR, 0))
		;

	Macro_Clear_Bit(TIM4->CR1, 0);
	Macro_Clear_Bit(TIM4->DIER, 0);
}

void TIM2_PWM_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->PSC = (unsigned int)(TIMXCLK / (double)(1000000) + 0.5) - 1;
	TIM2->ARR = 1000 - 1;

	TIM2->CCMR2 = 0;
	TIM2->CCER = 0;

	// Channel 3 (CH3)
	TIM2->CCMR2 |= (6 << 12); // PWM mode 1 on CH3 (OC3M = 110)
	TIM2->CCMR2 |= (1 << 11); // OC3PE = 1 (Preload enable)
	TIM2->CCER |= (1 << 8);	  // CC3E = 1 (Enable CH3 output)

	// Channel 4 (CH4)
	TIM2->CCMR2 |= (6 << 4); // PWM mode 1 on CH4 (OC4M = 110)
	TIM2->CCMR2 |= (1 << 3); // OC4PE = 1 (Preload enable)
	TIM2->CCER |= (1 << 12); // CC4E = 1 (Enable CH4 output)

	TIM2->EGR = TIM_EGR_UG;	   // Update event
	TIM2->CR1 |= TIM_CR1_ARPE; // Auto-reload preload enable
	TIM2->CR1 |= TIM_CR1_CEN;  // Enable Timer
}

// void PWM_Set_Duty_Cycle(int pin, int power)
// {
// 	int duty = power * 10 + (power != 0) * 10 + (power == 1) * 5;

// 	if (duty < 0)
// 		duty = 0;
// 	if (duty > 100)
// 		duty = 100;
// 	Uart1_Printf("Duty: %d for pin %d\n", duty, pin);
// 	uint16_t compare = (duty * (TIM2->ARR + 1)) / 100;

// 	if (pin == 2) // PA2
// 	{
// 		TIM2->CCR3 = compare; // Set compare value for pin 3
// 	}
// 	else if (pin == 3) // PA3
// 	{
// 		TIM2->CCR4 = compare; // Set compare value for pin 4
// 	}
// 	TIM2->EGR = TIM_EGR_UG; // Update event
// }

void PWM_Set_Duty_Cycle(int pin2Power, int pin3Power)
{
	int duty2 = pin2Power * 10 + (pin2Power != 0) * 10 + (pin2Power == 1) * 5;
	int duty3 = pin3Power * 10 + (pin3Power != 0) * 10 + (pin3Power == 1) * 5;

	if (duty2 < 0)
		duty2 = 0;
	if (duty2 > 100)
		duty2 = 100;
	if (duty3 < 0)
		duty3 = 0;
	if (duty3 > 100)
		duty3 = 100;
	Uart1_Printf("Duty: %d for pin 2, %d for pin 3\n", duty2, duty3);
	uint16_t compare2 = (duty2 * (TIM2->ARR + 1)) / 100;
	uint16_t compare3 = (duty3 * (TIM2->ARR + 1)) / 100;

	TIM2->CCR3 = compare2; // Set compare value for pin 3
	TIM2->CCR4 = compare3; // Set compare value for pin 4
	TIM2->EGR = TIM_EGR_UG; // Update event
}