#ifndef SHARED_ENUMS_H_
#define SHARED_ENUMS_H_

typedef enum
{
	PUSHED = 0,
	RELEASED = 1
} ButtonState_t;

typedef enum
{
	LED_ON = 0,
	LED_OFF = 1,
	LED_TOGGLE = 2
} LedState_t;

typedef enum
{
	GC_A = 0,
	GC_B = 1,
	GC_X = 2,
	GC_Y = 3,
	GC_L = 4,
	GC_R = 5,
	GC_Z = 6,
	GC_START = 7,
	GC_DPAD_UP = 8,
	GC_DPAD_DOWN = 9,
	GC_DPAD_LEFT = 10,
	GC_DPAD_RIGHT = 11,
	GC_MAIN_STICK_UP = 12,
	GC_MAIN_STICK_DOWN = 13,
	GC_MAIN_STICK_LEFT = 14,
	GC_MAIN_STICK_RIGHT = 15,
	GC_C_STICK_UP = 16,
	GC_C_STICK_DOWN = 17,
	GC_C_STICK_LEFT = 18,
	GC_C_STICK_RIGHT = 19,
	GC_MACRO = 20,
	GC_TILT = 21
} GCButtonInput_t;

#endif
