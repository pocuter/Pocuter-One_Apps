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
    }
    
    lastButtonState = currentButtonState;
}

uint8_t getInput(int bt) {
    if (0 <= bt && bt < BUTTON_COUNT)
        return buttonHandler[bt].input;
    return 0;
}

double interpolate(double a, double b, double s) {
    if (s < 0.0) s = 0.0;
    if (s > 1.0) s = 1.0;
    return a*(1.0-s) + b*s;
}

UG_COLOR interpolateColorRGB888(UG_COLOR ca, UG_COLOR cb, double s) {
    if (s < 0.0) s = 0.0;
    if (s > 1.0) s = 1.0;
    double t = 1.0 - s;
    
    uint32_t ra = (ca >> 16) & 0xFF;
    uint32_t ga = (ca >>  8) & 0xFF;
    uint32_t ba = (ca      ) & 0xFF;
    uint32_t rb = (cb >> 16) & 0xFF;
    uint32_t gb = (cb >>  8) & 0xFF;
    uint32_t bb = (cb      ) & 0xFF;

    uint32_t r = ra*t + rb*s;
    uint32_t g = ga*t + gb*s;
    uint32_t b = ba*t + bb*s;

    return (r << 16) | (g << 8) | b;
}

void UG_DrawPixelArray(int16_t x0, int16_t y0, int16_t x1, int16_t y1, UG_COLOR *pixels) {
    int i = 0;
    for (int y = y0; y <= y1; y += 1) {
        for (int x = x0; x <= x1; x += 1, i += 1) {
            pocuter->ugui->UG_DrawPixel(x, y, pixels[i]);
        }
    }
}
