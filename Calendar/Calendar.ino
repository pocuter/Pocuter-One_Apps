#include "settings.h"
#include "system.h"

const char *monthStr[] = {
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

long lastFrame = 0, lastTodaysDateCheck = 0;
bool editMonth = true;
int selYear = 2022, selMonth = 1;
int todayYear = 1900, todayMonth = 1, todayDay = 1;

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
    
    tm localTime;
    pocuter->PocTime->getLocalTime(&localTime);
    if (localTime.tm_year != 0) {
        todayDay = localTime.tm_mday;
        todayMonth = selMonth = localTime.tm_mon + 1;
        todayYear = selYear = localTime.tm_year + 1900;
    }
    
    lastFrame = micros();
    lastTodaysDateCheck = millis() - 1000;
}

void loop() {
    dt = (micros() - lastFrame) / 1000.0 / 1000.0;
    lastFrame = micros();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }

    // if today's year is 1900, it has not been retreived yet. try to get today's date once a second
    if (todayYear == 1900) {
        if (millis() - lastTodaysDateCheck > 1000) {
            lastTodaysDateCheck = millis();
            tm localTime;
            pocuter->PocTime->getLocalTime(&localTime);
            todayDay = localTime.tm_mday;
            todayMonth = localTime.tm_mon + 1;
            todayYear = localTime.tm_year + 1900;
        }
    }

    if (ACTION_SWAP)
        editMonth = !editMonth;
    if (editMonth) {
        if (ACTION_NEXT) {
            selMonth += 1;
            if (selMonth == 13) {
                selMonth = 1;
                selYear += 1;
            }
        }
        if (ACTION_PREVIOUS) {
            selMonth -= 1;
            if (selMonth == 0) {
                selMonth = 12;
                selYear -= 1;
            }
        }
    } else {
        if (ACTION_NEXT)
            selYear += 1;
        if (ACTION_PREVIOUS)
            selYear -= 1;
    }
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    char strbuf[256];

    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);

    int mc = selMonth;
    int yc = selYear;

    int mp = mc-1, mn = mc+1, yp = yc, yn = yc;
    if (mp == 0) {
        mp = 12;
        yp -= 1;
    }
    if (mn == 13) {
        mn = 1;
        yn += 1;
    }
    
    int dayOfWeekFirstP = getDayOfWeek(1, mp, yp);
    int dayOfWeekFirstM = getDayOfWeek(1, mc, yc);
    int dayOfWeekFirstN = getDayOfWeek(1, mn, yn);
    int daysInMonth      = 28 + ((dayOfWeekFirstN+6)%7 - dayOfWeekFirstM + 8)%7;
    int daysInMonthPrior = 28 + ((dayOfWeekFirstM+6)%7 - dayOfWeekFirstP + 8)%7;
    int weeksInMonth = (dayOfWeekFirstM + daysInMonth + 6)/7;

    int tileWidth = (sizeX-6)/7;
    int offsetX = (sizeX - tileWidth*7)/2;
    int offsetY = sizeY - 6*8 - 1;
    int tileHeight = (sizeY-offsetY-1)/weeksInMonth;

    //drawButtonText(gui, "Mon/Year", 0, sizeX, sizeY);

    gui->UG_SetForecolor(C_ORANGE);
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    
    sprintf(strbuf, "%s %d", monthStr[10], selYear); // 10 = september, longest name
    int maxDateWidth = gui->UG_StringWidth(strbuf);
    sprintf(strbuf, "%s", monthStr[selMonth-1]);
    int curMonthWidth = gui->UG_StringWidth(strbuf);
    sprintf(strbuf, "%s %d", monthStr[selMonth-1], selYear);
    int curDateWidth = gui->UG_StringWidth(strbuf);
    
    gui->UG_PutStringSingleLine(sizeX/2 + maxDateWidth/2 - curDateWidth + 1, offsetY/2-5, strbuf);
    int lx;
    if (editMonth) {
        gui->UG_DrawFrame(sizeX/2 + maxDateWidth/2 - curDateWidth - 1, offsetY/2-4, sizeX/2 + maxDateWidth/2 - curDateWidth + curMonthWidth + 2, offsetY/2+5, C_GREEN);
        lx = ((sizeX/2 + maxDateWidth/2 - curDateWidth - 1) + (sizeX/2 + maxDateWidth/2 - curDateWidth + curMonthWidth + 2))/2;
    } else {
        gui->UG_DrawFrame(sizeX/2 + maxDateWidth/2 - curDateWidth + curMonthWidth + 3, offsetY/2-4, sizeX/2 + maxDateWidth/2 + 2, offsetY/2+5, C_GREEN);
        lx = ((sizeX/2 + maxDateWidth/2 - curDateWidth + curMonthWidth + 3) + (sizeX/2 + maxDateWidth/2 + 2))/2;
    }
    
    sprintf(strbuf, "<<");
    gui->UG_PutStringSingleLine(3, offsetY/2-5, strbuf);
    gui->UG_DrawFrame(0, offsetY/2-4, 10, offsetY/2+5, C_GREEN);

    sprintf(strbuf, ">>");
    gui->UG_PutStringSingleLine(sizeX-3 - gui->UG_StringWidth(strbuf), offsetY/2-5, strbuf);
    gui->UG_DrawFrame(sizeX-11, offsetY/2-4, sizeX-1, offsetY/2+5, C_GREEN);
    
    gui->UG_DrawLine(lx, offsetY/2-4, lx, 0, C_GREEN);
    gui->UG_DrawLine(lx, 0, sizeX, 0, C_GREEN);

    gui->UG_DrawLine(2, offsetY/2+5, 2, sizeY-1, C_GREEN);
    gui->UG_DrawLine(0, sizeY-1, 2, sizeY-1, C_GREEN);

    gui->UG_DrawLine(sizeX-3, offsetY/2+5, sizeX-3, sizeY-1, C_GREEN);
    gui->UG_DrawLine(sizeX-3, sizeY-1, sizeX-1, sizeY-1, C_GREEN);
    
    gui->UG_FontSelect(&FONT_POCUTER_3X5);
    gui->UG_SetForecolor(0x8080FF);
    for (int i = 0; i < daysInMonth; i++) {
        int d = dayOfWeekFirstM + i;
        int x = offsetX + tileWidth *(d%7);
        int y = offsetY + tileHeight*(d/7);
        UG_COLOR tileColor = d&1 ? 0x202020 : 0x404040;
        if (selYear == todayYear && selMonth == todayMonth && i+1 == todayDay)
            tileColor = 0xFF0000;
        gui->UG_FillFrame(x, y, x+tileWidth-1, y+tileHeight-1, tileColor);
        sprintf(strbuf, "%d", i+1);
        gui->UG_PutStringSingleLine(x+sizeX/7/2 - gui->UG_StringWidth(strbuf)/2, y+tileHeight/2-5, strbuf);
    }
    
    gui->UG_SetForecolor(0xFFFFFF);
    for (int i = dayOfWeekFirstM-1, j=daysInMonthPrior; i >= 0; i--, j--) {
        int d = i;
        int x = offsetX + tileWidth *(d%7);
        int y = offsetY + tileHeight*(d/7);
        gui->UG_FillFrame(x, y, x+tileWidth-1, y+tileHeight-1, d&1 ? 0x202020 : 0x404040);
        sprintf(strbuf, "%d", j);
        gui->UG_PutStringSingleLine(x+sizeX/7/2 - gui->UG_StringWidth(strbuf)/2, y+tileHeight/2-5, strbuf);
    }
    for (int i = dayOfWeekFirstM + daysInMonth, j=1; i%7 != 0; i++, j++) {
        int d = i;
        int x = offsetX + tileWidth *(d%7);
        int y = offsetY + tileHeight*(d/7);
        gui->UG_FillFrame(x, y, x+tileWidth-1, y+tileHeight-1, d&1 ? 0x202020 : 0x404040);
        sprintf(strbuf, "%d", j);
        gui->UG_PutStringSingleLine(x+sizeX/7/2 - gui->UG_StringWidth(strbuf)/2, y+tileHeight/2-5, strbuf);
    }
    pocuter->Display->updateScreen();
}

int getDayOfWeek_1StMonthPrior(int m, int y) {
    int d = 1;
    if (m == 1) {
        m = 12;
        y--;
    } else {
        m--;
    }
    int weekday = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4 - y/100 + y/400)%7;
    return (weekday + 6) % 7;
}

int getDayOfWeek(int d, int m, int y) {
    int weekday = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4 - y/100 + y/400)%7;
    return (weekday + 6) % 7;
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
