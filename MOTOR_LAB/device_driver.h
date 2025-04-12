#include "stm32f10x.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

// Uart.c

#define Uart_Init			Uart1_Init
#define Uart_Send_Byte		Uart1_Send_Byte
#define Uart_Send_String	Uart1_Send_String
#define Uart_Printf			Uart1_Printf

extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt,...);
extern char Uart1_Get_Char(void);
extern char Uart1_Get_Pressed(void);
extern void Uart1_RX_Interrupt_Enable(int en);

// Led.c

extern void LED_Init(void);
extern void LED_Display(unsigned int num);
extern void LED_All_On(void);
extern void LED_All_Off(void);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Poll_Init(void);
extern int Key_Get_Pressed(void);
extern void Key_Wait_Key_Released(void);
extern int Key_Wait_Key_Pressed(void);
extern void Key_ISR_Enable(int en);

// SysTick.c

extern void SysTick_Run(unsigned int msec);
extern int SysTick_Check_Timeout(void);
extern unsigned int SysTick_Get_Time(void);
extern unsigned int SysTick_Get_Load_Time(void);
extern void SysTick_Stop(void);
extern void SysTick_OS_Tick(unsigned int msec);

// Timer.c

extern void TIM4_Delay(int time);
extern void TIM2_PWM_Init(void);
extern void PWM_Set_Duty_Cycle(int pin, int power);

// adc.c

extern void Adc_Cds_Init(void);
extern void Adc_Start(void);
extern void Adc_Stop(void);
extern int Adc_Get_Status(void);
extern int Adc_Get_Data(void);