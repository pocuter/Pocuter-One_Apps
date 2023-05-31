// ========================================
// INCLUDES
// ========================================

#include "../../settings.h"
#include "menu_systemcolor.h"
#include "menu_settings.h"

// ========================================
// MACROS
// ========================================

#define BLINK_SPEED					2.0
#define NUM_AVAILABLE_COLORS        8

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuSystemColor();
void updateMenuSystemColor();
void drawMenuSystemColor();

// ========================================
// GLOBALS
// ========================================

Menu menu_systemcolor = {
    "System Color",         // title
    NULL,                   // icon
    &menu_settings,         // parent menu
    false,                  // can loop items
    
    createMenuSystemColor,   // create func
    updateMenuSystemColor,   // update func
    drawMenuSystemColor      // draw func
};

const uint32_t availableColors[NUM_AVAILABLE_COLORS] = {
	C_WHITE,
	C_RED,
	C_GOLD,
	C_LIME,
	C_DARK_GREEN,
	C_AQUA,
	C_BLUE,
	C_MAGENTA
};

uint32_t systemColor;
int systemColorVal;
double blinkTime;

// ========================================
// FUNCTIONS
// ========================================

void createMenuSystemColor() {
	pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);
	
	systemColorVal = -1;
	for (int i = 0; i < NUM_AVAILABLE_COLORS; i++) {
		if (availableColors[i] == pocuterSettings.systemColor) {
			systemColorVal = i;
			break;
		}
	}
	if (systemColorVal == -1)
		systemColorVal = 0;
	
	blinkTime = 0;
}

void updateMenuSystemColor() {
    if (ACTION_UP) {
        if (systemColorVal > 0) {
			systemColorVal -= 1;
			pocuterSettings.systemColor = availableColors[systemColorVal];
			blinkTime = 0;
        }
    }
    if (ACTION_DOWN) {
        if (systemColorVal < NUM_AVAILABLE_COLORS-1) {
            systemColorVal += 1;
			pocuterSettings.systemColor = availableColors[systemColorVal];
			blinkTime = 0;
        }
    }
	if (ACTION_BACK || ACTION_OK) {
		setSetting("GENERAL", "SystemColor", pocuterSettings.systemColor);
	}
	
	if (ACTION_OK) {
		initMenuChange(menu_systemcolor.parentMenu);
	}
	
	updateMenuDefault();
}

void drawMenuSystemColor() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	blinkTime += BLINK_SPEED*dt;
	if (blinkTime > 1.0)
		blinkTime -= 1.0;

	int sizeColorBox = sizeX / NUM_AVAILABLE_COLORS - 2;
	
	for (int i = 0; i < NUM_AVAILABLE_COLORS; i++) {
		int x = sizeX/2 - ((sizeColorBox+2)*NUM_AVAILABLE_COLORS)/2 + (sizeColorBox+2)*i;
		int y = sizeY/2 - sizeColorBox/2;
		
		uint32_t color = availableColors[i];
		if (i == systemColorVal) {
			double blinkAmount = blinkTime*2;
			if (blinkAmount > 1.0)
				blinkAmount = 2.0 - blinkAmount;
			
			color = interpolateColorRGB888(color, C_BLACK, blinkAmount);
		}
		gui->UG_FillFrame(x, y, x+sizeColorBox, y+sizeColorBox, color);
	}
}