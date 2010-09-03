# Microsoft Developer Studio Project File - Name="brainBay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=brainBay - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "brainBay.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "brainBay.mak" CFG="brainBay - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "brainBay - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "brainBay - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "brainBay - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "brainBay - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib opengl32.lib glu32.lib SDL.lib SDL_net.lib fidlib.lib matheval.lib SDL_sound.lib modplug.lib vfw32.lib glaux.lib cv.lib cvcam.lib cxcore.lib highgui.lib libeng.lib libmx.lib skinstyle.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "brainBay - Win32 Release"
# Name "brainBay - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\brainbay.cpp
# End Source File
# Begin Source File

SOURCE=.\brainbay.rc
# End Source File
# Begin Source File

SOURCE=.\dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\files.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\midi.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_and.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_array3600.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_average.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_avi.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_ballgame.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_cam.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_com_writer.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_compare.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_comreader.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_constant.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_correlation.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_counter.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_debounce.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_deviation.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_doku.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_edf_reader.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_edf_writer.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_eeg.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_erpdetect.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_evaluator.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_fft.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_file_reader.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_file_writer.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_integrate.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_keystrike.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_magnitude.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_martini.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_matlab.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_mci.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_midi.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_mixer4.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_not.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_or.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_osci.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_particle.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_peakdetect.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_port_io.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_sample_hold.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_signal.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_skindialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_speller.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_tcp_receive.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_tcp_sender.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_threshold.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_translate.cpp
# End Source File
# Begin Source File

SOURCE=.\ob_wav.cpp
# End Source File
# Begin Source File

SOURCE=.\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\tty.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\base.h
# End Source File
# Begin Source File

SOURCE=.\brainBay.h
# End Source File
# Begin Source File

SOURCE=.\cv.h
# End Source File
# Begin Source File

SOURCE=.\fidlib.h
# End Source File
# Begin Source File

SOURCE=.\highgui.h
# End Source File
# Begin Source File

SOURCE=.\matheval.h
# End Source File
# Begin Source File

SOURCE=.\ob_and.h
# End Source File
# Begin Source File

SOURCE=.\ob_array3600.h
# End Source File
# Begin Source File

SOURCE=.\ob_average.h
# End Source File
# Begin Source File

SOURCE=.\ob_avi.h
# End Source File
# Begin Source File

SOURCE=.\ob_ballgame.h
# End Source File
# Begin Source File

SOURCE=.\ob_cam.h
# End Source File
# Begin Source File

SOURCE=.\ob_com_writer.h
# End Source File
# Begin Source File

SOURCE=.\ob_compare.h
# End Source File
# Begin Source File

SOURCE=.\ob_comreader.h
# End Source File
# Begin Source File

SOURCE=.\ob_constant.h
# End Source File
# Begin Source File

SOURCE=.\ob_correlation.h
# End Source File
# Begin Source File

SOURCE=.\ob_counter.h
# End Source File
# Begin Source File

SOURCE=.\ob_debounce.h
# End Source File
# Begin Source File

SOURCE=.\ob_deviation.h
# End Source File
# Begin Source File

SOURCE=.\ob_doku.h
# End Source File
# Begin Source File

SOURCE=.\ob_edf_reader.h
# End Source File
# Begin Source File

SOURCE=.\ob_edf_writer.h
# End Source File
# Begin Source File

SOURCE=.\ob_eeg.h
# End Source File
# Begin Source File

SOURCE=.\ob_erpdetect.h
# End Source File
# Begin Source File

SOURCE=.\ob_evaluator.h
# End Source File
# Begin Source File

SOURCE=.\ob_fft.h
# End Source File
# Begin Source File

SOURCE=.\ob_file_reader.h
# End Source File
# Begin Source File

SOURCE=.\ob_file_writer.h
# End Source File
# Begin Source File

SOURCE=.\ob_filter.h
# End Source File
# Begin Source File

SOURCE=.\ob_integrate.h
# End Source File
# Begin Source File

SOURCE=.\ob_keystrike.h
# End Source File
# Begin Source File

SOURCE=.\ob_magnitude.h
# End Source File
# Begin Source File

SOURCE=.\ob_martini.h
# End Source File
# Begin Source File

SOURCE=.\ob_matlab.h
# End Source File
# Begin Source File

SOURCE=.\ob_mci.h
# End Source File
# Begin Source File

SOURCE=.\ob_midi.h
# End Source File
# Begin Source File

SOURCE=.\ob_mixer4.h
# End Source File
# Begin Source File

SOURCE=.\ob_mouse.h
# End Source File
# Begin Source File

SOURCE=.\ob_not.h
# End Source File
# Begin Source File

SOURCE=.\ob_or.h
# End Source File
# Begin Source File

SOURCE=.\ob_osci.h
# End Source File
# Begin Source File

SOURCE=.\ob_particle.h
# End Source File
# Begin Source File

SOURCE=.\ob_peakdetect.h
# End Source File
# Begin Source File

SOURCE=.\ob_port_io.h
# End Source File
# Begin Source File

SOURCE=.\ob_sample_hold.h
# End Source File
# Begin Source File

SOURCE=.\ob_signal.h
# End Source File
# Begin Source File

SOURCE=.\ob_skindialog.h
# End Source File
# Begin Source File

SOURCE=.\ob_speller.h
# End Source File
# Begin Source File

SOURCE=.\ob_tcp_receive.h
# End Source File
# Begin Source File

SOURCE=.\ob_tcp_sender.h
# End Source File
# Begin Source File

SOURCE=.\ob_threshold.h
# End Source File
# Begin Source File

SOURCE=.\ob_translate.h
# End Source File
# Begin Source File

SOURCE=.\ob_wav.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SDL_sound.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\brainBay.ico
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow1.cur
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow2.cur
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow3.cur
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow4.cur
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow5.cur
# End Source File
# Begin Source File

SOURCE=.\debug\CURSORS\dwell_arrow6.cur
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\small.ico
# End Source File
# End Group
# End Target
# End Project
