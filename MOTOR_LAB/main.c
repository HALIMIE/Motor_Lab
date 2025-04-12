#include "device_driver.h"

/*---------- Enum ----------*/

// Direction of the motor
enum operationDirection
{
	STOP = 0,
	FORWARD = 1,
	REVERSE = 2
};

// Mode of the motor
enum operationMode
{
	MANUAL = 0,
	LIGHT_CONTROL = 1,
	TIME_CONTROL = 2
};

/*---------- Global Variables ----------*/

// External variables
extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;

// Internal variables
volatile int power = 9;
volatile int curOperation = STOP;
volatile int curMode = MANUAL;
const int lightInterval = 0x180;
volatile unsigned int cdsValue = 0;

/*---------- GPIO Related Function ----------*/
static void GPIOA_Init(void)
{
	// Enable GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// Configure PA2 and PA3 as push-pull alternate function output
	GPIOA->CRL &= ~((0xF << 8) | (0xF << 12));
	GPIOA->CRL |= (0b1011 << 8) | (0b1011 << 12);
}

/*---------- System Related Function ----------*/
static void Sys_Init(void)
{
	Clock_Init();
	LED_Init();
	Uart_Init(115200);
	Key_Poll_Init();
	GPIOA_Init();
	Adc_Cds_Init();
	TIM2_PWM_Init();

	SCB->VTOR = 0x08003000;
	SCB->SHCSR = 0;
}

/*---------- Moter Control Function ----------*/
static inline void stop()
{
	PWM_Set_Duty_Cycle(0, 0);
	curOperation = STOP;
}

static inline void forward()
{
	PWM_Set_Duty_Cycle(power, 0);
	curOperation = FORWARD;
}

static inline void reverse()
{
	PWM_Set_Duty_Cycle(0, power);
	curOperation = REVERSE;
}

static void updatePower()
{
	// No operation in STOP mode
	if (curOperation == STOP)
	{
		Uart1_Printf("No power related operation allowed in STOP\n");
		return;
	}

	// Time control related variable
	// True for increasing power, False for decreasing power
	static int high = 1;

	switch (curMode)
	{
	case MANUAL:
		// Uart power control
		// Key_Value should be char 0 ~ 9
		if (Uart1_Rx_In)
		{
			power = Uart1_Rx_Data - '0';
		}
		break;
	case LIGHT_CONTROL:
		power = cdsValue / lightInterval;
		break;
	case TIME_CONTROL:
		// Time control mode
		// power will be increased from 0 to 9 and decreased from 9 to 0
		high = (power == 9) ? 0 : (power == 0) ? 1 : high;
		power += high ? 1 : -1;
		break;
	}

	// Power should be 0 ~ 9
	power = (power < 0) ? 0 : (power > 9) ? 9 : power;

	// update PWM duty cycle
	switch (curOperation)
	{
	case STOP:
		stop();
		break;
	case FORWARD:
		forward();
		break;
	case REVERSE:
		reverse();
		break;
	}
}

static void rotationControl()
{
	switch (Uart1_Rx_Data)
	{
	case 'F':
		switch (curOperation)
		{
		case STOP:
			Uart1_Printf("STOP -> FORWARD\n");
			forward();
			break;
		case FORWARD:
			Uart1_Printf("Already FORWARD\n");
			break;
		case REVERSE:
			Uart1_Printf("REVERSE -> STOP -> FORWARD\n");
			stop();
			TIM4_Delay(500);
			forward();
			break;
		}
		break;
	case 'R':
		switch (curOperation)
		{
		case STOP:
			Uart1_Printf("STOP -> REVERSE\n");
			reverse();
			break;
		case FORWARD:
			Uart1_Printf("FORWARD -> STOP -> REVERSE\n");
			stop();
			TIM4_Delay(500);
			reverse();
			break;
		case REVERSE:
			Uart1_Printf("Already REVERSE\n");
			break;
		}
		break;
	case 'S':
		switch (curOperation)
		{
		case STOP:
			Uart1_Printf("Already STOP\n");
			break;
		case FORWARD:
			Uart1_Printf("FORWARD -> STOP \n");
			stop();
			curOperation = STOP;
			break;
		case REVERSE:
			Uart1_Printf("REVERSE -> STOP\n");
			stop();
			curOperation = STOP;
			break;
		}
		break;
	}
	return;
}

// KEY 0 OR VALUE 1: LIGHT MODE (MANUAL <-> LIGHT_CONTROL)
// KEY 1 OR VALUE 2: TIME MODE (MANUAL <-> TIME_CONTROL)
static void modeControl()
{
	switch (Key_Value)
	{
	case 1:
		curMode = (curMode == MANUAL) ? LIGHT_CONTROL : MANUAL;
		Uart1_Printf("Mode: %s\n", (curMode == MANUAL) ? "MANUAL" : "LIGHT_CONTROL");
		break;
	case 2:
		curMode = (curMode == MANUAL) ? TIME_CONTROL : MANUAL;
		Uart1_Printf("Mode: %s\n", (curMode == MANUAL) ? "MANUAL" : "TIME_CONTROL");
		break;
	}
}

// Light control mode
// 0x000 to 0%, 0xFFF to 100%
static void lightModeControl()
{
	Uart1_Printf("Light Control Mode\n");
	if (curOperation == STOP)
	{
		Uart1_Printf("Light Control Mode requires NON-STOP operation, changed into FORWARD operation\n");
		curOperation = FORWARD;
	}
	Adc_Start();
	while (!Adc_Get_Status())
		;
	cdsValue = Adc_Get_Data();
	updatePower();
	TIM4_Delay(1000);
}

// Time control mode
// 1s interval
static void timeModeControl()
{
	Uart1_Printf("Time Control Mode\n");
	if (curOperation == STOP)
	{
		Uart1_Printf("Time Control Mode requires NON-STOP operation, changed into FORWARD operation\n");
		curOperation = FORWARD;
	}
	updatePower();
	TIM4_Delay(1000);
}

/*---------- Main Function ----------*/
void Main(void)
{
	// System initialization
	Sys_Init();

	// YOLO
	Uart1_Printf("YOLO\n");

	// Enable interrupt
	Key_ISR_Enable(1);
	Uart1_RX_Interrupt_Enable(1);

	while (1)
	{
		if (Uart1_Rx_In && curMode == MANUAL)
		{
			char c = Uart1_Rx_Data;
			// Uart power control
			if (c >= '0' && c <= '9')
			{
				updatePower();
			}
			// Uart rotation control
			else if (c == 'F' || c == 'R' || c == 'S')
			{
				rotationControl();
			}
			// Uart flag clear
			Uart1_Rx_In = 0;
		}
		else if (Uart1_Rx_In && curMode != MANUAL)
		{
			char c = Uart1_Rx_Data;
			if (c >= '0' && c <= '9')
			{
				Uart1_Printf("Manual power control is only allowed in MANUAL mode\n");
				
			}
			else if (c == 'S')
			{
				Uart1_Printf("STOP is only allowed in MANUAL mode\n");
			}
			else if (c == 'F' || c == 'R')
			{
				rotationControl();
			}
			Uart1_Rx_In = 0;
		}
		if (Key_Value)
		{
			// Key mode control
			modeControl();
			// Key flag clear
			Key_Value = 0;
		}
		// Operation contol by mode
		switch (curMode)
		{
		case MANUAL:
			break;
		case LIGHT_CONTROL:
			lightModeControl();
			break;
		case TIME_CONTROL:
			timeModeControl();
			break;
		}
	}
}