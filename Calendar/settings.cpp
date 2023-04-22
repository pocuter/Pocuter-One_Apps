#ifndef _MENU_APPINFO_H_
#define _MENU_APPINFO_H_
// ========================================
// INCLUDES
// ========================================

#include "settings.h"
#include "system.h"

// ========================================
// MACROS
// ========================================

#define NUM_BUFFER_SIZE     10

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

// ========================================
// GLOBALS
// ========================================

PocuterSettings pocuterSettings;

char numBuffer[NUM_BUFFER_SIZE];

// ========================================
// FUNCTIONS
// ========================================

bool getSetting(const char *section, const char *name, char *dest, size_t maxLength) {
    PocuterConfig config((const uint8_t *) "settings");
    return config.get((const uint8_t *) section, (const uint8_t *) name, (uint8_t *) dest, maxLength);
}

char* getSetting(const char *section, const char *name, const char *defaultValue, char *dest, size_t maxLength) {
    if (!getSetting(section, name, dest, maxLength)) {
        strncpy(dest, defaultValue, maxLength-1);
        dest[maxLength-1] = '\0';
    }
    return dest;
}

int getSetting(const char *section, const char *name, int defaultValue) {
    if (!getSetting(section, name, numBuffer, NUM_BUFFER_SIZE))
        return defaultValue;

    return atoi(numBuffer);
}

uint32_t getSetting(const char *section, const char *name, uint32_t defaultValue) {
    return getSetting(section, name, (int) defaultValue);
}

double getSetting(const char *section, const char *name, double defaultValue) {
    if (!getSetting(section, name, numBuffer, NUM_BUFFER_SIZE))
        return defaultValue;

    return atof(numBuffer);
}

bool setSetting(const char *section, const char *name, const char *value) {
    PocuterConfig config((const uint8_t *) "settings");
    return config.set((const uint8_t *) section, (const uint8_t *) name, (const uint8_t *) value);
}

bool setSetting(const char *section, const char *name, int value) {
    snprintf(numBuffer, NUM_BUFFER_SIZE, "%d", value);
    return setSetting(section, name, (const char *) numBuffer);
}

bool setSetting(const char *section, const char *name, uint32_t value) {
    snprintf(numBuffer, NUM_BUFFER_SIZE, "%d", value);
    return setSetting(section, name, (const char *) numBuffer);
}

bool setSetting(const char *section, const char *name, double value) {
    snprintf(numBuffer, NUM_BUFFER_SIZE, "%f", value);
    return setSetting(section, name, (const char *) numBuffer);
}

#endif //_MENU_APPINFO_H_
