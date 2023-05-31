// ========================================
// INCLUDES
// ========================================

#include "icons.h"
#include "menu.h"
#include "popup.h"
#include "settings.h"

// ========================================
// MACROS
// ========================================

#define MENU_FADE_SPEED             10.0

#define SCROLL_SPEED                10.0

#define ITEM_TEXT_SCROLL_BEGIN      1.0
#define ITEM_TEXT_SCROLL_SPEED      (sizeX/4.0)
#define ITEM_TEXT_REPEAT_SPACE      (sizeX/3.0)

#define TOGGLE_SWITCH_DURATION      0.15

// for disabled status bar items - just draw off-screen
#define SB_DISAB {127, 127}

// ========================================
// TYPES
// ========================================

struct Pos2i {
    int8_t x, y;
};

struct StatusBarItems {
    Pos2i wifi, time, month, day, battery;
};

// ========================================
// PROTOTYPES
// ========================================

void initMenuChange(Menu *menu);
void drawMenuItems();
void drawMenuTitleBar();

// ========================================
// GLOBALS
// ========================================

Menu *currentMenu, *nextMenu;
MenuItem *menuItems[MAX_MENU_ITEMS];
int menuItemCount;
// menu fade: -1..0 means fading out, 0..1 means fading in. brightness is abs(menuFade)
double menuFade = 0.0;
double menuScroll, boxScroll, timeSelected, textScroll, toggleScroll;
const char *monthStr[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// menu status bar item positioning:     wifi      time     month      day     battery
StatusBarItems statusBarNoBatNoDate = {{-11, 6}, {-17, 0}, SB_DISAB, SB_DISAB, SB_DISAB};
StatusBarItems statusBarBatNoDate   = {{-18, 6}, {-23, 0}, SB_DISAB, SB_DISAB, {- 5, 0}};
StatusBarItems statusBarNoBatDate   = {{- 7, 6}, {-25, 6}, {-25, 0}, {- 0, 0}, SB_DISAB};
StatusBarItems statusBarBatDate     = {{-13, 6}, {-31, 6}, {-31, 0}, {- 6, 0}, {- 5, 0}};

// ========================================
// FUNCTIONS
// ========================================

void runMenu() {
    checkForPopups();
    
    if (handlePopup()) {
        pocuter->Display->updateScreen();
        return;
    }
    
    if (menuFade < 1.0) {
        double oldMenuFade = menuFade;
        menuFade += MENU_FADE_SPEED * dt;
        if (oldMenuFade < 0.0 && menuFade >= 0.0) {
            // clear screen
            UGUI* gui = pocuter->ugui;
            uint16_t sizeX;
            uint16_t sizeY;
            pocuter->Display->getDisplaySize(sizeX, sizeY);
            gui->UG_FillFrame(0, 0, sizeX-1, sizeY-1, C_BLACK);
            forceScreenUpdate();
            
            changeMenu(nextMenu);
        }
        if (menuFade > 1.0)
            menuFade = 1.0;

        pocuter->Display->setBrightness(abs(menuFade)*pocuterSettings.brightness);
    } else {
        if (currentMenu->updateFunc)
            currentMenu->updateFunc();
    }
    
    if (currentMenu->drawFunc)
        currentMenu->drawFunc();
    
    pocuter->Display->updateScreen();
}

void forceScreenUpdate() {
    pocuter->Display->updateScreen();
}

void initMenuChange(Menu *menu) {
    nextMenu = menu;
    menuFade = -1.0;
}

void changeMenu(Menu *menu) {
    currentMenu = menu;
    menuItemCount = 0;
    menuScroll = currentMenu->topOption;
    boxScroll = currentMenu->selectedOption;
    timeSelected = 0;
    
    if (currentMenu->createFunc)
        currentMenu->createFunc();
}

void addMenuItem(MenuItem *item, Icon *customIcon) {
    if (menuItemCount == MAX_MENU_ITEMS)
        return;
    menuItems[menuItemCount++] = item;
    
    if (customIcon == NULL)
        item->icon = getIcon(item->iconType);
    else
        item->icon = customIcon;
}

void clearMenuItems() {
    menuItemCount = 0;
}

void updateMenuDefault() {
    if (ACTION_UP) {
        if (currentMenu->selectedOption > 0) {
            currentMenu->selectedOption -= 1;
            if (currentMenu->topOption > currentMenu->selectedOption)
                currentMenu->topOption = currentMenu->selectedOption;
            timeSelected = 0;
        } else if (currentMenu->canLoopOptions) {
            currentMenu->selectedOption = menuItemCount-1;
            currentMenu->topOption = max(menuItemCount-MAX_OPTIONS_VISIBLE, 0);
            menuScroll = currentMenu->topOption;
            boxScroll = menuItemCount-1;
            timeSelected = 0;
        }
    }
    if (ACTION_DOWN) {
        if (currentMenu->selectedOption < menuItemCount-1) {
            currentMenu->selectedOption += 1;
            if (currentMenu->topOption < currentMenu->selectedOption - MAX_OPTIONS_VISIBLE+1)
                currentMenu->topOption = currentMenu->selectedOption - MAX_OPTIONS_VISIBLE+1;
            timeSelected = 0;
        } else if (currentMenu->canLoopOptions) {
            currentMenu->selectedOption = 0;
            currentMenu->topOption = 0;
            menuScroll = 0;
            boxScroll = 0;
            timeSelected = 0;
        }
    }
    
    if (ACTION_OK) {
        if (menuItemCount != 0) {
            MenuItem *item = menuItems[currentMenu->selectedOption];
            if (item->type == MENU_ITEM_TYPE_TOGGLE || item->type == MENU_ITEM_TYPE_THREEWAY_TOGGLE)
                toggleScroll = 0.0;
            
            if (item->selectFunc != NULL)
                item->selectFunc(item);
            if (item->nextMenu != NULL)
                initMenuChange(item->nextMenu);
        }
    }
    
    if (ACTION_BACK) {
        if (currentMenu->parentMenu != NULL)
            initMenuChange(currentMenu->parentMenu);
    }
}

void drawMenuDefault() {
    drawMenuItems();
    drawMenuTitleBar();
}

void drawMenuItems() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);

    // menu scroll animation
    if (menuScroll < currentMenu->topOption)
        menuScroll = min((double) currentMenu->topOption, menuScroll + SCROLL_SPEED * dt);
    else if (menuScroll > currentMenu->topOption)
        menuScroll = max((double) currentMenu->topOption, menuScroll - SCROLL_SPEED * dt);

    // box scroll animation
    if (boxScroll < currentMenu->selectedOption)
        boxScroll = min((double) currentMenu->selectedOption, boxScroll + SCROLL_SPEED * dt);
    else if (boxScroll > currentMenu->selectedOption)
        boxScroll = max((double) currentMenu->selectedOption, boxScroll - SCROLL_SPEED * dt);

    // time option is selected
    timeSelected += dt;
    toggleScroll += dt;

    // bg
    gui->UG_FillFrame(0, 0, sizeX-1, sizeY-1, C_BLACK);

    // items
    for (int i = 0; i < menuItemCount; i++) {
        double j = i - menuScroll;
        bool selected = (i == currentMenu->selectedOption);
        MenuItem *item = menuItems[i];

        // calculate vertical position
        int y = floor(13 + (i - menuScroll)*17);

        // draw item text
        char strbuf[65];
        strncpy(strbuf, item->name, 64);
        strbuf[64] = '\0';
        double optionTextScroll = 0.0;
        
        // smaller font on info text
        int textY;
        if (item->type == MENU_ITEM_TYPE_INFO) {
            gui->UG_FontSelect(&FONT_POCUTER_4X6);
            textY = y-2;
        } else {
            gui->UG_FontSelect(&FONT_POCUTER_5X7);
            textY = y+1;
        }
        
        // check if text is too long...
        int maxTextWidth = sizeX - 20 - 2;
        if (item->type == MENU_ITEM_TYPE_TOGGLE || item->type == MENU_ITEM_TYPE_THREEWAY_TOGGLE)
            maxTextWidth -= (14 + 2);
        if (gui->UG_StringWidth(strbuf) > maxTextWidth) {
            // if the option is not selected OR in the first two seconds of selection, don't move the text, and add '...' at end
            // afterwards, slowly scroll through the text
            if (!selected || timeSelected < ITEM_TEXT_SCROLL_BEGIN) {
                // add '...' at end
                int len = strlen(strbuf);
                strbuf[len-1] = strbuf[len-2] = strbuf[len-3] = '.';
    
                // remove one letter at a time until the name fits
                while (gui->UG_StringWidth(strbuf) > maxTextWidth && len > 3) {
                    len -= 1;
                    strbuf[len] = '\0';
                    strbuf[len-3] = '.';
                }

                // text scroll is disabled
                optionTextScroll = 0.0;
                if (selected)
                    textScroll = 0.0;
            } else {
                // scroll text
                textScroll += ITEM_TEXT_SCROLL_SPEED * dt;
                if (textScroll > gui->UG_StringWidth(strbuf) + ITEM_TEXT_REPEAT_SPACE)
                    textScroll -= (gui->UG_StringWidth(strbuf) + ITEM_TEXT_REPEAT_SPACE);
                
                optionTextScroll = textScroll;
            }
        } else {
            // text fits, no scrolling
            optionTextScroll = 0.0;
        }
        // text color: gray if disabled, orange if selected, cyan otherwise
        if (item->nextMenu == NULL && item->selectFunc == NULL) {
            gui->UG_SetForecolor(C_GRAY);
        } else {
            double s = min(abs(i - boxScroll), 1.0);
            UG_COLOR textColor = interpolateColorRGB888(C_ORANGE, C_CYAN, s);
            gui->UG_SetForecolor(textColor);
        }
        // print text
        gui->UG_PutStringSingleLine(floor(20 - optionTextScroll), textY, strbuf);
        if (optionTextScroll != 0.0)
            gui->UG_PutStringSingleLine(floor(20 - optionTextScroll + gui->UG_StringWidth(strbuf) + ITEM_TEXT_REPEAT_SPACE), textY, strbuf);

        if (item->type == MENU_ITEM_TYPE_INFO)
            gui->UG_PutStringSingleLine(sizeX - 2 - gui->UG_StringWidth(item->dataStr), y+6, item->dataStr);

        // draw black area around text
        gui->UG_FillFrame(0, y, 18, y+15, C_BLACK);
        gui->UG_FillFrame(sizeX-2, y, sizeX-1, y+15, C_BLACK);

        // draw item icon
        drawIcon(item->icon, 1, y);

        // if toggle: draw switch
        if (item->type == MENU_ITEM_TYPE_TOGGLE || item->type == MENU_ITEM_TYPE_THREEWAY_TOGGLE) {
            gui->UG_FillFrame(sizeX-18, y, sizeX-3, y+15, C_BLACK);
            gui->UG_DrawRoundFrame(sizeX-16, y+4, sizeX-3, y+11, 4, C_WHITE);

            int offX = sizeX-15, midX = sizeX-12, onX = sizeX-9, toggleX;
            UG_COLOR color = C_GRAY;

            if (selected && toggleScroll < TOGGLE_SWITCH_DURATION) {
                double s = toggleScroll / TOGGLE_SWITCH_DURATION;
                if (item->dataInt == 0) {
                    toggleX = interpolate( onX, offX, s);
                    color = interpolateColorRGB888(C_GREEN, C_GRAY, s);
                } else if (item->dataInt == 1 && item->type == MENU_ITEM_TYPE_TOGGLE) {
                    toggleX = interpolate(offX,  onX, s);
                    color = interpolateColorRGB888(C_GRAY, C_GREEN, s);
                } else if (item->dataInt == 1) { // three-way toggle
                    toggleX = interpolate(offX, midX, s);
                    color = interpolateColorRGB888(C_GRAY, C_YELLOW, s);
                } else if (item->dataInt == 2) {
                    toggleX = interpolate(midX,  onX, s);
                    color = interpolateColorRGB888(C_YELLOW, C_GRAY, s);
                }
            } else {
                if (item->dataInt == 0) {
                    toggleX = offX;
                    color = C_GRAY;
                } else if (item->type == MENU_ITEM_TYPE_THREEWAY_TOGGLE && item->dataInt == 1) {
                    toggleX = midX;
                    color = C_YELLOW;
                } else {
                    toggleX = onX;
                    color = C_GREEN;
                }
            }
            gui->UG_FillRoundFrame(toggleX, y+5, toggleX+5, y+10, 2, color);
        }
    }
    
    // 'selected item' box
    if (menuItemCount != 0) {
        int boxY = 13 + (boxScroll - menuScroll) * 17 + 0.5;
        gui->UG_DrawRoundFrame(0, boxY-1, 95, boxY+16, 4, pocuterSettings.systemColor);
    }
}

