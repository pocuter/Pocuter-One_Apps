// ========================================
// INCLUDES
// ========================================

#include "../../settings.h"
#include "menu_settings.h"
#include "menu_wifi.h"

// ========================================
// MACROS
// ========================================

#define MAX_NETWORK_COUNT		32
#define MAX_NETWORK_NAME		64
#define MAX_PASSWORD_LENGTH     64

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuWifi();
void updateMenuWifi();

void createMenuAPInfo();
void drawMenuAPInfo();

void createMenuWPSInfo();
void updateMenuWPSInfo();
void drawMenuWPSInfo();

void scanForWifiNetworks(MenuItem *item);
void createMenuWifiList();
void selectWifi(MenuItem *item);

void createMenuWifiPassword();
void updateMenuWifiPassword();
void drawMenuWifiPassword();
void sprintfWifiEditChar(char *strbuf, char c);
void tryToConnectToWifi();

// ========================================
// GLOBALS
// ========================================

extern Menu menu_accessPointInfo;
extern Menu menu_wpsInfo;

Menu menu_wifi = {
    "WiFi",                 // title
    NULL,                   // icon
    &menu_settings,         // parent menu
    false,                  // can loop items
    
    createMenuWifi,         // create func
    updateMenuWifi,         // update func
    drawMenuDefault         // draw func
};

MenuItem menuitem_startAccessPoint = {
    "Connect via Access Point", // name
    ICON_WIFI_AP,           // icon
    
    &menu_accessPointInfo,  // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_startWPS = {
    "Start WPS",            // name
    ICON_WIFI_WPS,          // icon
    
    &menu_wpsInfo,          // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_scanForWifi = {
    "Scan for WiFis",       // name
    ICON_WIFI_SCAN,         // icon
    
    NULL,                   // next menu
    scanForWifiNetworks,    // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

MenuItem menuitem_scanning = {
    "Scanning...",          // name
    ICON_WIFI_SCAN,         // icon
    
    NULL,                   // next menu
    NULL,                   // select func

    MENU_ITEM_TYPE_OPTION,  // item type
};

Menu menu_accessPointInfo = {
    "Access Point",         // title
    NULL,                   // icon
    &menu_wifi,             // parent menu
    false,                  // can loop items
    
    createMenuAPInfo,       // create func
    updateMenuDefault,      // update func
    drawMenuAPInfo          // draw func
};

Menu menu_wpsInfo = {
    "WPS",                  // title
    NULL,                   // icon
    &menu_wifi,             // parent menu
    false,                  // can loop items
    
    createMenuWPSInfo,      // create func
    updateMenuWPSInfo,      // update func
    drawMenuWPSInfo         // draw func
};

Menu menu_wifilist = {
    "WiFis found:",         // title
    NULL,                   // icon
    &menu_wifi,             // parent menu
    false,                  // can loop items
    
    createMenuWifiList,     // create func
    updateMenuDefault,      // update func
    drawMenuDefault         // draw func
};

Menu menu_wifipassword = {
    "Enter Password",       // title
    NULL,                   // icon
    NULL,                   // parent menu
    false,                  // can loop items
    
    createMenuWifiPassword, // create func
    updateMenuWifiPassword, // update func
    drawMenuWifiPassword    // draw func
};

long wpsMenuEnterTime;
PocuterWIFI::WIFIERROR wpsError;

char wifiNames[MAX_NETWORK_COUNT][MAX_NETWORK_NAME];
MenuItem menuitems_wifi[MAX_NETWORK_COUNT];
PocuterWIFI::apInfo availableWifis[MAX_NETWORK_COUNT];
uint16_t foundNetworkCount = 0;

char wifiPassword[MAX_PASSWORD_LENGTH+1];
int curWifiEdit, curWifiEditChar;
const char *allowedWifiPasswordCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,:;-_!?# \n"; // << \n == END
int allowedWifiPasswordCharactersCount = strlen(allowedWifiPasswordCharacters);

// ========================================
// FUNCTIONS
// ========================================

void createMenuWifi() {
    addMenuItem(&menuitem_startAccessPoint);
    addMenuItem(&menuitem_startWPS);
    addMenuItem(&menuitem_scanForWifi);
    
    // disable standby for wifi menu & submenus
    pocuter->Sleep->setInactivitySleep(0, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
}

void updateMenuWifi() {
    updateMenuDefault();
    
    // endable standby again
    if (ACTION_BACK)
        pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
}

void createMenuAPInfo() {
	PocuterWIFI::WIFIERROR error = pocuter->WIFI->startAccessPoint();
}

void drawMenuAPInfo() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	const char *info[] = {
		"Please connect your",
		"phone to 'POCUTER'",
		"and enter your",
		"Wifi credentials.",
		NULL,
	};
	
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
	
	for (int i = 0; info[i] != NULL; i++) {
		gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(info[i])/2, 14 + i*12, info[i]);
	}
}

