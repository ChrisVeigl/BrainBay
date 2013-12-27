# Microsoft Developer Studio Generated NMAKE File, Based on brainBay.dsp
!IF "$(CFG)" == ""
CFG=brainBay - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. brainBay - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "brainBay - Win32 Release" && "$(CFG)" != "brainBay - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "brainBay.mak" CFG="brainBay - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "brainBay - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "brainBay - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "brainBay - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\brainBay.exe"


CLEAN :
	-@erase "$(INTDIR)\brainbay.obj"
	-@erase "$(INTDIR)\brainbay.res"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\midi.obj"
	-@erase "$(INTDIR)\ob_and.obj"
	-@erase "$(INTDIR)\ob_average.obj"
	-@erase "$(INTDIR)\ob_avi.obj"
	-@erase "$(INTDIR)\ob_correlation.obj"
	-@erase "$(INTDIR)\ob_doku.obj"
	-@erase "$(INTDIR)\ob_eeg.obj"
	-@erase "$(INTDIR)\ob_evaluator.obj"
	-@erase "$(INTDIR)\ob_fft.obj"
	-@erase "$(INTDIR)\ob_filter.obj"
	-@erase "$(INTDIR)\ob_magnitude.obj"
	-@erase "$(INTDIR)\ob_midi.obj"
	-@erase "$(INTDIR)\ob_not.obj"
	-@erase "$(INTDIR)\ob_or.obj"
	-@erase "$(INTDIR)\ob_osci.obj"
	-@erase "$(INTDIR)\ob_particle.obj"
	-@erase "$(INTDIR)\ob_signal.obj"
	-@erase "$(INTDIR)\ob_tcp_receive.obj"
	-@erase "$(INTDIR)\ob_threshold.obj"
	-@erase "$(INTDIR)\ob_translate.obj"
	-@erase "$(INTDIR)\ob_wav.obj"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\brainBay.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\brainBay.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\brainbay.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\brainBay.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\brainBay.pdb" /machine:I386 /out:"$(OUTDIR)\brainBay.exe" 
LINK32_OBJS= \
	"$(INTDIR)\brainbay.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\midi.obj" \
	"$(INTDIR)\ob_and.obj" \
	"$(INTDIR)\ob_average.obj" \
	"$(INTDIR)\ob_avi.obj" \
	"$(INTDIR)\ob_correlation.obj" \
	"$(INTDIR)\ob_doku.obj" \
	"$(INTDIR)\ob_eeg.obj" \
	"$(INTDIR)\ob_evaluator.obj" \
	"$(INTDIR)\ob_fft.obj" \
	"$(INTDIR)\ob_filter.obj" \
	"$(INTDIR)\ob_magnitude.obj" \
	"$(INTDIR)\ob_midi.obj" \
	"$(INTDIR)\ob_not.obj" \
	"$(INTDIR)\ob_or.obj" \
	"$(INTDIR)\ob_osci.obj" \
	"$(INTDIR)\ob_particle.obj" \
	"$(INTDIR)\ob_signal.obj" \
	"$(INTDIR)\ob_tcp_receive.obj" \
	"$(INTDIR)\ob_threshold.obj" \
	"$(INTDIR)\ob_translate.obj" \
	"$(INTDIR)\ob_wav.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\brainbay.res"

"$(OUTDIR)\brainBay.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "brainBay - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\brainBay.exe"


