#ifndef _ICONS_H_
#define _ICONS_H_

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

enum IconType {
    ICON_APPS,
    ICON_APPSTORE,
    ICON_SETTINGS,
    ICON_DEVICE_INFO,
    ICON_START,
    ICON_UPDATE,
    ICON_DELETE,
    ICON_AUTHOR,
    ICON_VERSION,
    ICON_INSTALLED,
    ICON_DOWNLOAD,
    ICON_PREVPAGE,
    ICON_NEXTPAGE,
    ICON_DATEANDTIME,
    ICON_TIMEZONE,
    ICON_SHOWDATE,
    ICON_SYSTEMCOLOR,
    ICON_BRIGHTNESS,
    ICON_STANDBY,
    ICON_WIFI_AP,
    ICON_WIFI_WPS,
    ICON_WIFI_SCAN,
    ICON_RESTART,
    ICON_WIFI_FULL,
    ICON_WIFI_GOOD,
    ICON_WIFI_OKAY,
    ICON_WIFI_BAD,
    ICON_INFO_DEVICEID,
    ICON_MISSING,
    
    ICON_BRIGHTNESS_DARK,
    ICON_BRIGHTNESS_BRIGHT,
    
    TITLE_WIFI_DISCONNECTED,
    TITLE_WIFI_BAD,
    TITLE_WIFI_OKAY,
    TITLE_WIFI_GOOD,
    
    ICON_MAX
};

struct Icon {
    uint8_t width, height;
    const char *name;
    UG_COLOR *data;
};

// ========================================
// PROTOTYPES
// ========================================

extern void loadIcon(IconType icon);
extern Icon* getIcon(IconType icon);
extern void drawIcon(Icon *icon, int16_t x, int16_t y);

// ========================================
// GLOBALS
// ========================================

// ========================================
// FUNCTIONS
// ========================================

#endif //_ICONS_H_
