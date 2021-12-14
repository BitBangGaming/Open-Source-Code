#ifndef IO_MAPPING_STM32F411CE_BLACKPILL_WEACTSTUDIO_V3_0_H_
#define IO_MAPPING_STM32F411CE_BLACKPILL_WEACTSTUDIO_V3_0_H_

/* Pins for leds */
#define BLUE_LED_PIN		(13U)
#define BLUE_LED_PIN_HAL	(GPIO_PIN_13)
#define BLUE_LED_PORT 		(GPIOC)
#define BLUE_LED_BIT 		(1 << BLUE_LED_PIN)

/* Pins for expansion port communication */
#define COMMS_TX_PIN		(6U)
#define COMMS_TX_PIN_HAL	(GPIO_PIN_6)
#define COMMS_TX_PORT 		(GPIOC)
#define COMMS_TX_BIT 		(1 << COMMS_TX_PIN)

#define COMMS_RX_PIN		(7U)
#define COMMS_RX_PIN_HAL	(GPIO_PIN_7)
#define COMMS_RX_PORT 		(GPIOC)
#define COMMS_RX_BIT 		(1 << COMMS_RX_PIN)

/* Pins for GC communication */
#define GC_STOP_PIN			(5U)
#define GC_STOP_PIN_HAL		(GPIO_PIN_5)
#define GC_STOP_PORT 		(GPIOB)
#define GC_STOP_BIT 		(1 << GC_STOP_PIN)
#define GC_STOP_SET 		(GPIO_BSRR_BS5)
#define GC_STOP_CLEAR 		(GPIO_BSRR_BR5)

#define GC_TX_PIN			(6U)
#define GC_TX_PIN_HAL		(GPIO_PIN_6)
#define GC_TX_PORT 			(GPIOB)
#define GC_TX_BIT 			(1 << GC_TX_PIN)

#define GC_RX_PIN			(7U)
#define GC_RX_PIN_HAL		(GPIO_PIN_7)
#define GC_RX_PORT 			(GPIOB)
#define GC_RX_BIT 			(1 << GC_RX_PIN)

/* Pins for button inputs */
#define BUTTON_A_PIN			(12U)
#define BUTTON_A_PIN_HAL		(GPIO_PIN_12)
#define BUTTON_A_PORT 			(GPIOB)
#define BUTTON_A_BIT 			(1 << BUTTON_A_PIN)
#define BUTTON_A_SET 			(GPIO_BSRR_BS12)
#define BUTTON_A_CLEAR 			(GPIO_BSRR_BR12)

#define BUTTON_B_PIN			(13U)
#define BUTTON_B_PIN_HAL		(GPIO_PIN_13)
#define BUTTON_B_PORT			(GPIOB)
#define BUTTON_B_BIT 			(1 << BUTTON_B_PIN)
#define BUTTON_B_SET 			(GPIO_BSRR_BS13)
#define BUTTON_B_CLEAR 			(GPIO_BSRR_BR13)

#define BUTTON_X_PIN			(14U)
#define BUTTON_X_PIN_HAL		(GPIO_PIN_14)
#define BUTTON_X_PORT 			(GPIOB)
#define BUTTON_X_BIT 			(1 << BUTTON_X_PIN)
#define BUTTON_X_SET 			(GPIO_BSRR_BS14)
#define BUTTON_X_CLEAR 			(GPIO_BSRR_BR14)

#define BUTTON_Y_PIN			(15U)
#define BUTTON_Y_PIN_HAL		(GPIO_PIN_15)
#define BUTTON_Y_PORT 			(GPIOB)
#define BUTTON_Y_BIT 			(1 << BUTTON_Y_PIN)
#define BUTTON_Y_SET 			(GPIO_BSRR_BS15)
#define BUTTON_Y_CLEAR 			(GPIO_BSRR_BR15)

