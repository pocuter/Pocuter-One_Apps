#include "settings.h"
#include "system.h"

#define MAX_PLAYERS 12
#define RING_PIXELS 360

long lastFrame = 0;

enum ScoreTrackerStatus {
    CREATE_NEW_TRACKER,
    TRACKING,
};

struct Score {
    int16_t xPos, yPos;
    int value;
};

struct RingColor {
    int16_t xPos, yPos;
    double s;
};

Score scores[MAX_PLAYERS];
RingColor ringColors[RING_PIXELS];
ScoreTrackerStatus status = CREATE_NEW_TRACKER;
int currentPlayer = 0, playerCount = 6;
double currentRotation = 0;

uint8_t circleDots[MAX_PLAYERS+1] = {
//  0   1   2   3   4   5   6   7   8   9   10  11  12
    0,  0,  36, 36, 36, 35, 36, 35, 32, 36, 30, 33, 36
};

void setup() {
    pocuter = new Pocuter();
    //pocuter->begin();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    disableDoubleClick(BUTTON_INCREASE);
    disableDoubleClick(BUTTON_DECREASE);
    disableDoubleClick(BUTTON_NEXT);

    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    for (int i = 0; i < RING_PIXELS; i++) {
        ringColors[i].s = (double) i / RING_PIXELS;
        int x, y;
        getScoreCirclePosition(ringColors[i].s*2*PI, sizeX, sizeY, &x, &y);
        ringColors[i].xPos = x;
        ringColors[i].yPos = y;
        
    }
    
    lastFrame = millis();
}

void loop() {
    dt = (millis() - lastFrame) / 1000.0;
    lastFrame = millis();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }

    switch (status) {
        case CREATE_NEW_TRACKER:
            handleCreatorInput();
            drawCreator();
            break;
        case TRACKING:
            handleTrackerInput();
            drawTracker();
            break;
    }
    
    pocuter->Display->updateScreen();
}

void handleCreatorInput() {
    if (ACTION_INCREASE && playerCount < MAX_PLAYERS) playerCount += 1;
    if (ACTION_DECREASE && playerCount >           2) playerCount -= 1;
    if (ACTION_START)
        status = TRACKING;
}

void handleTrackerInput() {
    if (ACTION_INCREASE) scores[currentPlayer].value += 1;
    if (ACTION_DECREASE) scores[currentPlayer].value -= 1;
    if (ACTION_NEXT)     currentPlayer = (currentPlayer+1) % playerCount;
}

void drawCreator() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    char strbuf[128];
    
    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    gui->UG_FontSelect(&FONT_POCUTER_5X7);

    gui->UG_SetForecolor(pocuterSettings.systemColor);
    sprintf(strbuf, "Score Tracker");
    gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(strbuf)/2, sizeY/4, strbuf);

    gui->UG_SetForecolor(C_ORANGE);
    sprintf(strbuf, "Players: %d", playerCount);
    gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(strbuf)/2, sizeY/2, strbuf);
    
    
    drawButtonText(gui, "Add Player", 0, sizeX, sizeY);
    drawButtonText(gui, "Remove Player", 1, sizeX, sizeY);
    drawButtonText(gui, "Start", 2, sizeX, sizeY);
}

