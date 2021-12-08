#include "comms_n64_console.h"

// Initializer
void CommsN64Console_Init()
{
	/* Set parameters for n64 communication pins */

	// Clocks
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();

	// Init structure
	GPIO_InitTypeDef GPIO_InitStruct_CommsN64Console = {0};

	// Stop bit control
	GPIO_InitStruct_CommsN64Console.Pin = COMMS_N64_STOP_PIN_HAL;
	GPIO_InitStruct_CommsN64Console.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct_CommsN64Console.Pull = GPIO_NOPULL;
	GPIO_InitStruct_CommsN64Console.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(COMMS_N64_STOP_PORT, &GPIO_InitStruct_CommsN64Console);
	COMMS_N64_STOP_PORT->BSRR = COMMS_N64_STOP_SET;

	// External pullup control
	GPIO_InitStruct_CommsN64Console.Pin = COMMS_N64_PULLUP_PIN_HAL;
	GPIO_InitStruct_CommsN64Console.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_CommsN64Console.Pull = GPIO_NOPULL;
	GPIO_InitStruct_CommsN64Console.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(COMMS_N64_PULLUP_PORT, &GPIO_InitStruct_CommsN64Console);

	// USART1 TX/RX
	GPIO_InitStruct_CommsN64Console.Pin = COMMS_N64_TX_PIN_HAL | COMMS_N64_RX_PIN_HAL;
	GPIO_InitStruct_CommsN64Console.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct_CommsN64Console.Alternate = GPIO_AF7_USART1;
	GPIO_InitStruct_CommsN64Console.Pull = GPIO_NOPULL;
	GPIO_InitStruct_CommsN64Console.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(COMMS_N64_TX_PORT, &GPIO_InitStruct_CommsN64Console);

	// Configure USART1
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 1300000;	// Started w/ 1,250,000 but experiments show 1,300,000 is more robust
	huart1.Init.WordLength = USART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_8;

	HAL_UART_Init(&huart1);
}

void CommsN64Console_SetPullup(N64PullupState_t pull)
{
	if(pull == N64_NOPULL)
	{
		HAL_GPIO_WritePin(COMMS_N64_PULLUP_PORT, COMMS_N64_PULLUP_PIN_HAL, GPIO_PIN_RESET);
	}
	else if(pull == N64_PULLUP)
	{
		HAL_GPIO_WritePin(COMMS_N64_PULLUP_PORT, COMMS_N64_PULLUP_PIN_HAL, GPIO_PIN_SET);
	}
	else
	{
		// invalid argument
	}
}

void CommsN64Console_SendStopBit()
{
	/* The timing of the stop bit does not need to be so precise. */
	COMMS_N64_STOP_PORT->BSRR = COMMS_N64_STOP_CLEAR;
	volatile uint32_t counter = 1;
	while(counter--);
	COMMS_N64_STOP_PORT->BSRR = COMMS_N64_STOP_SET;
}