#define BUTTON_L_PIN			(8U)
#define BUTTON_L_PIN_HAL		(GPIO_PIN_8)
#define BUTTON_L_PORT 			(GPIOA)
#define BUTTON_L_BIT 			(1 << BUTTON_L_PIN)
#define BUTTON_L_SET 			(GPIO_BSRR_BS8)
#define BUTTON_L_CLEAR 			(GPIO_BSRR_BR8)

#define BUTTON_R_PIN			(9U)
#define BUTTON_R_PIN_HAL		(GPIO_PIN_9)
#define BUTTON_R_PORT 			(GPIOA)
#define BUTTON_R_BIT 			(1 << BUTTON_R_PIN)
#define BUTTON_R_SET 			(GPIO_BSRR_BS9)
#define BUTTON_R_CLEAR 			(GPIO_BSRR_BR9)

#define BUTTON_Z_PIN			(10U)
#define BUTTON_Z_PIN_HAL		(GPIO_PIN_10)
#define BUTTON_Z_PORT 			(GPIOA)
#define BUTTON_Z_BIT 			(1 << BUTTON_Z_PIN)
#define BUTTON_Z_SET 			(GPIO_BSRR_BS10)
#define BUTTON_Z_CLEAR 			(GPIO_BSRR_BR10)

#define BUTTON_START_PIN		(11U)
#define BUTTON_START_PIN_HAL	(GPIO_PIN_11)
#define BUTTON_START_PORT 		(GPIOA)
#define BUTTON_START_BIT 		(1 << BUTTON_START_PIN)
#define BUTTON_START_SET 		(GPIO_BSRR_BS11)
#define BUTTON_START_CLEAR 		(GPIO_BSRR_BR11)

#define BUTTON_DU_PIN			(5U)
#define BUTTON_DU_PIN_HAL		(GPIO_PIN_5)
#define BUTTON_DU_PORT 			(GPIOA)
#define BUTTON_DU_BIT 			(1 << BUTTON_DU_PIN)
#define BUTTON_DU_SET 			(GPIO_BSRR_BS5)
#define BUTTON_DU_CLEAR 		(GPIO_BSRR_BR5)

#define BUTTON_DD_PIN			(4U)
#define BUTTON_DD_PIN_HAL		(GPIO_PIN_4)
#define BUTTON_DD_PORT 			(GPIOA)
#define BUTTON_DD_BIT 			(1 << BUTTON_DD_PIN)
#define BUTTON_DD_SET 			(GPIO_BSRR_BS4)
#define BUTTON_DD_CLEAR 		(GPIO_BSRR_BR4)

#define BUTTON_DL_PIN			(1U)
#define BUTTON_DL_PIN_HAL		(GPIO_PIN_1)
#define BUTTON_DL_PORT 			(GPIOA)
#define BUTTON_DL_BIT 			(1 << BUTTON_DL_PIN)
#define BUTTON_DL_SET 			(GPIO_BSRR_BS1)
#define BUTTON_DL_CLEAR 		(GPIO_BSRR_BR1)

#define BUTTON_DR_PIN			(0U)
#define BUTTON_DR_PIN_HAL		(GPIO_PIN_0)
#define BUTTON_DR_PORT 			(GPIOA)
#define BUTTON_DR_BIT 			(0 << BUTTON_DR_PIN)
#define BUTTON_DR_SET 			(GPIO_BSRR_BS0)
#define BUTTON_DR_CLEAR 		(GPIO_BSRR_BR0)

#define BUTTON_LSU_PIN			(1U)
#define BUTTON_LSU_PIN_HAL		(GPIO_PIN_1)
#define BUTTON_LSU_PORT 		(GPIOB)
#define BUTTON_LSU_BIT 			(1 << BUTTON_LSU_PIN)
#define BUTTON_LSU_SET 			(GPIO_BSRR_BS1)
#define BUTTON_LSU_CLEAR 		(GPIO_BSRR_BR1)