CLEAN :
	-@erase "$(INTDIR)\brainbay.obj"
	-@erase "$(INTDIR)\brainbay.res"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\midi.obj"
	-@erase "$(INTDIR)\ob_and.obj"
	-@erase "$(INTDIR)\ob_average.obj"
	-@erase "$(INTDIR)\ob_avi.obj"
	-@erase "$(INTDIR)\ob_correlation.obj"
	-@erase "$(INTDIR)\ob_doku.obj"
	-@erase "$(INTDIR)\ob_eeg.obj"
	-@erase "$(INTDIR)\ob_evaluator.obj"
	-@erase "$(INTDIR)\ob_fft.obj"
	-@erase "$(INTDIR)\ob_filter.obj"
	-@erase "$(INTDIR)\ob_magnitude.obj"
	-@erase "$(INTDIR)\ob_midi.obj"
	-@erase "$(INTDIR)\ob_not.obj"
	-@erase "$(INTDIR)\ob_or.obj"
	-@erase "$(INTDIR)\ob_osci.obj"
	-@erase "$(INTDIR)\ob_particle.obj"
	-@erase "$(INTDIR)\ob_signal.obj"
	-@erase "$(INTDIR)\ob_tcp_receive.obj"
	-@erase "$(INTDIR)\ob_threshold.obj"
	-@erase "$(INTDIR)\ob_translate.obj"
	-@erase "$(INTDIR)\ob_wav.obj"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\brainBay.exe"
	-@erase "$(OUTDIR)\brainBay.ilk"
	-@erase "$(OUTDIR)\brainBay.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\brainbay.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\brainBay.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib opengl32.lib glu32.lib SDL.lib SDL_net.lib fidlib.lib matheval.lib SDL_sound.lib modplug.lib vfw32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\brainBay.pdb" /debug /machine:I386 /out:"$(OUTDIR)\brainBay.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\brainbay.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\midi.obj" \
	"$(INTDIR)\ob_and.obj" \
	"$(INTDIR)\ob_average.obj" \
	"$(INTDIR)\ob_avi.obj" \
	"$(INTDIR)\ob_correlation.obj" \
	"$(INTDIR)\ob_doku.obj" \
	"$(INTDIR)\ob_eeg.obj" \
	"$(INTDIR)\ob_evaluator.obj" \
	"$(INTDIR)\ob_fft.obj" \
	"$(INTDIR)\ob_filter.obj" \
	"$(INTDIR)\ob_magnitude.obj" \
	"$(INTDIR)\ob_midi.obj" \
	"$(INTDIR)\ob_not.obj" \
	"$(INTDIR)\ob_or.obj" \
	"$(INTDIR)\ob_osci.obj" \
	"$(INTDIR)\ob_particle.obj" \
	"$(INTDIR)\ob_signal.obj" \
	"$(INTDIR)\ob_tcp_receive.obj" \
	"$(INTDIR)\ob_threshold.obj" \
	"$(INTDIR)\ob_translate.obj" \
	"$(INTDIR)\ob_wav.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\brainbay.res"

"$(OUTDIR)\brainBay.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("brainBay.dep")
!INCLUDE "brainBay.dep"
!ELSE 
!MESSAGE Warning: cannot find "brainBay.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "brainBay - Win32 Release" || "$(CFG)" == "brainBay - Win32 Debug"
SOURCE=.\brainbay.cpp

"$(INTDIR)\brainbay.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\brainbay.rc

"$(INTDIR)\brainbay.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\dialogs.cpp

"$(INTDIR)\dialogs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\draw.cpp

"$(INTDIR)\draw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\files.cpp

"$(INTDIR)\files.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\globals.cpp

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\midi.cpp

"$(INTDIR)\midi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_and.cpp

"$(INTDIR)\ob_and.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_average.cpp

"$(INTDIR)\ob_average.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_avi.cpp

"$(INTDIR)\ob_avi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_correlation.cpp

"$(INTDIR)\ob_correlation.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_doku.cpp

"$(INTDIR)\ob_doku.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_eeg.cpp

"$(INTDIR)\ob_eeg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_evaluator.cpp

"$(INTDIR)\ob_evaluator.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_fft.cpp

"$(INTDIR)\ob_fft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_filter.cpp

"$(INTDIR)\ob_filter.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_magnitude.cpp

"$(INTDIR)\ob_magnitude.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_midi.cpp

"$(INTDIR)\ob_midi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_not.cpp

"$(INTDIR)\ob_not.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_or.cpp

"$(INTDIR)\ob_or.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_osci.cpp

"$(INTDIR)\ob_osci.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_particle.cpp

"$(INTDIR)\ob_particle.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_signal.cpp

"$(INTDIR)\ob_signal.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_tcp_receive.cpp

"$(INTDIR)\ob_tcp_receive.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_threshold.cpp

"$(INTDIR)\ob_threshold.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_translate.cpp

"$(INTDIR)\ob_translate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\ob_wav.cpp

"$(INTDIR)\ob_wav.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\timer.cpp

"$(INTDIR)\timer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"


SOURCE=.\tty.cpp

"$(INTDIR)\tty.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\brainBay.pch"



!ENDIF 

