#include "settings.h"
#include "system.h"

#define DEVICE_COUNT                     9 // 6 GPIOs + [Mic] + Light + Temp + Accel
#define PIN_COUNT                       11 // 6 GPIOs + [Mic] + Light + Temp + Accel*3
#define SELECTED_PIN_ANIMATION_SPEED    10.0
#define DATA_ARRAY_LENGTH_MAX           256
#define DATA_POLL_RATE_FREQUENCY        30
#define DATA_POLL_RATE_TIME             (1.0/DATA_POLL_RATE_FREQUENCY)

int readPinValue(int num);
//int readMicrophoneValue(int unused);
int readLightSensorValue(int unused);
int readTemperatureValue(int unused);
int readAccelerometerValue(int axis);

UG_COLOR colors[PIN_COUNT] = {
    0xFF0000,
    0x00FF00,
    0x0000FF,
    0xFFFF00,
    0xFF00FF,
    0x00FFFF,
    0xFF8000,
    0x0080FF,
    0x80FF00,
    0x8000FF,
    0x80FF80,
};

struct PinInfo {
    const char *name;
    const char *shortName;
    int (*readValue)(int param);
    int readValueFuncParam;
    bool active;
    int xPos, yPos, width;
    int data[DATA_ARRAY_LENGTH_MAX];
    int minData, maxData;
};

PinInfo pinInfo[PIN_COUNT] = {
    {"Pin 0",        "Pin 0", readPinValue,           0, false},
    {"Pin 1",        "Pin 1", readPinValue,           1, false},
    {"Pin 2",        "Pin 2", readPinValue,           2, false},
    {"Pin 3",        "Pin 3", readPinValue,           3, false},
    {"Pin 4",        "Pin 4", readPinValue,           4, false},
    {"Pin 5",        "Pin 5", readPinValue,           5, false},
    //  {"Microphone",   "Mic.",  readMicrophoneValue,    0, false},
    {"Light Sensor", "Light", readLightSensorValue,   0, false},
    {"Temperature",  "Temp.", readTemperatureValue,   0, false},
    {"Accelerom.",   "Acc.X", readAccelerometerValue, 0, false},
    {"Accelerom. Y", "Acc.Y", readAccelerometerValue, 1, false},
    {"Accelerom. Z", "Acc.Z", readAccelerometerValue, 2, false},
};

long lastFrame;
bool pinSelectionMenu = true;
int selectedPin = 0;
double selectedPinAnimation = 0;
int dataArrayLength;
int currentDataEntry;
double nextDataPoll, totalTime;
bool hideUI = false;

