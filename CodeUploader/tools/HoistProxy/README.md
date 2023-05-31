# PocuterUtil :: Code Uploader Hoist Proxy -- Code Upload Server Boot Proxy
**This application auto-launches the 'Code Upload Server' application**

## Description
This compainion application exists because the next ***setNextAppID(...)*** function will not re-load a running app from the sd card. When the 'Code Upload Server' detects that it is updating itself it launches the hoist proxy which automatically re-launches the 'Code Upload Server' forcing the updated code to be loaded from disk.

The Code Upload server expects the Hoist Proxy to have an Application ID of #8081
