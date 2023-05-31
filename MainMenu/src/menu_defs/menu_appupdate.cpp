// ========================================
// INCLUDES
// ========================================

#include "../../settings.h"
#include "menu_appupdate.h"

// ========================================
// MACROS
// ========================================

// ========================================
// TYPES
// ========================================

enum UpdateStatus {
	UPDATE_PREPARATION,
	UPDATE_DOWNLOADING,
	UPDATE_VALIDATING,
	UPDATE_FINISHED,
	
	UPDATE_ERROR_NO_UPDATE_AVAILABLE,
	UPDATE_ERROR_NO_SD,
	UPDATE_ERROR_NO_WIFI,
	UPDATE_ERROR_CONNECT_FAILED,
	UPDATE_ERROR_DOWNLOAD_FAILED,
	UPDATE_ERROR_CRC_FAILED,
};

// ========================================
// PROTOTYPES
// ========================================

void createMenuUpdateApp();
void updateMenuUpdateApp();
void drawMenuUpdateApp();
const char* getDownloadErrorMsg(PocuterHTTP::HTTPERROR error);

// ========================================
// GLOBALS
// ========================================

Menu menu_updateapp = {
    "Updating...",          // title
    NULL,                   // icon
    NULL,                   // parent menu
    false,                  // can loop items
    
    createMenuUpdateApp,    // create func
    updateMenuUpdateApp,    // update func
    drawMenuUpdateApp       // draw func
};

const uint8_t *appUpdateURL;
char updateSdLocationFolder[256];
char updateSdLocationStaged[256];
char updateSdLocationFinal[256];
char updateSdLocationBackup[256];
UpdateStatus updateStatus;
long menuEnterMS;
uint8_t downloadPercentage;
PocuterHTTP::HTTPERROR downloadError;
void (*exitFuncSuccess)();
void (*exitFuncFailure)();
bool autoExitOnSuccess;

// ========================================
// FUNCTIONS
// ========================================

void downloadAppUpdate(uint64_t id, const uint8_t *url, void (*exitSuccess)(), void (*exitFailure)(), bool autoExitOnSuccesss) {
	appUpdateURL = url;
	exitFuncSuccess = exitSuccess;
	exitFuncFailure = exitFailure;
    autoExitOnSuccess = autoExitOnSuccesss;
	
	sprintf(updateSdLocationFolder, APP_SD_LOCATION_FOLDER, id);
	sprintf(updateSdLocationStaged, APP_SD_LOCATION_STAGED, id);
	sprintf(updateSdLocationFinal,  APP_SD_LOCATION_FINAL,  id);
	sprintf(updateSdLocationBackup, APP_SD_LOCATION_BACKUP, id);
	
	initMenuChange(&menu_updateapp);
}

void createMenuUpdateApp() {
	/*if (!pocuter->SDCard->cardIsMounted())
		updateStatus = UPDATE_ERROR_NO_SD;
	else*/ if (pocuter->WIFI->getState() != PocuterWIFI::WIFI_STATE_CONNECTED)
		updateStatus = UPDATE_ERROR_NO_WIFI;
	else if (appUpdateURL == NULL)
		updateStatus = UPDATE_ERROR_NO_UPDATE_AVAILABLE;
	else 
		updateStatus = UPDATE_PREPARATION;
	
    pocuter->Sleep->setInactivitySleep(0, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);

	menuEnterMS = millis();
	downloadPercentage = 0;
}

void updateMenuUpdateApp() {
	switch (updateStatus) {
		case UPDATE_PREPARATION:
			if (millis() - menuEnterMS > 1000) {
				updateStatus = UPDATE_DOWNLOADING;
				// create folder
				mkdir(updateSdLocationFolder, 0777);
				// remove old staged file
				remove(updateSdLocationStaged);
			}
			break;
			
		case UPDATE_DOWNLOADING:
			downloadError = pocuter->HTTP->downloadFile((const uint8_t*) appUpdateURL, (const uint8_t*) updateSdLocationStaged, true, &downloadPercentage, pocuter->Server->getServerRootCa());
			if (downloadError != PocuterHTTP::HTTPERROR_MORE_STEPS) {
				if (downloadError == PocuterHTTP::HTTPERROR_OK)
					updateStatus = UPDATE_VALIDATING;
				else if (downloadError == PocuterHTTP::HTTPERROR_CONNECT_FAILED)
					updateStatus = UPDATE_ERROR_CONNECT_FAILED;
				else
					updateStatus = UPDATE_ERROR_DOWNLOAD_FAILED;
			}
			break;
			
		case UPDATE_VALIDATING:
			// TODO do CRC check on the downloaded file
			
			// delete old backup
			remove(updateSdLocationBackup);
			// make current binary the new backup
			rename(updateSdLocationFinal, updateSdLocationBackup);
			// rename downloaded binary
			rename(updateSdLocationStaged, updateSdLocationFinal);
			updateStatus = UPDATE_FINISHED;
			break;
			
		case UPDATE_FINISHED:
            pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
			if (autoExitOnSuccess || ACTION_UP || ACTION_DOWN || ACTION_OK)
				exitFuncSuccess();
			break;
			
		case UPDATE_ERROR_NO_UPDATE_AVAILABLE:
            pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
            // when auto-updating multiple apps, 'no update (anymore)' means all apps got downloaded successfully
            // however, when trying to update the main menu, 'no update' counts as failure
			if (autoExitOnSuccess)
				exitFuncSuccess();
            
		case UPDATE_ERROR_NO_SD:
		case UPDATE_ERROR_NO_WIFI:
		case UPDATE_ERROR_CONNECT_FAILED:
		case UPDATE_ERROR_DOWNLOAD_FAILED:
		case UPDATE_ERROR_CRC_FAILED:
            pocuter->Sleep->setInactivitySleep(pocuterSettings.timeUntilStandby, PocuterSleep::SLEEPTIMER_INTERRUPT_BY_BUTTON, true);
			if (ACTION_OK)
				exitFuncFailure();
			break;
	}
}

