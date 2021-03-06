#include "GC_controller_emulation.h"

// Macros //
#define NUM_OF_BUTTON_INPUTS	22

// Enumerations //
/* GC Bits to UART Bytes */
typedef enum
{
	GC_BITS_00_CASE1 = 0x08,
	GC_BITS_00_CASE2 = 0x88,
	GC_BITS_01_CASE1 = 0xE8,
	GC_BITS_01_CASE2 = 0xC8,
	GC_BITS_10_CASE1 = 0x0F,
	GC_BITS_10_CASE2 = 0x8F,
	GC_BITS_11_CASE1 = 0xEF,
	GC_BITS_11_CASE2 = 0xCF,
	GC_BITS_STOP_BIT = 0xFF
} GCBitsUartByte_t;

/* GC Commands */
typedef enum
{
	GC_COMMAND_PROBE = 0,
	GC_COMMAND_PROBE_ORIGIN = 1,
	GC_COMMAND_POLL_AND_TURN_RUMBLE_OFF = 2,
	GC_COMMAND_POLL_AND_TURN_RUMBLE_ON = 3,
	GC_COMMAND_UNKNOWN = 4
} GCCommand_t;

// Variables //
/* UART for faking 1-wire protocol */
static UART_HandleTypeDef huart1;

/* Snapshot of button states */
static ButtonState_t gcButtonInputSnapShot[NUM_OF_BUTTON_INPUTS] = {};

/* Processed snapshot button states */
static ButtonState_t gcProcessedButtonStates[NUM_OF_BUTTON_INPUTS] = {};

/* Data from console before being converted into a command */
static uint8_t gcConsoleResponse[MAX_GC_CONSOLE_BYTES];

/* Command from console after converted */
static GCCommand_t command;

// Function Prototypes //
/* Gets a button state */
ButtonState_t GCControllerEmulation_GetButtonState(GCButtonInput_t);

/* Waits for a command from the console. Not that this uses the UART
 * module so if catching a falling edge in the middle of the
 * transmission, its garbage. But any functions that uses
 * the data from the console will know how to ignore it. So
 * calling it again next loop around self-corrects the issue.
 * That means getting in sync with the console is not necessary.
 */
inline static uint8_t GCControllerEmulation_GetConsoleCommand(void);

/* Sends a stop bit to indicate end of GC data transmission */
inline static void GCControllerEmulation_SendStopBit(void);

/* Sends correct response to poll command */
inline static void GCControllerEmulation_SendProbeResponse(void);

/* Sends current states of buttons and joystick to console */
inline static void GCControllerEmulation_SendControllerState(GCCommand_t);

/* Processes raw inputs to proper signals (example: socd cleaning) */
inline static void GCControllerEmulation_ProcessSwitchSnapshot();

// Function Implementations //
/* Initializes this module to properly emulate a GC controller */
void GCControllerEmulation_Init()
{
	/* Setup GC communication */
	// Clocks
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();

	// Init structure
	GPIO_InitTypeDef GPIO_InitStruct_GCControllerEmulation = {0};

	// Stop bit control
	GPIO_InitStruct_GCControllerEmulation.Pin = GC_STOP_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_NOPULL;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GC_STOP_PORT, &GPIO_InitStruct_GCControllerEmulation);
	GC_STOP_PORT->BSRR = GC_STOP_SET;

	// USART1 TX/RX
	GPIO_InitStruct_GCControllerEmulation.Pin = GC_TX_PIN_HAL | GC_RX_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct_GCControllerEmulation.Alternate = GPIO_AF7_USART1;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_NOPULL;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GC_TX_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// Configure USART1
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 1100000; // 1100000 works
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_8;
	HAL_UART_Init(&huart1);

	// Default command state from console
	command = GC_COMMAND_UNKNOWN;

	/* Setup buttons */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// A Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_A_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_A_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// B Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_B_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_B_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// X Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_X_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_X_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// Y Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_Y_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_Y_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// L Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_L_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_L_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// R Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_R_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_R_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// Z Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_Z_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_Z_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// START Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_START_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_START_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// DU Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_DU_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_DU_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// DD Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_DD_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_DD_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// DL Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_DL_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_DL_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// DR Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_DR_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_DR_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// LSU Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_LSU_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_LSU_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// LSD Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_LSD_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_LSD_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// LSL Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_LSL_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_LSL_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// LSR Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_LSR_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_LSR_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// CU Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_CU_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_CU_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// CD Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_CD_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_CD_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// CL Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_CL_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_CL_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// CR Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_CR_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_CR_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// MACRO Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_MACRO_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_MACRO_PORT, &GPIO_InitStruct_GCControllerEmulation);

	// TILT Button
	GPIO_InitStruct_GCControllerEmulation.Pin = BUTTON_TILT_PIN_HAL;
	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_PULLUP;
	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(BUTTON_TILT_PORT, &GPIO_InitStruct_GCControllerEmulation);
}

