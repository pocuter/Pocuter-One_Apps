// ========================================
// INCLUDES
// ========================================

#include "../../settings.h"
#include "menu_settings.h"
#include "menu_standby.h"

// ========================================
// MACROS
// ========================================

#define STANDBY_ENTRY_NUM 9

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuStandby();
void updateMenuStandby();
void drawMenuStandby();

// ========================================
// GLOBALS
// ========================================

Menu menu_standby = {
    "Standby",              // title
    NULL,                   // icon
    &menu_settings,         // parent menu
    false,                  // can loop items
    
    createMenuStandby,      // create func
    updateMenuStandby,      // update func
    drawMenuStandby         // draw func
};

int timeUntilStandbyValues[STANDBY_ENTRY_NUM] = {
	5,
	15,
	30,
	60,
	2*60,
	5*60,
	10*60,
	15*60,
	0
};

const char *timeUntilStandbyDescriptions[STANDBY_ENTRY_NUM] = {
	"5 Seconds",
	"10 Seconds",
	"30 Seconds",
	"1 Minute",
	"2 Minutes",
	"5 Minutes",
	"10 Minutes",
	"15 Minutes",
	"Never"
};

int selectedStandbyEntry;

// ========================================
// FUNCTIONS
// ========================================

void createMenuStandby() {
	selectedStandbyEntry = 2;
	for (int i = 0; i < STANDBY_ENTRY_NUM; i++) {
		if (pocuterSettings.timeUntilStandby == timeUntilStandbyValues[i]) {
			selectedStandbyEntry = i;
			break;
		}
	}
}

void updateMenuStandby() {
    if (ACTION_UP) {
        if (selectedStandbyEntry > 0) {
			selectedStandbyEntry -= 1;
        }
    }
    if (ACTION_DOWN) {
        if (selectedStandbyEntry < STANDBY_ENTRY_NUM-1) {
            selectedStandbyEntry += 1;
        }
    }
	if (ACTION_BACK || ACTION_OK) {
		pocuterSettings.timeUntilStandby = timeUntilStandbyValues[selectedStandbyEntry];
		setSetting("GENERAL", "TimeUntilStandby", pocuterSettings.timeUntilStandby);
		pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
	}
	
	if (ACTION_OK) {
		initMenuChange(menu_standby.parentMenu);
	}
	
	updateMenuDefault();
}

void drawMenuStandby() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	for (int i = 0; i <= 2; i++) {
		int n = selectedStandbyEntry-1+i;
		if (n < 0 || n >= STANDBY_ENTRY_NUM)
			continue;
		
		int y = (i+2) * sizeY*1/6;
		
		gui->UG_FontSelect(&FONT_POCUTER_4X6);
		gui->UG_SetForecolor(i == 1 ? 0x00FF00 : 0x008000);
		gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(timeUntilStandbyDescriptions[n])/2, y, timeUntilStandbyDescriptions[n]);
	}
}