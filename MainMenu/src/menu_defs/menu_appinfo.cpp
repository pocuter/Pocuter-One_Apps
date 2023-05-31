// ========================================
// INCLUDES
// ========================================

#include "menu_appinfo.h"
#include "menu_apps.h"
#include "menu_appupdate.h"
#include "menu_updatemenu.h"
#include "../../settings.h"

// ========================================
// MACROS
// ========================================

#define MAX_APP_INFO_LENGTH		64

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuAppInfo();
void startApp(MenuItem *item);
void updateApp(MenuItem *item);
void deleteApp(MenuItem *item);

// ========================================
// GLOBALS
// ========================================

Menu menu_appinfo = {
    "",                     // title
    NULL,                   // icon
    &menu_apps,             // parent menu
    false,                  // can loop items
    
    createMenuAppInfo,      // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_startapp = {
    "Start",                // name
    ICON_START,             // icon
    
    NULL,                   // next menu
    startApp,               // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_updateapp = {
    "Update",               // name
    ICON_UPDATE,            // icon
    
    NULL,                   // next menu
    updateApp,              // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_deleteapp = {
    "Delete",               // name
    ICON_DELETE,            // icon
    
    NULL,                   // next menu
    deleteApp,              // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_author = {
    "Author:",              // name
    ICON_AUTHOR,            // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

MenuItem menuitem_version = {
    "Version:",             // name
    ICON_VERSION,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

uint64_t appID;
const char *appName;
char author [MAX_APP_INFO_LENGTH];
char version[MAX_APP_INFO_LENGTH];
const uint8_t *updateURL;

// ========================================
// FUNCTIONS
// ========================================

void setAppInfoIDAndName(uint64_t id, const char *name) {
	appID = id;
	appName = name;
}

void createMenuAppInfo() {
	showPleaseWaitScreen();
	
	menu_appinfo.title = appName;
	menuitem_author.dataStr = author;
	menuitem_version.dataStr = version;
	
	addMenuItem(&menuitem_startapp);
	
	updateURL = pocuter->Server->checkNewestAppVersion(appID);
	if (updateURL != NULL) {
		addMenuItem(&menuitem_updateapp);
	}
	
	//addMenuItem(&menuitem_deleteapp);
	
	PocuterConfig config(appID);
	if (config.get((const uint8_t*) "APPDATA", (const uint8_t*) "Author", (uint8_t *) author, MAX_APP_INFO_LENGTH))
		addMenuItem(&menuitem_author);
	
	uint8_t vMajor, vMinor, vPatch;
	if (pocuter->OTA->getAppVersion(appID, &vMajor, &vMinor, &vPatch) == PocuterOTA::OTAERROR_OK) {
		sprintf(version, "%d.%d.%d", vMajor, vMinor, vPatch);
		addMenuItem(&menuitem_version);
	}
}

void startApp(MenuItem *item) {
	// check if menu is installed on the SD. if not, download it - or the user is unable to ever get back to it
	uint8_t a, b, c;
	if (pocuter->OTA->getAppVersion(1, &a, &b, &c) == PocuterOTA::OTAERROR_FILE_NOT_FOUND) {
		initMenuChange(&menu_updatemenu);
	} else {
		pocuter->OTA->setNextAppID(appID);
		pocuter->OTA->restart();
	}
}

void exitAppUpdateMenu() {
	initMenuChange(&menu_appinfo);
}

void updateApp(MenuItem *item) {
	downloadAppUpdate(appID, updateURL, exitAppUpdateMenu, exitAppUpdateMenu);
}

void deleteApp(MenuItem *item) {
	char strbuf[256];
	sprintf(strbuf, APP_SD_LOCATION_STAGED, appID);
	remove(strbuf);
	sprintf(strbuf, APP_SD_LOCATION_FINAL,  appID);
	remove(strbuf);
	sprintf(strbuf, APP_SD_LOCATION_BACKUP, appID);
	remove(strbuf);
	sprintf(strbuf, APP_SD_LOCATION_FOLDER, appID);
	remove(strbuf);
	initMenuChange(&menu_apps);
}