/* Emulate a GC controller forever. Note that this loop
 * is polling based. When the data is sent to the console,
 * we have about 11-12ms to do something else. That is
 * quite a bit of time for a fast uC. We use this time
 * to grab button states.
 */
void GCControllerEmulation_Run()
{
	while(1)
	{
		/* Grab the GC console command */
		command = GCControllerEmulation_GetConsoleCommand();


		/* Performs command's request */
		switch(command)
		{
			case GC_COMMAND_PROBE:
				GCControllerEmulation_SendProbeResponse();
				break;

			case GC_COMMAND_PROBE_ORIGIN:
				GCControllerEmulation_SendControllerState(GC_COMMAND_PROBE_ORIGIN);
				break;

			case GC_COMMAND_POLL_AND_TURN_RUMBLE_OFF:
				GCControllerEmulation_SendControllerState(GC_COMMAND_POLL_AND_TURN_RUMBLE_OFF);
				break;

			case GC_COMMAND_POLL_AND_TURN_RUMBLE_ON:
				GCControllerEmulation_SendControllerState(GC_COMMAND_POLL_AND_TURN_RUMBLE_ON);
				break;

			case GC_COMMAND_UNKNOWN:
			default:
				// Do nothing
				break;
		}
	}
}

/* Gets all inputs from GC Anti-Pad Hack Board */
void GCControllerEmulation_GetSwitchSnapshot()
{
	/* Update all button input states */
	gcButtonInputSnapShot[GC_A] = GCControllerEmulation_GetButtonState(GC_A);
	gcButtonInputSnapShot[GC_B] = GCControllerEmulation_GetButtonState(GC_B);
	gcButtonInputSnapShot[GC_X] = GCControllerEmulation_GetButtonState(GC_X);
	gcButtonInputSnapShot[GC_Y] = GCControllerEmulation_GetButtonState(GC_Y);
	gcButtonInputSnapShot[GC_L] = GCControllerEmulation_GetButtonState(GC_L);
	gcButtonInputSnapShot[GC_R] = GCControllerEmulation_GetButtonState(GC_R);
	gcButtonInputSnapShot[GC_Z] = GCControllerEmulation_GetButtonState(GC_Z);
	gcButtonInputSnapShot[GC_START] = GCControllerEmulation_GetButtonState(GC_START);
	gcButtonInputSnapShot[GC_DPAD_UP] = GCControllerEmulation_GetButtonState(GC_DPAD_UP);
	gcButtonInputSnapShot[GC_DPAD_DOWN] = GCControllerEmulation_GetButtonState(GC_DPAD_DOWN);
	gcButtonInputSnapShot[GC_DPAD_LEFT] = GCControllerEmulation_GetButtonState(GC_DPAD_LEFT);
	gcButtonInputSnapShot[GC_DPAD_RIGHT] = GCControllerEmulation_GetButtonState(GC_DPAD_RIGHT);
	gcButtonInputSnapShot[GC_MAIN_STICK_UP] = GCControllerEmulation_GetButtonState(GC_MAIN_STICK_UP);
	gcButtonInputSnapShot[GC_MAIN_STICK_DOWN] = GCControllerEmulation_GetButtonState(GC_MAIN_STICK_DOWN);
	gcButtonInputSnapShot[GC_MAIN_STICK_LEFT] = GCControllerEmulation_GetButtonState(GC_MAIN_STICK_LEFT);
	gcButtonInputSnapShot[GC_MAIN_STICK_RIGHT] = GCControllerEmulation_GetButtonState(GC_MAIN_STICK_RIGHT);
	gcButtonInputSnapShot[GC_C_STICK_UP] = GCControllerEmulation_GetButtonState(GC_C_STICK_UP);
	gcButtonInputSnapShot[GC_C_STICK_DOWN] = GCControllerEmulation_GetButtonState(GC_C_STICK_DOWN);
	gcButtonInputSnapShot[GC_C_STICK_LEFT] = GCControllerEmulation_GetButtonState(GC_C_STICK_LEFT);
	gcButtonInputSnapShot[GC_C_STICK_RIGHT] = GCControllerEmulation_GetButtonState(GC_C_STICK_RIGHT);
	gcButtonInputSnapShot[GC_MACRO] = GCControllerEmulation_GetButtonState(GC_MACRO);
	gcButtonInputSnapShot[GC_TILT] = GCControllerEmulation_GetButtonState(GC_TILT);
}

