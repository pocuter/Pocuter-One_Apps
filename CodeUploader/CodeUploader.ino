
// Copyright 2023 Kallistisoft
// Copyright 2023 Pocuter GmbH, Pocuter Inc.
// GNU GPL-3 https://www.gnu.org/licenses/gpl-3.0.txt
/*
* [Pocuter-One_Apps]/CodeUploader/CodeUploader.ino
* 
* Code Upload Server Application
*
* See README.md file for details, examples, and usage guide
*/

#include "settings.h"
#include "system.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#include "md5.h"
MD5 md5sum;


#define DEBUG_TEMPFILE_ONLY 0

#define WWW_UPLOAD_TIMEOUT 10.0
double www_upload_timer = 0.0;
bool is_receiving_file = false;


// logging and error message macros
#define CENTER_TEXT(y,text) \
	gui->UG_PutStringSingleLine(sizeX/2 - gui->UG_StringWidth(text)/2, y, text);

#define NEXTLINE(text) \
	gui->UG_PutStringSingleLine(0, 18+text_y, text); text_y += 12;



// logging and error message macros
#define REMOVE_TEMPFILE() \
	if( www_image_file ) fclose( www_image_file ); \
	if( www_image_file ) remove( www_path_temp ); \
	is_receiving_file = false;\
	www_upload_timer=0;

#define WWW_ERROR(...) \
	printf( "\n[%s] ", timestamp ); \
	sprintf( www_error_msg, __VA_ARGS__ ); \
	printf( "%s\n", www_error_msg ); \
	delay(10); \
	REMOVE_TEMPFILE(); \
	return;

#define LOGMSG( _format_, ... ) \
	printf("[%s] " _format_ "\n", timestamp, __VA_ARGS__ ); delay(10);



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Import index.html file contents as string literal
const char index_html[] PROGMEM = 
#include "index_html.h" 
;

// char* GetCurrentTimeString() :: format current date time string
char time_str[256];
char* GetCurrentTimeString() {
	tm time;
	pocuter->PocTime->getLocalTime( &time );
    snprintf(
		time_str, 256, 
		"%04d-%02d-%02d %02d:%02d:%02d", 
		time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,	
		time.tm_hour, time.tm_min, time.tm_sec
	);
	return &time_str[0];
}

// void DEBUG_HTTP_REQUEST( *request ) :: print request debug info to serial
void DEBUG_HTTP_REQUEST( AsyncWebServerRequest *request ) {
	char *timestamp = GetCurrentTimeString();

	// debug: print request method and URL
	printf("\n");
	LOGMSG( "%s %s", request->method() == HTTP_GET ? "GET" : "POST", request->url().c_str() );

	// debug: print request parameter values
	int params = request->params();
	for(int i=0; i < params; i++){
		AsyncWebParameter* p = request->getParam(i);
		if(p->isFile()){ //p->isPost() is also true
			LOGMSG("FILE[ %s ]: '%s', %u bytes", p->name().c_str(), p->value().c_str(), p->size());
		} else if(p->isPost()){
			LOGMSG("POST[ %s ]: %s", p->name().c_str(), p->value().c_str());
		} else {
			LOGMSG("GET[ %s ]: %s", p->name().c_str(), p->value().c_str());
		}
	}

}



// global variables for response state tracking
char  www_error_msg   [256] = "";
char  www_path_image  [256] = "";
char  www_path_backup [256] = "";
char  www_path_temp   [256] = "";

char  www_image_hash  [33]  = "";
FILE* www_image_file = 0;
long  www_image_size = 0;

long  www_app_size = 0;


