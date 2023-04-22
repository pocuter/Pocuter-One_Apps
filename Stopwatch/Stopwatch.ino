#include "settings.h"
#include "system.h"

#define MAX_LAPS 256
#define LAP_VIEW_SCROLL_SPEED 10.0

long lastFrame;

boolean running = false, lapView = false;
double stoppedTime = 0;
double lapEnterTimes[MAX_LAPS];
int lapCount = 0;
double lapViewScroll = 0, lapViewScrollAnim = 0;

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
}

void loop() {
    dt = (micros() - lastFrame) / 1000.0 / 1000.0;
    lastFrame = micros();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }

    if (running)
        stoppedTime += dt;

    bool setToMaxScroll = false;
    if (lapView) {
        if (ACTION_LAP_VIEW_UP)
            lapViewScroll -= 1.0;
        if (ACTION_LAP_VIEW_DOWN)
            lapViewScroll += 1.0;
        if (ACTION_TOGGLE_LAP_VIEW)
            lapView = false;
    } else if (!running) {
        if (ACTION_RESET) {
            stoppedTime = 0;
            lapCount = 0;
            lapViewScroll = lapViewScrollAnim = 0;
        }
        if (ACTION_RESUME)
            running = true;
        if (ACTION_TOGGLE_LAP_VIEW && lapCount != 0)
            lapView = true;
    } else {
        if (ACTION_PAUSE)
            running = false;
        if (ACTION_LAP) {
            if (lapCount < MAX_LAPS) {
                lapEnterTimes[lapCount] = stoppedTime;
                lapCount++;
            }
            setToMaxScroll = true;
        }
    }
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);

    // 10 laps: (80 + 52) / 8 = 132/8 = 16.5
    // 10 laps: (80 - 52) / 8 =  28/8 =  3.5
    // 6  laps: (48 - 52) / 8 =  -4/8 = -0.5
    // 7  laps: (56 - 52) / 8 =   4/8 =  0.5
    double maxLapScroll = (8.0*lapCount - (sizeY - 12)) / 8.0; // the button on the bottom is 12 px in height
    if (setToMaxScroll) {
        lapViewScrollAnim = maxLapScroll - 1;
        lapViewScroll = maxLapScroll;
    }
    
    if (lapViewScroll > maxLapScroll)
        lapViewScroll = lapViewScrollAnim = maxLapScroll;
    if (lapViewScroll < 0.0)
        lapViewScroll = lapViewScrollAnim = 0.0;

    if (lapViewScrollAnim < lapViewScroll) {
        lapViewScrollAnim += LAP_VIEW_SCROLL_SPEED * dt;
        if (lapViewScrollAnim > lapViewScroll)
            lapViewScrollAnim = lapViewScroll;
    }
    if (lapViewScrollAnim > lapViewScroll) {
        lapViewScrollAnim -= LAP_VIEW_SCROLL_SPEED * dt;
        if (lapViewScrollAnim < lapViewScroll)
            lapViewScrollAnim = lapViewScroll;
    }

    char strbuf[128];
    
    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    gui->UG_DrawLine(sizeX/2, 0, sizeX/2, sizeY, C_GREEN);

    gui->UG_FontSelect(&FONT_POCUTER_5X7);
    gui->UG_SetForecolor(pocuterSettings.systemColor);

    gui->UG_PutStringSingleLine(sizeX*3/4 - gui->UG_StringWidth("Total:")/2, sizeY*2/5-5, "Total:");
    sprintfTime(strbuf, stoppedTime);
    gui->UG_PutStringSingleLine(sizeX*3/4 - gui->UG_StringWidth(strbuf)/2, sizeY*3/5-5, strbuf);

    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    for (int i = 0; i < lapCount; i++) {
        int y = -2 + floor(8 * (i - lapViewScrollAnim));
        sprintf(strbuf, "%d", i+1);
        gui->UG_PutStringSingleLine(0, y, strbuf);

        double lapTime = lapEnterTimes[i];
        if (i > 0)
            lapTime -= lapEnterTimes[i-1];
        sprintfTime(strbuf, lapTime);
        gui->UG_PutStringSingleLine(sizeX*1/2 - gui->UG_StringWidth(strbuf) - 1, y, strbuf);
    }

    if (lapView) {
        drawButtonText(gui, "Up", 0, sizeX, sizeY);
        drawButtonText(gui, "Down", 1, sizeX, sizeY);
        drawButtonText(gui, "Back", 2, sizeX, sizeY);
    } else if (!running) {
        if (stoppedTime == 0.0) {
            drawButtonText(gui, "Start", 1, sizeX, sizeY);
        } else {
            drawButtonText(gui, "Resume", 1, sizeX, sizeY);
            drawButtonText(gui, "Reset", 0, sizeX, sizeY);
            if (lapCount != 0)
                drawButtonText(gui, "View Laps", 2, sizeX, sizeY);
        }
    } else {
        drawButtonText(gui, "Pause", 0, sizeX, sizeY);
        drawButtonText(gui, "Lap", 1, sizeX, sizeY);
    }
    
    pocuter->Display->updateScreen();
}

void sprintfTime(char *strbuf, double t) {
    int hs = (int) (t * 100    ) % 100;
    int s  = (int) (t          ) % 60;
    int m  = (int) (t / 60     ) % 60;
    int h  = (int) (t / 60 / 60);
    
    if (h == 0)
        sprintf(strbuf, "%d:%02d.%02d", m, s, hs);
    else
        sprintf(strbuf, "%d:%02d:%02d.%02d", h, m, s, hs);
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
