#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "io_mapping_stm32f411ce_blackpill_weactstudio_v3_0.h"
#include "shared_enums.h"

/* Public Functions */
void Main_Init(void);
void Main_SetBlueLed(LedState_t);

#endif
