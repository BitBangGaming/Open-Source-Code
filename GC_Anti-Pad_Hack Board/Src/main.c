#include "main.h"

int main(void)
{
	/* Start up HAL */
	HAL_Init();

	/* Configure clock to 100 MHz */
	SystemClock_Config();

	/* Start up main application code */
	Main_Init();

	/* Get the GC emulation ready */
	GCControllerEmulation_Init();

	/* Run the GC emulation */
	while(1)
	{
		GCControllerEmulation_Run();
	}
}

/* Main Functions */
void Main_Init()
{
	/* Initialize the blue led */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct_Main = {0};

	GPIO_InitStruct_Main.Pin = BLUE_LED_PIN_HAL;
	GPIO_InitStruct_Main.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_Main.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Main.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(BLUE_LED_PORT, &GPIO_InitStruct_Main);

	/* Blue LED off by default */
	//Main_SetBlueLed(LED_OFF);
}

//void Main_SetBlueLed(LedState_t ledState)
//{
//	/* Process act desired */
//	switch(ledState)
//	{
//		// Turn blue LED on or off
//		case LED_ON:
//		case LED_OFF:
//			HAL_GPIO_WritePin(BLUE_LED_PORT, BLUE_LED_PIN_HAL, ledState);
//			break;
//
//		// Toggle blue LED
//		case LED_TOGGLE:
//			HAL_GPIO_TogglePin(BLUE_LED_PORT, BLUE_LED_PIN_HAL);
//			break;
//
//		// Invalid command
//		default:
//			// Do nothing
//			break;
//	}
//}

void SysTick_Handler(void)
{
	HAL_IncTick();
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    //Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}