// Private Function Implementations //
ButtonState_t GCControllerEmulation_GetButtonState(GCButtonInput_t gcButton)
{
	ButtonState_t gcButtonState = RELEASED;

	switch(gcButton)
	{
		case GC_A:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_A_PORT, BUTTON_A_PIN_HAL);
			break;

		case GC_B:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_B_PORT, BUTTON_B_PIN_HAL);
			break;

		case GC_X:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_X_PORT, BUTTON_X_PIN_HAL);
			break;

		case GC_Y:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_Y_PORT, BUTTON_Y_PIN_HAL);
			break;

		case GC_L:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_L_PORT, BUTTON_L_PIN_HAL);
			break;

		case GC_R:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_R_PORT, BUTTON_R_PIN_HAL);
			break;

		case GC_Z:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_Z_PORT, BUTTON_Z_PIN_HAL);
			break;

		case GC_START:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_START_PORT, BUTTON_START_PIN_HAL);
			break;

		case GC_DPAD_UP:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_DU_PORT, BUTTON_DU_PIN_HAL);
			break;

		case GC_DPAD_DOWN:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_DD_PORT, BUTTON_DD_PIN_HAL);
			break;

		case GC_DPAD_LEFT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_DL_PORT, BUTTON_DL_PIN_HAL);
			break;

		case GC_DPAD_RIGHT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_DR_PORT, BUTTON_DR_PIN_HAL);
			break;

		case GC_MAIN_STICK_UP:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_LSU_PORT, BUTTON_LSU_PIN_HAL);
			break;

		case GC_MAIN_STICK_DOWN:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_LSD_PORT, BUTTON_LSD_PIN_HAL);
			break;

		case GC_MAIN_STICK_LEFT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_LSL_PORT, BUTTON_LSL_PIN_HAL);
			break;

		case GC_MAIN_STICK_RIGHT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_LSR_PORT, BUTTON_LSR_PIN_HAL);
			break;

		case GC_C_STICK_UP:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_CU_PORT, BUTTON_CU_PIN_HAL);
			break;

		case GC_C_STICK_DOWN:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_CD_PORT, BUTTON_CD_PIN_HAL);
			break;

		case GC_C_STICK_LEFT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_CL_PORT, BUTTON_CL_PIN_HAL);
			break;

		case GC_C_STICK_RIGHT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_CR_PORT, BUTTON_CR_PIN_HAL);
			break;

		case GC_MACRO:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_MACRO_PORT, BUTTON_MACRO_PIN_HAL);
			break;

		case GC_TILT:
			gcButtonState = (ButtonState_t)HAL_GPIO_ReadPin(BUTTON_TILT_PORT, BUTTON_TILT_PIN_HAL);
			break;

		default:
			break;
	}

	return gcButtonState;
}