void drawTracker() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    char strbuf[128];

    if (currentRotation != currentPlayer) {
        double oldRotation = currentRotation;
        currentRotation += 3.0 * dt;
        if (currentRotation >= playerCount) {
            oldRotation -= playerCount;
            currentRotation -= playerCount;
        }
        if (currentRotation > currentPlayer && oldRotation < currentPlayer)
            currentRotation = currentPlayer;
    }
    
    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    gui->UG_FontSelect(&FONT_POCUTER_5X7);
    gui->UG_SetForecolor(C_ORANGE);
    //gui->UG_FontSelect(&FONT_POCUTER_4X6);

    //double b = millis() / 500.0;
    //double rotation = (double) currentPlayer / playerCount * 2*PI;
    double rotation = currentRotation / playerCount * 2*PI;
    for (int i = 0; i < playerCount; i++) {
        double s = (double) i / playerCount;
        
        double a = s * 2*PI - rotation;
        int x, y;
        getScoreCirclePosition(a, sizeX, sizeY, &x, &y);
        scores[i].xPos = x;
        scores[i].yPos = y;
    }

    for (int i = 0; i < RING_PIXELS; i++) {
        double h = ringColors[i].s + currentRotation / playerCount;
        h = fmod(h, 1);
        UG_COLOR color = getHue(h);
        gui->UG_DrawPixel(ringColors[i].xPos, ringColors[i].yPos+7, color); 
    }
    
    for (int y = 0; y < sizeY; y++) {
        for (int i = 0; i < playerCount; i++) {
            if (y != scores[i].yPos)
                continue;
            
            int x = scores[i].xPos;
            double s = (double) i / playerCount;
            UG_COLOR color = getHue(s);
            
            sprintf(strbuf, "%d", scores[i].value);
            int w = gui->UG_StringWidth(strbuf);
            int h = 10;

            if (x-w/2-2 < 0)
                x = w/2+2;
            if (x+(w-w/2)+2 >= sizeX)
                x = sizeX-1-(w-w/2)-1;
            
            gui->UG_FillFrame(x-w/2-2, y-h/2, x+(w-w/2)+1, y+h/2, C_BLACK);
            gui->UG_DrawFrame(x-w/2-2, y-h/2, x+(w-w/2)+1, y+h/2, color);
            gui->UG_DrawPixel(scores[i].xPos, y+6, color); 
            
            gui->UG_PutStringSingleLine(x-w/2, y - 6, strbuf);
        }
    }
    
    drawButtonText(gui, "Inc.", 0, sizeX, sizeY);
    drawButtonText(gui, "Dec.", 1, sizeX, sizeY);
    drawButtonText(gui, "Next", 2, sizeX, sizeY);
}

void drawButtonText(UGUI *gui, const char *str, int corner, int sizeX, int sizeY) {
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
    UG_COLOR rectColor = C_GRAY;
    if (corner == 0) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str)-3, 0, sizeX, 10, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, 0, sizeX, 10, rectColor);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, -1, str);
    } else if (corner == 1) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str)-3, sizeY-11, sizeX, sizeY-1, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, sizeY-11, sizeX, sizeY-1, rectColor);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, sizeY-12, str);
    } else if (corner == 2) {
        gui->UG_FillFrame(-1, sizeY-11, gui->UG_StringWidth(str)+2, sizeY-1, C_BLACK);
        gui->UG_DrawFrame(-1, sizeY-11, gui->UG_StringWidth(str)+2, sizeY-1, rectColor);
        gui->UG_PutString(1, sizeY-12, str);
    }
}

void getScoreCirclePosition(double a, uint16_t sizeX, uint16_t sizeY, int *storeX, int *storeY) {
    double x = sin(a);
    double y = cos(a);

    x = x + x*(y+1)/8;

    *storeX = round(sizeX/2.0 - x * sizeX*12/32.0);
    *storeY = round(sizeY/2.0 + y * sizeY*12/32.0);
}

// get an RGB color from a hue (0..1).
// input color to HSV conversion would be (h*360, 1, 1).
UG_COLOR getHue(double h) {
    double s = 1.0, v = 1.0;
    double c = v*s;
    double x = c*(1 - fabs(fmod(h*6.0, 2) - 1));
    double m = v-c;
    
    double r, g, b;
    int i = (int) (h*6.0);
    switch (i) {
        case 0: r=c; g=x; b=0; break;
        case 1: r=x; g=c; b=0; break;
        case 2: r=0; g=c; b=x; break;
        case 3: r=0; g=x; b=c; break;
        case 4: r=x; g=0; b=c; break;
        case 5: r=c; g=0; b=x; break;
        default: r=g=b=1.0; break;
    }

    uint8_t ri = ((int) round((r+m) * 0xFF)) & 0xFF;
    uint8_t gi = ((int) round((g+m) * 0xFF)) & 0xFF;
    uint8_t bi = ((int) round((b+m) * 0xFF)) & 0xFF;

    UG_COLOR result = (ri << 16) | (gi << 8) | (bi);
    return result;
}