void setup() {
    pocuter = new Pocuter();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    pocuter->Ports->initPort(PocuterPorts::PORT_0, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_ADC, PocuterPorts::PORT_PULLUP_OFF);
    pocuter->Ports->initPort(PocuterPorts::PORT_1, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_BINARY, PocuterPorts::PORT_PULLUP_OFF);
    pocuter->Ports->initPort(PocuterPorts::PORT_2, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_BINARY, PocuterPorts::PORT_PULLUP_OFF);
    pocuter->Ports->initPort(PocuterPorts::PORT_3, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_BINARY, PocuterPorts::PORT_PULLUP_OFF);
    pocuter->Ports->initPort(PocuterPorts::PORT_4, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_BINARY, PocuterPorts::PORT_PULLUP_OFF);
    pocuter->Ports->initPort(PocuterPorts::PORT_5, PocuterPorts::PORT_DIRECTION_IN, PocuterPorts::PORT_MODE_ADC, PocuterPorts::PORT_PULLUP_OFF);
    
    disableDoubleClick(BUTTON_A);
    disableDoubleClick(BUTTON_B);
    disableDoubleClick(BUTTON_C);
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    
    for (int i = 0; i < DEVICE_COUNT; i++) {
        int col = i / (DEVICE_COUNT / 2 + 1);
        int n   = i % (DEVICE_COUNT / 2 + 1);
        
        if (col == 0) {
            pinInfo[i].xPos = 0;
            pinInfo[i].yPos = 12 + 8 * n;
            pinInfo[i].width = sizeX / 3 - 2;
        } else {
            pinInfo[i].xPos = sizeX / 3;
            pinInfo[i].yPos = 12 + 8 * n;
            pinInfo[i].width = sizeX * 2 / 3 - 2;
        }
        pinInfo[i].width = gui->UG_StringWidth(pinInfo[i].name) + 8 + 2;
    }
    
    dataArrayLength = sizeX * 3 / 4;
    if (dataArrayLength > DATA_ARRAY_LENGTH_MAX)
        dataArrayLength = DATA_ARRAY_LENGTH_MAX;
    
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
    
    if (pinSelectionMenu) {
        if (ACTION_TOGGLE_PIN)
            pinInfo[selectedPin].active = !pinInfo[selectedPin].active;
        if (ACTION_NEXT_PIN)
            selectedPin = (selectedPin + 1) % DEVICE_COUNT;
        if (ACTION_TOGGLE_PLOTTER) {
            bool hasActivePin = false;
            for (int i = 0; i < DEVICE_COUNT; i++) {
                if (pinInfo[i].active)
                    hasActivePin = true;
            }
            if (!hasActivePin)
                return;
            
            pinSelectionMenu = false;
            hideUI = false;
            currentDataEntry = 0;
            nextDataPoll = 0.0;
            totalTime = 0.0;
            
            // ugly way to copy accel-X to accel-Y and accel-Z
            pinInfo[PIN_COUNT - 1].active = pinInfo[PIN_COUNT - 2].active = pinInfo[PIN_COUNT - 3].active;
            
            while (!pinInfo[selectedPin].active)
                selectedPin = (selectedPin + 1) % PIN_COUNT;
        }
    } else {
        if (ACTION_TOGGLE_PLOTTER) {
            pinSelectionMenu = true;
            if (selectedPin >= DEVICE_COUNT)
                selectedPin = DEVICE_COUNT - 1;
            selectedPinAnimation = selectedPin;
        }
        if (ACTION_NEXT_PIN) {
            do {
                selectedPin = (selectedPin + 1) % PIN_COUNT;
            } while (!pinInfo[selectedPin].active);
        }
        if (ACTION_TOGGLE_UI)
            hideUI = !hideUI;
        
        nextDataPoll -= dt;
        if (nextDataPoll <= 0.0) {
            nextDataPoll += DATA_POLL_RATE_TIME;
            totalTime += DATA_POLL_RATE_TIME;
    
            // ugly way to update accelerometer values
            if (pinInfo[PIN_COUNT - 1].active)
                updateAccelerometerValues();
            
            for (int i = 0; i < PIN_COUNT; i++) {
                if (pinInfo[i].active) {
                    pinInfo[i].data[currentDataEntry % dataArrayLength] = pinInfo[i].readValue(pinInfo[i].readValueFuncParam);
                    
                    pinInfo[i].minData =  1000000;
                    pinInfo[i].maxData = -1000000;
                    for (int j = 0; j < min(currentDataEntry, dataArrayLength); j++) {
                        if (pinInfo[i].minData > pinInfo[i].data[j])
                            pinInfo[i].minData = pinInfo[i].data[j];
                        if (pinInfo[i].maxData < pinInfo[i].data[j])
                            pinInfo[i].maxData = pinInfo[i].data[j];
                    }
                }
            }
            currentDataEntry += 1;
        }
    }

    if (selectedPinAnimation != selectedPin) {
        selectedPinAnimation += SELECTED_PIN_ANIMATION_SPEED * dt;
        if (selectedPinAnimation >= DEVICE_COUNT)
            selectedPinAnimation -= DEVICE_COUNT;
        if (selectedPinAnimation > selectedPin && selectedPinAnimation < selectedPin + DEVICE_COUNT / 2)
            selectedPinAnimation = selectedPin;
    }
    
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
    
    char strbuf[128];
    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    
    if (pinSelectionMenu) {
        for (int i = 0; i < DEVICE_COUNT; i++) {
            int x = pinInfo[i].xPos;
            int y = pinInfo[i].yPos;
            int w = pinInfo[i].width;
            
            gui->UG_PutStringSingleLine(x + 9, y - 2, pinInfo[i].name);
#if 0
            gui->UG_FillFrame(x + 1, y + 1, x + 7, y + 7, pinInfo[i].active ? C_GREEN : C_RED);
            gui->UG_DrawFrame(x + 1, y + 1, x + 7, y + 7, C_GRAY);
#else
            gui->UG_FillCircle(x + 4, y + 4, 3, pinInfo[i].active ? C_GREEN : C_RED);
            gui->UG_DrawCircle(x + 4, y + 4, 3, C_GRAY);
#endif
        }

        int entryA = (int) selectedPinAnimation;
        int entryB = (entryA + 1) % DEVICE_COUNT;
        float s = selectedPinAnimation - entryA;
        int boxX = pinInfo[entryA].xPos  * (1 - s) + pinInfo[entryB].xPos  * s;
        int boxY = pinInfo[entryA].yPos  * (1 - s) + pinInfo[entryB].yPos  * s;
        int boxW = pinInfo[entryA].width * (1 - s) + pinInfo[entryB].width * s;
        int boxH = 8;
        gui->UG_DrawFrame(boxX, boxY, boxX + boxW, boxY + boxH, pocuterSettings.systemColor);
        
        drawButtonText(gui, "View Plotter", 0, sizeX, sizeY);
        drawButtonText(gui, "Next Pin", 1, sizeX, sizeY);
        drawButtonText(gui, pinInfo[selectedPin].active ? "Disable" : "Enable", 2, sizeX, sizeY);
    } else {
        int minData = pinInfo[selectedPin].minData;
        int maxData = pinInfo[selectedPin].maxData;
        if (maxData - minData <= 5) {
            minData -= (5 - (maxData - minData)  ) / 2;
            maxData += (4 - (maxData - minData)  ) / 2;
        }

        int n = maxData - minData + 1;
        int s = 1;
        while   (n / 10 >= 3) {
            n /= 10;
            s *= 10;
        }
        if      (n / 5  >= 3) {
            n /=  5;
            s *=  5;
        }
        else if (n / 2  >= 3) {
            n /=  2;
            s *=  2;
        }

        gui->UG_SetForecolor(colors[selectedPin]);
        char strbuf[256];
        for (int m = (minData / s) * s; m <= (maxData / s)*s + s; m += s) {
            int y = (m - minData) * (sizeY - 1) / (maxData - minData), yStr = y - 6;
            int x = sizeX / 4;
            if (y < 0 || y >= sizeY)
                continue;
        
            if (yStr <        -3) yStr =        -3;
            if (yStr > sizeY - 9) yStr = sizeY - 9;
            sprintf(strbuf, "%d", m);
            gui->UG_PutStringSingleLine(x - gui->UG_StringWidth(strbuf), yStr, strbuf);
            drawDottedHorizontalLine(gui, x + 3 - currentDataEntry % 3, 96, y, C_GRAY);
        }

        for (int x = sizeX - currentDataEntry % DATA_POLL_RATE_FREQUENCY; x >= sizeX / 4; x -= DATA_POLL_RATE_FREQUENCY) {
            drawDottedVerticalLine(gui, x, 0, 64, C_GRAY);
        }
        
        for (int k = 0; k <= PIN_COUNT; k++) {
            // following code ensures that the currently selected pin is always drawn on top of other pins.
            int i = k;
            if (k == selectedPin)
                continue;
            if (k == PIN_COUNT)
                i = selectedPin;
            
            if (!pinInfo[i].active)
                continue;
            
            UG_COLOR color = colors[i];
            if (i != selectedPin)
                color = (color & 0xFCFCFC) >> 2;
            
            minData = pinInfo[i].minData;
            maxData = pinInfo[i].maxData;
            if (maxData - minData <= 5) {
                minData -= (5 - (maxData - minData)) / 2;
                maxData += (4 - (maxData - minData)) / 2;
            }
    
            int py;
            for (int j = 0; j < min(currentDataEntry, dataArrayLength) - 1; j++) {
                int entry = (currentDataEntry - 1 - j) % dataArrayLength;
                int x = sizeX - j;
                int y = (pinInfo[i].data[entry] - minData) * (sizeY - 1) / (maxData - minData);
                
                if (j != 0)
                gui->UG_DrawLine(x, py, x - 1, y, color);
                
                py = y;
            }
        }
        
        if (!hideUI) {
            int pinNameWidth = gui->UG_StringWidth(pinInfo[selectedPin].shortName);
            gui->UG_FillFrame(sizeX / 2 - pinNameWidth / 2 - 2, 0, sizeX / 2 + (pinNameWidth - 1) / 2 + 2, 9, C_BLACK);
            gui->UG_DrawFrame(sizeX / 2 - pinNameWidth / 2 - 2, 0, sizeX / 2 + (pinNameWidth - 1) / 2 + 2, 9, colors[selectedPin]);
            gui->UG_PutString(sizeX / 2 - pinNameWidth / 2, -1, pinInfo[selectedPin].shortName);
            
            drawButtonText(gui, "Back", 0, sizeX, sizeY);
            drawButtonText(gui, "Next Pin", 1, sizeX, sizeY);
            drawButtonText(gui, "Hide UI", 2, sizeX, sizeY);
        }
    }
    
    pocuter->Display->updateScreen();
}

