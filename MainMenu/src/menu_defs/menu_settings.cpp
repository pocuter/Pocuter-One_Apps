// ========================================
// INCLUDES
// ========================================

#include "menu_apps.h"
#include "menu_brightness.h"
#include "menu_dateandtime.h"
#include "menu_mainmenu.h"
#include "menu_standby.h"
#include "menu_systemcolor.h"
#include "menu_updatemenu.h"
#include "menu_wifi.h"
#include <dirent.h> 

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuSettings();
void updateMenu();
void restartPocuterFunc(MenuItem *item);

void createMenuResetDevice();
void updateMenuResetDevice();
void drawMenuResetDevice();
void resetDevice();

// ========================================
// GLOBALS
// ========================================

extern Menu menu_resetdevice;

Menu menu_settings = {
    "Settings",             // title
    NULL,                   // icon
    &menu_mainmenu,         // parent menu
    false,                  // can loop items
    
    createMenuSettings,     // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_account = {
    "Account",              // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_applistorder = {
    "Sort App List",        // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_datetime = {
    "Date & Time",          // name
    ICON_DATEANDTIME,       // icon
    
    &menu_dateandtime,      // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_systemcolor = {
    "System Color",         // name
    ICON_SYSTEMCOLOR,       // icon
    
    &menu_systemcolor,      // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_brightness = {
    "Brightness",           // name
    ICON_BRIGHTNESS,        // icon
    
    &menu_brightness,       // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_standby = {
    "Standby",              // name
    ICON_STANDBY,           // icon
    
    &menu_standby,          // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_wifi = {
    "WiFi",                 // name
    ICON_WIFI_FULL,         // icon
    
    &menu_wifi,             // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_bluetooth = {
    "Bluetooth",            // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_ejectsd = {
    "Eject SD Card",        // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_firmwareupdate = {
    "Update Menu",          // name
    ICON_UPDATE,            // icon
    
    &menu_updatemenu,       // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_restartdevice = {
    "Restart Pocuter",      // name
    ICON_RESTART,           // icon
    
    NULL,                   // next menu
    restartPocuterFunc,     // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_resetdevice = {
    "Reset Pocuter",        // name
    ICON_DELETE,            // icon
    
    &menu_resetdevice,      // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

Menu menu_resetdevice = {
    "Reset Pocuter",        // title
    NULL,                   // icon
    &menu_mainmenu,         // parent menu
    false,                  // can loop items
    
    createMenuResetDevice,  // create func
    updateMenuResetDevice,  // update func
    drawMenuResetDevice     // draw func
};

boolean resetDeviceFlag, resetDone;

// ========================================
// FUNCTIONS
// ========================================

void createMenuSettings() {
    //addMenuItem(&menuitem_account);
    //addMenuItem(&menuitem_applistorder);
    addMenuItem(&menuitem_datetime);
    addMenuItem(&menuitem_systemcolor);
    addMenuItem(&menuitem_brightness);
    addMenuItem(&menuitem_standby);
    addMenuItem(&menuitem_wifi);
    //addMenuItem(&menuitem_bluetooth);
    //addMenuItem(&menuitem_ejectsd);
    addMenuItem(&menuitem_firmwareupdate);
    addMenuItem(&menuitem_restartdevice);
    //addMenuItem(&menuitem_resetdevice);
}

void restartPocuterFunc(MenuItem *item) {
	pocuter->OTA->setNextAppID(1);
	pocuter->OTA->forceBootloaderToReflashApp();
	pocuter->OTA->restart();
}

void createMenuResetDevice() {
	resetDeviceFlag = false;
	resetDone = false;
}

void updateMenuResetDevice() {
	if (resetDeviceFlag) {
		resetDevice();
		return;
	}
	
	if (ACTION_OK) {
		// ok button is "no" here
		initMenuChange(&menu_settings);
	}
	if (ACTION_DOWN) {
		// down button is "yes" here
		resetDeviceFlag = true;
	}
}

void drawMenuResetDevice() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	if (!resetDeviceFlag) {
		gui->UG_FontSelect(&FONT_POCUTER_4X6);
		gui->UG_SetForecolor(C_RED);
		
		const char *str[] = {
			"This will delete",
			"all settings and files",
			"on the SD card.",
			"Continue?",
			NULL
		};
		
		for (int i = 0; str[i] != NULL; i++) {
			gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str[i])/2, sizeY*1/5 + i*8, str[i]);
		}
		
		gui->UG_SetForecolor(C_YELLOW);
		gui->UG_PutStringSingleLine(0, sizeY-10, "< No");
		gui->UG_PutStringSingleLine(sizeX-gui->UG_StringWidth("Yes >"), sizeY-10, "Yes >");
	} else if (!resetDone) {
		gui->UG_FontSelect(&FONT_POCUTER_4X6);
		gui->UG_SetForecolor(C_WHITE);
		gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth("Please wait...")/2, sizeY*2/4, "Please wait...");
	} else {
		gui->UG_FontSelect(&FONT_POCUTER_4X6);
		gui->UG_SetForecolor(C_WHITE);
		gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth("Done.")/2, sizeY*2/4, "Done.");
	}
}

int removeDirectory(const char *path) {
	DIR *directory = opendir(path);
	
	if (directory) {
		struct dirent *entry;
		
		while (entry = readdir(directory)) {
			if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) {
				continue;
			}
			
			char filename[strlen(path) + strlen(entry->d_name) + 2];
			sprintf(filename, "%s/%s", path, entry->d_name);
			
			char exclude[64];
			snprintf(exclude, 64, "%s/apps/1/esp32c3.app", pocuter->SDCard->getMountPoint());
			if (!strcmp(exclude, path))
				continue;
			
			int (*const remove_func)(const char*) = entry->d_type == DT_DIR ? removeDirectory : remove;
			remove_func(filename);
		}
		closedir(directory);
	}
	return remove(path);
}

void resetDevice() {
	char rootDir[64];
	snprintf(rootDir, 64, "%s", pocuter->SDCard->getMountPoint());
	forceScreenUpdate();
	removeDirectory(rootDir);
	
	resetDone = true;
}