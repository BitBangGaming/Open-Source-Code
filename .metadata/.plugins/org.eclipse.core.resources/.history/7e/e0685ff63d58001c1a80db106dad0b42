#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "io_mapping_stm32f411ce_blackpill_weactstudio_v3_0.h"
#include "comms_n64_console.h"

// Macros
/* Bit positions for MSI register */
#define MSI_UPPER_LEFT		0
#define MSI_UPPER_RIGHT		1
#define MSI_UPPER_DOWN		2
#define MSI_UPPER_UP		3
#define MSI_UPPER_START		4
#define MSI_UPPER_SELECT	5
#define MSI_UPPER_HOME		6
#define MSI_UPPER_BIT7		7

#define MSI_LOWER_1P	0
#define MSI_LOWER_2P	1
#define MSI_LOWER_3P	2
#define MSI_LOWER_4P	3
#define MSI_LOWER_1K	4
#define MSI_LOWER_2K	5
#define MSI_LOWER_3K	6
#define MSI_LOWER_4K	7

// Private enums
static GPIO_PinState BLUE_LED_ON = GPIO_PIN_RESET;
static GPIO_PinState BLUE_LED_OFF = GPIO_PIN_SET;

// Private variables
/* Structure of MSI upper byte
 *  BIT #: 7 | 6    | 5      | 4     | 3  | 2    | 1     | 0
 * BUTTON: 1 | HOME | SELECT | START | UP | DOWN | RIGHT | LEFT
 *    N64: 1 | Z    | 0      | START | DU | DD   | DR    | DL
 */
static uint8_t registerMSIupper;

/* Structure of MSI lower byte
 *  BIT #: 7  | 6  | 5  | 4  | 3  | 2  | 1  | 0
 * BUTTON: 4K | 3K | 2K | 1K | 4P | 3P | 2P | 1P
 *    N64: L  | CR | CD | A  | R  | CU | CL | B
 */
static uint8_t registerMSIlower;

/* Structure of LSX upper byte
 *  BIT #: 7 | 6 | 5 | 4 | 3       | 2       | 1      | 0
 * BUTTON: 0 | 0 | 0 | 0 | XBIT11  | XBIT10  | XBIT9  | XBIT8
 *    N64: 0 | 0 | 0 | 0 | 'XBIT11 | 'XBIT10 | 'XBIT9 | 'XBIT8
 * Note that the bits from N64 are not one to one. That means
 * these bits need to come from a conversion. In other words,
 * the 8 bit axis value must be converted to a 12 bit axis value.
 */
static uint8_t registerLSXupper;

/* Structure of LSY upper byte
 *  BIT #: 7      | 6      | 5      | 4      | 3      | 2      | 1      | 0
 * BUTTON: XBIT7  | XBIT6  | XBIT5  | XBIT4  | XBIT3  | XBIT2  | XBIT1  | XBIT0
 *    N64: 'XBIT7 | 'XBIT6 | 'XBIT5 | 'XBIT4 | 'XBIT3 | 'XBIT2 | 'XBIT1 | 'XBIT0
 * Note that the bits from N64 are not one to one. That means
 * these bits need to come from a conversion. In other words,
 * the 8 bit axis value must be converted to a 12 bit axis value.
 */
static uint8_t registerLSYupper;

// Initializer
void Main_Init(void);

// Setters
void Main_SetBlueLed(GPIO_PinState);
void Main_SetRegisters(void);

// Misc
void SystemClock_Config(void);

#endif