void createMenuWPSInfo() {
	wpsMenuEnterTime = millis();
	wpsError = PocuterWIFI::WIFIERROR_UNKNOWN;
}

void updateMenuWPSInfo() {
	// startWPS() blocks for up to 60 seconds, so wait until the menu transition finished, then start it
	if (millis() - wpsMenuEnterTime > 1000) {
		if (wpsError == PocuterWIFI::WIFIERROR_UNKNOWN)
			wpsError = pocuter->WIFI->startWPS();
		else
			updateMenuDefault();
	}
}

void drawMenuWPSInfo() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	const char *info[6][3] = {
		{ // OK
			"",
			"Connection successful.",
			"",
		},
		{ // init failed
			"Error:",
			"WPS Initialization",
			"Failed.",
		},
		{ // could not set wifi mode
			"Error:",
			"Failed to set.",
			"WiFi mode.",
		},
		{ // no credentials
			"Error:",
			"Failed to retreive",
			"Credentials.",
		},
		{ // timeout
			"Error:",
			"Timeout during WPS.",
			"Please try again.",
		},
		{ // unknown error -> just started
			"Waiting for WPS...",
			"This may take up",
			"to 60 seconds.",
		},
	};
	
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
	
	for (int i = 0; i < 3; i++) {
		gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(info[wpsError][i])/2, 20 + i*12, info[wpsError][i]);
	}
	
	//if (pocuter->WIFI->getState() != PocuterWIFI::WIFI_WAITING_WPS)
	//	initMenuChange(&menu_wifilist);
}

void scanForWifiNetworks(MenuItem *item) {
	// clear items, change "scan" to "scanning" and force redraw
	clearMenuItems();
    addMenuItem(&menuitem_startAccessPoint);
    addMenuItem(&menuitem_startWPS);
    addMenuItem(&menuitem_scanning);
	menu_wifi.drawFunc();
	forceScreenUpdate();
	
	// search for networks
	uint16_t maxNetworkCount = MAX_NETWORK_COUNT;
	foundNetworkCount = 0;
	PocuterWIFI::WIFIERROR err = pocuter->WIFI->scanAPs(availableWifis, &maxNetworkCount, &foundNetworkCount);
	
	// for every found Wifi, create a menu item
	for (int i = 0; i < foundNetworkCount; i++) {
		snprintf(wifiNames[i], 64, "%s c%d s%d a%d", (char*) availableWifis[i].ssid, availableWifis[i].channel, availableWifis[i].signalStrength, availableWifis[i].authMode);
		//snprintf(wifiNames[i], 64, "%s", (char*) availableWifis[i].ssid);
		menuitems_wifi[i].name = wifiNames[i];
		menuitems_wifi[i].type = MENU_ITEM_TYPE_OPTION;
		menuitems_wifi[i].selectFunc = selectWifi;
		menuitems_wifi[i].nextMenu = NULL;
		
		if      (availableWifis[i].signalStrength > -50) menuitems_wifi[i].iconType = ICON_WIFI_FULL;
		else if (availableWifis[i].signalStrength > -60) menuitems_wifi[i].iconType = ICON_WIFI_GOOD;
		else if (availableWifis[i].signalStrength > -70) menuitems_wifi[i].iconType = ICON_WIFI_OKAY;
		else                                             menuitems_wifi[i].iconType = ICON_WIFI_BAD;
	}
	
	// now, change menu
	initMenuChange(&menu_wifilist);
}

