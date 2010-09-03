;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "BrainBay"
  OutFile "Setup_BrainBay.exe"

  ;Default installation folder
  InstallDir $PROGRAMFILES\BrainBay
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\BrainBay" "Install_Dir"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

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
  
  File "debug\*.exe"
  File "debug\*.dll"
  File "debug\*.txt"
  File "debug\*.sys"
  File "debug\*.xml"

  SetOutPath $INSTDIR\ARCHIVES
  File "debug\ARCHIVES\adxl202.arc"
  File "debug\ARCHIVES\alpha.arc"
  File "debug\ARCHIVES\Calib.edf"
  File "debug\ARCHIVES\Osas2002.edf"
  File "debug\ARCHIVES\heart.arc"
  File "debug\ARCHIVES\muskel_training1.arc"
  File "debug\ARCHIVES\emg.arc"
  File "debug\ARCHIVES\monolith1.arc"
  File "debug\ARCHIVES\psytask_test.edf"
  File "debug\ARCHIVES\ekg_chn1.arc"
  File "debug\ARCHIVES\2_channel_test.arc"
  SetOutPath $INSTDIR\CONFIGURATIONS
  File /r "debug\CONFIGURATIONS\*.*"
  SetOutPath $INSTDIR\GRAPHICS
  File "debug\GRAPHICS\*.*"
  SetOutPath $INSTDIR\HELPPAGES
  File /r "debug\HELPPAGES\*.*"
  SetOutPath $INSTDIR\MOVIES
  File "debug\MOVIES\face.avi"
  SetOutPath $INSTDIR\NETWORK
  File "debug\NETWORK\*.*"
  SetOutPath $INSTDIR\PALETTES
  File "debug\PALETTES\*.*"
  SetOutPath $INSTDIR\PATTERNS
  File "debug\PATTERNS\*.*"
  SetOutPath $INSTDIR\SKINDIALOGS
  File "debug\SKINDIALOGS\*.*"
  SetOutPath $INSTDIR\SOUNDS
  File "debug\SOUNDS\*.*"
  SetOutPath $INSTDIR\TONESCALES
  File "debug\TONESCALES\*.*"
  SetOutPath $INSTDIR\NeurobitRuntime
  File "debug\NeurobitRuntime\*.*"
  SetOutPath $INSTDIR\dictionary
  File "debug\dictionary\*.*"
  
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

  CreateDirectory "$SMPROGRAMS\BrainBay"
  CreateShortCut "$SMPROGRAMS\BrainBay\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\BrainBay\BrainBay.lnk" "$INSTDIR\brainbay.exe" "" "$INSTDIR\brainbay.exe" 0
  CreateShortCut "$SMPROGRAMS\BrainBay\Readme.lnk" "$INSTDIR\ReadMe_BrainBay.txt" "" "$INSTDIR\ReadMe_BrainBay.txt" 0
  
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
