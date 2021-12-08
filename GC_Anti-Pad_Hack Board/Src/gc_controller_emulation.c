#include "GC_controller_emulation.h"

// Private Macos //
#define  GC_TURBO_INTERVAL      1U
#define  GC_TURBO_RESET			2U

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
static UART_HandleTypeDef huart3;

/* Snapshot of button states */
static ButtonState_t gcSnapshotMSI[16] = {};
static ButtonState_t gcSnapshotSSI[8] = {};

/* Data from console before being converted into a command */
static uint8_t GCConsoleResponse[MAX_GC_CONSOLE_BYTES];

/* Command from console after converted */
static GCCommand_t command;

// Private Function Prototypes //
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

// Public Function Implementations //
/* Initializes this module to properly emulate a GC controller */
void GCControllerEmulation_Init()
{
//	// Clocks
//	__HAL_RCC_GPIOC_CLK_ENABLE();
//	__HAL_RCC_USART3_CLK_ENABLE();
//
//	// Init structure
//	GPIO_InitTypeDef GPIO_InitStruct_GCControllerEmulation = {0};
//
//	// Stop bit control
//	GPIO_InitStruct_GCControllerEmulation.Pin = GC_STOP_PIN_HAL;
//	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_OUTPUT_OD;
//	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_NOPULL;
//	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	HAL_GPIO_Init(GC_STOP_PORT, &GPIO_InitStruct_GCControllerEmulation);
//	GC_STOP_PORT->BSRR = GC_STOP_SET;
//
//	// USART3 TX/RX
//	GPIO_InitStruct_GCControllerEmulation.Pin = GC_TX_PIN_HAL | GC_RX_PIN_HAL;
//	GPIO_InitStruct_GCControllerEmulation.Mode = GPIO_MODE_AF_OD;
//	GPIO_InitStruct_GCControllerEmulation.Alternate = GPIO_AF7_USART3;
//	GPIO_InitStruct_GCControllerEmulation.Pull = GPIO_NOPULL;
//	GPIO_InitStruct_GCControllerEmulation.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	HAL_GPIO_Init(GC_TX_PORT, &GPIO_InitStruct_GCControllerEmulation);
//
//	// Configure USART3
//	huart3.Instance = USART3;
//	huart3.Init.BaudRate = 100000; // not 100kbit/s but ~1.1Mbit/s (because main clock is 168MHz)
//	huart3.Init.WordLength = UART_WORDLENGTH_8B;
//	huart3.Init.StopBits = UART_STOPBITS_1;
//	huart3.Init.Parity = UART_PARITY_NONE;
//	huart3.Init.Mode = UART_MODE_TX_RX;
//	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//	huart3.Init.OverSampling = UART_OVERSAMPLING_8;
//	HAL_UART_Init(&huart3);
//
//	// Indicate no buttons are in turbo mode
//	gcControllerTurboEnableRegister = 0;
//
//	// Define last turbo state
//	gcControllerLastTurboState = RELEASED;
//
//	// Default command state from console
//	command = GC_COMMAND_UNKNOWN;
//
//	// Configure socd settings
//	Socd_SetCleanerFromSwitch();
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

// Private Function Implementations //
void GCControllerEmulation_GetSwitchSnapshot()
{
//	// Acquire the MSI inputs
//	gcSnapshotMSI[(uint8_t)P1] = ButtonInput_GetStateMSI(P1);
//	gcSnapshotMSI[(uint8_t)P2] = ButtonInput_GetStateMSI(P2);
//	gcSnapshotMSI[(uint8_t)P3] = ButtonInput_GetStateMSI(P3);
//	gcSnapshotMSI[(uint8_t)P4] = ButtonInput_GetStateMSI(P4);
//	gcSnapshotMSI[(uint8_t)K1] = ButtonInput_GetStateMSI(K1);
//	gcSnapshotMSI[(uint8_t)K2] = ButtonInput_GetStateMSI(K2);
//	gcSnapshotMSI[(uint8_t)K3] = ButtonInput_GetStateMSI(K3);
//	gcSnapshotMSI[(uint8_t)K4] = ButtonInput_GetStateMSI(K4);
//	gcSnapshotMSI[(uint8_t)LEFT] = ButtonInput_GetStateMSI(LEFT);
//	gcSnapshotMSI[(uint8_t)RIGHT] = ButtonInput_GetStateMSI(RIGHT);
//	gcSnapshotMSI[(uint8_t)DOWN] = ButtonInput_GetStateMSI(DOWN);
//	gcSnapshotMSI[(uint8_t)UP] = ButtonInput_GetStateMSI(UP);
//	gcSnapshotMSI[(uint8_t)START] = ButtonInput_GetStateMSI(START);
//	gcSnapshotMSI[(uint8_t)SELECT] = ButtonInput_GetStateMSI(SELECT);
//	gcSnapshotMSI[(uint8_t)HOME] = ButtonInput_GetStateMSI(HOME);
//	gcSnapshotMSI[(uint8_t)MSI_BIT_15] = ButtonInput_GetStateMSI(MSI_BIT_15);
//
//	// Acquire the SSI inputs
//	gcSnapshotSSI[(uint8_t)LS] = ButtonInput_GetStateSSI(LS);
//	gcSnapshotSSI[(uint8_t)RS] = ButtonInput_GetStateSSI(RS);
//	gcSnapshotSSI[(uint8_t)DP] = ButtonInput_GetStateSSI(DP);
//	gcSnapshotSSI[(uint8_t)LOCK] = ButtonInput_GetStateSSI(LOCK);
//	gcSnapshotSSI[(uint8_t)TBKEY] = ButtonInput_GetStateSSI(TBKEY);
//	gcSnapshotSSI[(uint8_t)TPKEY] = ButtonInput_GetStateSSI(TPKEY);
//	gcSnapshotSSI[(uint8_t)L3] = ButtonInput_GetStateSSI(L3);
//	gcSnapshotSSI[(uint8_t)R3] = ButtonInput_GetStateSSI(R3);
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
//	USART3->CR1 |= USART_CR1_RE;
//
//	/* First byte (possibly only byte) from console */
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 7 & BIT 6
//	GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 5 & BIT 4
//	GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 3 & BIT 2
//	GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 1 & BIT 0
//	GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Acquire the stop for later processing
//	GCConsoleResponse[GC_CONSOLE_TEMP_BITS] = USART3->DR;
//
//	// See if the last UART byte is a GC stop bit
//	if(GCConsoleResponse[GC_CONSOLE_TEMP_BITS] == GC_BITS_STOP_BIT)
//	{
//		// If it is, stop listening for commands and perform proper command conversion
//		if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
//			( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//			( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//			( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
//		{
//			// Interpreted command
//			command = GC_COMMAND_PROBE;
//		}
//		else if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE2) ) &&
//				 ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//				 ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//				 ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_01_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_01_CASE2) )    )
//		{
//			// Interpreted command
//			command = GC_COMMAND_PROBE_ORIGIN;
//		}
//		else
//		{
//			// Unknown command
//			command = GC_COMMAND_UNKNOWN;
//		}
//
//		// Disable the receiver
//		USART3->CR1 &= ~USART_CR1_RE;
//
//		// Return command
//		return command;
//	}
//	else
//	{
//		// It was not a GC stop bit so keep reading the rest of the command bytes
//	}
//
//	/* Second byte from console */
//	// Remember that last UART byte isn't a stop bit so quickly re-map
//	GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] = GCConsoleResponse[GC_CONSOLE_TEMP_BITS];
//
//    // Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 5 & BIT 4
//	GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 3 & BIT 2
//	GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 1 & BIT 0
//	GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] = USART3->DR;
//
//	// There is no two byte GC command (afaik) so DO NOT interpret a stop bit next
//
//	/* Third byte from console */
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 7 & BIT 6
//	GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 5 & BIT 4
//	GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 3 & BIT 2
//	GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Grab states for BIT 1 & BIT 0
//	GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] = USART3->DR;
//
//	// Make sure the receive data register is not empty before receiving next byte
//	while(!(USART3->SR & USART_SR_RXNE)){};
//	// Acquire the stop bit and do nothing with it
//	GCConsoleResponse[GC_CONSOLE_TEMP_BITS] = USART3->DR;
//
//	/* This marks the end as three byte commands from GC console is as big as needed for this project.
//	 * Even though this code does not need to interpret all three bytes to perform the necessary action,
//	 * still do the comparison logic so that if future commands are necessary, it can be integrated
//	 * easily. But the lazy way out would be to only interpret the last byte to see if rumble needs to
//	 * occur. The really lazy way would be to just respond the button states and never investigate
//	 * any byte and rely on deduction.
//	 */
//	// Figure out first byte
//	if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT7_BIT6] == GC_BITS_01_CASE2) ) &&
//		( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//		( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//		( (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_0_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
//	{
//		// So far we see 0x40, check possible next byte possibilities
//		if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
//		    ( (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//		    ( (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//		    ( (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] == GC_BITS_11_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_1_BIT1_BIT0] == GC_BITS_11_CASE2) )    )
//		{
//			// So far we see 0x40, 0x03, check possible next byte possibilities
//			if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
//				( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//				( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//				( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_00_CASE2) )    )
//			{
//				// Interpreted command
//				command = GC_COMMAND_POLL_AND_TURN_RUMBLE_OFF;
//			}
//			else if( ( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT7_BIT6] == GC_BITS_00_CASE2) ) &&
//					( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT5_BIT4] == GC_BITS_00_CASE2) ) &&
//					( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT3_BIT2] == GC_BITS_00_CASE2) ) &&
//					( (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_01_CASE1) || (GCConsoleResponse[GC_CONSOLE_BYTE_2_BIT1_BIT0] == GC_BITS_01_CASE2) )    )
//			{
//				// Interpreted command
//				command = GC_COMMAND_POLL_AND_TURN_RUMBLE_ON;
//			}
//			else
//			{
//				// Unknown command - 0x40, 0x03, 0x??
//				command = GC_COMMAND_UNKNOWN;
//			}
//		}
//		else
//		{
//			// Unknown command - 0x40, 0x??, 0x??
//			command = GC_COMMAND_UNKNOWN;
//		}
//	}
//	else
//	{
//		// Unknown command
//		command = GC_COMMAND_UNKNOWN;
//	}
//
//	// Disable the receiver
//	USART3->CR1 &= ~USART_CR1_RE;
//
//	// Return command
//	return command;
}

void GCControllerEmulation_SendStopBit()
{
//	/* The timing of the stop bit does not need to be so precise. But
//	 * it has been optimized for this uC to be 1us. */
//	GC_STOP_PORT->BSRR = GC_STOP_CLEAR;
//	volatile uint32_t counter = 17;
//	while(counter--);
//	GC_STOP_PORT->BSRR = GC_STOP_SET;
}

void GCControllerEmulation_SendProbeResponse()
{
//	/* First byte to console - 0x09 */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for A and B
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	//while(!(USART3->SR & USART_SR_TC)){};
//	// Send states for Z and START
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for DU and DD
//	USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for DL and DR
//	USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//
//	/* Second byte to console 0x00 */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for RESET and RESERVED
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for L and R
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for CU and CD
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for CL and CR
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Third byte to console - 0x03 */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for X-AXIS BIT7 & BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for X-AXIS BIT5 & BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for X-AXIS BIT3 & BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for X-AXIS BIT1 & BIT0
//	USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//
//	/* Stop bit to console */
//	// Make sure the last UART byte transmission is complete before sending stop bit
//	while(!(USART3->SR & USART_SR_TC)){};
//	GCControllerEmulation_SendStopBit();
}

void GCControllerEmulation_SendControllerState(GCCommand_t command)
{
//	/* For some reason, placing the bit states for inputs in a variable
//	 * causes the UART to delay a little. Most reliable way is to place
//	 * code directly into if...then statements for direct data transmission
//	 * via DR in UART.
//	 */
//	/* Get snapshot of all button and switch inputs */
//	GCControllerEmulation_GetSwitchSnapshot();
//
//	/* Set action for lock-able buttons; if locked default action to none (released) */
//	if(gcSnapshotSSI[LOCK] == PUSHED)
//	{
//		// Configure which buttons are lockable for GC
//		gcSnapshotMSI[START] = RELEASED;
//		gcSnapshotMSI[SELECT] = RELEASED;
//		//GCSnapshotMSI[HOME] = RELEASED;
//		gcSnapshotSSI[L3] = RELEASED;
//		gcSnapshotSSI[R3] = RELEASED;
//		gcSnapshotSSI[TPKEY] = RELEASED;
//		gcSnapshotSSI[TBKEY] = RELEASED;
//	}
//
//	/* Check to see if a new remote SOCD mode should be programmed */
//	if(gcSnapshotMSI[LEFT] == PUSHED && gcSnapshotMSI[RIGHT] == PUSHED &&
//	   gcSnapshotMSI[DOWN] == PUSHED && gcSnapshotMSI[UP] == PUSHED &&
//	   gcSnapshotMSI[HOME] == PUSHED)
//	{
//		/* Wait for all of the buttons to be released before allowing
//		 * player to choose the mode.
//		 */
//		while(gcSnapshotMSI[LEFT] == PUSHED && gcSnapshotMSI[RIGHT] == PUSHED &&
//			  gcSnapshotMSI[DOWN] == PUSHED && gcSnapshotMSI[UP] == PUSHED &&
//			  gcSnapshotMSI[HOME] == PUSHED)
//		{
//			// Keep getting a new snap shot of buttons while waiting for player to release buttons
//			GCControllerEmulation_GetSwitchSnapshot();
//		}
//
//		/* Give the player 6 seconds to pick their setting (but tell them to hold for 8 seconds) */
//		AcrylicLed_SetState(LED_OFF);	// Visual indicator that playe entered SOCD configuration mode
//		HAL_Delay(63000);  				// Write cycle delay ~6s @ 168MHz
//		GCControllerEmulation_GetSwitchSnapshot();	// Read what the player picked for the SOCD setting
//		AcrylicLed_SetState(LED_ON);	// Visual indicator that player exited SOCD configuration mode
//
//		/* Take player's chosen setting and save into the board's eeprom */
//		Socd_SetRemoteCleanerMode(gcSnapshotMSI[LEFT], gcSnapshotMSI[RIGHT], gcSnapshotMSI[DOWN], gcSnapshotMSI[UP]);
//
//		/* Read from board's eeprom and apply the SOCD settings now */
//		Socd_SetCleanerFromSwitch();
//	}
//
//	/* Check to see if turbo button is PUSHED */
//	if(gcSnapshotSSI[TBKEY] == PUSHED)
//	{
//		// Update turbo config only if entering from turbo was last released
//		if(gcControllerLastTurboState == RELEASED)
//		{
//			// Update current turbo button state
//			gcControllerLastTurboState = PUSHED;
//
//			// Complement the turbo setting for each button
//			if(gcSnapshotMSI[P1] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)P1);}
//			if(gcSnapshotMSI[P2] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)P2);}
//			if(gcSnapshotMSI[P3] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)P3);}
//			if(gcSnapshotMSI[P4] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)P4);}
//			if(gcSnapshotMSI[K1] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)K1);}
//			if(gcSnapshotMSI[K2] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)K2);}
//			if(gcSnapshotMSI[K3] == PUSHED){gcControllerTurboEnableRegister ^= (1 << (uint32_t)K3);}
//		}
//	}
//	else
//	{
//		// Turbo was not PUSHED so update this
//		gcControllerLastTurboState = RELEASED;
//	}
//
//	/* Loop through each of the buttons that are turbo enabled */
//	for(uint8_t i = (uint8_t)P1; i <= (uint8_t)K3; i = i + 1)
//	{
//		MainSwitchInput_t mainButton = (MainSwitchInput_t)i;
//
//		// Place status of main action (choose between turbo and regular modes)
//		if( (gcControllerTurboEnableRegister >> (uint32_t)mainButton) & 1U )
//		{
//			/* Turbo is enabled for this button */
//			// If the button is released, perform no action
//			if(gcSnapshotMSI[mainButton] == RELEASED)
//			{
//				gcSnapshotMSI[mainButton] = RELEASED;
//			}
//			// If the button is PUSHED, perform the proper turbo action (next action state)
//			else
//			{
//				// Next action is "calculated" by using intervals of program execution cycles
//				if(gcControllerActionTurboCounters[mainButton] < GC_TURBO_INTERVAL)
//				{
//					gcSnapshotMSI[mainButton] = RELEASED;
//					Main_SetTurboLedState(LED_OFF);
//				}
//				else
//				{
//					gcSnapshotMSI[mainButton] = PUSHED;
//					Main_SetTurboLedState(LED_OFF);
//				}
//				gcControllerActionTurboCounters[mainButton] = gcControllerActionTurboCounters[mainButton] + 1;
//				if(gcControllerActionTurboCounters[mainButton] == GC_TURBO_RESET)
//				{
//					gcControllerActionTurboCounters[mainButton] = 0;
//				}
//			}
//		}
//		else
//		{
//			/* Turbo is not enabled for this button */
//			// Leave snapshot as is
//		}
//	} // End of button looping for turbo featured main buttons
//
//	// Feed GCSnapshot directionals into the socd cleaner for computation
//	Socd_Update(gcSnapshotMSI[LEFT], gcSnapshotMSI[RIGHT], gcSnapshotMSI[DOWN], gcSnapshotMSI[UP]);
//
//	// State holders (left means left most bit, right means right most bit)
//	ButtonState_t leftButtonState;
//	ButtonState_t rightButtonState;
//
//	/* First byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for RESERVED and RESERVED
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Update RESERVED and START
//	rightButtonState = gcSnapshotMSI[(uint8_t)START];
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for RESERVED and START
//	if(rightButtonState == RELEASED)
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//
//	// Update Y and X
//	leftButtonState = gcSnapshotMSI[(uint8_t)P1];
//	rightButtonState = gcSnapshotMSI[(uint8_t)P2];
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for Y and X
//	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	// Update B and A
//	leftButtonState = gcSnapshotMSI[(uint8_t)K2];
//	rightButtonState = gcSnapshotMSI[(uint8_t)K1];
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for B and A
//	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	/* Second byte to console */
//	// Update RESERVED and L
//	rightButtonState = gcSnapshotMSI[(uint8_t)P3];
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for RESERVED and L
//	if(rightButtonState == RELEASED)
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	// Update R and Z
//	leftButtonState = gcSnapshotMSI[(uint8_t)K3];
//	rightButtonState = gcSnapshotMSI[(uint8_t)P4];
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for R and Z
//	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	// Update DU and DD
//	leftButtonState = Socd_GetState(UP);
//	rightButtonState = Socd_GetState(DOWN);
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for DU and DD
//	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	// Update DR and DL
//	leftButtonState = Socd_GetState(RIGHT);
//	rightButtonState = Socd_GetState(LEFT);
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for DR and DL
//	if( (leftButtonState == RELEASED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == RELEASED) && (rightButtonState == PUSHED) )
//	{
//		USART3->DR = (GC_BITS_01_CASE1 & 0xFF);
//	}
//	else if( (leftButtonState == PUSHED) && (rightButtonState == RELEASED) )
//	{
//		USART3->DR = (GC_BITS_10_CASE1 & 0xFF);
//	}
//	else
//	{
//		USART3->DR = (GC_BITS_11_CASE1 & 0xFF);
//	}
//
//	/* Third byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK X AXIS BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK X AXIS BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK X AXIS BIT2 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK X AXIS BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Fourth byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK Y AXIS BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK Y AXIS BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK Y AXIS BIT3 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for MAIN STICK Y AXIS BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Fifth byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK X AXIS BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK X AXIS BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK X AXIS BIT3 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK X AXIS BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Sixth byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK Y AXIS BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK Y AXIS BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK Y AXIS BIT3 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for C STICK Y AXIS BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Seventh byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for L TRIGGER BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for L TRIGGER BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for L TRIGGER BIT3 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for L TRIGGER BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	/* Eighth byte to console */
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for R TRIGGER BIT7 AND BIT6
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for R TRIGGER BIT5 AND BIT4
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for R TRIGGER BIT3 AND BIT2
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Make sure the transmit data register is empty before sending next byte
//	while(!(USART3->SR & USART_SR_TXE)){};
//	// Send states for R TRIGGER BIT1 AND BIT0
//	USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//	// Optional bytes to send
//	if(command == GC_COMMAND_PROBE_ORIGIN)
//	{
//		/* Ninth byte to console */
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		/* Tenth byte to console */
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//
//		// Make sure the transmit data register is empty before sending next byte
//		while(!(USART3->SR & USART_SR_TXE)){};
//		// Send states for
//		USART3->DR = (GC_BITS_00_CASE1 & 0xFF);
//	}
//
//	/* Stop bit to console */
//	// Make sure the last UART byte transmission is complete before sending stop bit
//	while(!(USART3->SR & USART_SR_TC)){};
//	GCControllerEmulation_SendStopBit();
}
