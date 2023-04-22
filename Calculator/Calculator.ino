#include "settings.h"
#include "system.h"

#define MAX_DIGITS_PER_NUMBER 10

long lastFrame = 0;

enum CalculatorStatus {
    SHOW_CONTROLS,
    ENTER_NUMBER_A,
    ENTER_NUMBER_B,
    RESULT,
    RESULT_NO_CONTINUE,
    ERROR_DIVISION_BY_ZERO,
};

struct Button {
    int16_t xPos, yPos;
    uint16_t width, height;
    char value;
    const char *name;
};

Button buttons[] = {
    {0, 0, 0, 0, '0', "0"},
    {0, 0, 0, 0, '1', "1"},
    {0, 0, 0, 0, '2', "2"},
    {0, 0, 0, 0, '3', "3"},
    {0, 0, 0, 0, '4', "4"},
    {0, 0, 0, 0, '5', "5"},
    {0, 0, 0, 0, '6', "6"},
    {0, 0, 0, 0, '7', "7"},
    {0, 0, 0, 0, '8', "8"},
    {0, 0, 0, 0, '9', "9"},
    {0, 0, 0, 0, '.', "."},
    {0, 0, 0, 0, '+', "+"},
    {0, 0, 0, 0, '-', "-"},
    {0, 0, 0, 0, 'x', "x"},
    {0, 0, 0, 0, ':', ":"},
    {0, 0, 0, 0, '=', "="},
    {0, 0, 0, 0, 'C', "C"},
    {0, 0, 0, 0, 'A', "AC"},
};

int buttonCount = sizeof(buttons) / sizeof(buttons[0]);
int selectedButton = 0;
CalculatorStatus status = SHOW_CONTROLS;

//int64_t numberA = 0, numberB = 0;
double numberA, numberB, result;
char editNumber[MAX_DIGITS_PER_NUMBER+1+1+1]; // digits + sign + dot + terminator
int editNumberCur = 0;
char operation = '?';

void setButtonDim(int i, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    buttons[i].xPos   = x;
    buttons[i].yPos   = y;
    buttons[i].width  = w;
    buttons[i].height = h;
}

void setup() {
    pocuter = new Pocuter();
    //pocuter->begin();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    disableDoubleClick(BUTTON_LEFT);
    disableDoubleClick(BUTTON_RIGHT);
    disableDoubleClick(BUTTON_SELECT);

    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    
    int numberRowY = sizeY*7/12;
    int symbolRowY = sizeY*10/12;
    int iconWidth = sizeX*1/12;
    int iconHeight = sizeY*1/6;
    
    int i = 0;
    for (; i < 10; i++) {
        int x = sizeX/2 + (i-10/2.0)*iconWidth;
        setButtonDim(i, x, numberRowY, iconWidth, iconHeight);
    }
    for (; i < buttonCount; i++) {
        int x = sizeX/2 + ((i-10)-10/2.0)*iconWidth;

        if (i == buttonCount-1)
            iconWidth *= (20-buttonCount+1);
        
        setButtonDim(i, x, symbolRowY, iconWidth, iconHeight);
    }

    clearEditNumber();
    
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
    
    if (status == SHOW_CONTROLS) {
        char str[64];
        UGUI* gui = pocuter->ugui;
        uint16_t sizeX;
        uint16_t sizeY;
        pocuter->Display->getDisplaySize(sizeX, sizeY);
        
        gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
        
        gui->UG_FontSelect(&FONT_POCUTER_5X7);
        gui->UG_SetForecolor(pocuterSettings.systemColor);
        
        sprintf(str, "Controls");
        gui->UG_PutString(sizeX/2 - gui->UG_StringWidth(str)/2, 10, str);
        
        sprintf(str, "Press any button");
        gui->UG_PutString(sizeX/2 - gui->UG_StringWidth(str)/2, sizeY/2, str);
        sprintf(str, "to continue");
        gui->UG_PutString(sizeX/2 - gui->UG_StringWidth(str)/2, sizeY/2+10, str);
        
        gui->UG_FontSelect(&FONT_POCUTER_4X6);
        gui->UG_SetForecolor(C_ORANGE);

        sprintf(str, "Select");
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, 0, sizeX, 10, C_GRAY);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, -1, str);
        
        sprintf(str, "Right");
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str)-3, sizeY-11, sizeX, sizeY-1, C_GRAY);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str)-1, sizeY-12, str);
        
        sprintf(str, "Left");
        gui->UG_DrawFrame(-1, sizeY-11, gui->UG_StringWidth(str)+2, sizeY-1, C_GRAY);
        gui->UG_PutString(1, sizeY-12, str);
        
        if (ACTION_LEFT || ACTION_RIGHT || ACTION_SELECT)
            status = ENTER_NUMBER_A;
    } else {
        handleInput();
        draw();
    }
    
    pocuter->Display->updateScreen();
}