GCCommand_t GCControllerEmulation_GetConsoleCommand()
{
	/* To receive or send bytes with the GC protocol, it must be understood
	 * that 1 GC byte = 4 UART bytes.
	 *
	 * For receiving, note that the GC stop bit is interpreted as a UART byte.
	 * Currently, we do nothing with it as this code only handles the polling
	 * command from the GC console.
	 *
	 * For sending, you need to send a stop bit. However, instead of sending
	 * 0xFF from the UART we can tie another open drain output to the GC data
	 * line. The reason is because the GC console can respond very quickly.
	 * And while it is technically not required for when emulating as a
	 * controller, you need to do this when asking data from a controller.
	 * To be consistent, this controller emulation sticks to the separate
	 * data line idea for performing a stop bit.
	 *
	 * It is important to note that since the RX and TX of the UART are tied
	 * together to fake a 1-wire setup, the receiver must be disabled when
	 * sending data on the TX line. Or else you receive your own data
	 * possibly making hard to understand who sent what.
	 */
	/* Variable to hold return value */
	GCCommand_t command;

	/* Below is grabbing a response from the console */
	// Enable the UART receiver
	USART1->CR1 |= USART_CR1_RE;

	/* First byte (possibly only byte) from console */
	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 7 & BIT 6
	gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 5 & BIT 4
	gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 3 & BIT 2
	gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 1 & BIT 0
	gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Acquire the stop for later processing
	gcConsoleResponse[GC_CONSOLE_TEMP_BITS] = USART1->DR;

	// See if the last UART byte is a GC stop bit
	if(gcConsoleResponse[GC_CONSOLE_TEMP_BITS] == GC_BITS_STOP_BIT)
	{
		// If it is, stop listening for commands and perform proper command conversion
		if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
			( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
			( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
			( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
		{
			// Interpreted command
			command = GC_COMMAND_PROBE;
		}
		else if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE2) ) &&
				 ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
				 ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
				 ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_01_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_01_CASE2) )    )
		{
			// Interpreted command
			command = GC_COMMAND_PROBE_ORIGIN;
		}
		else
		{
			// Unknown command
			command = GC_COMMAND_UNKNOWN;
		}

		// Disable the receiver
		USART1->CR1 &= ~USART_CR1_RE;

		// Return command
		return command;
	}
	else
	{
		// It was not a GC stop bit so keep reading the rest of the command bytes
	}

	/* Second byte from console */
	// Remember that last UART byte isn't a stop bit so quickly re-map
	gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] = gcConsoleResponse[GC_CONSOLE_TEMP_BITS];

    // Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 5 & BIT 4
	gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 3 & BIT 2
	gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 1 & BIT 0
	gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] = USART1->DR;

	// There is no two byte GC command (afaik) so DO NOT interpret a stop bit next

	/* Third byte from console */
	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 7 & BIT 6
	gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 5 & BIT 4
	gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 3 & BIT 2
	gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Grab states for BIT 1 & BIT 0
	gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] = USART1->DR;

	// Make sure the receive data register is not empty before receiving next byte
	while(!(USART1->SR & USART_SR_RXNE)){};
	// Acquire the stop bit and do nothing with it
	gcConsoleResponse[GC_CONSOLE_TEMP_BITS] = USART1->DR;

	/* This marks the end as three byte commands from GC console is as big as needed for this project.
	 * Even though this code does not need to interpret all three bytes to perform the necessary action,
	 * still do the comparison logic so that if future commands are necessary, it can be integrated
	 * easily. But the lazy way out would be to only interpret the last byte to see if rumble needs to
	 * occur. The really lazy way would be to just respond the button states and never investigate
	 * any byte and rely on deduction.
	 */
	// Figure out first byte
	if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE2) ) &&
		( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
		( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
		( (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
	{
		// So far we see 0x40, check possible next byte possibilities
		if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
		    ( (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
		    ( (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
		    ( (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] == GC_BITS_11_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] == GC_BITS_11_CASE2) )    )
		{
			// So far we see 0x40, 0x03, check possible next byte possibilities
			if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
				( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
				( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
				( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
			{
				// Interpreted command
				command = GC_COMMAND_POLL_AND_TURN_RUMBLE_OFF;
			}
			else if( ( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
					( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
					( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
					( (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_01_CASE1) || (gcConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_01_CASE2) )    )
			{
				// Interpreted command
				command = GC_COMMAND_POLL_AND_TURN_RUMBLE_ON;
			}
			else
			{
				// Unknown command - 0x40, 0x03, 0x??
				command = GC_COMMAND_UNKNOWN;
			}
		}
		else
		{
			// Unknown command - 0x40, 0x??, 0x??
			command = GC_COMMAND_UNKNOWN;
		}
	}
	else
	{
		// Unknown command
		command = GC_COMMAND_UNKNOWN;
	}

	// Disable the receiver
	USART1->CR1 &= ~USART_CR1_RE;

	// Return command
	return command;
}

void GCControllerEmulation_SendStopBit()
{
	/* The timing of the stop bit does not need to be so precise. But
	 * it has been optimized for this uC to be 1us. */
	GC_STOP_PORT->BSRR = GC_STOP_CLEAR;
	volatile uint32_t counter = 17;
	while(counter--);
	GC_STOP_PORT->BSRR = GC_STOP_SET;
}

void GCControllerEmulation_SendProbeResponse()
{
	/* First byte to console - 0x09 */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for A and B
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	//while(!(USART1->SR & USART_SR_TC)){};
	// Send states for Z and START
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for DU and DD
	USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for DL and DR
	USART1->DR = (GC_BITS_01_CASE1 & 0xFF);

	/* Second byte to console 0x00 */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for RESET and RESERVED
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for L and R
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for CU and CD
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for CL and CR
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	/* Third byte to console - 0x03 */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for X-AXIS BIT7 & BIT6
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for X-AXIS BIT5 & BIT4
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for X-AXIS BIT3 & BIT2
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for X-AXIS BIT1 & BIT0
	USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

	/* Stop bit to console */
	// Make sure the last UART byte transmission is complete before sending stop bit
	while(!(USART1->SR & USART_SR_TC)){};
	GCControllerEmulation_SendStopBit();
}

void GCControllerEmulation_SendControllerState(GCCommand_t command)
{
	/* For some reason, placing the bit states for inputs in a variable
	 * causes the UART to delay a little. Most reliable way is to place
	 * code directly into if...then statements for direct data transmission
	 * via DR in UART.
	 */
	/* Get snapshot of all button and switch inputs */
	GCControllerEmulation_GetSwitchSnapshot();

	/* Process button snapshot and update data we will send to the console */
	GCControllerEmulation_ProcessSwitchSnapshot();

	// State holders (left means left most bit, right means right most bit)
	ButtonState_t leftButtonState;
	ButtonState_t rightButtonState;

	/* First byte to console */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for RESERVED and RESERVED
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Update RESERVED and START
	rightButtonState = gcProcessedButtonStates[GC_START];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for RESERVED and START
	if(rightButtonState == RELEASED)
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}

	// Update Y and X
	leftButtonState = gcProcessedButtonStates[GC_Y];
	rightButtonState = gcProcessedButtonStates[GC_X];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for Y and X
	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	// Update B and A
	leftButtonState = gcProcessedButtonStates[GC_B];
	rightButtonState = gcProcessedButtonStates[GC_A];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for B and A
	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	/* Second byte to console */
	// Update RESERVED and L
	rightButtonState = gcProcessedButtonStates[GC_L];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for RESERVED and L
	if(rightButtonState == RELEASED)
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	// Update R and Z
	leftButtonState = gcProcessedButtonStates[GC_R];
	rightButtonState = gcProcessedButtonStates[GC_Z];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for R and Z
	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	// Update DU and DD
	leftButtonState = gcProcessedButtonStates[GC_DPAD_UP];
	rightButtonState = gcProcessedButtonStates[GC_DPAD_DOWN];;
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for DU and DD
	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	// Update DR and DL
	leftButtonState = gcProcessedButtonStates[GC_DPAD_RIGHT];
	rightButtonState = gcProcessedButtonStates[GC_DPAD_LEFT];
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for DR and DL
	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
	{
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
	{
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);
	}
	else
	{
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}

	/* Third byte to console */
	if(gcProcessedButtonStates[GC_MAIN_STICK_LEFT] == PUSHED)
	{
		if(gcProcessedButtonStates[GC_TILT] == PUSHED)
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_01_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
		}
		else
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
		}
	}
	else if(gcProcessedButtonStates[GC_MAIN_STICK_RIGHT] == PUSHED)
	{
		if(gcProcessedButtonStates[GC_TILT] == PUSHED)
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
		}
		else
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK X AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
		}
	}
	else
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK X AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK X AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK X AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK X AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}

	/* Fourth byte to console */
	// Update main stick y-axis
	if(gcProcessedButtonStates[GC_MAIN_STICK_DOWN] == PUSHED)
	{
		if(gcProcessedButtonStates[GC_TILT] == PUSHED)
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_01_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
		}
		else
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
		}
	}
	else if(gcProcessedButtonStates[GC_MAIN_STICK_UP] == PUSHED)
	{
		if(gcProcessedButtonStates[GC_TILT] == PUSHED)
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
		}
		else
		{
			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
			USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
			USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT2 AND BIT2
			USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

			// Make sure the transmit data register is empty before sending next byte
			while(!(USART1->SR & USART_SR_TXE)){};
			// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
			USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
		}
	}
	else
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK Y AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}

	/* Fifth byte to console */
	// Update c stick x-axis
	if(gcProcessedButtonStates[GC_C_STICK_LEFT] == PUSHED)
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if(gcProcessedButtonStates[GC_C_STICK_RIGHT] == PUSHED)
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);
	}
	else
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK X AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}

	/* Sixth byte to console */
	// Update c stick y-axis
	if(gcProcessedButtonStates[GC_C_STICK_DOWN] == PUSHED)
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}
	else if(gcProcessedButtonStates[GC_C_STICK_UP] == PUSHED)
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_11_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_01_CASE1 & 0xFF);
	}
	else
	{
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT7 AND BIT6
		USART1->DR = (GC_BITS_10_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT5 AND BIT4
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT2 AND BIT2
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for C STICK Y AXIS BIT1 AND BIT0
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}

	/* Seventh byte to console */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for L TRIGGER BIT7 AND BIT6
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for L TRIGGER BIT5 AND BIT4
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for L TRIGGER BIT3 AND BIT2
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for L TRIGGER BIT1 AND BIT0
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	/* Eighth byte to console */
	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for R TRIGGER BIT7 AND BIT6
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for R TRIGGER BIT5 AND BIT4
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for R TRIGGER BIT3 AND BIT2
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send states for R TRIGGER BIT1 AND BIT0
	USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

	// Optional bytes to send
	if(command == GC_COMMAND_PROBE_ORIGIN)
	{
		/* Ninth byte to console */
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		/* Tenth byte to console */
		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);

		// Make sure the transmit data register is empty before sending next byte
		while(!(USART1->SR & USART_SR_TXE)){};
		// Send states for ?
		USART1->DR = (GC_BITS_00_CASE1 & 0xFF);
	}

	/* Stop bit to console */
	// Make sure the last UART byte transmission is complete before sending stop bit
	while(!(USART1->SR & USART_SR_TC)){};
	GCControllerEmulation_SendStopBit();
}

