## Exporting an App

#### Setup

The Pocuter App converter is needed to build apps, download it and put the .exe into the root folder of your local repository.

#### Buiding

To build an app, open it in Arduino and click *Sketch -> Export Compiled Binary* and wait until it has finished compiling.  
Then open the command line, move to the root folder of your repository and type `appconverter.exe -image [Name]\[Name].ino.bin -meta [Name]\[Name].ini -id [ID] -version [X.Y.Z]`. Make sure the ID does not clash with any other app (see below). The converter will create a folder named 'apps' that contains the created app.  
Alternatively, you can add the app to the convert-all.bat file and use that to export all apps at once.

## App IDs and Data

Following app IDs are already in use (Main Menu added for reference):

| ID   | Name                | Version |
| ---: | :------------------ | ------: |
| 1    | Main Menu           | 0.8.8   |
| 2    | Calculator          | 0.2.2   |
| 3    | Stopwatch           | 0.2.2   |
| 4    | Score Tracker       | 0.2.2   |
| 5    | Random Generator    | 0.2.4   |
| 6    | Pin Plotter         | 0.2.4   |
| 7    | Clock               | 0.2.2   |
| 8    | Calendar            | 0.2.2   |
| 8080 | Code Uploader       | 1.0.0   |