void createMenuWifiList() {
	for (int i = 0; i < foundNetworkCount; i++) {
		addMenuItem(&menuitems_wifi[i]);
	}
}

void selectWifi(MenuItem *item) {
	int wifiNum = menu_wifilist.selectedOption;
	if (availableWifis[wifiNum].authMode == PocuterWIFI::WIFIAUTH_OPEN)
		tryToConnectToWifi();
	else
		initMenuChange(&menu_wifipassword);
}

void createMenuWifiPassword() {
	curWifiEdit = 0;
	wifiPassword[curWifiEdit] = '\0';
	curWifiEditChar = 0;
}

void updateMenuWifiPassword() {
    if (ACTION_UP) {
		curWifiEditChar -= 1;
		if (curWifiEditChar < 0)
			curWifiEditChar += allowedWifiPasswordCharactersCount;
    }
    if (ACTION_DOWN) {
		curWifiEditChar += 1;
		if (curWifiEditChar >= allowedWifiPasswordCharactersCount)
			curWifiEditChar -= allowedWifiPasswordCharactersCount;
	}
	if (ACTION_OK) {
		char c = allowedWifiPasswordCharacters[curWifiEditChar];
		if (c == '\n') {
			tryToConnectToWifi();
			initMenuChange(&menu_wifilist);
		} else {
			if (curWifiEdit == MAX_PASSWORD_LENGTH)
				return;
			
			wifiPassword[curWifiEdit] = c;
			curWifiEdit += 1;
			wifiPassword[curWifiEdit] = '\0';
		}
	}
}

void drawMenuWifiPassword() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	char strbuf[256];
	
    gui->UG_FontSelect(&FONT_POCUTER_5X8);
    gui->UG_SetForecolor(C_ORANGE);
	int x = gui->UG_StringWidth(wifiPassword) + 1;
	int y = sizeY*1/2;
	
	if (x > sizeX*3/4)
		x = sizeX*3/4;
	
	gui->UG_PutStringSingleLine(x - gui->UG_StringWidth(wifiPassword) - 1, y, wifiPassword);
	
    gui->UG_SetForecolor(C_GREEN);
	if ((millis() % 500) < 250) {
		sprintfWifiEditChar(strbuf, allowedWifiPasswordCharacters[curWifiEditChar]);
		gui->UG_PutStringSingleLine(x, y, strbuf);
	}
	
    gui->UG_SetForecolor(C_DARK_GREEN);
	int prevWifiEditChar = (curWifiEditChar + allowedWifiPasswordCharactersCount - 1) % allowedWifiPasswordCharactersCount;
	int nextWifiEditChar = (curWifiEditChar                                      + 1) % allowedWifiPasswordCharactersCount;
	
	sprintfWifiEditChar(strbuf, allowedWifiPasswordCharacters[prevWifiEditChar]);
	gui->UG_PutStringSingleLine(x, y-12, strbuf);
	
	sprintfWifiEditChar(strbuf, allowedWifiPasswordCharacters[nextWifiEditChar]);
	gui->UG_PutStringSingleLine(x, y+12, strbuf);
}

void sprintfWifiEditChar(char *strbuf, char c) {
	switch (c) {
		case ' ':
			sprintf(strbuf, "[ ]");
			break;
		case '\n':
			sprintf(strbuf, "[OK]");
			break;
		default:
			sprintf(strbuf, "%c", c);
			break;
	}
}

void tryToConnectToWifi() {
	int wifiNum = menu_wifilist.selectedOption;
	PocuterWIFI::wifiCredentials credentials;
	strcpy((char*) credentials.ssid, (char*) availableWifis[wifiNum].ssid);
	strcpy((char*) credentials.password, wifiPassword);
	PocuterWIFI::WIFIERROR err = pocuter->WIFI->connect(&credentials);
}