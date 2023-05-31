#ifndef _SETTINGS_H_
#define _SETTINGS_H_
// ========================================
// INCLUDES
// ========================================

#include "system.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

struct PocuterSettings {
    uint32_t systemColor;
    int brightness;
    bool showDateInTitleBar;
    bool showBatteryInTitleBar;
    uint32_t lastDayAskedForUpdate;
    int timeUntilStandby;
};

// ========================================
// PROTOTYPES
// ========================================

extern char*    getSetting(const char *section, const char *name, const char *defaultValue, char *dest, size_t maxLength);
extern int      getSetting(const char *section, const char *name, int defaultValue);
extern uint32_t getSetting(const char *section, const char *name, uint32_t defaultValue);
extern double   getSetting(const char *section, const char *name, double defaultValue);

extern bool     setSetting(const char *section, const char *name, char *value);
extern bool     setSetting(const char *section, const char *name, int value);
extern bool     setSetting(const char *section, const char *name, uint32_t value);
extern bool     setSetting(const char *section, const char *name, double value);

// ========================================
// GLOBALS
// ========================================

extern PocuterSettings pocuterSettings;

// ========================================
// FUNCTIONS
// ========================================


#endif //_SETTINGS_H_
