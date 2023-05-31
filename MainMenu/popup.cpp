// ========================================
// INCLUDES
// ========================================

#include "popup.h"
#include "settings.h"
#include "src/menu_defs/menu_downloadapps.h"
#include "src/menu_defs/menu_updatemenu.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

bool popupCheckMenuUpdate();
bool popupCheckAppDownload();
bool popupCheckSDCardOut();
bool popupCheckSDCardIn();
void exitPopupContinue();
void exitPopupRestart();
void exitPopupDownloadMenuUpdate();
void exitPopupDownloadApps();

// ========================================
// GLOBALS
// ========================================

Popup popupDefs[] = {
    // update available
    {
        // type
        PUPUP_TYPE_UPDATE_AVAILABLE,
        // title
        "Update available!",
        { // description
            "Do you want",
            "to download",
            "the newest version",
            "of the PocuterOS?",
            NULL
        },
        // accept & dismiss text
        "Yes", "Later",
        // check, accept & dismiss functions
        popupCheckMenuUpdate,
        exitPopupDownloadMenuUpdate,
        exitPopupContinue,
    },
    
    // apps available
    {
        // type
        PUPUP_TYPE_APPS_AVAILABLE,
        // title
        "Apps available!",
        { // description
            //
            "Do you want to",
            "download the",
            "requested apps",
            "from the web",
            "app store?"
        },
        // accept & dismiss text
        "Yes", "Later",
        // check, accept & dismiss functions
        popupCheckAppDownload,
        exitPopupDownloadApps,
        exitPopupContinue,
    },
    
    // sd pulled out
    {
        // type
        POPUP_TYPE_SD_PULLED_OUT,
        // title
        "SD card pulled out",
        { // description
            "Please reinsert",
            "the SD card and",
            "restart the device.",
            NULL,
            NULL
        },
        // accept & dismiss text
        "Restart", "Close",
        // check, accept & dismiss functions
        popupCheckSDCardOut,
        exitPopupRestart,
        exitPopupContinue,
    },
    
    // sd plugged in
    {
        // type
        POPUP_TYPE_SD_PLUGGED_IN,
        // title
        "SD card plugged in",
        { // description
            "The Device has",
            "to be restarted to",
            "save settings and",
            "load apps.",
            "Restart now?"
        },
        // accept & dismiss text
        "Restart", "Close",
        // check, accept & dismiss functions
        popupCheckSDCardIn,
        exitPopupRestart,
        exitPopupContinue,
    },
    
    // battery low
    {
        // type
        POPUP_TYPE_BATTERY_LOW,
        // title
        "Battery low",
        { // description
            "Description1",
            "Description2",
            "Description3",
            "Description4",
            "Description5"
        },
        // accept & dismiss text
        "Restart", "Close",
        // check, accept & dismiss functions
        NULL,
        NULL,
        NULL,
    },
};

Popup *currentPopup = NULL;

bool askedForMenuUpdate = false;
bool askedForAppDownload = false;
long lastMenuUpdateCheck = 0;
long lastAppDownloadCheck = 15000;
int sdCardInserted = -1;

// ========================================
// FUNCTIONS
// ========================================

void checkForPopups() {
    if (!currentPopup) {
        for (int i = 0; i < POPUP_ENUM_COUNT; i++) {
            if (popupDefs[i].checkFunc && popupDefs[i].checkFunc()) {
                currentPopup = &popupDefs[i];
                break;
            }
        }
    }

    sdCardInserted = pocuter->SDCard->cardInSlot();
}

bool handlePopup() {
    if (!currentPopup)
        return false;
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    
    gui->UG_FillFrame(0, 0, sizeX-1, sizeY-1, C_BLACK);
    
    gui->UG_FontSelect(&FONT_POCUTER_5X8);
    gui->UG_SetForecolor(pocuterSettings.systemColor);
    gui->UG_PutStringSingleLine(sizeX/2-gui->UG_StringWidth(currentPopup->title)/2, -2, currentPopup->title);
    
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
    for (int i = 0; i < MAX_DESCRIPTION_LINES && currentPopup->description[i] != NULL; i++) {
        int textWidth = gui->UG_StringWidth(currentPopup->description[i]);
        int textX = sizeX/2-textWidth/2;
        gui->UG_PutStringSingleLine(textX, sizeY*1/5 + i*sizeY*1/8, currentPopup->description[i]);
    }

    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_GREEN);
    char strbuf[64];
    if (currentPopup->textDismiss) {
        sprintf(strbuf, "< %s", currentPopup->textDismiss);
        gui->UG_PutStringSingleLine(0, sizeY-10, strbuf);
    }
    if (currentPopup->textAccept) {
        sprintf(strbuf, "%s >", currentPopup->textAccept);
        gui->UG_PutStringSingleLine(sizeX-gui->UG_StringWidth(strbuf), sizeY-10, strbuf);
    }

    if (ACTION_ACCEPT) {
        if (currentPopup->acceptFunc) {
            currentPopup->acceptFunc();
            currentPopup = NULL;
            return true;
        }
    }
    if (ACTION_DISMISS) {
        if (currentPopup->dismissFunc) {
            currentPopup->dismissFunc();
            currentPopup = NULL;
            return true;
        }
    }

    return true;
}


bool popupCheckMenuUpdate() {
    if (askedForMenuUpdate)
        return false;

    if (millis() - lastMenuUpdateCheck < 30000)
        return false;

    uint8_t a, b, c;
    if (pocuter->OTA->getAppVersion(1, &a, &b, &c) == PocuterOTA::OTAERROR_FILE_NOT_FOUND)
        return false;

    if (pocuter->WIFI->getState() != PocuterWIFI::WIFI_STATE_CONNECTED)
        return false;
    
    tm localTime;
    pocuter->PocTime->getLocalTime(&localTime);
    
    if (pocuterSettings.lastDayAskedForUpdate == localTime.tm_mday) {
        askedForMenuUpdate = true;
        return false;
    }

    lastMenuUpdateCheck = millis();
    if (pocuter->Server->checkNewestAppVersion(1)) {
        askedForMenuUpdate = true;
        pocuterSettings.lastDayAskedForUpdate = localTime.tm_mday;
        setSetting("MAINMENU", "LastDayAskedForUpdate", pocuterSettings.lastDayAskedForUpdate);
        return true;
    }
    
    return false;
}

bool popupCheckAppDownload() {
    if (askedForAppDownload)
        return false;
    
    if (millis() - lastAppDownloadCheck < 30000)
        return false;

    if (pocuter->WIFI->getState() != PocuterWIFI::WIFI_STATE_CONNECTED)
        return false;

    lastAppDownloadCheck = millis();
    uint64_t id;
    if (pocuter->Server->checkForAppInstallRequest(id)) {
        askedForAppDownload = true;
        return true;
    }
    
    return false;
}

bool popupCheckSDCardOut() {
    if (sdCardInserted == -1)
        return false;
    return sdCardInserted && !pocuter->SDCard->cardInSlot();
}

bool popupCheckSDCardIn() {
    if (sdCardInserted == -1)
        return false;
    return !sdCardInserted && pocuter->SDCard->cardInSlot();
}

void exitPopupContinue() {}

void exitPopupRestart() {
   esp_restart();
}

void exitPopupDownloadMenuUpdate() {
    initMenuChange(&menu_updatemenu);
}

void exitPopupDownloadApps() {
    initMenuChange(&menu_downloadapps);
}
