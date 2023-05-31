#ifndef _POPUP_H_
#define _POPUP_H_
// ========================================
// INCLUDES
// ========================================

#include "system.h"

// ========================================
// MACROS
// ========================================

#define MAX_DESCRIPTION_LINES   5

// ========================================
// TYPES
// ========================================

enum PopupType {
    PUPUP_TYPE_UPDATE_AVAILABLE = 0,
    PUPUP_TYPE_APPS_AVAILABLE,
    POPUP_TYPE_SD_PULLED_OUT,
    POPUP_TYPE_SD_PLUGGED_IN,
    POPUP_TYPE_BATTERY_LOW,

    POPUP_ENUM_COUNT
};

struct Popup {
    PopupType type;
    const char *title;
    const char *description[MAX_DESCRIPTION_LINES];
    
    const char *textAccept;
    const char *textDismiss;

    bool (*checkFunc)();
    void (*acceptFunc)();
    void (*dismissFunc)();
};

// ========================================
// PROTOTYPES
// ========================================

extern void checkForPopups();
extern bool handlePopup();

// ========================================
// GLOBALS
// ========================================

// ========================================
// FUNCTIONS
// ========================================

#endif //_POPUP_H_
