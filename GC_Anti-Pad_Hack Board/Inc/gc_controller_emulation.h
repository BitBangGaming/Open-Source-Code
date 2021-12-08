#ifndef GC_CONTROLLER_EMULATION_H_
#define GC_CONTROLLER_EMULATION_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "io_mapping_stm32f411ce_blackpill_weactstudio_v3_0.h"
#include "shared_enums.h"

// Notes //
/* NOTE 1:
 * This module will emulate a GC controller. It currently will
 * only process some commands from the console, and that is the
 * PROBE, PROBE_ORIGIN, and POLL commands. Other commands can be
 * easily extended but there might be a slight change to how the
 * input data is stored.
 *
 * The trick this module employs is that it uses a UART to emulate
 * the GC controller protocol, which is 1-wire, going at a
 * pretty fast speed.
 */

/* NOTE 2:
 * ~ PROBE Command ~
 * This command sent from the console as: 0x00, STOP.
 * When responding to a PROBE command, below is the controller
 * response byte structure. MSB is sent first. Terminate with
 * stop bit after all bytes are sent.
 *
 * BYTE 0: 0x09
 * BYTE 1: 0x00
 * BYTE 2: 0x03
 */

/* NOTE 3:
 * ~ PROBE ORIGIN Command ~
 * This command sent from the console as: 0x41, STOP.
 * When responding to a PROBE ORIGIN command, below is the
 * controller response byte structure. MSB is sent first.
 * Terminate with stop bit after all bytes are sent. Terminate
 * with stop bit after all bytes are sent.
 *
 *  BIT  #: 7     | 6       | 5 | 4     | 3  | 2  | 1 | 0
 * BYTE  0: 0     | 0       | 0 | START | Y  | X  | B | A
 * BYTE  1: 1     | L       | R | Z     | DU | DD | DR| DL
 * BYTE  2:               X AXIS: MAIN STICK
 * BYTE  3:               Y AXIS: MAIN STICK
 * BYTE  4:               X AXIS: C STICK
 * BYTE  5:               Y AXIS: C STICK
 * BYTE  6:               L TRIGGER
 * BYTE  7:               R TRIGGER
 * BYTE  8:                  0x00
 * BYTE  9:                  0x00
 */

/* NOTE 4:
 * ~ POLL Command ~
 * This command sent from the console as: 0x40, 0x03, (0x00 or 0x01), STOP.
 * 0x00 means turn rumble off, and 0x01 means turns rumble on.
 * When responding to a POLL command, below is the
 * controller response byte structure. MSB is sent first.
 * Terminate with stop bit after all bytes are sent. Terminate
 * with stop bit after all bytes are sent.
 *
 *  BIT  #: 7     | 6       | 5 | 4     | 3  | 2  | 1 | 0
 * BYTE  0: 0     | 0       | 0 | START | Y  | X  | B | A
 * BYTE  1: 1     | L       | R | Z     | DU | DD | DR| DL
 * BYTE  2:               MAIN STICK X AXIS
 * BYTE  3:               MAIN STICK Y AXIS
 * BYTE  4:               C STICK X AXIS
 * BYTE  5:               C STICK Y AXIS
 * BYTE  6:               L TRIGGER
 * BYTE  7:               R TRIGGER
 */

/* Note 5:
 * There is some error when receiving bytes. For example if you might get
 * 0xE8 or 0xC8. The error is timing related and baud rate dependent.
 * Even though there is an error, and a different byte is received,
 * it still means the same bit pattern pair so its error tolerant.
 *
 * Since a UART byte is two GC bits, the left bit is send first and
 * the right bit is sent second. For example 01 as bit pair. This means
 * Z = 0 & START = 1. Or Z = RELEASED & START = PUSHED.
 */

// Public Macros //
/* Byte order for GC console response */
#define GC_CONSOLE_BYTE_0_BIT7_BIT6	0
#define GC_CONSOLE_BYTE_0_BIT5_BIT4	1
#define GC_CONSOLE_BYTE_0_BIT3_BIT2	2
#define GC_CONSOLE_BYTE_0_BIT1_BIT0	3
#define GC_CONSOLE_BYTE_1_BIT7_BIT6	4
#define GC_CONSOLE_BYTE_1_BIT5_BIT4	5
#define GC_CONSOLE_BYTE_1_BIT3_BIT2	6
#define GC_CONSOLE_BYTE_1_BIT1_BIT0	7
#define GC_CONSOLE_BYTE_2_BIT7_BIT6	8
#define GC_CONSOLE_BYTE_2_BIT5_BIT4	9
#define GC_CONSOLE_BYTE_2_BIT3_BIT2	10
#define GC_CONSOLE_BYTE_2_BIT1_BIT0	11
#define GC_CONSOLE_TEMP_BITS		12

/* Number of maximum UART bytes to receive from console */
#define MAX_GC_CONSOLE_BYTES	13

// Public Function Prototypes //
/* Call before using this module */
void GCControllerEmulation_Init(void);

/* Call to run the controller emulation forever */
void GCControllerEmulation_Run(void);

/* Get all button states*/
void GCControllerEmulation_GetSwitchSnapshot(void);

/* Get a particular button state */
ButtonState_t GCControllerEmulation_GetButtonState(GCButtonInput_t);

#endif /* GC_CONTROLLER_EMULATION_H_ */