#define BUTTON_LSD_PIN			(0U)
#define BUTTON_LSD_PIN_HAL		(GPIO_PIN_0)
#define BUTTON_LSD_PORT 		(GPIOB)
#define BUTTON_LSD_BIT 			(1 << BUTTON_LSD_PIN)
#define BUTTON_LSD_SET 			(GPIO_BSRR_BS0)
#define BUTTON_LSD_CLEAR 		(GPIO_BSRR_BR0)

#define BUTTON_LSL_PIN			(7U)
#define BUTTON_LSL_PIN_HAL		(GPIO_PIN_7)
#define BUTTON_LSL_PORT 		(GPIOA)
#define BUTTON_LSL_BIT 			(1 << BUTTON_LSL_PIN)
#define BUTTON_LSL_SET 			(GPIO_BSRR_BS7)
#define BUTTON_LSL_LSLEAR 		(GPIO_BSRR_BR7)

#define BUTTON_LSR_PIN			(6U)
#define BUTTON_LSR_PIN_HAL		(GPIO_PIN_6)
#define BUTTON_LSR_PORT 		(GPIOA)
#define BUTTON_LSR_BIT 			(0 << BUTTON_LSR_PIN)
#define BUTTON_LSR_SET 			(GPIO_BSRR_BS6)
#define BUTTON_LSR_CLEAR 		(GPIO_BSRR_BR6)

#define BUTTON_CU_PIN			(11U)
#define BUTTON_CU_PIN_HAL		(GPIO_PIN_11)
#define BUTTON_CU_PORT 			(GPIOA)
#define BUTTON_CU_BIT 			(1 << BUTTON_CU_PIN)
#define BUTTON_CU_SET 			(GPIO_BSRR_BS11)
#define BUTTON_CU_CLEAR 		(GPIO_BSRR_BR11)

#define BUTTON_CD_PIN			(12U)
#define BUTTON_CD_PIN_HAL		(GPIO_PIN_12)
#define BUTTON_CD_PORT 			(GPIOA)
#define BUTTON_CD_BIT 			(1 << BUTTON_CD_PIN)
#define BUTTON_CD_SET 			(GPIO_BSRR_BS12)
#define BUTTON_CD_CLEAR 		(GPIO_BSRR_BR12)

#define BUTTON_CL_PIN			(3U)
#define BUTTON_CL_PIN_HAL		(GPIO_PIN_3)
#define BUTTON_CL_PORT 			(GPIOB)
#define BUTTON_CL_BIT 			(1 << BUTTON_CL_PIN)
#define BUTTON_CL_SET 			(GPIO_BSRR_BS3)
#define BUTTON_CL_CLEAR 		(GPIO_BSRR_BR3)

#define BUTTON_CR_PIN			(4U)
#define BUTTON_CR_PIN_HAL		(GPIO_PIN_4)
#define BUTTON_CR_PORT 			(GPIOB)
#define BUTTON_CR_BIT 			(0 << BUTTON_CR_PIN)
#define BUTTON_CR_SET 			(GPIO_BSRR_BS4)
#define BUTTON_CR_CLEAR 		(GPIO_BSRR_BR4)

#define BUTTON_MACRO_PIN		(15U)
#define BUTTON_MACRO_PIN_HAL	(GPIO_PIN_15)
#define BUTTON_MACRO_PORT 		(GPIOC)
#define BUTTON_MACRO_BIT 		(1 << BUTTON_MACRO_PIN)
#define BUTTON_MACRO_SET 		(GPIO_BSRR_BS15)
#define BUTTON_MACRO_CLEAR 		(GPIO_BSRR_BR15)

#define BUTTON_TILT_PIN			(14U)
#define BUTTON_TILT_PIN_HAL		(GPIO_PIN_14)
#define BUTTON_TILT_PORT 		(GPIOC)
#define BUTTON_TILT_BIT 		(1 << BUTTON_TILT_X_PIN)
#define BUTTON_TILT_SET 		(GPIO_BSRR_BS14)
#define BUTTON_TILT_CLEAR 		(GPIO_BSRR_BR14)


#endif
