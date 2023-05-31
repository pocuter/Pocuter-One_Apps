// ========================================
// INCLUDES
// ========================================

#include "menu_deviceinfo.h"
#include "menu_mainmenu.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuDeviceInfo();

// ========================================
// GLOBALS
// ========================================

Menu menu_deviceinfo = {
    "Device Info",          // title
    NULL,                   // icon
    &menu_mainmenu,         // parent menu
    false,                  // can loop items
    
    createMenuDeviceInfo,   // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_osversion = {
    "PocuterOS Version",    // name
    ICON_VERSION,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

MenuItem menuitem_osversion_memory = {
    "OS Version (in memory)", // name
    ICON_VERSION,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

MenuItem menuitem_osversion_sd = {
    "OS Version (on SD card)", // name
    ICON_VERSION,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

MenuItem menuitem_serialnumber = {
    "Device ID",            // name
    ICON_INFO_DEVICEID,     // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

char menuVersion[16], menuVersionMemory[16];

// ========================================
// FUNCTIONS
// ========================================

void createMenuDeviceInfo() {
	uint8_t vMajor, vMinor, vPatch;

    sprintf(menuVersionMemory, "%d.%d.%d", MENU_VERSION_MAJOR, MENU_VERSION_MINOR, MENU_VERSION_PATCH);
    menuitem_osversion_memory.dataStr = menuVersionMemory;
    
    if (pocuter->OTA->getAppVersion(1, &vMajor, &vMinor, &vPatch) == PocuterOTA::OTAERROR_OK) {
		sprintf(menuVersion, "%d.%d.%d", vMajor, vMinor, vPatch);
        menuitem_osversion.dataStr = menuVersion;
        menuitem_osversion_sd.dataStr = menuVersion;
        
        if (vMajor == MENU_VERSION_MAJOR && vMinor == MENU_VERSION_MINOR && vPatch == MENU_VERSION_PATCH) {
            addMenuItem(&menuitem_osversion);
        } else {
            addMenuItem(&menuitem_osversion_memory);
            addMenuItem(&menuitem_osversion_sd);
        }
    } else {
        addMenuItem(&menuitem_osversion_memory);
    }
	
	menuitem_serialnumber.dataStr = (char*) pocuter->HMAC->getChipID();
    addMenuItem(&menuitem_serialnumber);
}