void handleInput() {
    if (ACTION_LEFT)   selectedButton = (selectedButton - 1 + buttonCount) % buttonCount;
    if (ACTION_RIGHT)  selectedButton = (selectedButton + 1              ) % buttonCount;
    if (ACTION_SELECT) handleButtonPress();
}

void handleButtonPress() {
    if (status == RESULT_NO_CONTINUE || status == ERROR_DIVISION_BY_ZERO) {
        // "AC" call is the only option here
        clearEditNumber();
        status = ENTER_NUMBER_A;
        return;
    }
    
    switch (buttons[selectedButton].value) {
        case '0':
            if (strcmp(editNumber, "0") == 0 || strcmp(editNumber, "-0") == 0) {
                return;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            int digits = 0;
            for (int i = 0; i < editNumberCur; i++) {
                if ('0' <= editNumber[i] && editNumber[i] <= '9')
                    digits += 1;
            }
            if (digits == MAX_DIGITS_PER_NUMBER)
                return;
            editNumber[editNumberCur] = buttons[selectedButton].value;
            editNumberCur += 1;
            return;
        }
        
        case '.': {
            if (strcmp(editNumber, "") == 0) {
                editNumber[0] = '0';
                editNumber[1] = '.';
                editNumberCur = 2;
                return;
            }
            if (strcmp(editNumber, "-") == 0) {
                editNumber[0] = '-';
                editNumber[1] = '0';
                editNumber[2] = '.';
                editNumberCur = 3;
                return;
            }
            for (int i = 0; i < editNumberCur; i++) {
                if (editNumber[i] == '.')
                    return;
            }
            editNumber[editNumberCur] = '.';
            editNumberCur += 1;
            return;
        }

        case '-':
            if (strcmp(editNumber, "") == 0) {
                editNumber[0] = '-';
                editNumberCur = 1;
                return;
            }
        case '+':
        case 'x':
        case ':':
            if (status == ENTER_NUMBER_A) {
                if (strcmp(editNumber, "") == 0 || strcmp(editNumber, "-") == 0)
                    return;
                
                numberA = atof(editNumber);
                operation = buttons[selectedButton].value;
                clearEditNumber();
                status = ENTER_NUMBER_B;
            } else if (status == ENTER_NUMBER_B) {
                if (strcmp(editNumber, "") == 0 || strcmp(editNumber, "-") == 0)
                    return;
                
                numberB = atof(editNumber);
                if (!calculateResult())
                    return;
                clearEditNumber();
                if (canContinueFromResult()) {
                    numberA = result;
                    operation = buttons[selectedButton].value;
                    status = ENTER_NUMBER_B;
                } else {
                    status = RESULT_NO_CONTINUE;
                }
            } else if (status == RESULT) {
                numberA = result;
                operation = buttons[selectedButton].value;
                clearEditNumber();
                status = ENTER_NUMBER_B;
            }
            return;

        case '=':
            if (strcmp(editNumber, "") == 0 || strcmp(editNumber, "-") == 0) {
                return;
            }
            if (status == ENTER_NUMBER_B) {
                numberB = atof(editNumber);
                if (!calculateResult())
                    return;
                clearEditNumber();
                if (canContinueFromResult())
                    status = RESULT;
                else
                    status = RESULT_NO_CONTINUE;
            }
            return;
        
        case 'C':
            clearEditNumber();
            return;
        
        case 'A':
            clearEditNumber();
            status = ENTER_NUMBER_A;
            return;
        
        default: {
            return;
        }
    }
}

bool calculateResult() {
    switch (operation) {
        case '+':
            result = numberA + numberB;
            break;
        case '-':
            result = numberA - numberB;
            break;
        case 'x':
            result = numberA * numberB;
            break;
        case ':':
            if (numberB == 0.0) {
                status = ERROR_DIVISION_BY_ZERO;
                return false;
            }
            result = numberA / numberB;
            break;
    }
    return true;
}

void clearEditNumber() {
    memset(editNumber, 0, sizeof(editNumber));
    editNumberCur = 0;
}

