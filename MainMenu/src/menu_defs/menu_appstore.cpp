// ========================================
// INCLUDES
// ========================================

#include "menu_appupdate.h"
#include "menu_mainmenu.h"
#include "../../settings.h"

// ========================================
// MACROS
// ========================================

// how many apps should be loaded & shown at once.
// respect menu.h -> MAX_MENU_ITEMS when changing
#define APPSTORE_PAGE_SIZE  8

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuAppStore();
void nextAppStorePage(MenuItem *item);
void previousAppStorePage(MenuItem *item);
void downloadAppList();
void selectAppStoreEntry(MenuItem *item);
void createMenuAppDetails();
void exitAppDownloadMenu();
void downloadApp(MenuItem *item);

// ========================================
// GLOBALS
// ========================================

Menu menu_appstore = {
    "App Store",            // title
    NULL,                   // icon
    &menu_mainmenu,         // parent menu
    false,                  // can loop items
    
    createMenuAppStore,     // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_appstore_nextpage = {
    "Next Page",            // name
    ICON_NEXTPAGE,          // icon
    
    &menu_appstore,         // next menu
    nextAppStorePage,       // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_appstore_previouspage = {
    "Previous Page",        // name
    ICON_PREVPAGE,          // icon
    
    &menu_appstore,         // next menu
    previousAppStorePage,   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_appstoreapps[APPSTORE_PAGE_SIZE];

Menu menu_appstore_appdetails = {
    "",                     // title
    NULL,                   // icon
    &menu_appstore,         // parent menu
    false,                  // can loop items
    
    createMenuAppDetails,   // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_appdetail_downloadapp = {
    "",                     // name
    ICON_MISSING,           // icon
    
    NULL,                   // next menu
    downloadApp,            // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_appdetail_author = {
    "Author:",              // name
    ICON_AUTHOR,            // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

MenuItem menuitem_appdetail_version = {
    "Version:",             // name
    ICON_VERSION,           // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_INFO,    // item type
};

int appStorePage = 0;
PocuterServer::AppStoreEntry appStoreEntries[APPSTORE_PAGE_SIZE];
PocuterServer::AppStoreEntry *selectedEntry;
const char* appStoreText_Installed = "Installed";
const char* appStoreText_UpdateReady = "Update Ready";
const char* appStoreText_Available = "Available";
const char* appStoreText_ComingSoom = "Coming Soon";
const uint8_t *downloadURL = NULL;
bool appStoreSuccess;

// ========================================
// FUNCTIONS
// ========================================

void createMenuAppStore() {
	showPleaseWaitScreen();
    
    downloadAppList();
    
    menuCursorOnFirstItem();
}

void nextAppStorePage(MenuItem *item) {
    appStorePage += 1;
}

void previousAppStorePage(MenuItem *item) {
    appStorePage -= 1;
}

void downloadAppList() {
    if (appStorePage > 0) {
        addMenuItem(&menuitem_appstore_previouspage);
        menuitem_appstore_previouspage.nextMenu = &menu_appstore;
        menuitem_appstore_previouspage.selectFunc = previousAppStorePage;
    } else {
        menuitem_appstore_previouspage.nextMenu = NULL;
        menuitem_appstore_previouspage.selectFunc = NULL;
    }
    
    int count = APPSTORE_PAGE_SIZE;
    appStoreSuccess = pocuter->Server->getAppStoreList(appStoreEntries, &count, appStorePage*APPSTORE_PAGE_SIZE);
    
    for (int i = 0; i < count; i++) {
        MenuItem *item = &menuitem_appstoreapps[i];
        PocuterServer::AppStoreEntry *entry = &appStoreEntries[i];
        
        item->name = entry->name;
        item->nextMenu = &menu_appstore_appdetails;
        item->selectFunc = selectAppStoreEntry;
        item->type = MENU_ITEM_TYPE_INFO;
        item->data = entry;
        
        uint8_t vMajorS, vMinorS, vPatchS;
        uint8_t vMajorL, vMinorL, vPatchL;
        char *str = entry->version;
        vMajorS = strtol(str,   &str, 10);
        vMinorS = strtol(str+1, &str, 10);
        vPatchS = strtol(str+1, &str, 10);
        
        PocuterOTA::OTAERROR otaErr = pocuter->OTA->getAppVersion(entry->id, &vMajorL, &vMinorL, &vPatchL);
        if (otaErr != PocuterOTA::OTAERROR_OK)
            vMajorL = vMinorL = vPatchL = 0;
        
        if (otaErr != PocuterOTA::OTAERROR_OK) {
            // not installed on SD
            if (vMajorS == 0 && vMinorS == 0 && vPatchS == 0) {
                // not available for download -> COMING SOON
                item->iconType = ICON_MISSING;
                item->dataStr = (char*) appStoreText_ComingSoom;
                item->nextMenu = NULL;
                item->selectFunc = NULL;
            } else {
                // available for download -> AVAILABLE
                item->iconType = ICON_DOWNLOAD;
                item->dataStr = (char*) appStoreText_Available;
            }
        } else {
            // installed on SD
            if (vMajorL == vMajorS && vMinorL == vMinorS && vPatchL == vPatchS) {
                // versions are the same -> INSTALLED
                item->iconType = ICON_INSTALLED;
                item->dataStr = (char*) appStoreText_Installed;
            } else {
                // versions are not the same -> UPDATE READY
                item->iconType = ICON_UPDATE;
                item->dataStr = (char*) appStoreText_UpdateReady;
            }
        }
        
        addMenuItem(item);
    }
    
    if (count == APPSTORE_PAGE_SIZE) {
        addMenuItem(&menuitem_appstore_nextpage);
        menuitem_appstore_nextpage.nextMenu = &menu_appstore;
        menuitem_appstore_nextpage.selectFunc = nextAppStorePage;
    } else {
        menuitem_appstore_nextpage.nextMenu = NULL;
        menuitem_appstore_nextpage.selectFunc = NULL;
    }
}

void selectAppStoreEntry(MenuItem *item) {
    selectedEntry = (PocuterServer::AppStoreEntry*) item->data;
    if (selectedEntry == NULL)
        return;
    
    // copy store item details to detail item details
    // so we don't have to check for installed/downloadable/updateable again
    menuitem_appdetail_downloadapp.name = item->dataStr;
    menuitem_appdetail_downloadapp.iconType = item->iconType;
    
    if (item->iconType == ICON_INSTALLED)
        item->selectFunc = NULL;
    else
        item->selectFunc = downloadApp;
}

void createMenuAppDetails() {
    if (selectedEntry == NULL)
        return;
    
    downloadURL = pocuter->Server->checkNewestAppVersion(selectedEntry->id);
    
    menu_appstore_appdetails.title = selectedEntry->name;
    menuitem_appdetail_author.dataStr = selectedEntry->author;
    menuitem_appdetail_version.dataStr = selectedEntry->version;
    
    addMenuItem(&menuitem_appdetail_downloadapp);
    addMenuItem(&menuitem_appdetail_author);
    addMenuItem(&menuitem_appdetail_version);
}

void exitAppDownloadMenu() {
	initMenuChange(&menu_appstore);
}

void downloadApp(MenuItem *item) {
	downloadAppUpdate(selectedEntry->id, downloadURL, exitAppDownloadMenu, exitAppDownloadMenu);
}