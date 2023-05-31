// ========================================
// INCLUDES
// ========================================

#include "menu_appinfo.h"
#include "menu_apps.h"
#include "menu_mainmenu.h"
#include <dirent.h> 

// ========================================
// MACROS
// ========================================

#define MAX_APP_NAME		64
#define APP_ICON_WIDTH		16
#define APP_ICON_HEIGHT		16

// how many apps should be loaded & shown at once.
// respect menu.h -> MAX_MENU_ITEMS when changing
#define APPLIST_PAGE_SIZE  16

// ========================================
// TYPES
// ========================================

enum AppError {
	APP_ERROR_NO_ERROR = 0,
	APP_ERROR_NO_CARD_INSERTED,
	APP_ERROR_CARD_READ_ERROR,
	APP_ERROR_NO_APPS_PRESENT
};

struct AppInfo {
    uint64_t id;
    char name[MAX_APP_NAME+1];
    UG_COLOR appIconData[APP_ICON_WIDTH * APP_ICON_HEIGHT];
    Icon appIcon;
};

// ========================================
// PROTOTYPES
// ========================================

void createMenuApps();
void drawMenuApps();
void nextAppListPage(MenuItem *item);
void previousAppListPage(MenuItem *item);
void loadAppData();
void setAppInfos(MenuItem *item);

// ========================================
// GLOBALS
// ========================================

Menu menu_apps = {
    "Apps",                 // title
    NULL,                   // icon
    &menu_mainmenu,         // parent menu
    false,                  // can loop items
    
    createMenuApps,         // create func
    updateMenuDefault,      // update func
    drawMenuApps            // draw func
};

MenuItem menuitem_applist_nextpage = {
    "Next Page",            // name
    ICON_NEXTPAGE,          // icon
    
    &menu_apps,             // next menu
    nextAppListPage,        // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_applist_previouspage = {
    "Previous Page",        // name
    ICON_PREVPAGE,          // icon
    
    &menu_apps,             // next menu
    previousAppListPage,    // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_applist_entries[APPLIST_PAGE_SIZE];

AppInfo appInfos[APPLIST_PAGE_SIZE];
int appsInstalled = 0;
int appListPage = 0;
AppError appError;

// ========================================
// FUNCTIONS
// ========================================

void createMenuApps() {
	appsInstalled = pocuter->OTA->getAppsCount();
    
    loadAppData();
    
    menuCursorOnFirstItem();
}

void drawMenuApps() {
	drawMenuDefault();
	
	if (appError == APP_ERROR_NO_ERROR)
		return;
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	char str1[64], str2[64], str3[64], str4[64];
	
	switch (appError) {
		case APP_ERROR_CARD_READ_ERROR:
			sprintf(str1, "Error:");
			sprintf(str2, "Could not read SD card.");
			sprintf(str3, "Please try to restart");
			sprintf(str4, "your pocuter.");
			break;
			
		case APP_ERROR_NO_CARD_INSERTED:
			sprintf(str1, "Error:");
			sprintf(str2, "No SD card inserted.");
			sprintf(str3, "");
			sprintf(str4, "");
			break;
			
		case APP_ERROR_NO_APPS_PRESENT:
			sprintf(str1, "Error:");
			sprintf(str2, "No apps found");
			sprintf(str3, "on SD card.");
			sprintf(str4, "");
			break;
	}
	
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_RED);
	
	if (str1 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str1)/2, sizeY*2/6, str1);
	if (str2 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str2)/2, sizeY*3/6, str2);
	if (str3 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str3)/2, sizeY*4/6, str3);
	if (str4 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str4)/2, sizeY*5/6, str4);
}

void nextAppListPage(MenuItem *item) {
    appListPage += 1;
}

void previousAppListPage(MenuItem *item) {
    appListPage -= 1;
}

void loadAppData() {
	std::vector<uint64_t> apps;
	PocuterOTA::OTAERROR otaError = pocuter->OTA->getApps(&apps, APPLIST_PAGE_SIZE, appListPage*APPLIST_PAGE_SIZE);
    
    if (appListPage > 0)
        addMenuItem(&menuitem_applist_previouspage);
    
	appError = APP_ERROR_NO_ERROR;
	if (otaError == PocuterOTA::OTAERROR_APP_READ_ERROR) {
		if (!pocuter->SDCard->cardInSlot()) {
			appError = APP_ERROR_NO_CARD_INSERTED;
			return;
		}
		DIR *directory = opendir(pocuter->SDCard->getMountPoint());
		if (directory) {
			closedir(directory);
			appError = APP_ERROR_NO_APPS_PRESENT;
		} else {
			appError = APP_ERROR_CARD_READ_ERROR;
		}
		return;
	}
    
	for (int i = 0; i < apps.size(); i++) {
		int appID = apps.at(i);
        AppInfo *info = &appInfos[i];
        MenuItem *item = &menuitem_applist_entries[i];
        
		PocuterConfig config(appID);
		if (!config.get((const uint8_t*) "APPDATA", (const uint8_t*) "Name", (uint8_t *) info->name, MAX_APP_NAME))
            sprintf(info->name, "%d", info->id);
		info->id = appID;
		item->data = info;
        
		item->name = info->name;
        item->type = MENU_ITEM_TYPE_OPTION;
		
		if (appID != 1) {
			item->nextMenu = &menu_appinfo;
			item->selectFunc = setAppInfos;
		} else {
			item->nextMenu = NULL;
			item->selectFunc = NULL;
		}
		
		if (!config.getBinary((const uint8_t*) "APPDATA", (const uint8_t*) "AppIcon", info->appIconData, APP_ICON_WIDTH*APP_ICON_HEIGHT*4))
			memcpy(info->appIconData, getIcon(ICON_MISSING)->data, APP_ICON_WIDTH*APP_ICON_HEIGHT*4);
        info->appIcon = {APP_ICON_WIDTH, APP_ICON_HEIGHT, "", info->appIconData};
        
		addMenuItem(item, &info->appIcon);
	}
    
    if ((appListPage+1)*APPLIST_PAGE_SIZE <= appsInstalled)
        addMenuItem(&menuitem_applist_nextpage);
}

void setAppInfos(MenuItem *item) {
    AppInfo *info = (AppInfo*) item->data;
	setAppInfoIDAndName(info->id, info->name);
}