void drawMenuUpdateApp() {
	drawMenuDefault();
	
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	char str1[64], str2[64], str3[64];
	UG_COLOR color;
	
	switch (updateStatus) {
		case UPDATE_PREPARATION:
			sprintf(str1, "");
			sprintf(str2, "Preparing...");
			sprintf(str3, "");
			color = C_GREEN;
			break;
			
		case UPDATE_DOWNLOADING:
			sprintf(str1, "");
			sprintf(str2, "Downloading...");
			sprintf(str3, "");
			color = C_GREEN;
			gui->UG_DrawFrame(sizeX*1/12,   sizeY*6/8,   sizeX*11/12,                                           sizeY*7/8,   C_GREEN);
			gui->UG_FillFrame(sizeX*1/12+2, sizeY*6/8+2, sizeX*1/12+2 + (sizeX*10/12-4)*downloadPercentage/100, sizeY*7/8-2, C_GREEN);
			break;
			
		case UPDATE_VALIDATING:
			sprintf(str1, "");
			sprintf(str2, "Validating...");
			sprintf(str3, "");
			color = C_GREEN;
			break;
			
		case UPDATE_FINISHED:
			sprintf(str1, "Done!");
			sprintf(str2, "Press any button");
			sprintf(str3, "to continue.");
			color = C_GREEN;
			break;
		
		case UPDATE_ERROR_NO_UPDATE_AVAILABLE:
			sprintf(str1, "");
			sprintf(str2, "No Update available");
			sprintf(str3, "");
			color = C_YELLOW;
			break;
			
		case UPDATE_ERROR_NO_SD:
			sprintf(str1, "Error:");
			sprintf(str2, "No SD present");
			sprintf(str3, "");
			color = C_RED;
			break;
			
		case UPDATE_ERROR_NO_WIFI:
			sprintf(str1, "Error:");
			sprintf(str2, "No WiFi connection");
			sprintf(str3, "");
			color = C_RED;
			break;
			
		case UPDATE_ERROR_CONNECT_FAILED:
			sprintf(str1, "Error:");
			sprintf(str2, "Unable to connect");
			sprintf(str3, "");
			color = C_RED;
			break;
			
		case UPDATE_ERROR_DOWNLOAD_FAILED:
			sprintf(str1, "Error:");
			sprintf(str2, "Download failed:");
			sprintf(str3, "%s", getDownloadErrorMsg(downloadError));
			color = C_RED;
			break;
			
		case UPDATE_ERROR_CRC_FAILED:
			sprintf(str1, "Error:");
			sprintf(str2, "File validation failed");
			sprintf(str3, "");
			color = C_RED;
			break;
	}
	
    gui->UG_FontSelect(&FONT_POCUTER_4X6);
    gui->UG_SetForecolor(color);
	
	if (str1 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str1)/2, sizeY*2/6, str1);
	if (str2 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str2)/2, sizeY*3/6, str2);
	if (str3 != NULL) gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(str3)/2, sizeY*4/6, str3);
}

const char* getDownloadErrorMsg(PocuterHTTP::HTTPERROR error) {
	switch (error) {
		case PocuterHTTP::HTTPERROR_OK: 				return "OK";
		case PocuterHTTP::HTTPERROR_CONNECT_FAILED: 	return "Connection error";
		case PocuterHTTP::HTTPERROR_NO_MEMORY: 			return "Out of memory";
		case PocuterHTTP::HTTPERROR_FILE_OPEN_FAILED: 	return "Cannot create file";
		case PocuterHTTP::HTTPERROR_DOWNLOAD_FAILED: 	return "Error while downloading";
		case PocuterHTTP::HTTPERROR_MORE_STEPS: 		return "Download not finished";
		case PocuterHTTP::HTTPERROR_UNKNOWN: 			return "Unknown error";
		default: 										return "Other error";

	}
}