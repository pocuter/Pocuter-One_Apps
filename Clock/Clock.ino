#include "settings.h"
#include "system.h"

long lastFrame = 0;
int clockCenterX, clockCenterY, clockRadius;

void setup() {
    pocuter = new Pocuter();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    disableDoubleClick(BUTTON_A);
    disableDoubleClick(BUTTON_B);
    disableDoubleClick(BUTTON_C);
    
    lastFrame = micros();
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    char strbuf[256];

    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);

    clockRadius = (sizeX < sizeY ? sizeX : sizeY)/2-1;
    clockCenterX = sizeX/2;
    clockCenterY = sizeY/2;
}

void loop() {
    dt = (micros() - lastFrame) / 1000.0 / 1000.0;
    lastFrame = micros();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    char strbuf[256];
    int x, y;
    gui->UG_FontSelect(&FONT_POCUTER_5X7);
    gui->UG_SetForecolor(C_WHITE);

    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);

    gui->UG_FillCircle(clockCenterX, clockCenterY, clockRadius, 0x202020);
    gui->UG_DrawCircle(clockCenterX, clockCenterY, clockRadius, 0xFFFFFF);

    for (int i = 1; i <= 12; i += 1) {
        getClockPosition(i * 30, 0.85, &x, &y);
        if (i % 3 == 0) {
            sprintf(strbuf, "%d", i);
            gui->UG_PutStringSingleLine(x-gui->UG_StringWidth(strbuf)/2, y-6, strbuf);   
        } else {
            gui->UG_DrawPixel(x, y, C_WHITE);
        }
    }
    
    tm localTime;
    pocuter->PocTime->getLocalTime(&localTime);

    // hour hand
    getClockPosition(localTime.tm_hour*30 + localTime.tm_min/2, 0.35, &x, &y);
    drawThickLine(gui, clockCenterX, clockCenterY, x, y, pocuterSettings.systemColor);
    // minute hand
    getClockPosition(localTime.tm_min*6 + localTime.tm_sec/10, 0.55, &x, &y);
    drawThickLine(gui, clockCenterX, clockCenterY, x, y, pocuterSettings.systemColor);
    // second hand
    getClockPosition(localTime.tm_sec*6, 0.70, &x, &y);
    gui->UG_DrawLine(clockCenterX, clockCenterY, x, y, pocuterSettings.systemColor);
    
    pocuter->Display->updateScreen();
}

void getClockPosition(int degree, double distance, int *storeX, int *storeY) {
    *storeX = clockCenterX + round( sin(degree / 360.0 * 2*PI) * clockRadius*distance);
    *storeY = clockCenterY + round(-cos(degree / 360.0 * 2*PI) * clockRadius*distance);
}

void drawThickLine(UGUI *gui, int x0, int y0, int x1, int y1, UG_COLOR color) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            gui->UG_DrawLine(x0+dx, y0+dy, x1+dx, y1+dy, color);
        }
    }
}

void drawButtonText(UGUI *gui, const char *str, int corner, int sizeX, int sizeY) {
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
    if (corner == 0) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str)-3, 0, sizeX, 10, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, 0, sizeX, 10, C_GREEN);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, -1, str);
    } else if (corner == 1) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str)-3, sizeY-11, sizeX, sizeY-1, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, sizeY-11, sizeX, sizeY-1, C_GREEN);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, sizeY-12, str);
    } else if (corner == 2) {
        gui->UG_FillFrame(-1, sizeY-11, gui->UG_StringWidth(str)+2, sizeY-1, C_BLACK);
        gui->UG_DrawFrame(-1, sizeY-11, gui->UG_StringWidth(str)+2, sizeY-1, C_GREEN);
        gui->UG_PutString(1, sizeY-12, str);
    }
}
