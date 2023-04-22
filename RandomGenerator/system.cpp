// ========================================
// INCLUDES
// ========================================

#include "system.h"

// ========================================
// MACROS
// ========================================

#define BUTTON_COUNT        3
#define CLICK_TIMEOUT_MS    200
#define ENTER_HOLD_MS       300
#define REPEAT_HOLD_MS      100

// ========================================
// TYPES
// ========================================

enum ButtonDetectionState {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_HOLD,
    BUTTON_STATE_SINGLE_CLICK,
    BUTTON_STATE_SINGLE_CLICK_THEN_PRESSED,
};

struct ButtonDetectionHandler {
    ButtonDetectionState state;
    long lastEventTime;
    uint8_t input;
    bool doubleClickEnabled;
};

// ========================================
// PROTOTYPES
// ========================================

// ========================================
// GLOBALS
// ========================================

Pocuter* pocuter;
double dt;
uint8_t lastButtonState;
ButtonDetectionHandler buttonHandler[BUTTON_COUNT];

// ========================================
// FUNCTIONS
// ========================================

void enableDoubleClick(int bt) {
    if (0 <= bt && bt < BUTTON_COUNT)
        buttonHandler[bt].doubleClickEnabled = true;
}

void disableDoubleClick(int bt) {
    if (0 <= bt && bt < BUTTON_COUNT)
        buttonHandler[bt].doubleClickEnabled = false;
}

void updateInput() {
    long ms = millis();
    uint8_t currentButtonState = pocuter->Buttons->getButtonState();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        ButtonDetectionHandler *handler = &buttonHandler[i];
        
        bool currentState = currentButtonState & (1 << i);
        bool lastState    = lastButtonState    & (1 << i);
        
        bool pressed  = !lastState &&  currentState;
        bool released =  lastState && !currentState;

        handler->input = 0;

        ButtonDetectionState newState = handler->state;
        switch (handler->state) {
            default:
            case BUTTON_STATE_IDLE:
                if (pressed)
                    newState = BUTTON_STATE_PRESSED;
                break;
            case BUTTON_STATE_PRESSED:
                if (released) {
                    if (!handler->doubleClickEnabled) {
                        handler->input = SINGLE_CLICK;
                        newState = BUTTON_STATE_IDLE;
                    } else
                        newState = BUTTON_STATE_SINGLE_CLICK;
                } else {
                    if (ms - handler->lastEventTime > ENTER_HOLD_MS) {
                        handler->input = HOLD;
                        newState = BUTTON_STATE_HOLD;
                    }
                }
                break;
            case BUTTON_STATE_HOLD:
                if (released) {
                    if (!handler->doubleClickEnabled) {
                        handler->input = SINGLE_CLICK;
                        newState = BUTTON_STATE_IDLE;
                    } else
                        newState = BUTTON_STATE_SINGLE_CLICK;
                } else {
                    if (ms - handler->lastEventTime > REPEAT_HOLD_MS) {
                        handler->input = HOLD;
                    }
                }
                break;
            case BUTTON_STATE_SINGLE_CLICK:
                if (pressed)
                    newState = BUTTON_STATE_SINGLE_CLICK_THEN_PRESSED;
                if (ms - handler->lastEventTime > CLICK_TIMEOUT_MS) {
                    handler->input = SINGLE_CLICK;
                    newState = BUTTON_STATE_IDLE;
                }
                break;
            case BUTTON_STATE_SINGLE_CLICK_THEN_PRESSED:
                if (released) {
                    handler->input = DOUBLE_CLICK;
                    newState = BUTTON_STATE_IDLE;
                }
                break;
        }
        if (handler->state != newState || handler->input != 0)
            handler->lastEventTime = millis();
        handler->state = newState;

        if (handler->state == BUTTON_STATE_PRESSED || handler->state == BUTTON_STATE_HOLD || handler->state == BUTTON_STATE_SINGLE_CLICK_THEN_PRESSED)
            handler->input |= PRESSED_CONT;
    }
    
    lastButtonState = currentButtonState;
}

uint8_t getInput(int bt) {
    if (0 <= bt && bt < BUTTON_COUNT)
        return buttonHandler[bt].input;
    return 0;
}
