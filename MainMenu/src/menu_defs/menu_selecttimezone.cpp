// ========================================
// INCLUDES
// ========================================

#include "menu_settings.h"
#include "menu_dateandtime.h"

// ========================================
// MACROS
// ========================================

#define TIME_ZONE_FOLDER_COUNT		11

// ========================================
// TYPES
// ========================================

struct TimeZoneFolder {
	const char *name;
	uint16_t firstEntry, lastEntry;
};

// ========================================
// PROTOTYPES
// ========================================

void createMenuSelectTimeZone();
void updateMenuSelectTimeZone();
void drawMenuSelectTimeZone();
void formatTZString(char *dest, const char *src, boolean cutFolder);

// ========================================
// GLOBALS
// ========================================

Menu menu_selecttimezone = {
    "Time Zone",            // title
    NULL,                   // icon
    &menu_dateandtime,      // parent menu
    false,                  // can loop items
    
    createMenuSelectTimeZone, // create func
    updateMenuSelectTimeZone, // update func
    drawMenuSelectTimeZone  // draw func
};

TimeZoneFolder timeZoneFolders[TIME_ZONE_FOLDER_COUNT] = {
	{"Africa",		  0,  51},
	{"America",		 52, 200}, 
	{"Antarctica",	201, 211},
	{"Arctic",		212, 212},
	{"Asia",		213, 294},
	{"Atlantic",	295, 304},
	{"Australia",	305, 316},
	{"Europe",		317, 376},
	{"Indian",		377, 387},
	{"Pacific",		388, 425},
	{"Etc",			426, 460}
};

int selectedFolder, selectedTimeZone;
const PocuterTime::pocuterTimezone *timeZones;
uint16_t timeZoneCount;

// ========================================
// FUNCTIONS
// ========================================

void createMenuSelectTimeZone() {
	selectedFolder = 0;
	selectedTimeZone = -1;
	pocuter->PocTime->getAllTimezones(&timeZones, &timeZoneCount);
}

void updateMenuSelectTimeZone() {
	if (selectedTimeZone == -1) {
		if (ACTION_UP)
			selectedFolder -= 1;
		if (ACTION_DOWN)
			selectedFolder += 1;
		selectedFolder = (selectedFolder + TIME_ZONE_FOLDER_COUNT) % TIME_ZONE_FOLDER_COUNT;
		
		if (ACTION_OK)
			selectedTimeZone = timeZoneFolders[selectedFolder].firstEntry;
	} else {
		if (ACTION_UP) {
			selectedTimeZone -= 1;
			if (selectedTimeZone < timeZoneFolders[selectedFolder].firstEntry)
				selectedTimeZone = timeZoneFolders[selectedFolder].lastEntry;
		}
		if (ACTION_DOWN) {
			selectedTimeZone += 1;
			if (selectedTimeZone > timeZoneFolders[selectedFolder].lastEntry)
				selectedTimeZone = timeZoneFolders[selectedFolder].firstEntry;
		}
		
		if (ACTION_OK) {
			pocuter->PocTime->setTimezone(&timeZones[selectedTimeZone], true);
			initMenuChange(menu_selecttimezone.parentMenu);
		}
	}
	
	updateMenuDefault();
}

void drawMenuSelectTimeZone() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
	
	char strbuf[128];
	
	if (selectedTimeZone == -1) {
		for (int i = -1; i <= 1; i++) {
			TimeZoneFolder *folder = &timeZoneFolders[(selectedFolder + TIME_ZONE_FOLDER_COUNT + i) % TIME_ZONE_FOLDER_COUNT];
			formatTZString(strbuf, folder->name, false);
			int x = sizeX/2 - gui->UG_StringWidth(strbuf)/2;
			int y = sizeY * (2+i) / 8;
			gui->UG_SetForecolor((i == 0) ? C_ORANGE : C_GRAY);
			gui->UG_PutStringSingleLine(x, y, strbuf);
		}
	} else {
		TimeZoneFolder *folder = &timeZoneFolders[selectedFolder];
		formatTZString(strbuf, folder->name, false);
		int x = sizeX/2 - gui->UG_StringWidth(strbuf)/2;
		int y = sizeY*2/8;
		gui->UG_SetForecolor(C_GRAY);
		gui->UG_PutStringSingleLine(x, y, strbuf);
		
		for (int i = -1; i <= 1; i++) {
			int tzIndex = selectedTimeZone + i;
			if (tzIndex < timeZoneFolders[selectedFolder].firstEntry)
				tzIndex = timeZoneFolders[selectedFolder].lastEntry;
			if (tzIndex > timeZoneFolders[selectedFolder].lastEntry)
				tzIndex = timeZoneFolders[selectedFolder].firstEntry;
			
			const PocuterTime::pocuterTimezone *timeZone = &timeZones[tzIndex];
			formatTZString(strbuf, timeZone->name, true);
			int x = sizeX/2 - gui->UG_StringWidth(strbuf)/2;
			if (x < 0)
				x = 0;
			int y = sizeY * (5+i) / 8;
			gui->UG_SetForecolor((i == 0) ? C_ORANGE : C_GRAY);
			gui->UG_PutStringSingleLine(x, y, strbuf);
		}
	}
}

void formatTZString(char *dest, const char *src, boolean cutFolder) {
	if (cutFolder) {
		while (*src != '/')
			src++;
		src++;
	}
	
	while (*src != '\0') {
		if (*src == '_')
			*dest = ' ';
		else
			*dest = *src;
		
		src++;
		dest++;
	}
	*dest = '\0';
}