#ifndef _MENU_UPDATEAPP_H_
#define _MENU_UPDATEAPP_H_
// ========================================
// INCLUDES
// ========================================

#include "../../menu.h"

// ========================================
// MACROS
// ========================================

#define APP_SD_LOCATION_FOLDER	"/sd/apps/%d"							// the folder
#define APP_SD_LOCATION_STAGED  "/sd/apps/%d/esp32c3_staged.app"		// the location where the menu binary should be downloaded to
#define APP_SD_LOCATION_FINAL   "/sd/apps/%d/esp32c3.app"				// the actual location the binary should be stored after successfully downloading
#define APP_SD_LOCATION_BACKUP  "/sd/apps/%d/esp32c3_backup.app"		// the location the old binary gets moved to, serves as backup

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

extern void downloadAppUpdate(uint64_t id, const uint8_t *url, void (*exitSuccess)(), void (*exitFailure)(), bool autoLeaveOnSuccess = false);

// ========================================
// GLOBALS
// ========================================

// ========================================
// FUNCTIONS
// ========================================


#endif //_MENU_UPDATEAPP_H_
