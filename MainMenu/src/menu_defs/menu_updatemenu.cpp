// ========================================
// INCLUDES
// ========================================

#include "menu_settings.h"
#include "menu_updatemenu.h"
#include "menu_appupdate.h"


// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuUpdateMenu();
void exitMenuUpdateMenuSuccess();
void exitMenuUpdateMenuFailure();

// ========================================
// GLOBALS
// ========================================

Menu menu_updatemenu = {
    "Checking...",          // title
    NULL,                   // icon
    NULL,                   // parent menu
    false,                  // can loop items
    
    createMenuUpdateMenu,   // create func
    NULL,                   // update func
    NULL                    // draw func
};

// ========================================
// FUNCTIONS
// ========================================

void createMenuUpdateMenu() {
	const uint8_t *updateURL = pocuter->Server->checkNewestAppVersion(1);
	downloadAppUpdate(1, updateURL, exitMenuUpdateMenuSuccess, exitMenuUpdateMenuFailure);
}

void exitMenuUpdateMenuSuccess() {
	pocuter->OTA->setNextAppID(1);
	pocuter->OTA->forceBootloaderToReflashApp();
	pocuter->OTA->restart();
}

void exitMenuUpdateMenuFailure() {
	initMenuChange(&menu_settings);
}