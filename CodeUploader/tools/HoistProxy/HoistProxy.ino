
// Copyright 2023 Kallistisoft
// Copyright 2023 Pocuter GmbH, Pocuter Inc.
// GNU GPL-3 https://www.gnu.org/licenses/gpl-3.0.txt

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
    disableDoubleClick(BUTTON_A);
    disableDoubleClick(BUTTON_B);
    disableDoubleClick(BUTTON_C);

    // setup your app here    
    lastFrame = micros();

    printf("\n\n");
    printf("**********************************************\n");
    printf("... Code Uploader Self-Hoisting Boot Proxy ...\n");
    printf("**********************************************\n");

    delay(100);  
}

// launch: Code Uploader application 
void loop() { 
    pocuter->OTA->setNextAppID( 8080 );
    pocuter->OTA->restart();      
    delay(1000); 
}
