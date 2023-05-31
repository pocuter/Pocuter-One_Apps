// ========================================
// INCLUDES
// ========================================

#include "icons.h"
#include "menu.h"
#include "settings.h"
#include "src/menu_defs/menu_mainmenu.h"
#include "system.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

// ========================================
// PROTOTYPES
// ========================================

// ========================================
// GLOBALS
// ========================================

long lastFrame = 0;

// ========================================
// FUNCTIONS
// ========================================

void setup() {
    pocuter = new Pocuter();
    //pocuter->begin();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);
    pocuterSettings.showDateInTitleBar = getSetting("MAINMENU", "ShowDateInTitleBar", true);
    pocuterSettings.lastDayAskedForUpdate = getSetting("MAINMENU", "LastDayAskedForUpdate", 0);
    pocuterSettings.timeUntilStandby = getSetting("GENERAL", "TimeUntilStandby", 30);
    pocuterSettings.showBatteryInTitleBar = false;
    
    pocuter->Sleep->setSleepMode(PocuterSleep::SLEEP_MODE_LIGHT);
    pocuter->Sleep->setWakeUpModes(PocuterSleep::WAKEUP_MODE_ANY_BUTTON);
    pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON);
    
    disableDoubleClick(BUTTON_UP);
    disableDoubleClick(BUTTON_DOWN);
    enableDoubleClick(BUTTON_OK);

    for (int i = 0; i < (int) ICON_MAX; i += 1)
        loadIcon((IconType) i);

    changeMenu(&menu_mainmenu);
    
    lastFrame = millis();
}

void loop() {
    dt = (millis() - lastFrame) / 1000.0;
    if (dt > 0.025)
        dt = 0.025;
    lastFrame = millis();
    updateInput();
    runMenu();
}

void onPause() {}
void onResume() {}
