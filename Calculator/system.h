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

#define SINGLE_CLICK    0x1
#define DOUBLE_CLICK    0x2
#define HOLD            0x4
#define PRESSED_CONT    0x8

#define BUTTON_SELECT   0
#define BUTTON_RIGHT    1
#define BUTTON_LEFT     2

#define ACTION_LEFT             (getInput(BUTTON_LEFT  ) & (SINGLE_CLICK | HOLD))
#define ACTION_RIGHT            (getInput(BUTTON_RIGHT ) & (SINGLE_CLICK | HOLD))
#define ACTION_SELECT           (getInput(BUTTON_SELECT) & SINGLE_CLICK)
#define ACTION_BACK_TO_MENU    ((getInput(1) & PRESSED_CONT) && (getInput(2) & PRESSED_CONT))

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

// ========================================
// GLOBALS
// ========================================

extern Pocuter* pocuter;
extern double dt;

// ========================================
// FUNCTIONS
// ========================================



#endif //_SYSTEM_H_
