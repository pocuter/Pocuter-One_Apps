# PocuterUtil :: Code Uploader -- Pocuter Application Upload Server
**This application exposes a convenient web interface for installing applications onto a Pocuter**


## Application Features
- HTML5 Web interface for installing a Pocuter application via Drag & Drop
- CLI tool [**pocuter-deploy**](https://github.com/pocuter/pocuter-deploy) for building, packaging, and uploading Pocuter applications
- Robust error checking of uploaded files - MD5 hash check, backup creation, and temp file used for uploads
- Automatically launches uploaded application after file verification
- Can be used in place of the 'Menu Application' for rapid development cycles

## Example Video
[![Watch the example video](https://img.youtube.com/vi/qk0EkwArBsY/default.jpg)](https://youtu.be/qk0EkwArBsY)

***

## Web Application Usage
When the application is run it presents the user with the IP address of the server. Entering this address in a web browser will open a web interface for uploading applications.

To upload an application simply drag and drop the application folder onto the target area in the web app. This folder can be the root folder of the arduino sketch, the ./apps/ sub-folder, or just the numbered sub-folder containing the program image file.

The web app will scan the dropped folder for a numbered directory containing a packaged Pocuter program ('esp32c3.app'). The app will then calculate an MD5 hash of the file and extract the program metadata from the program image. Afterwards, the application will automatically upload the image file to the 'Code Upload Server' running on the Pocuter.

The uploaded file will be verified by the 'Code Upload Server' and then launched automatically!

### Recommended Browser:
For the best experience use a WebKit based browser: Chrome, Chromium, Safari, or Edge

***

## Command-line Deployment Tool
In addition to the web app interface there is also a command line tool [pocuter-deploy](https://github.com/pocuter/pocuter-deploy) for deploying pocuter applications to the server. This tool also is capable of compiling and packaging the application before uploading it.

[Please see the README.md file for the pocuter-deploy tool for more information...](https://github.com/pocuter/pocuter-deploy)

***

## Rapid Development
While your application is under development you can use the 'Code Upload Server' as your 'Back to Menu' action. Simply set the application ID for ***setNextAppID(...)*** to the Code Uploader application ID: **8080**

```C
	if( ACTION_BACK_TO_MENU ) {
		pocuter->OTA->setNextAppID( 8080 );
		pocuter->OTA->restart();
	}
```

This will cause the upload server to automatically be re-loaded when you exit you application via the back to menu button action.

***

## Known Bugs and Browser Compatability Issues
The web app has a (200ms) timout event for processing a dropped folder, if your computer is extremely slow this may result in a message that a program image couldn't be found. If this is the case please use the [pocuter-deploy](https://github.com/pocuter/pocuter-deploy) command line tool.

### Disability and Accessability:
The web app is not compatible with accessability features as it requires drag and drop to function. If you have accessability needs please use the [pocuter-deploy](https://github.com/pocuter/pocuter-deploy) command line tool.

### Browser Compatability Tests:
The web application has been tested with the folowing browsers:

| Browser | Version    | Operating System                               |
|---------|------------|------------------------------------------------|
| **Firefox** | 108, 109   | Linux 6.0.9, Windows 10 (19042), MacOS 10.15.7 |
| **Chrome**  | 107, 108   | Linux 6.0.9, Windows 10 (19042), MacOS 10.15.7 |
| **Edge**    | 107        | Windows 10 (19042)                             |
| **Safari**  | 15.6       | MacOS 10.15                                    |

### Firefox Issues:
- Firefox (all OSs) will not follow symlinked folders
- Firefox (all OSs) doesn't show custom progress bar visual styling
- Firefox (Windows, MacOS) shows incorrect progress bar percentage -- use the progress bar on the Pocuter if you need accurate transfer tracking

***

## Developer Notes
If you plan on forking or modifying this application please be aware of the following:

### Patch Required for ESPAsyncWebSrv:
This project requires the [***ESPAsyncWebSrv Arduino Library***](https://github.com/dvarrel/ESPAsyncWebSrv)  - available to install from the Arduino GUI. This library has a bug in the esp32-c3 implementation that requires a patch in-order to compile properly. The patch for the file ***'/src/AsyncWebSocket.cpp'*** can be found in the ***./patch/*** sub-folder. It consists of a single change at line #832:

```C
 IPAddress AsyncWebSocketClient::remoteIP() {
     if(!_client) {
-        return IPAddress(0U);
+        return IPAddress((uint32_t) 0);
     }
     return _client->remoteIP();
 }
```

### Self-Hoisting Companion Application:
There is a companion application ['Code Uploader Hoist Proxy'](https://github.com/pocuter/Pocuter-One_Apps/tree/main/CodeUploader/tools/HoistProxy) that is used to hoist the 'Code Upload Server' when the upload server detects that it is updating itself.

### Web Application Compiler:
There is a script in the ***./gui/*** folder called ***compile_index_html*** which is used to compile the web application into the C header file ***index_html.h*** - this structure allows the web application to be tested locally with a live-loading server.