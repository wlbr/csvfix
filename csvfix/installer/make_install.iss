; create setup for csvfix

[Setup]
AppName=CSVfix
AppVerName=CSVfix 1.5
AppPublisher=Neil Butterworth
AppPublisherURL=http://code.google.com/p/csvfix/
AppSupportURL=http://code.google.com/p/csvfix/
AppUpdatesURL=http://code.google.com/p/csvfix/
DefaultDirName={pf}\CSVfix
DefaultGroupName=CSVfix
AllowNoIcons=yes
LicenseFile=C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\LICENSE
OutputBaseFilename=CSVFix_Setup
SetupIconFile=C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\csvfix.ico
Compression=lzma
SolidCompression=yes
InfoAfterFile=C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\readme.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\csvfix.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\csvfix.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\readme.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\neilb\home\devel\csvfix\csvfix\installer\Files\data\*"; DestDir: "{app}\Data"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\CSVfix Help"; Filename: "{app}\csvfix.chm"
Name: "{group}\{cm:ProgramOnTheWeb,CSVfix}"; Filename: "http://code.google.com/p/csvfix/"
Name: "{group}\{cm:UninstallProgram,CSVfix}"; Filename: "{uninstallexe}"

[Run]