void GCControllerEmulation_ProcessSwitchSnapshot()
{
	/* @Bad64: This is where we have the chance to process the raw button inputs
	 * immediately after sending the last update to the console. This part of the code
	 * should be handled as fast as possible. For the GC, we must process this within
	 * 650us because this is the minimum time before the console polls again. For example
	 * I wrote very basic SOCD code to clean to neutral. I also copied over the
	 * gcButtonInputSnapShot array (raw inputs) into the gcProcessedButtonStates array,
	 * (processed raw inputs).
	 *
	 * You need to decide what you want to do for the "digital action buttons" and the
	 * "digital feature buttons". I know you want to program behavior of the "digital
	 * feature buttons" like the tilt buttons, but I have no idea if you want to do
	 * something with the "digital action buttons" like the A, B, X, etc buttons.
	 */
	/* Apply basic SOCD cleaning (clean to neutral) */
	// Clean d-pad x-axis
	if ( (gcButtonInputSnapShot[GC_DPAD_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_DPAD_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_DPAD_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_RIGHT] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_DPAD_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_DPAD_RIGHT] == PUSHED) )
	{
		gcProcessedButtonStates[GC_DPAD_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_RIGHT] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_DPAD_LEFT] == PUSHED) && (gcButtonInputSnapShot[GC_DPAD_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_DPAD_LEFT] = PUSHED;
		gcProcessedButtonStates[GC_DPAD_RIGHT] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_DPAD_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_RIGHT] = RELEASED;
	}

	// Clean d-pad y-axis
	if ( (gcButtonInputSnapShot[GC_DPAD_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_DPAD_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_DPAD_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_UP] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_DPAD_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_DPAD_UP] == PUSHED) )
	{
		gcProcessedButtonStates[GC_DPAD_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_UP] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_DPAD_DOWN] == PUSHED) && (gcButtonInputSnapShot[GC_DPAD_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_DPAD_DOWN] = PUSHED;
		gcProcessedButtonStates[GC_DPAD_UP] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_DPAD_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_DPAD_UP] = RELEASED;
	}

	// Clean main stick x-axis
	if ( (gcButtonInputSnapShot[GC_MAIN_STICK_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_MAIN_STICK_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_RIGHT] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_MAIN_STICK_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_MAIN_STICK_RIGHT] == PUSHED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_RIGHT] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_MAIN_STICK_LEFT] == PUSHED) && (gcButtonInputSnapShot[GC_MAIN_STICK_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_LEFT] = PUSHED;
		gcProcessedButtonStates[GC_MAIN_STICK_RIGHT] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_MAIN_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_RIGHT] = RELEASED;
	}

	// Clean main stick y-axis
	if ( (gcButtonInputSnapShot[GC_MAIN_STICK_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_MAIN_STICK_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_UP] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_MAIN_STICK_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_MAIN_STICK_UP] == PUSHED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_UP] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_MAIN_STICK_DOWN] == PUSHED) && (gcButtonInputSnapShot[GC_MAIN_STICK_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_MAIN_STICK_DOWN] = PUSHED;
		gcProcessedButtonStates[GC_MAIN_STICK_UP] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_MAIN_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_MAIN_STICK_UP] = RELEASED;
	}

	// Clean c stick x-axis
	if ( (gcButtonInputSnapShot[GC_C_STICK_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_C_STICK_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_C_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_RIGHT] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_C_STICK_LEFT] == RELEASED) && (gcButtonInputSnapShot[GC_C_STICK_RIGHT] == PUSHED) )
	{
		gcProcessedButtonStates[GC_C_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_RIGHT] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_C_STICK_LEFT] == PUSHED) && (gcButtonInputSnapShot[GC_C_STICK_RIGHT] == RELEASED) )
	{
		gcProcessedButtonStates[GC_C_STICK_LEFT] = PUSHED;
		gcProcessedButtonStates[GC_C_STICK_RIGHT] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_C_STICK_LEFT] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_RIGHT] = RELEASED;
	}

	// Clean c stick y-axis
	if ( (gcButtonInputSnapShot[GC_C_STICK_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_C_STICK_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_C_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_UP] = RELEASED;
	}
	else if ( (gcButtonInputSnapShot[GC_C_STICK_DOWN] == RELEASED) && (gcButtonInputSnapShot[GC_C_STICK_UP] == PUSHED) )
	{
		gcProcessedButtonStates[GC_C_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_UP] = PUSHED;
	}
	else if ( (gcButtonInputSnapShot[GC_C_STICK_DOWN] == PUSHED) && (gcButtonInputSnapShot[GC_C_STICK_UP] == RELEASED) )
	{
		gcProcessedButtonStates[GC_C_STICK_DOWN] = PUSHED;
		gcProcessedButtonStates[GC_C_STICK_UP] = RELEASED;
	}
	else
	{
		gcProcessedButtonStates[GC_C_STICK_DOWN] = RELEASED;
		gcProcessedButtonStates[GC_C_STICK_UP] = RELEASED;
	}
	/* End of SOCD cleaning */

	/* Process digital action buttons. For the meantime, they do not need
	 * any sort of special processing so just copy them over.
	 */
	gcProcessedButtonStates[GC_A] = gcButtonInputSnapShot[GC_A];
	gcProcessedButtonStates[GC_B] = gcButtonInputSnapShot[GC_B];
	gcProcessedButtonStates[GC_X] = gcButtonInputSnapShot[GC_X];
	gcProcessedButtonStates[GC_Y] = gcButtonInputSnapShot[GC_Y];
	gcProcessedButtonStates[GC_L] = gcButtonInputSnapShot[GC_L];
	gcProcessedButtonStates[GC_R] = gcButtonInputSnapShot[GC_R];
	gcProcessedButtonStates[GC_Z] = gcButtonInputSnapShot[GC_Z];
	gcProcessedButtonStates[GC_START] = gcButtonInputSnapShot[GC_START];
	/* End processing of digital action buttons */

	/* Process digital feature buttons. For the meantime, they do not need
	 * any sort of special processing so just copy them over.
	 */
	gcProcessedButtonStates[GC_MACRO] = gcButtonInputSnapShot[GC_MACRO];
	gcProcessedButtonStates[GC_TILT] = gcButtonInputSnapShot[GC_TILT];
	/* End processing of digital action buttons */
}
