// ========================================
// INCLUDES
// ========================================

#include "../../icons.h"
#include "../../settings.h"
#include "menu_brightness.h"
#include "menu_settings.h"

// ========================================
// MACROS
// ========================================

#define SCROLL_SPEED                10.0
#define MAX_BRIGHTNESS              15

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

void createMenuBrightness();
void updateMenuBrightness();
void drawMenuBrightness();

// ========================================
// GLOBALS
// ========================================

Menu menu_brightness = {
    "Brightness",           // title
    NULL,                   // icon
    &menu_settings,         // parent menu
    false,                  // can loop items
    
    createMenuBrightness,   // create func
    updateMenuBrightness,   // update func
    drawMenuBrightness      // draw func
};

double brightnessAnim;

// ========================================
// FUNCTIONS
// ========================================

void createMenuBrightness() {
	brightnessAnim = pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
}

void updateMenuBrightness() {
    if (ACTION_UP) {
        if (pocuterSettings.brightness > 0) {
			pocuterSettings.brightness -= 1;
			pocuter->Display->setBrightness(pocuterSettings.brightness);
        }
    }
    if (ACTION_DOWN) {
        if (pocuterSettings.brightness < MAX_BRIGHTNESS) {
            pocuterSettings.brightness += 1;
			pocuter->Display->setBrightness(pocuterSettings.brightness);
        }
    }
	if (ACTION_BACK || ACTION_OK) {
		setSetting("GENERAL", "Brightness", pocuterSettings.brightness);
	}
	
	if (ACTION_OK) {
		initMenuChange(menu_brightness.parentMenu);
	}
	
	updateMenuDefault();
}

void drawMenuBrightness() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	int sliderXL = sizeX * 1/4;
	int sliderXR = sizeX * 3/4;
	int sliderY  = sizeY * 1/2;
	
	gui->UG_DrawLine(sliderXL, sliderY, sliderXR, sliderY, C_WHITE);
	
	int iconY = sliderY;
	int leftIconX  = sliderXL - 12;
	int rightIconX = sliderXR + 12;
	
	drawIcon(getIcon(ICON_BRIGHTNESS_DARK  ), leftIconX -10, iconY-9);
	drawIcon(getIcon(ICON_BRIGHTNESS_BRIGHT), rightIconX- 9, iconY-9);
	
	if (brightnessAnim > pocuterSettings.brightness) {
		brightnessAnim -= SCROLL_SPEED * dt;
		if (brightnessAnim < pocuterSettings.brightness)
			brightnessAnim = pocuterSettings.brightness;
	}
	if (brightnessAnim < pocuterSettings.brightness) {
		brightnessAnim += SCROLL_SPEED * dt;
		if (brightnessAnim > pocuterSettings.brightness)
			brightnessAnim = pocuterSettings.brightness;
	}
	
	double s = (brightnessAnim) / MAX_BRIGHTNESS;
	int cx = round(sliderXL + s*(sliderXR-sliderXL));
	gui->UG_FillCircle(cx, sliderY, 3, C_ORANGE);
	/*
	gui->UG_DrawFrame(0, 0, 95, 63, C_WHITE);
	gui->UG_DrawLine( 1,  1, 48, 32, C_RED);
	gui->UG_DrawLine( 1, 62, 48, 32, C_RED);
	gui->UG_DrawLine(94,  1, 48, 32, C_RED);
	gui->UG_DrawLine(94, 62, 48, 32, C_RED);*/
}