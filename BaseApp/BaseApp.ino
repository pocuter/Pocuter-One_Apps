#include "settings.h"
#include "system.h"

long lastFrame;

void setup() {
    pocuter = new Pocuter();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    // enable or disable double click (disabling can achieve faster reaction to single clicks)
    enableDoubleClick(BUTTON_A);
    enableDoubleClick(BUTTON_B);
    enableDoubleClick(BUTTON_C);

    // setup your app here
    
    lastFrame = micros();
}

void loop() {
    dt = (micros() - lastFrame) / 1000.0 / 1000.0;
    lastFrame = micros();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }

    // update your app here
    // dt contains the amount of time that has passed since the last update, in seconds
}
