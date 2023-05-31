#ifndef _SYSTEM_H_
#define _SYSTEM_H_
// ========================================
// INCLUDES
// ========================================

#include <Pocuter.h>
#include <Arduino.h>

// ========================================
// MACROS
// ========================================

#define MENU_VERSION_MAJOR  0
#define MENU_VERSION_MINOR  8
#define MENU_VERSION_PATCH  8

#define SINGLE_CLICK    0x1
#define DOUBLE_CLICK    0x2
#define HOLD            0x4

#define BUTTON_UP       0
#define BUTTON_DOWN     1
#define BUTTON_OK       2
#define BUTTON_ACCEPT   1
#define BUTTON_DISMISS  2

#define ACTION_UP       (getInput(BUTTON_UP     ) & (SINGLE_CLICK | HOLD))
#define ACTION_DOWN     (getInput(BUTTON_DOWN   ) & (SINGLE_CLICK | HOLD))
#define ACTION_OK       (getInput(BUTTON_OK     ) & SINGLE_CLICK)
#define ACTION_BACK     (getInput(BUTTON_OK     ) & DOUBLE_CLICK)
#define ACTION_ACCEPT   (getInput(BUTTON_ACCEPT ) & SINGLE_CLICK)
#define ACTION_DISMISS  (getInput(BUTTON_DISMISS) & SINGLE_CLICK)

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

extern void enableDoubleClick(int bt);
extern void disableDoubleClick(int bt);
extern void updateInput();
extern uint8_t getInput(int bt);

extern double interpolate(double a, double b, double s);
extern UG_COLOR interpolateColorRGB888(UG_COLOR a, UG_COLOR b, double s);

extern void UG_DrawPixelArray(int16_t x0, int16_t y0, int16_t x1, int16_t y1, UG_COLOR *pixels);

// ========================================
// GLOBALS
// ========================================

extern Pocuter* pocuter;
extern double dt;

// ========================================
// FUNCTIONS
// ========================================

#endif //_SYSTEM_H_
