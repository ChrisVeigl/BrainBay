;--------------------------------
;Include Modern UI 2

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "BrainBay"
  RequestExecutionLevel admin
  OutFile "Setup_BrainBay.exe"

  ;Default installation folder
;  InstallDir $PROGRAMFILES\BrainBay
   InstallDir "$LOCALAPPDATA\BrainBay"
  
  ;Get installation folder from registry if available
;  InstallDirRegKey HKLM "Software\BrainBay" "Install_Dir"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_UNCONFIRMPAGE_TEXT_TOP "!! W A R N I N G !!    \
>> Read this first. <<   \
*Uninstall* will remove ALL FILES from the folder below, \
INCLUDING any of your own .con files \
created in CONFIGURATIONS or other sub-folders. \
YOU MUST SAVE your own files BEFORE \
you press the *Uninstall* button."

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "Readme_License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Application" SecApp

  SectionIn RO

  SetOutPath "$INSTDIR"
  
  File "bin\*.exe"
  File "bin\*.cfg"
  File "bin\*.dll"
  File "bin\*.txt"
  File "bin\*.sys"
  File "bin\*.pdf"

  SetOutPath $INSTDIR\ARCHIVES
  File "bin\ARCHIVES\adxl202.arc"
  File "bin\ARCHIVES\alpha.arc"
  File "bin\ARCHIVES\Calib.edf"
  File "bin\ARCHIVES\Osas2002.edf"
  File "bin\ARCHIVES\heart.arc"
  File "bin\ARCHIVES\muskel_training1.arc"
  File "bin\ARCHIVES\emg.arc"
  File "bin\ARCHIVES\monolith1.arc"
  File "bin\ARCHIVES\ekg_chn1.arc"
  File "bin\ARCHIVES\2_channel_test.arc"
  File "bin\ARCHIVES\testsignal.txt"
  File "bin\ARCHIVES\testsignal.csv"
  
  SetOutPath $INSTDIR\CONFIGURATIONS
  File "bin\CONFIGURATIONS\demomenu.con"

  SetOutPath $INSTDIR\CONFIGURATIONS\Assistive
  File /r "bin\CONFIGURATIONS\Assistive\*.*"

  SetOutPath $INSTDIR\CONFIGURATIONS\ECG
  File /r "bin\CONFIGURATIONS\ECG\*.*"

  SetOutPath $INSTDIR\CONFIGURATIONS\EEG
  File /r "bin\CONFIGURATIONS\EEG\*.*"

  SetOutPath $INSTDIR\CONFIGURATIONS\EMG
  File /r "bin\CONFIGURATIONS\EMG\*.*"

  SetOutPath $INSTDIR\CONFIGURATIONS\MultimediaDemos
  File /r "bin\CONFIGURATIONS\MultimediaDemos\*.*"

  SetOutPath $INSTDIR\CONFIGURATIONS\OtherDemos
  File /r "bin\CONFIGURATIONS\OtherDemos\*.*"

  SetOutPath $INSTDIR\GRAPHICS
  File "bin\GRAPHICS\*.*"
  SetOutPath $INSTDIR\MOVIES
  File "bin\MOVIES\face.avi"
  File "bin\MOVIES\bear.wmv"
  File "bin\MOVIES\flower.wmv"
  SetOutPath $INSTDIR\NETWORK
  File "bin\NETWORK\*.*"
  SetOutPath $INSTDIR\PALETTES
  File "bin\PALETTES\*.*"
  SetOutPath $INSTDIR\PATTERNS
  File "bin\PATTERNS\*.*"
  SetOutPath $INSTDIR\REPORTS
  File "bin\REPORTS\*.*"
  SetOutPath $INSTDIR\SKINDIALOGS
  File "bin\SKINDIALOGS\*.*"
  SetOutPath $INSTDIR\SOUNDS
  File "bin\SOUNDS\*.*"
  SetOutPath $INSTDIR\TONESCALES
  File "bin\TONESCALES\*.*"
  SetOutPath $INSTDIR\NeurobitRuntime
  File "bin\NeurobitRuntime\*.*"
  SetOutPath $INSTDIR\dictionary
  File "bin\dictionary\*.*"
  SetOutPath $INSTDIR\ComputerVision
  File "bin\ComputerVision\*.*"
  
  SetOutPath $SYSDIR
  File "bin\NeurobitRuntime\ftd2xx.dll"


; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\BrainBay "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BrainBay" "DisplayName" "Brainbay Application"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BrainBay" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BrainBay" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BrainBay" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\BrainBay"
  CreateShortCut "$SMPROGRAMS\BrainBay\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\BrainBay\BrainBay.lnk" "$INSTDIR\brainbay.exe" "" "$INSTDIR\brainbay.exe" 0
  CreateShortCut "$SMPROGRAMS\BrainBay\Readme.lnk" "$INSTDIR\ReadMe_BrainBay.txt" "" "$INSTDIR\ReadMe_BrainBay.txt" 0
  CreateShortCut "$DESKTOP\BrainBay.lnk" "$INSTDIR\BrainBay.exe" ""  
  CreateShortCut "$DESKTOP\Uninstall_BrainBay.lnk" "$INSTDIR\uninstall.exe" ""  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BrainBay"
  DeleteRegKey HKLM SOFTWARE\BrainBay

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*
;  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\BrainBay\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\BrainBay"
  RMDir /r "$INSTDIR"

SectionEnd
