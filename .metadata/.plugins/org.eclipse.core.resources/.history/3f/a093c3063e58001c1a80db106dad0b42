#include "main.h"

int main(void)
{
	/* Initialize all required modules and clocks */
	HAL_Init();
	SystemClock_Config();
	CommsN64Console_Init();
	Main_Init();

	/* Apply a pullup on n64 data line */
	CommsN64Console_SetPullup(N64_PULLUP);

	/* Give some delay for initialization */
	HAL_Delay(100);

	/* Main application loop */
    while(1)
    {
    	// Add about 100us of delay (mcu clock dependent)
    	volatile uint32_t counter = 200;
    	while(counter--);

    	// Poll for n64 controller state
    	CommsN64Console_GetContollerInputs();

    	// Parse inputs from controller
    	CommsN64Console_ParseContollerInputs();

    	// Update registers for BBG control board
    	Main_SetRegisters();

    	// Send registers to BBG control board
		HAL_Delay(2);
		while(!(USART1->SR & USART_SR_TXE)){};
		USART1->DR = (registerMSIlower & 0xFF);
		while(!(USART1->SR & USART_SR_TC)){};
		HAL_Delay(2);
    }
}

// Setters
void Main_SetBlueLed(GPIO_PinState pinState)
{
	HAL_GPIO_WritePin(BLUE_LED_PORT, BLUE_LED_PIN_HAL, pinState);
}

void Main_SetRegisters()
{
	// Helper variable
	uint8_t bitState;

	/* Update the MSI upper byte register */
	// BIT7
	registerMSIupper |= (1 << MSI_UPPER_BIT7);

	// HOME
	bitState = (controllerRegisters[0] >> N64_BYTE1_Z) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_HOME);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_HOME);
	}

	// SELECT
	registerMSIupper &= ~(1 << MSI_UPPER_SELECT);

	// START
	bitState = (controllerRegisters[0] >> N64_BYTE1_START) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_START);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_START);
	}

	// UP
	bitState = (controllerRegisters[0] >> N64_BYTE1_DU) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_UP);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_UP);
	}

	// DOWN
	bitState = (controllerRegisters[0] >> N64_BYTE1_DD) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_DOWN);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_DOWN);
	}

	// RIGHT
	bitState = (controllerRegisters[0] >> N64_BYTE1_DR) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_RIGHT);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_RIGHT);
	}

	// LEFT
	bitState = (controllerRegisters[0] >> N64_BYTE1_DL) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIupper &= ~(1 << MSI_UPPER_LEFT);
	}
	else
	{
		registerMSIupper |= (1 << MSI_UPPER_LEFT);
	}

	// 4K
	bitState = (controllerRegisters[1] >> N64_BYTE2_L) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_4K);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_4K);
	}

	// 3K
	bitState = (controllerRegisters[1] >> N64_BYTE2_CR) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_3K);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_3K);
	}

	// 2K
	bitState = (controllerRegisters[1] >> N64_BYTE2_CD) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_2K);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_2K);
	}

	// 1K
	bitState = (controllerRegisters[0] >> N64_BYTE1_A) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_1K);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_1K);
	}

	// 4P
	bitState = (controllerRegisters[1] >> N64_BYTE2_R) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_4P);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_4P);
	}

	// 3P
	bitState = (controllerRegisters[1] >> N64_BYTE2_CU) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_3P);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_3P);
	}

	// 2P
	bitState = (controllerRegisters[1] >> N64_BYTE2_CL) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_2P);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_2P);
	}

	// 1P
	bitState = (controllerRegisters[0] >> N64_BYTE1_B) & (0x01);
	if(bitState == 0x01)
	{
		registerMSIlower &= ~(1 << MSI_LOWER_1P);
	}
	else
	{
		registerMSIlower |= (1 << MSI_LOWER_1P);
	}
}

// Initializer
void Main_Init()
{
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct_Main = {0};

	GPIO_InitStruct_Main.Pin = GPIO_PIN_8;
	GPIO_InitStruct_Main.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct_Main.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Main.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct_Main.Alternate = GPIO_AF0_MCO;
	/* Only enable below if you want to check the clock
	 * frequency.
	 */
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Main);

	/* Initialize the blue led */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct_Main.Pin = BLUE_LED_PIN_HAL;
	GPIO_InitStruct_Main.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_Main.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Main.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(BLUE_LED_PORT, &GPIO_InitStruct_Main);

	/* Keep the blue LED on to show the program is running. */
	Main_SetBlueLed(BLUE_LED_ON);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
//  {
//    Error_Handler();
//  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}