void drawButtonText(UGUI *gui, const char *str, int corner, int sizeX, int sizeY) {
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(C_ORANGE);
    if (corner == 0) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str) - 3, 0, sizeX, 10, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str) - 3, 0, sizeX, 10, C_GREEN);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str) - 1, -1, str);
    } else if (corner == 1) {
        gui->UG_FillFrame(sizeX - gui->UG_StringWidth(str) - 3, sizeY - 11, sizeX, sizeY - 1, C_BLACK);
        gui->UG_DrawFrame(sizeX - gui->UG_StringWidth(str) - 3, sizeY - 11, sizeX, sizeY - 1, C_GREEN);
        gui->UG_PutString(sizeX - gui->UG_StringWidth(str) - 1, sizeY - 12, str);
    } else if (corner == 2) {
        gui->UG_FillFrame(-1, sizeY - 11, gui->UG_StringWidth(str) + 2, sizeY - 1, C_BLACK);
        gui->UG_DrawFrame(-1, sizeY - 11, gui->UG_StringWidth(str) + 2, sizeY - 1, C_GREEN);
        gui->UG_PutString(1, sizeY - 12, str);
    }
}

void drawDottedHorizontalLine(UGUI *gui, int x1, int x2, int y, UG_COLOR color) {
    for (int x = x1; x <= x2; x += 3) {
        gui->UG_DrawPixel(x, y, color);
    }
}

