#ifndef _MENU_H_
#define _MENU_H_
// ========================================
// INCLUDES
// ========================================

#include "icons.h"
#include "system.h"

// ========================================
// MACROS
// ========================================

#define MAX_OPTIONS_VISIBLE         3
#define MAX_MENU_ITEMS              32

// ========================================
// TYPES
// ========================================

struct Menu {
    const char *title;
    int *icon;//UG_BMP *icon;
    Menu *parentMenu;
    bool canLoopOptions;
    
    void (*createFunc)();
    void (*updateFunc)();
    void (*drawFunc)();
    
    int selectedOption, topOption; // top option = option currently displayed at the top of the screen
};

enum MenuItemType {
    MENU_ITEM_TYPE_OPTION = 0,              // default clickable option, usually expanding into a different screen
    MENU_ITEM_TYPE_TOGGLE,                  // an on/off toggle, displaying a switch: gray/green
    MENU_ITEM_TYPE_THREEWAY_TOGGLE,         // a three-way toggle, displaying a switch: gray/yellow/green
    MENU_ITEM_TYPE_INFO,                    // a not clickable item, displaying a description string and an info string
};

struct MenuItem {
    const char *name;
    IconType iconType;//UG_BMP *icon;

    // on selecting an option, the following calls are performed:
    // if selectFunc is not NULL, it is called.
    // if nextMenu is not NULL, the current menu is discarded and nextMenu is created.
    // if both are NULL, nothing happens.
    // if both are set, first selectFunc is called, then enterMenu(nextMenu).
    // the 'fade' transition effect will only occur if nextMenu is set.
    Menu *nextMenu;
    void (*selectFunc)(MenuItem *item);

    MenuItemType type;
    int dataInt;
    char *dataStr;
    void *data;

    Icon *icon;
};

// ========================================
// PROTOTYPES
// ========================================

extern void runMenu();
extern void forceScreenUpdate();
  
extern void initMenuChange(Menu *menu);
extern void changeMenu(Menu *menu);
extern void addMenuItem(MenuItem *item, Icon *customIcon = NULL);
extern void clearMenuItems();

extern void updateMenuDefault();
extern void drawMenuDefault();

extern void menuCursorOnFirstItem();

extern void showPleaseWaitScreen();

extern void selectFuncToggleDummy();
extern void selectFuncToggle3WayDummy();

// ========================================
// GLOBALS
// ========================================

// ========================================
// FUNCTIONS
// ========================================


#endif //_MENU_H_