// Parsers
void CommsN64Console_ParseContollerInputs()
{
	// Helper variables
	uint8_t leftBit = 0;
	uint8_t rightBit = 0;

	/* TODO: Improve the algorithm below. It was made just to ensure functionality
	 * but it ought to be better coded.
	 */
	// Update n64 register, byte 1
	for(uint8_t j = 0; j < 4; j = j + 1)
	{
		// Choose "left" and "right" bits
		switch(j)
		{
			case 0:
				leftBit = 7;
				rightBit = 6;
				break;

			case 1:
				leftBit = 5;
				rightBit = 4;
				break;

			case 2:
				leftBit = 3;
				rightBit = 2;
				break;

			case 3:
				leftBit = 1;
				rightBit = 0;
				break;

			default:
				break;
		}

		// Manipulate the chosen bits
		if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE1) ||
			(controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE2)	)
		{
			controllerRegisters[N64_BYTE1] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE1] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE2)	)
		{
			controllerRegisters[N64_BYTE1] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE1] |= (1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE2)	)
		{
			controllerRegisters[N64_BYTE1] |= (1 << leftBit);
			controllerRegisters[N64_BYTE1] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE2)	)
		{
			controllerRegisters[N64_BYTE1] |= (1 << leftBit);
			controllerRegisters[N64_BYTE1] |= (1 << rightBit);
		}
		else
		{
			// invalid
		}
	}

	// Update n64 register, byte 2
	for(uint8_t j = 4; j < 8; j = j + 1)
	{
		// Choose "left" and "right" bits
		switch(j)
		{
			case 4:
				leftBit = 7;
				rightBit = 6;
				break;

			case 5:
				leftBit = 5;
				rightBit = 4;
				break;

			case 6:
				leftBit = 3;
				rightBit = 2;
				break;

			case 7:
				leftBit = 1;
				rightBit = 0;
				break;

			default:
				break;
		}

		// Manipulate the chosen bits
		if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE1) ||
			(controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE2)	)
		{
			controllerRegisters[N64_BYTE2] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE2] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE2)	)
		{
			controllerRegisters[N64_BYTE2] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE2] |= (1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE2)	)
		{
			controllerRegisters[N64_BYTE2] |= (1 << leftBit);
			controllerRegisters[N64_BYTE2] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE2)	)
		{
			controllerRegisters[N64_BYTE2] |= (1 << leftBit);
			controllerRegisters[N64_BYTE2] |= (1 << rightBit);
		}
		else
		{
			// invalid
		}
	}

	// Update n64 register, byte 3
	for(uint8_t j = 8; j < 12; j = j + 1)
	{
		// Choose "left" and "right" bits
		switch(j)
		{
			case 8:
				leftBit = 7;
				rightBit = 6;
				break;

			case 9:
				leftBit = 5;
				rightBit = 4;
				break;

			case 10:
				leftBit = 3;
				rightBit = 2;
				break;

			case 11:
				leftBit = 1;
				rightBit = 0;
				break;

			default:
				break;
		}

		// Manipulate the chosen bits
		if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE1) ||
			(controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE2)	)
		{
			controllerRegisters[N64_BYTE3] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE3] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE2)	)
		{
			controllerRegisters[N64_BYTE3] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE3] |= (1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE2)	)
		{
			controllerRegisters[N64_BYTE3] |= (1 << leftBit);
			controllerRegisters[N64_BYTE3] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE2)	)
		{
			controllerRegisters[N64_BYTE3] |= (1 << leftBit);
			controllerRegisters[N64_BYTE3] |= (1 << rightBit);
		}
		else
		{
			// invalid
		}
	}

	// Update n64 register, byte 3
	for(uint8_t j = 12; j < 16; j = j + 1)
	{
		// Choose "left" and "right" bits
		switch(j)
		{
			case 12:
				leftBit = 7;
				rightBit = 6;
				break;

			case 13:
				leftBit = 5;
				rightBit = 4;
				break;

			case 14:
				leftBit = 3;
				rightBit = 2;
				break;

			case 15:
				leftBit = 1;
				rightBit = 0;
				break;

			default:
				break;
		}

		// Manipulate the chosen bits
		if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE1) ||
			(controllerResponse[j] == UART_BYTE_2_N64_BITS_00_CASE2)	)
		{
			controllerRegisters[N64_BYTE4] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE4] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_01_CASE2)	)
		{
			controllerRegisters[N64_BYTE4] &= ~(1 << leftBit);
			controllerRegisters[N64_BYTE4] |= (1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_10_CASE2)	)
		{
			controllerRegisters[N64_BYTE4] |= (1 << leftBit);
			controllerRegisters[N64_BYTE4] &= ~(1 << rightBit);
		}
		else if( (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE1) ||
			     (controllerResponse[j] == UART_BYTE_2_N64_BITS_11_CASE2)	)
		{
			controllerRegisters[N64_BYTE4] |= (1 << leftBit);
			controllerRegisters[N64_BYTE4] |= (1 << rightBit);
		}
		else
		{
			// invalid
		}
	}
}

void CommsN64Console_GetContollerStatus()
{
	// Future implementation
}

