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

#define BUTTON_INCREASE 0
#define BUTTON_DECREASE 1
#define BUTTON_NEXT     2

#define ACTION_INCREASE         (getInput(BUTTON_INCREASE) & (SINGLE_CLICK | HOLD))
#define ACTION_DECREASE         (getInput(BUTTON_DECREASE) & (SINGLE_CLICK | HOLD))
#define ACTION_NEXT             (getInput(BUTTON_NEXT    ) &  SINGLE_CLICK)
#define ACTION_START            (getInput(BUTTON_NEXT    ) &  SINGLE_CLICK)
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
