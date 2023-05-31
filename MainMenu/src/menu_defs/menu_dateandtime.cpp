// ========================================
// INCLUDES
// ========================================

#include "../../settings.h"
#include "menu_dateandtime.h"
#include "menu_selecttimezone.h"
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

void createMenuDateAndTime();
void toggleShowDateInTitleBar(MenuItem *item);

// ========================================
// GLOBALS
// ========================================

Menu menu_dateandtime = {
    "Date & Time",          // title
    NULL,                   // icon
    &menu_settings,         // parent menu
    false,                  // can loop items
    
    createMenuDateAndTime,  // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_selecttimezone = {
    "Select Time Zone",     // name
    ICON_TIMEZONE,          // icon
    
    &menu_selecttimezone,   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_showdate = {
    "Show Date In Menu",    // name
    ICON_SHOWDATE,          // icon
    
    NULL,                   // next menu
    &toggleShowDateInTitleBar, // select func

    MENU_ITEM_TYPE_TOGGLE,  // item type
};

// ========================================
// FUNCTIONS
// ========================================

void createMenuDateAndTime() {
	menuitem_showdate.dataInt = pocuterSettings.showDateInTitleBar;
    addMenuItem(&menuitem_selecttimezone);
    addMenuItem(&menuitem_showdate);
}

void toggleShowDateInTitleBar(MenuItem *item) {
	selectFuncToggleDummy();
	pocuterSettings.showDateInTitleBar = !pocuterSettings.showDateInTitleBar;
	setSetting("MAINMENU", "ShowDateInTitleBar", pocuterSettings.showDateInTitleBar);
}