void CommsN64Console_GetContollerInputs()
{
	/* Send send a byte of data to the n64, you must send 4 bytes of data from
	 * the usart1 module. This means that for every byte sent byte sent by
	 * usart1, 2 bits are transferred to the n64 data line. Note that at the
	 * end of the 4th byte sent from usart1, a stop bit must be sent. It is
	 * possible to send 0xFF from usart1 to achieve this but the n64 controller
	 * will respond before the usart1 transmission of 0xFF is complete. Instead
	 * we can tie another open drain output to the n64 data line. When the 4th
	 * usart1 byte is sent, we can toggle this additional open drain output. Doing
	 * so will act as a stop but required for the n64 protocol. After this is
	 * performed, there is about 2.5 microseconds left before the n64 controller
	 * begins to respond. For a STM32F411CE running at 100MHz, this be more than
	 * enough time to prepare a way to capture the data.
	 */
	// Disable the receiver
	USART1->CR1 &= ~USART_CR1_RE;

	// Make sure the transmit data register is empty before sending next byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send a byte (2 n64 bits)
	USART1->DR = (UART_BYTE_2_N64_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending a byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send a byte (2 n64 bits)
	USART1->DR = (UART_BYTE_2_N64_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending a byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send a byte (2 n64 bits)
	USART1->DR = (UART_BYTE_2_N64_BITS_00_CASE1 & 0xFF);

	// Make sure the transmit data register is empty before sending a byte
	while(!(USART1->SR & USART_SR_TXE)){};
	// Send a byte (2 n64 bits)
	USART1->DR = (UART_BYTE_2_N64_BITS_01_CASE1 & 0xFF);

	// Make sure the last uart byte transmission is complete before sending stop bit
	while(!(USART1->SR & USART_SR_TC)){};
	CommsN64Console_SendStopBit();

	/* Receive 4 n64 bytes from the controller. This means that our uart will
	 * have to receive 16 bytes because 1 uart byte = 2 n64 bits and therefore
	 * 4 uart bytes = 1 n64 byte. But remember, the controller will send a stop
	 * bit after this 4 n64 byte transmission. That means the uart receiver must
	 * be disabled after the last acquired byte. Of course before grabbing the
	 * first byte, enabled the uart receiver.
	 */
	// Enable the uart receiver
	USART1->CR1 |= USART_CR1_RE;

	// Grab states for A and B
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_A_B] = USART1->DR;

	// Grab states for Z and START
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_Z_START] = USART1->DR;

	// Grab states for DU and DD
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_DU_DD] = USART1->DR;

	// Grab states for DL and DR
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_DL_DR] = USART1->DR;

	// Grab states for RESET and RESERVED
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_RESET_RESERVED] = USART1->DR;

	// Grab states for L and R
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_L_R] = USART1->DR;

	// Grab states for CU and CD
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_CU_CD] = USART1->DR;

	// Grab states for CL and CR
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_CL_CR] = USART1->DR;

	// Grab states for X-AXIS BIT7 & BIT6
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_X_AXIS_BIT7_BIT6] = USART1->DR;

	// Grab states for X-AXIS BIT5 & BIT4
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_X_AXIS_BIT5_BIT4] = USART1->DR;

	// Grab states for X-AXIS BIT3 & BIT2
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_X_AXIS_BIT3_BIT2] = USART1->DR;

	// Grab states for X-AXIS BIT1 & BIT0
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_X_AXIS_BIT1_BIT0] = USART1->DR;

	// Grab states for Y-AXIS BIT7 & BIT6
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_Y_AXIS_BIT7_BIT6] = USART1->DR;

	// Grab states for Y-AXIS BIT5 & BIT4
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_Y_AXIS_BIT5_BIT4] = USART1->DR;

	// Grab states for Y-AXIS BIT3 & BIT2
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_Y_AXIS_BIT3_BIT2] = USART1->DR;

	// Grab states for Y-AXIS BIT1 & BIT0
	while(!(USART1->SR & USART_SR_RXNE)){};
	controllerResponse[N64_Y_AXIS_BIT1_BIT0] = USART1->DR;

	// Disable the receiver
	USART1->CR1 &= ~USART_CR1_RE;

	/* Debugging code for logic analyzer
	 * To use this, simply uncomment it, hook up your n64 data
	 * line to a logic analyzer and observe a byte sent right
	 * after the n64 controller responds.
	 */
//	HAL_Delay(2);
//	while(!(USART1->SR & USART_SR_TXE)){};
//	USART1->DR = (<byte to send> & 0xFF);
//	while(!(USART1->SR & USART_SR_TC)){};
//	HAL_Delay(5);
}