void drawDottedVerticalLine(UGUI *gui, int x, int y1, int y2, UG_COLOR color) {
    for (int y = y1; y <= y2; y += 3) {
        gui->UG_DrawPixel(x, y, color);
    }
}

void drawDottedLine(UGUI *gui, int x0, int y0, int x1, int y1, uint8_t pxBetweenDots, UG_COLOR color) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    int p = 0;
    
    while (true) {
        if (p = 0) {
            gui->UG_DrawPixel(x0, y0, color);
            p = pxBetweenDots;
        }
        p--;
        
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 > dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int readPinValue(int num) {
    PocuterPorts::PORT_NUMBER port = (PocuterPorts::PORT_NUMBER) (PocuterPorts::PORT_0 + num);
    if (num == 0 || num == 5) {
        uint16_t value;
        pocuter->Ports->getValue(port, &value);
        return value;
    } else {
        bool value;
        pocuter->Ports->getValue((PocuterPorts::PORT_NUMBER) (PocuterPorts::PORT_0 + num), &value);
        return value ? 1 : 0;
    }
}

/*int readMicrophoneValue(int unused) {
    return rand() % 32;
}*/

int readLightSensorValue(int unused) {
    return pocuter->LightSensor->getValue();
}

int accelerometerValuesBuffer[3];

void updateAccelerometerValues() {
    PocuterAccelerometer::State state;
    if (pocuter->Accelerometer->getState(&state) != PocuterAccelerometer::ACCERROR_OK)
        return;
    accelerometerValuesBuffer[0] = state.x;
    accelerometerValuesBuffer[1] = state.y;
    accelerometerValuesBuffer[2] = state.z;
}

int readAccelerometerValue(int axis) {
    return accelerometerValuesBuffer[axis];
}

int readTemperatureValue(int unused) {
    int8_t temp;
    
    pocuter->Accelerometer->getTemperature(&temp);
    return temp;
}