void drawMenuTitleBar() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    
    // title
    gui->UG_FillFrame(0, 0, sizeX-1, 11, C_BLACK);
    //gui->UG_DrawLine(0, 9, sizeX-1, 9, C_DIM_GRAY);
    gui->UG_FontSelect(&FONT_POCUTER_5X8);
    gui->UG_SetForecolor(pocuterSettings.systemColor);
    int titleWidth = gui->UG_StringWidth(currentMenu->title);
    int titleX = sizeX/2-titleWidth/2;
    if (titleX < 1)
        titleX = 1;
    gui->UG_PutStringSingleLine(0, -2, currentMenu->title);

    StatusBarItems sbItems;
    if (!pocuterSettings.showBatteryInTitleBar && !pocuterSettings.showDateInTitleBar)
            sbItems = statusBarNoBatNoDate;
    else if (pocuterSettings.showBatteryInTitleBar && !pocuterSettings.showDateInTitleBar)
            sbItems = statusBarBatNoDate;
    else if (!pocuterSettings.showBatteryInTitleBar && pocuterSettings.showDateInTitleBar)
            sbItems = statusBarNoBatDate;
    else if (pocuterSettings.showBatteryInTitleBar && pocuterSettings.showDateInTitleBar)
            sbItems = statusBarBatDate;
    
    gui->UG_FillFrame(min(sizeX+sbItems.battery.x-1, sizeX+sbItems.time.x-1), 0, sizeX-1, 11, C_BLACK);
    gui->UG_FontSelect(&FONT_POCUTER_3X5);
    gui->UG_SetForecolor(C_WHITE);
    
    int icon = (millis() % 4000) / 800;
    //drawIcon(getIcon((IconType) (TITLE_BATTERY_EMPTY + icon)), sizeX+sbItems.battery.x, sbItems.battery.y);
    
    tm localTime;
    pocuter->PocTime->getLocalTime(&localTime);
    char strbuf[16];
    sprintf(strbuf, "%02d:%02d", localTime.tm_hour, localTime.tm_min);
    gui->UG_PutStringSingleLine(sizeX+sbItems.time.x, sbItems.time.y-3, strbuf);
    sprintf(strbuf, "%s", monthStr[localTime.tm_mon]);
    gui->UG_PutStringSingleLine(sizeX+sbItems.month.x, sbItems.month.y-3, strbuf);
    sprintf(strbuf, "%d", localTime.tm_mday);
    gui->UG_PutStringSingleLine(sizeX+sbItems.day.x - gui->UG_StringWidth(strbuf), sbItems.day.y-3, strbuf);
    
    PocuterWIFI::WIFI_STATE state = pocuter->WIFI->getState();
    const char *str = NULL;
    IconType iconType;
    UG_COLOR wifiColor;
    switch (state) {
        case PocuterWIFI::WIFI_STATE_INIT_FAILED:                                        str = "ER";  wifiColor = C_RED;    break;
        case PocuterWIFI::WIFI_STATE_DISCONNECTED:   iconType = TITLE_WIFI_DISCONNECTED; str = NULL;  wifiColor = C_RED;    break;
        case PocuterWIFI::WIFI_STATE_TRY_CONNECTING: iconType = TITLE_WIFI_DISCONNECTED; str = NULL;  wifiColor = C_YELLOW; break;
        case PocuterWIFI::WIFI_STATE_CONNECTED:      iconType = TITLE_WIFI_GOOD;         str = NULL;  wifiColor = C_GREEN;  break;
        case PocuterWIFI::WIFI_WAITING_WPS:                                              str = "W";   wifiColor = C_GREEN;  break;
        case PocuterWIFI::WIFI_WAITING_AP:                                               str = "AP";  wifiColor = C_GREEN;  break;
    }
    if (str == NULL) {
        drawIcon(getIcon(iconType), sizeX+sbItems.wifi.x, sbItems.wifi.y);
    } else {
        gui->UG_SetForecolor(wifiColor);
        gui->UG_PutStringSingleLine(sizeX+sbItems.wifi.x, sbItems.wifi.y-3, str);
    }
}

void menuCursorOnFirstItem() {
    menuScroll = currentMenu->topOption = 0;
    boxScroll = currentMenu->selectedOption = 0;
    timeSelected = 0;
}

void showPleaseWaitScreen() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX, sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    gui->UG_FillFrame(0, 0, sizeX-1, sizeY-1, C_BLACK);
    gui->UG_FontSelect(&FONT_POCUTER_5X7);
    gui->UG_SetForecolor(C_WHITE);
    const char *str = "Please wait...";
    gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(str), sizeY-10, str);
    forceScreenUpdate();
    
    // menu transition uses brightness to fade in/out so we have to set it manually since normally it would be 0 here
    pocuter->Display->setBrightness(pocuterSettings.brightness);
}

void selectFuncToggleDummy() {
    MenuItem *item = menuItems[currentMenu->selectedOption];
    item->dataInt = !item->dataInt;
}

void selectFuncToggle3WayDummy() {
    MenuItem *item = menuItems[currentMenu->selectedOption];
    item->dataInt = (item->dataInt+1)%3;
}