/***************************************************************************************************
// void setup() -- Application Setup Routine
****************************************************************************************************/
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

	printf("\n\nStarting Code Uploader Application...\n");

	// route: GET /
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	printf("* Creating route for GET /...\n");	
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		DEBUG_HTTP_REQUEST( request );
		request->send_P(200, "text/html", index_html );
	});

	// route: POST /upload [appID] [appImage]
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	printf("* Creating route for POST /upload...\n");
	server.on("/upload", HTTP_POST, 

	// POST: Verify uploaded file and launch application
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	[] (AsyncWebServerRequest *request) {
		DEBUG_HTTP_REQUEST( request );
		char *timestamp = GetCurrentTimeString();

		// error: upload handler generated an error message
		if( strlen(www_error_msg) ) {
			LOGMSG("%s", www_error_msg );
			request->send(200, "text/plain", www_error_msg );
			www_error_msg[0] = '\0';
			return;
		} 

		// verify: request has all parameters and proper types (string,file)
		if( !(
			request->hasParam("appID",true) && 
			request->hasParam("appMD5",true) && 
			request->hasParam("appSize",true) && 
			request->hasParam("appImage",true,true)) 
		) {
			WWW_ERROR("Error: Missing or incorrect request parameters!",0);
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// get request parameters
		AsyncWebParameter* paramID = request->getParam("appID",true);
		AsyncWebParameter* paramMD5 = request->getParam("appMD5",true);
		AsyncWebParameter* paramSize = request->getParam("appSize",true);
		AsyncWebParameter* paramImage = request->getParam("appImage",true,true);
		long appID = atol( paramID->value().c_str() );
		const char *appMD5 = paramMD5->value().c_str();
		long appSize = atol( paramSize->value().c_str() );
		long image_size = paramImage->size();

		// verify: uploaded file is same size as declared size
		if( www_image_size != appSize ) {
			WWW_ERROR("Error: Uploaded file size doesn't match declared file size: %u -> %u", www_image_size, appSize );
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// verify: appImage has a valid size
		if( www_image_size < 600*1024 ) {
			WWW_ERROR("Error: Invalid size for upload file (%u) -- must be larger than 600KiB!", www_image_size );
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// verify: MD5 hash of uploaded file matches declared MD5 hash
		if( strcmp( www_image_hash, appMD5 ) != 0 ) {
			WWW_ERROR("Error: Uploaded MD5 hash doesn't equal declared file hash: %s -> %s", www_image_hash, appMD5 );
			request->send(200, "text/plain", www_error_msg );
			return;			
		}

		// debug: upload debug mode -- skip writing file unless self-hoisting
		if( DEBUG_TEMPFILE_ONLY && appID != 8080 ){
			remove( www_path_temp );			
			LOGMSG("DEBUG: skipping installation of uploaded image file...", 0 );
			request->send(200, "text/plain", "DEBUG: skipping installation of temporary image..." );
			return;
		}

		// remove: existing application backup file
		LOGMSG(" DEL: %s", www_path_backup );
		remove( www_path_backup );

		// rename: existing application file
		LOGMSG("MOVE: %s -> %s", www_path_image, www_path_backup );
		rename( www_path_image, www_path_backup );

		// rename: temporary file
		LOGMSG("MOVE: %s -> %s", www_path_temp, www_path_image );
		rename( www_path_temp, www_path_image );

		// test: are we self-hoisting the 'Code Uploader' application?
		if( appID == 8080 ) {
			printf(" ----\n");
			LOGMSG("HOIST: Changing target application to 'Self-Hoisting Boot Proxy' - #8081", 0 );
			appID = 8081;
		}

		// send: request response
		printf(" ----\n");
		LOGMSG(" RUN: %u\n", appID );
		request->send(200, "text/plain", "OK: Launching application...");
		delay(100);

		// launch: application
		pocuter->OTA->setNextAppID( appID );
		pocuter->OTA->restart();
	},

	// UPLOAD: File upload request handler -- save streamed file...
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t size, bool final) {
		char image_name[256];
		char *timestamp = GetCurrentTimeString();
		char *numtest;

		// get: image file basname (webkit sends basename while firefox sends full path)
		memset( image_name, 0, 256 );
		const char *raw_image_name = filename.c_str();
		if( char *basname = strrchr( raw_image_name, '/' ) ) {
			strncpy( image_name, basname + 1, 255 );
		} else {
			strncpy( image_name, raw_image_name, 255 );
		}

		// start: verify parameters, mkdir, fopen
		if( index == 0 ) {
			printf("\n\n");
			DEBUG_HTTP_REQUEST( request );

			// reset: error message + file pointer\			
			www_error_msg[0]   = '\0';
			www_path_temp[0]   = '\0';
			www_path_image[0]  = '\0';
			www_path_backup[0] = '\0';
			www_image_hash[0]  = '\0';
			www_image_file = NULL;			
			www_image_size = 0;
			www_app_size = 0;
			www_upload_timer = 0.0;
			md5sum.reset();


			// verify: request has valid appID parameter
			if( !(request->hasParam("appID",true) ) ) {
				WWW_ERROR("Error: Missing appID parameter!",0);
			}

			// verify: appID is numeric and >= 2
			AsyncWebParameter* paramID = request->getParam("appID",true);
			long appID = strtol( paramID->value().c_str(), &numtest, 10);
			if( *numtest || appID < 2 ) {
				WWW_ERROR("Error: appID isn't a number >= 2!",0);
			}


			// verify: request has appSize parameter
			if( !(request->hasParam("appSize",true) ) ) {
				WWW_ERROR("Error: Missing appSize parameter!",0);
			}

			// verify: appSize is numeric
			AsyncWebParameter* paramSize = request->getParam("appSize",true);
			long appSize = strtol( paramSize->value().c_str(), &numtest, 10);
			if( *numtest ) {
				WWW_ERROR("Error: appSize isn't a number!",0);
			}
			www_app_size = appSize;


			// verify: appImage filename
			if( strcmp( image_name, "esp32c3.app" ) != 0 ) {
				WWW_ERROR( "Error: Invalid name for upload image file!: '%s'", image_name );
			}


			// debug: begin writing file to sd card
			LOGMSG( "IMAGE: %S", image_name );
			is_receiving_file = true;

			// mkdir: app folder
			char dirpath[256];
			snprintf( dirpath, 255, "%s/apps/%u", pocuter->SDCard->getMountPoint(), appID );
			LOGMSG( " PATH: %s", dirpath );
			if( !access( dirpath, F_OK) == 0 ) {
				if( !mkdir( dirpath, S_IRWXU ) == 0 ) {
					WWW_ERROR( "Error: Creating application folder '%s': errno: %u", dirpath, errno );
				}
			}

			// calc: temporary, backup, and image file names
			snprintf( www_path_image,  255, "%s/esp32c3.app",        dirpath );			
			snprintf( www_path_backup, 255, "%s/esp32c3.app.backup", dirpath );
			snprintf( www_path_temp,   255, "%s/esp32c3.app.upload", dirpath );			

			// open: image file handle
			LOGMSG( "WRITE: %s", www_path_temp );
			www_image_file = fopen( www_path_temp, "w" );
			if( !www_image_file ) {
				WWW_ERROR( "Error: Opening image file for writting '%s': %u", www_path_temp, errno );
			}
		}

		// error: ignore data
		if( strlen(www_error_msg) ) return;

		// write: image data stream
		if( size != 0 && www_image_file ) {
			//LOGMSG( " DATA: %u bytes", size );
			long bytes = fwrite( data, 1, size, www_image_file );
			if( bytes != size ) {
				WWW_ERROR( "Error: Writting file '%s' - block size mismatch: %u -> %u", www_path_temp, size, bytes );
			}
			www_image_size += size;
			md5sum.add( data, size );
		}

		// stop: close open file
		if( final && www_image_file ) {
			LOGMSG(" DONE: %u bytes", www_image_size );
			fclose( www_image_file ); 			

 			strncpy( www_image_hash, md5sum.getHash().c_str(), 32 );
 			LOGMSG(" HASH: %s", www_image_hash );

			is_receiving_file = false;			
 			printf(" ----");
		}

	});

  	// Start server
	printf("* Starting Web Server...\n\n");
  	server.begin();
}



/***************************************************************************************************
// void loop() -- Application Loop
****************************************************************************************************/
void loop() {
	uint text_y = 0;
	pocuter->Sleep->setInactivitySleep( 0, (PocuterSleep::SLEEPTIMER_INTERRUPTS) 0x03 );

	dt = (micros() - lastFrame) / 1000.0 / 1000.0;
	lastFrame = micros();
	updateInput();
	
	if (ACTION_BACK_TO_MENU) {
		pocuter->OTA->setNextAppID(1);
		pocuter->OTA->restart();
	}

	// check: upload timout counter
	if( is_receiving_file ) www_upload_timer += dt;
	if( www_upload_timer >= WWW_UPLOAD_TIMEOUT ) {
		char *timestamp = GetCurrentTimeString();
		LOGMSG(" QUIT: File transfer timed out!",0);
		REMOVE_TEMPFILE();
	}

	// dt contains the amount of time that has passed since the last update, in seconds
	UGUI* gui = pocuter->ugui;
	uint16_t sizeX;
	uint16_t sizeY;
	pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	gui->UG_FillScreen( C_BLACK );
	gui->UG_FillFrame(0, 0, sizeX, 13, C_BLUE);
	gui->UG_FontSelect(&FONT_POCUTER_5X7);
	gui->UG_SetForecolor( C_YELLOW );
    CENTER_TEXT( 0, "Code Uploader" );
	gui->UG_SetForecolor( C_WHITE );

	// sdcard: is not mounted
	if( !pocuter->SDCard->cardIsMounted() ) {
		gui->UG_SetForecolor( C_YELLOW );
		NEXTLINE( "SD card is not" );
		NEXTLINE( "mounted.");
		text_y += 2;
		NEXTLINE( "Please restart" );
		NEXTLINE( "the application!" );
		return;
	}

	// get: wifi credentials
	PocuterWIFI::wifiCredentials cred;
	pocuter->WIFI->getCredentials( &cred );	

	// wifi: connection not established or unavailable
	if( pocuter->WIFI->getState() != PocuterWIFI::WIFI_STATE_CONNECTED ) {
		if( cred.ssid ) {
			NEXTLINE( "Trying to connect" );
			NEXTLINE( "to WiFi network:" );
			gui->UG_SetForecolor( C_PLUM);
			text_y += 2;
			NEXTLINE( (char*)cred.ssid );
		}
		else {
			NEXTLINE( "Unable to connect" );
			NEXTLINE( "to WiFi network!" );
		}
		pocuter->Display->updateScreen();
		return;
	}

	// xfer: currently receiving a file - display progress  %
	if( is_receiving_file ) {
		float pct = ((float)www_image_size / (float)www_app_size);

		char xfer_bytes[24];
		snprintf(xfer_bytes, 24, "%0.02f Kib", (float)www_image_size / 1024.0);		

		uint top = 24;
		uint margin = 6;
		uint height = 12;
		uint width = sizeX - (2*margin) - 2;
		uint level = ((float)width * pct);
		gui->UG_FillFrame( margin,   top,   sizeX - margin,     top + height,     C_WHITE );
		gui->UG_FillFrame( margin+1, top+1, sizeX - margin - 1, top + height - 1, C_BLACK );
		gui->UG_FillFrame( margin+2, top+2, margin + 2 + level, top + height - 2, C_WHITE );

		gui->UG_SetForecolor( C_PLUM );		
		CENTER_TEXT( top + 16, xfer_bytes );

		//NEXTLINE( xfer_bytes );
		pocuter->Display->updateScreen();
		return;		
	}

	// wifi: show server ip address
	const PocuterWIFI::ipInfo* info = pocuter->WIFI->getIpInfo();
	if( info->ipV4 ) {
		char addr[17];		
		snprintf(addr, 17, "%s", inet_ntoa(info->ipV4));

		NEXTLINE( "Server address:" );
		gui->UG_SetForecolor( C_PLUM );
		text_y += 2;		
		NEXTLINE( addr );
	} 
	
	// wifi: waiting for ip address
	else {
		NEXTLINE( "Obtaining DHCP" );
		NEXTLINE( "address from:" );
		gui->UG_SetForecolor( C_PLUM );
		text_y += 2;
		NEXTLINE( (char*)cred.ssid );
	}

	// update display
	pocuter->Display->updateScreen();
}
