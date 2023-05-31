// ========================================
// INCLUDES
// ========================================

#include "menu_mainmenu.h"
#include "menu_downloadapps.h"
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

void createMenuDownloadApps();
void downloadNextApp();
void exitMenuDownloadAppSuccess();
void exitMenuDownloadAppFailure();

// ========================================
// GLOBALS
// ========================================

Menu menu_downloadapps = {
    "Checking...",          // title
    NULL,                   // icon
    NULL,                   // parent menu
    false,                  // can loop items
    
    createMenuDownloadApps, // create func
    NULL,                   // update func
    NULL                    // draw func
};

uint64_t appDownloadID;

// ========================================
// FUNCTIONS
// ========================================

void createMenuDownloadApps() {
    downloadNextApp();
}

void downloadNextApp() {
    const uint8_t *downloadURL = pocuter->Server->checkForAppInstallRequest(appDownloadID);
    
    if (downloadURL == NULL) {
        initMenuChange(&menu_mainmenu);
        return;
    }
    
    downloadAppUpdate(appDownloadID, downloadURL, exitMenuDownloadAppSuccess, exitMenuDownloadAppFailure, true);
}

void exitMenuDownloadAppSuccess() {
    // draw "synchronizing" 
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX, sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    const char* str = "Synchronizing...";
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_GREEN);
	gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str)/2, sizeY*3/6, str);
    
    forceScreenUpdate();
    
    if (pocuter->Server->appInstalledSuccessfully(appDownloadID)) {
        downloadNextApp();
    } else {
        initMenuChange(&menu_mainmenu);
    }
}

void exitMenuDownloadAppFailure() {
    initMenuChange(&menu_mainmenu);
}