void draw() {
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    
    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    gui->UG_FontSelect(&FONT_POCUTER_5X7);

    int bSelected = selectedButton;
    if (status == RESULT_NO_CONTINUE || status == ERROR_DIVISION_BY_ZERO)
        bSelected = buttonCount-1; // auto-select AC
    
    for (int i = 0; i < buttonCount; i++) {
        gui->UG_SetForecolor((i == bSelected) ? 0xFFFFFF : 0x808080);
        gui->UG_FillFrame(buttons[i].xPos, buttons[i].yPos, buttons[i].xPos+buttons[i].width-1, buttons[i].yPos+buttons[i].height-1, (i == bSelected) ? 0x0000FF : 0x000040);
        gui->UG_DrawFrame(buttons[i].xPos, buttons[i].yPos, buttons[i].xPos+buttons[i].width-1, buttons[i].yPos+buttons[i].height-1, 0x000080);
        gui->UG_PutString(buttons[i].xPos + buttons[i].width/2 - gui->UG_StringWidth(buttons[i].name)/2, buttons[i].yPos-1, buttons[i].name);
    }

    int numberAY = 0;
    int numberBY = sizeY*1/6;
    int resultY  = sizeY*2/6;
    
    char strbuf[256];
    gui->UG_SetForecolor(pocuterSettings.systemColor);

    if (status == ENTER_NUMBER_A) {
        sprintf(strbuf, "%s%c", editNumber, '_');
        int w = gui->UG_StringWidth(strbuf);
        sprintf(strbuf, "%s%c", editNumber, millis() % 1000 < 500 ? ' ' : '_');
        gui->UG_PutStringSingleLine(sizeX - w, numberAY, strbuf);
    }
    else if (status == ENTER_NUMBER_B) {
        formatDoubleString(strbuf, numberA);
        gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(strbuf), numberAY, strbuf);
        
        sprintf(strbuf, "%c", operation);
        gui->UG_PutStringSingleLine(0, numberBY, strbuf);
        
        sprintf(strbuf, "%s%c", editNumber, '_');
        int w = gui->UG_StringWidth(strbuf);
        sprintf(strbuf, "%s%c", editNumber, millis() % 1000 < 500 ? ' ' : '_');
        gui->UG_PutStringSingleLine(sizeX - w, numberBY, strbuf);
    }
    else if (status == RESULT || status == RESULT_NO_CONTINUE) {
        formatDoubleString(strbuf, numberA);
        gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(strbuf), numberAY, strbuf);
        
        sprintf(strbuf, "%c", operation);
        gui->UG_PutStringSingleLine(0, numberBY, strbuf);
        
        formatDoubleString(strbuf, numberB);
        gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(strbuf), numberBY, strbuf);
        
        sprintf(strbuf, "%c", '=');
        gui->UG_PutStringSingleLine(0, resultY, strbuf);
        
        formatDoubleString(strbuf, result);
        gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(strbuf), resultY, strbuf);
    }
    else if (status == ERROR_DIVISION_BY_ZERO) {
        sprintf(strbuf, "Error:");
        gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(strbuf)/2, numberAY, strbuf);
        
        sprintf(strbuf, "Division by zero");
        gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(strbuf)/2, numberBY, strbuf);
    }
    /*
    // debug
    double d = atof(editNumber);
    formatDoubleString(strbuf, d);
    gui->UG_SetForecolor(C_ORANGE);
    gui->UG_PutStringSingleLine(sizeX - gui->UG_StringWidth(strbuf), 35, strbuf);*/
}

void formatDoubleString(char *strbuf, double d) {
    if (!isInMaxDigits(d)) {
        sprintf(strbuf, "%e", d);
        return;
    }
    
    // print into strbuf
    sprintf(strbuf, "%20.10lf", d);
    
    // count digits
    int digits = 0;
    int len = strlen(strbuf);
    int i;
    for (i = 0; i < len && digits < 10; i++) {
        if ('0' <= strbuf[i] && strbuf[i] <= '9')
            digits++;
    }

    // max. 10 digits
    if (digits == 10)
        strbuf[i] = '\0';
    
    // look up decimal dot position
    len = strlen(strbuf);
    int dotIndex;
    for (dotIndex = 0; dotIndex < len; dotIndex++) {
        if (strbuf[dotIndex] == '.')
            break;
    }

    // we have no fraction - exit
    if (dotIndex == len)
        return;

    // clear up zeroes at the end
    for (int i = len-1; i > dotIndex; i--) {
        if (strbuf[i] == '0')
            strbuf[i] = '\0';
        else
            break;
    }
    
    // clear potential decimal dot at the end
    len = strlen(strbuf);
    if (strbuf[len-1] == '.')
        strbuf[len-1] = '\0';
}

bool canContinueFromResult() {
    return isInMaxDigits(result);
}

bool isInMaxDigits(double d) {
    double n = 1;
    for (int i = 0; i < MAX_DIGITS_PER_NUMBER; i++)
        n *= 10;
    return abs(d) < n;
}
