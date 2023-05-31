// ========================================
// INCLUDES
// ========================================

#include "menu_apps.h"
#include "menu_appstore.h"
#include "menu_deviceinfo.h"
#include "menu_mainmenu.h"
#include "menu_settings.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuMainMenu();

// ========================================
// GLOBALS
// ========================================

Menu menu_mainmenu = {
    "Home",                 // title
    NULL,                   // icon
    NULL,                   // parent menu
    false,                  // can loop items
    
    createMenuMainMenu,     // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_apps = {
    "Apps",                 // name
    ICON_APPS,              // icon
    
    &menu_apps,             // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_appstore = {
    "App Store",            // name
    ICON_APPSTORE,          // icon
    
    &menu_appstore,         // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_status = {
    "Status",               // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_settings = {
    "Settings",             // name
    ICON_SETTINGS,          // icon
    
    &menu_settings,         // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_deviceInfo = {
    "Device Information",   // name
    ICON_DEVICE_INFO,       // icon
    
    &menu_deviceinfo,       // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

// ========================================
// FUNCTIONS
// ========================================

void createMenuMainMenu() {
    addMenuItem(&menuitem_apps);
    addMenuItem(&menuitem_appstore);
    //addMenuItem(&menuitem_status);
    addMenuItem(&menuitem_settings);
    addMenuItem(&menuitem_deviceInfo);
}
