  /* ----------------------------------------------------------------------------------

  BrainBay  -  OpenSource Application for realtime BodySignalProcessing & HCI
               with the OpenEEG hardware, GPL 2003-2010
			   
  
  MODULE:  GLOBALS.CPP
  Author: Chris Veigl, contact: chris@shifz.org
		
		  provides functions for dialog-, object- and format-handling
   
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include <tchar.h>
#include <process.h>
#include <tlhelp32.h>

#include "ob_evaluator.h"
#include "ob_evaluator_exprtk.h"
#include "ob_fft.h"
#include "ob_midi.h"
#include "ob_osci.h"
#include "ob_eeg.h"
#include "ob_filter.h"
#include "ob_threshold.h"
#include "ob_magnitude.h"
#include "ob_signal.h"
#include "ob_particle.h"
#include "ob_translate.h"
#include "ob_and.h"
#include "ob_avi.h"
#include "ob_or.h"
#include "ob_not.h"
#include "ob_wav.h"
#include "ob_tcp_receive.h"
#include "ob_doku.h"
#include "ob_average.h"
#include "ob_correlation.h"
#include "ob_edf_reader.h"
#include "ob_edf_writer.h"
#include "ob_tcp_sender.h"
#include "ob_compare.h"
#include "ob_ballgame.h"
#include "ob_mixer4.h"
#include "ob_mouse.h"
#include "ob_erpdetect.h"
#include "ob_com_writer.h"
#include "ob_cam.h"
#include "ob_integrate.h"
#include "ob_debounce.h"
#include "ob_sample_hold.h"
#include "ob_constant.h"
#include "ob_matlab.h"
#include "ob_counter.h"
#ifndef MINGW
#include "ob_skindialog.h"
#endif
#include "ob_file_writer.h"
#include "ob_deviation.h"
#include "ob_mci.h"
#include "ob_keystrike.h"
#include "ob_peakdetect.h"
#include "ob_speller.h"
#include "ob_martini.h"
#include "ob_file_reader.h"
#include "ob_port_io.h"
#include "ob_array3600.h"
#include "ob_comreader.h"
#include "ob_neurobit.h"
#include "ob_min.h"
#include "ob_max.h"
#include "ob_round.h"
#include "ob_differentiate.h"
#include "ob_delay.h"
#include "ob_limiter.h"
#include "ob_emotiv.h"
#include "ob_floatvector.h"
#include "ob_vectorfloat.h"
#include "ob_displayvector.h"
#include "ob_buffer.h"
#include "ob_ganglion.h"
#include "ob_sessiontime.h"
#include "ob_sessionmanager.h"
#include "ob_keycapture.h"
#include "ob_button.h"
#include "ob_shadow.h"
#include "ob_volume.h"
#include "ob_osc_sender.h"
#include "ob_biosemi.h"
#include "ob_brainflow.h"

//
// GLOBAL VARIABLES
//

HINSTANCE  hInst;			 //  instance of main class
HACCEL     ghAccel;			 //  keyboard accelerator
HWND       ghWndMain;		 //  handle of main window
HWND	   ghWndToolbox;     //  handle of toolbox window (actual object settings)
HWND	   ghWndAnimation;   //  handle of Animation Window
HWND	   ghWndStatusbox;      //  handle of Status Window
HWND	   ghWndSettings;      //  handle of Settings Window
HWND	   ghWndDesign;      //  handle of Design Window
HGLRC      GLRC_Animation;	 // Handle to GL-context of Animation Window

int PACKETSPERSECOND=DEF_PACKETSPERSECOND;

BASE_CL * objects[MAX_OBJECTS];
BASE_CL * actobject;
BASE_CL * deviceobject;
BASE_CL * copy_object;
int    actport;
struct LINKStruct * actconnect;

struct TTYStruct		    TTY;
struct PACKETStruct		    PACKET;
struct DRAWStruct		    DRAW;
struct GLOBALStruct         GLOBAL;
struct CAPTFILEStruct	    CAPTFILE;
struct MIDIPORTStruct       MIDIPORTS[MAX_MIDIPORTS];
struct SCALEStruct          LOADSCALE;
struct TIMINGStruct         TIMING;

char objnames[OBJECT_COUNT][20]      = { OBJNAMES };
char dimensions[10][10]      = {"uV","mV","V","Hz","%","DegC","DegF","uS","kOhm","BPM" };
int  fft_bin_values[10]    = { 32,64,128,256,512,1024,2048,4096,0 };

int singletonObjects [] = {OB_EEG,OB_WAV,OB_CAM,OB_SKINDIALOG,OB_NEUROBIT,OB_GANGLION,OB_SESSIONMANAGER,OB_BRAINFLOW,-1};
	

//
// Create-Object- Function
// Add new Objects here :
//

void create_object(int type)
{
	int i;

	for (i=0;singletonObjects[i]!=-1;i++) {
		if (type==singletonObjects[i]){
			for (int t=0; t<GLOBAL.objects;t++)
				if (objects[t]->type==type) return;
		}
	}
	close_toolbox();
	write_logfile("creating object: %s",objnames[type]);


	switch(type) 
	{ 
		case OB_EEG:		 actobject=new EEGOBJ(GLOBAL.objects); 
							 deviceobject=actobject;
							 actobject->object_size=sizeof(EEGOBJ);break;
		case OB_MIDI:		 actobject=new MIDIOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(MIDIOBJ);break;
		case OB_OSCI:		 actobject=new OSCIOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(OSCIOBJ);break;
		case OB_FFT:		 actobject=new FFTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(FFTOBJ);break;
		case OB_THRESHOLD:	 actobject=new THRESHOLDOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(THRESHOLDOBJ);break;
		case OB_FILTER:		 actobject=new FILTEROBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(FILTEROBJ);break;
		case OB_MAGNITUDE:	 actobject=new MAGNITUDEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MAGNITUDEOBJ);break;
		case OB_PARTICLE:	 actobject=new PARTICLEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(PARTICLEOBJ);break;
		case OB_TRANSLATE:	 actobject=new TRANSLATEOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(TRANSLATEOBJ);break;
		case OB_SIGNAL:		 actobject=new SIGNALOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SIGNALOBJ);break;
		case OB_AND:		 actobject=new ANDOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(ANDOBJ);break;
		case OB_OR:			 actobject=new OROBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(OROBJ);break;
		case OB_NOT:		 actobject=new NOTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(NOTOBJ);break;
		case OB_WAV:		 actobject=new WAVOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(WAVOBJ);break;
		case OB_TCP_RECEIVER:actobject=new TCP_RECEIVEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(TCP_RECEIVEOBJ);break;
		case OB_DOKU:        actobject=new DOKUOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DOKUOBJ);break;
		case OB_EVAL:		 actobject=new EVALOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(EVALOBJ);break;
		case OB_EVAL_EXPRTK: createEvaluatorExprtk(GLOBAL.objects, &actobject); break;

		case OB_AVI:		 actobject=new AVIOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(AVIOBJ);break;
		case OB_AVERAGE:	 actobject=new AVERAGEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(AVERAGEOBJ);break;
		case OB_CORR:		 actobject=new CORRELATIONOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(CORRELATIONOBJ);break;
		case OB_EDF_READER:	 actobject=new EDF_READEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(EDF_READEROBJ);break;
		case OB_EDF_WRITER:	 actobject=new EDF_WRITEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(EDF_WRITEROBJ);break;
		case OB_TCP_SENDER:	 actobject=new TCP_SENDEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(TCP_SENDEROBJ);break;
		case OB_COMPARE:	 actobject=new COMPAREOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(COMPAREOBJ);break;
		case OB_BALLGAME:	 actobject=new BALLGAMEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(BALLGAMEOBJ);break;
		case OB_MIXER4:		 actobject=new MIXER4OBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MIXER4OBJ);break;
		case OB_MOUSE:		 actobject=new MOUSEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MOUSEOBJ);break;
		case OB_ERPDETECT:   actobject=new ERPDETECTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(ERPDETECTOBJ);break;
		case OB_COM_WRITER:  actobject=new COM_WRITEROBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(COM_WRITEROBJ);break;
		case OB_CAM:		 actobject=new CAMOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(CAMOBJ);break;
		case OB_INTEGRATE:	 actobject=new INTEGRATEOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(INTEGRATEOBJ);break;
		case OB_DEBOUNCE:	 actobject=new DEBOUNCEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DEBOUNCEOBJ);break;
		case OB_SAMPLE_HOLD: actobject=new SAMPLE_HOLDOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(SAMPLE_HOLDOBJ);break;
		case OB_CONSTANT:    actobject=new CONSTANTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(CONSTANTOBJ);break;
        case OB_MATLAB: 	 actobject=new MATLABOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MATLABOBJ);break;
        case OB_COUNTER: 	 actobject=new COUNTEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(COUNTEROBJ);break;
#ifndef MINGW
        case OB_SKINDIALOG:  actobject=new SKINDIALOGOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SKINDIALOGOBJ);break;
#endif
        case OB_FILE_WRITER: actobject=new FILE_WRITEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(FILE_WRITEROBJ);break;
        case OB_DEVIATION:   actobject=new DEVIATIONOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DEVIATIONOBJ);break;
		case OB_MCIPLAYER:   actobject=new MCIOBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(MCIOBJ);break;
		case OB_KEYSTRIKE:   actobject=new KEYSTRIKEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(KEYSTRIKEOBJ);break;
		case OB_PEAKDETECT:  actobject=new PEAKDETECTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(PEAKDETECTOBJ);break;
		case OB_SPELLER:	 actobject=new SPELLEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SPELLEROBJ);break;
		case OB_MARTINI:	 actobject=new MARTINIOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MARTINIOBJ);break;
		case OB_FILE_READER: actobject=new FILE_READEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(FILE_READEROBJ);break;
		case OB_PORT_IO:	 actobject=new PORTOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(PORTOBJ);break;
		case OB_ARRAY3600:   actobject=new ARRAY3600OBJ(GLOBAL.objects);
							 actobject->object_size=sizeof(ARRAY3600OBJ);break;
		case OB_COMREADER:   actobject=new COMREADEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(COMREADEROBJ);break;
		case OB_NEUROBIT:      actobject=new NEUROBITOBJ(GLOBAL.objects); 
							 deviceobject=actobject;
							 actobject->object_size=sizeof(NEUROBITOBJ);break;
		case OB_MIN:		 actobject=new MINOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MINOBJ);break;
		case OB_MAX:         actobject=new MAXOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(MAXOBJ);break;
		case OB_ROUND:      actobject=new ROUNDOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(ROUNDOBJ);break;
		case OB_DIFFERENTIATE:      actobject=new DIFFERENTIATEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DIFFERENTIATEOBJ);break;
		case OB_DELAY:      actobject=new DELAYOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DELAYOBJ);break;
		case OB_LIMITER:      actobject=new LIMITEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(LIMITEROBJ);break;
		case OB_EMOTIV:		 actobject=new EMOTIVOBJ(GLOBAL.objects);
							 deviceobject=actobject;
							 actobject->object_size=sizeof(EMOTIVOBJ);break;
		case OB_FLOATVECTOR: actobject=new FLOATVECTOROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(FLOATVECTOROBJ);break;
		case OB_VECTORFLOAT: actobject=new VECTORFLOATOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(VECTORFLOATOBJ);break;
		case OB_DISPLAYVECTOR: actobject=new DISPLAYVECTOROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(DISPLAYVECTOROBJ);break;
		case OB_BUFFER: actobject=new BUFFEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(BUFFEROBJ);break;
		case OB_GANGLION:    actobject=new GANGLIONOBJ(GLOBAL.objects); 
							 deviceobject=actobject;
							 actobject->object_size=sizeof(GANGLIONOBJ);break;
		case OB_SESSIONTIME: actobject=new SESSIONTIMEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SESSIONTIMEOBJ);break;
		case OB_SESSIONMANAGER: actobject=new SESSIONMANAGEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SESSIONMANAGEROBJ);break;
		case OB_KEYCAPTURE:  actobject=new KEYCAPTUREOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(KEYCAPTUREOBJ);break;
		case OB_BUTTON:		 actobject=new BUTTONOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(BUTTONOBJ);break;
		case OB_SHADOW:		 actobject=new SHADOWOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(SHADOWOBJ);break;
		case OB_VOLUME:		 actobject=new VOLUMEOBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(VOLUMEOBJ);break;
		case OB_OSC_SENDER:  actobject=new OSC_SENDEROBJ(GLOBAL.objects); 
							 actobject->object_size=sizeof(OSC_SENDEROBJ);break;
		case OB_BIOSEMI:	 actobject = new BIOSEMIOBJ(GLOBAL.objects); 
				 			 actobject->object_size = sizeof(BIOSEMIOBJ); break;
		case OB_BRAINFLOW:
#if _MSC_VER >= 1900
							 actobject = new BRAINFLOWOBJ(GLOBAL.objects);
							 deviceobject = actobject;
							 actobject->object_size = sizeof(BRAINFLOWOBJ); break;
#else
			MessageBox(NULL, "Brainflow element is only supported if BrainBay is compiled with newer version of Visual Studio (Platform toolset V142)", "Error", MB_OK);
#endif

	}
	if (actobject)
	{
		actobject->type = type;
		actobject->xPos=50; actobject->yPos=40;
		i=actobject->inports;
        if (i<actobject->outports) i=actobject->outports;
		if (!actobject->width) actobject->width=55; 
		if (!actobject->height) actobject->height=CON_START+i*CON_HEIGHT+5;
		if (!actobject->tag[0]) strcpy(actobject->tag,objnames[type]);

		for (i=0;i<MAX_CONNECTS;i++)
		{ 
		   actobject->out[i].from_object=GLOBAL.objects; actobject->out[i].from_port=-1;
		   actobject->out[i].to_object=-1; actobject->out[i].to_port=-1; 
		   strcpy(actobject->out[i].dimension,"uV");
		   actobject->out[i].min=-250;actobject->out[i].max=250;
		}
		objects[GLOBAL.objects]=actobject;
		GLOBAL.objects++;
        if (!GLOBAL.loading)
		{
			actobject->make_dialog();
//			InvalidateRect(ghWndMain,NULL,TRUE);
			InvalidateRect(ghWndDesign,NULL,TRUE);
			if (actobject->displayWnd) 
				SetWindowPos(actobject->displayWnd,HWND_TOP,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
		}
		if (actobject->displayWnd)   SetForegroundWindow(actobject->displayWnd);

	}
}

int count_objects(int type)
{ 
	int t,f=0;
	for (t=0;t<GLOBAL.objects;t++) if (objects[t]->type==type) f++;
	return(f);
}


void reset_oscilloscopes(void)
{
	int t;
	for (t=0;t<GLOBAL.objects;t++)
	{
		if (objects[t]->type==OB_OSCI)
		{
			OSCIOBJ* st=(OSCIOBJ *) objects[t];
			st->signal_pos=0;
			st->timercount=0;
			st->newpixels=0;
			st->laststamp=(float)TIMING.packetcounter/(float)PACKETSPERSECOND;
			
			InvalidateRect(st->displayWnd,NULL,FALSE);
		}
		if (objects[t]->type==OB_FFT)
		{
			FFTOBJ* st=(FFTOBJ *) objects[t];
			st->h_pos=0;
			st->chnBufPos=0;
			InvalidateRect(st->displayWnd,NULL,TRUE);
		}
	}

}


void print_time(char * str, float f, int mode)
{
	int hour,min,sec;

	if (mode==0)
	{
		if (GLOBAL.add_archivetime) f+=GLOBAL.addtime;
		hour= ((int)(f/3600))%24;
		min = ((int)(f/60))%60;
		sec= ((int)f)%60;
		if (hour==0) sprintf(str," %02d:%02d ",min,sec);
		else sprintf(str," %02d:%02d:%02d ",hour,min,sec);
	}
	else
	{
		if (GLOBAL.add_archivetime) f+=GLOBAL.addtime;
		hour= ((int)(f/3600))%24;
		min = ((int)(f/60))%60;
		sec= ((int)f)%60;
		
		sprintf(str," %02d:%02d:%02d.%03d ",hour,min,sec,(int)((f-(int)f)*1000));
	}
}

float get_time (char * str)
{
	float f;
	char * pos;
	char sav[20];
	int min=0;


	strcpy(sav,str);

	if ((pos=strstr(sav,":")))
	{
		*pos=0;
		min=atoi(sav);
		pos++;

	}
	else pos=sav;

	f=(float)atof(pos)+(float)min*60;
	return(f);

}


void get_session_length(void)
{
	int t;
	long act;

	GLOBAL.session_length=0;

	for (t=0;t<GLOBAL.objects;t++)
	{
		act=objects[t]->session_length();
		if (GLOBAL.session_length<act) GLOBAL.session_length=act;
	}
	update_status_window();
	GLOBAL.session_start=0;
	GLOBAL.session_end=GLOBAL.session_length;
	SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,0);
	SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELEND,TRUE,get_sliderpos(GLOBAL.session_end));
    update_statusinfo();
}


void set_session_pos(long pos)
{
	int t,barpos;
	char szdata[20];
	int runflag=0;

	if (GLOBAL.running) {runflag=1; stop_timer();}

	TIMING.packetcounter= pos; 
	barpos= (int) ((float)pos/(float)GLOBAL.session_length*1000.0f);

	for (t=0;t<GLOBAL.objects;t++) objects[t]->session_pos(pos);

	print_time(szdata,(float)pos/(float)PACKETSPERSECOND,1);

	SetDlgItemText(ghWndStatusbox,IDC_TIME,szdata);
//	SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETSELSTART,TRUE,barpos);
//	SendMessage(GetDlgItem(ghWndStatusbox,IDC_SESSIONPOS),TBM_SETPOS,TRUE,barpos);
//	SetDlgItemText(ghWndStatusbox,IDC_JUMPPOS,szdata);
	reset_oscilloscopes();
	if (runflag) start_timer();

}


void register_classes (HINSTANCE hInstance)
{

	WNDCLASSEX wcex;


	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
    wcex.hIconSm          = LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hInstance        = hInstance;
    wcex.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName     = NULL;
	wcex.lpfnWndProc = (WNDPROC)FFTWndHandler;
    wcex.lpszClassName    = "FFTClass";
    if (!RegisterClassEx(&wcex))  report_error("Can't register FFTWindowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
    wcex.hIconSm          = LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hInstance        = hInstance;
    wcex.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName     = NULL;
	wcex.lpfnWndProc = (WNDPROC)AVIWndHandler;
    wcex.lpszClassName    = "AVIClass";
    if (!RegisterClassEx(&wcex))  report_error("Can't register AVIWindowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
    wcex.hIconSm          = LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
	wcex.lpfnWndProc = (WNDPROC)AnimationWndHandler;
    wcex.hInstance        = hInstance;
    wcex.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszClassName    = "AnimationClass";
    wcex.lpszMenuName     = NULL;

    if (!RegisterClassEx(&wcex))  report_error("Can't register ParticleWindowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)MainWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_MAINMENU;
	wcex.lpszClassName	= "brainBay_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Main-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)OsciWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Osci_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Osci-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)BallgameWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Ballgame_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Ballgame-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)MartiniWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Martini_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Martini-Windowclass");
	

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)SpellerWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Speller_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Speller-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS ;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)DesignWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Design_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Design-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)MeterWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Meter_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Meter-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)SessionManagerWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "SessionManager_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register SessionManager-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)ButtonWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Button_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Button-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)CounterWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Counter_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Counter-Windowclass");

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)ShadowWndHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MYEEG);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Shadow_Class";
	wcex.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	if(!RegisterClassEx(&wcex))
        report_error("Can't register Shadow-Windowclass");
	
}    

BOOL killProcess(char * name )
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;

  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {   
    return( FALSE );
  }

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );

  // Retrieve information about the first process,
  // and exit if unsuccessful
  if( !Process32First( hProcessSnap, &pe32 ) )
  {   
    CloseHandle( hProcessSnap );  // clean the snapshot object
    return( FALSE );
  }

  // Now walk the snapshot of processes 
  do
  {  
    if (!(strcmp(pe32.szExeFile,name)))
	{
		DWORD dwDesiredAccess = PROCESS_TERMINATE;
		BOOL  bInheritHandle  = FALSE;
		HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, pe32.th32ProcessID);
		if (hProcess != NULL) {
 		   BOOL result = TerminateProcess(hProcess, 1);
		   CloseHandle(hProcess);
		}
    } 
  } while( Process32Next( hProcessSnap, &pe32 ) );

  CloseHandle( hProcessSnap );
  return( TRUE );
}



void init_devicetype(void)
{
	TTY.BAUDRATE=DEF_BAUDRATE;
	TTY.devicetype=DEV_MODEEG_P2;        // Default = Modular EEG Firmware Version P2
	TTY.amount_to_read=AMOUNT_TO_READ[TTY.devicetype];
	TTY.bytes_per_packet=BYTES_PER_PACKET[TTY.devicetype];
}

void init_path()
{
	char * newpath;

	GetModuleFileName(NULL,GLOBAL.resourcepath,sizeof(GLOBAL.resourcepath));
    low_chars(GLOBAL.resourcepath,GLOBAL.resourcepath);
	newpath=strstr(GLOBAL.resourcepath,"brainbay.exe");
	if (!newpath) report_error("filename error: brainbay.exe not found");
	else *newpath=0;
}

void GlobalInitialize()
{
    InitCommonControls(); 

/*	if(SDL_WasInit(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_VIDEO) < 0)
			report_error("Couldn't init SDL");
	}
*/
		
	if(SDL_Init(SDL_INIT_EVERYTHING)<0) 
        printf("Couldn't init SDL");
	
   if (!Sound_Init()) report_error("Couldn't init SDL_Sound");


	if(SDLNet_Init()<0)	report_error("Couldn't init SDL_Net");

	GLOBAL.os_version=check_OS();


    TTY.ThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (TTY.ThreadExitEvent == NULL)
        report_error("CreateEvent (Thread exit event)");       

	init_devicetype();
	TTY.COMDEV=INVALID_HANDLE_VALUE;
	TTY.CONNECTED=FALSE;
	TTY.read_pause=TRUE;
	TTY.amount_to_write=0;
	TTY.writeMutex=	CreateMutex( NULL, FALSE, NULL ); 

	TTY.FLOW_CONTROL=0;

	PACKET.readstate=0;
	PACKET.info=0;
	PACKET.requestedinfo=0;

	GLOBAL.objects=0;	actobject=NULL; actconnect=NULL;
	GLOBAL.tool_left=400;GLOBAL.tool_top=100;
	GLOBAL.tool_right=800;GLOBAL.tool_bottom=400;
	GLOBAL.anim_left=20;GLOBAL.anim_right=420;
	GLOBAL.anim_top=350;GLOBAL.anim_bottom=700;
	GLOBAL.design_left=20;GLOBAL.design_right=500;
	GLOBAL.design_top=20;GLOBAL.design_bottom=400;
	GLOBAL.startup=0; GLOBAL.autorun=0; GLOBAL.configfile[0]=0;
	GLOBAL.startdesign=0;
	GLOBAL.locksession=0;
	GLOBAL.syncloss=0;
	GLOBAL.dialog_interval=DIALOG_UPDATETIME;
	GLOBAL.draw_interval=DRAW_UPDATETIME;
	GLOBAL.neurobit_available=0;
	strcpy(GLOBAL.neurobit_device,DEFAULT_NEUROBIT_DEVICE);
	GLOBAL.emotiv_available=0;
	GLOBAL.biosemi_available = 0;
	GLOBAL.ganglion_available=0;
	GLOBAL.brainflow_available = 0;
	GLOBAL.ganglion_bledongle=1;  // BLED112 dongle is default
	GLOBAL.use_cv_capture=0;
	strcpy(GLOBAL.emotivpath,"C:\\Program Files (x86)\\Emotiv Development Kit_v1.0.0.3-PREMIUM");
	strcpy(GLOBAL.ganglionhubpath,"C:\\Program Files\\OpenBCI_GUI\\data\\OpenBCIHub\\OpenBCIHub.exe");
	strcpy(GLOBAL.gangliondevicename,"idle");
	strcpy(GLOBAL.startdesignpath,"");

	GLOBAL.loading=false;
	GLOBAL.read_tcp=0;
	GLOBAL.packet_parsed=0;
	GLOBAL.actcolumn=0;
	GLOBAL.running=false;
	GLOBAL.showdesign=TRUE;
	GLOBAL.showtoolbox=-1;
	GLOBAL.session_length=0;
	GLOBAL.session_start=0;
	GLOBAL.session_end=0;
	GLOBAL.session_loop =0 ;
	GLOBAL.session_sliding=0;

	GLOBAL.main_maximized=0;
	GLOBAL.minimized=0;
	GLOBAL.statusWindowHeight=41;
	GLOBAL.statusWindowHeightWithPlayer=76;
	GLOBAL.statusWindowMargin=48;
	GLOBAL.statusWindowMarginWithPlayer=83;

	GLOBAL.run_exception=0;
	GLOBAL.fly=0;
	GLOBAL.pressed_key=0;

	GLOBAL.P3ALC1=12;
	GLOBAL.P3ALC2=12;

	TIMING.timerid=0;

	CAPTFILE.filetype=FILE_INTMODE;
	CAPTFILE.filehandle=INVALID_HANDLE_VALUE;
	CAPTFILE.file_action=0;
	CAPTFILE.do_read=0;
	CAPTFILE.do_write=0;
	CAPTFILE.length=0;
	CAPTFILE.start=0;
	strcpy(CAPTFILE.filename,"none");

	ghWndAnimation=NULL;
    ghWndToolbox=NULL;
    ghWndSettings=NULL;
    ghWndDesign=NULL;
	copy_object=NULL;

	init_draw();
	init_midi();

	load_settings();
	
	TIMING.timerid=0;
	TIMING.pause_timer=0;
	init_system_time();

	write_logfile("application init successful.");
    return ;
}



void GlobalCleanup()
{
	int t;

	stop_timer();
	for (t=0;t<GLOBAL.objects;t++) objects[t]->session_stop();
	
	while (GLOBAL.objects>0)   free_object(0);
	
	BreakDownCommPort();
	// for (t=0;t<GLOBAL.objects;t++) free_object(0);

    CloseHandle(TTY.ThreadExitEvent);
	for (t=0;t<GLOBAL.midiports;t++)
  	  if (MIDIPORTS[t].midiout) midiOutClose(MIDIPORTS[t].midiout);

	if (CAPTFILE.filehandle!=0) CloseHandle(CAPTFILE.filehandle);

	SDLNet_Quit();
	SDL_Quit();

	DeleteObject(DRAW.brush_blue);
	DeleteObject(DRAW.pen_blue);
	DeleteObject(DRAW.pen_white);
	write_logfile("BrainBay normal shutdown.");

    return;
}



float size_value(float min,float max, float x, float to_min, float to_max, int clip)
{  
	float r,si,ti;


	si=max-min; ti=to_max-to_min;
	r=(x-min)/si*ti+to_min;
	if (clip)
	{
	  if (r>to_max) return to_max;
	  if (r<to_min) return to_min;
	}
	return(r);
}


void low_chars(char * to, char * from)
{
	int t;
	for (t=0;from[t];t++)
	{

		if ((from[t]>='A')&&(from[t]<='Z')) to[t]=from[t]-'A'+'a';
		else to[t]=from[t];
	}
	to[t]=0;
}

void up_chars(char * to, char * from)
{
	int t;
	for (t=0;from[t];t++)
	{

		if ((from[t]>='a')&&(from[t]<='z')) to[t]=from[t]-'a'+'A';
		else to[t]=from[t];
	}
	to[t]=0;
}

void copy_string(char * source, int from, int to, char * target)
{
	int x,y=0;
	for (x=from; (x<to)&&(source[x]); x++) target[y++]=source[x];
	target[y]=0; // --y
}

int get_int(char * str, int pos, int * target)
{
		int temp=0,f=1;
		
		while (((str[pos]<'0')||(str[pos]>'9'))&&(str[pos]!='-')&&(str[pos]!=0)) pos++;
		if (str[pos]=='-') { f=-1; pos++;}
		while ((str[pos]>='0')&&(str[pos]<='9')) {temp*=10; temp+=str[pos]-'0'; pos++;}
		* target = temp*f;
		return pos;
}
	
int get_float(char * source, const char * param, float * result)
{
	char actline[100];
	char * pos;
	int i,tempint;
	float tempfloat;

	for (i=0; (source[i]!=10)&&(source[i]!=13)&&(source[i]!=')')&&(i<100); i++)
		actline[i]=source[i];
	actline[i]=0;
	if (pos=strstr(actline,param))
	{ 
		i=get_int(pos,0, &tempint);
		* result = (float) tempint;
		if(pos[i]=='.')
		{
		  i=get_int(pos,i, &tempint);
		  tempfloat=(float) tempint; while (tempfloat>1) tempfloat/=10.0f;
		  if (*result<0) tempfloat=-tempfloat;
		  * result += tempfloat;
		}
		return (i);
	}
	return(0);
}

int get_string(char * source, const char * param, char * result)
{
	char actline[200];
	char * pos;
	int i;

	for (i=0; (source[i])&&(source[i]!=10)&&(source[i]!=13)&&(source[i]!=')')&&(i<200); i++)
		actline[i]=source[i];
	actline[i]=0;
	if (pos=strstr(actline,param))
	{ 
		pos+=strlen(param);
		for (i=0; (*pos)&&(*pos!=10)&&(*pos!=13)&&(*pos!=')')&&(i<9); i++,pos++)
		  result[i]=*pos;
		result[i]=0;
		return (i);
	}
	return(0);
}

void free_object(int actobj)
{
	delete objects[actobj];
	memcpy(&objects[actobj],&objects[actobj+1],sizeof(objects[0])*(GLOBAL.objects-actobj));
	GLOBAL.objects--;
}



void swap_objects(int a, int b)
{
	int x,i;
	BASE_CL * sav;

    for (x=0;x<GLOBAL.objects;x++)
		for (i=0;objects[x]->out[i].to_object!=-1;i++)
		{
			if (objects[x]->out[i].to_object==a) objects[x]->out[i].to_object=b;
			else if (objects[x]->out[i].to_object==b) objects[x]->out[i].to_object=a;
			if (objects[x]->out[i].from_object==a) objects[x]->out[i].from_object=b;
			else if (objects[x]->out[i].from_object==b) objects[x]->out[i].from_object=a;
		}

	if (GLOBAL.drawfromobj==a) GLOBAL.drawfromobj=b;
	else if (GLOBAL.drawfromobj==b) GLOBAL.drawfromobj=a;

	sav=objects[a];
    objects[a]=objects[b];
    objects[b]=sav;

}

int sort_objects(void)
{
	int t,u,i,it,sav_timerpause;

	sav_timerpause=TIMING.pause_timer;
	TIMING.pause_timer=1;
	for (u=0;u<GLOBAL.objects;u++)
	  for (i=0;(i<MAX_CONNECTS)&&(objects[u]->out[i].to_object!=-1);i++)
		  objects[u]->out[i].visited=0;

	it=0;
	for (t=0;t<GLOBAL.objects;t++)
	 for (u=0;u<GLOBAL.objects;u++)
	  for (i=0;(i<MAX_CONNECTS)&&(objects[u]->out[i].to_object!=-1);i++)
		if ((objects[u]->out[i].to_object==t)&&(u>t)&&(objects[u]->out[i].visited==0))
		{
		   objects[u]->out[i].visited=1;
		   swap_objects(u,t);
		   t=0;u=GLOBAL.objects; i=MAX_CONNECTS;
/*
		   it++; if (it>10000) 
		   {  // report("loop"); 
			   return(FALSE);
		   }
		   */

		}
	TIMING.pause_timer=sav_timerpause;
	return(TRUE);
}



struct LINKStruct * get_link (BASE_CL * obj,int port)
{
	int t,i;

	for (t=0;t<GLOBAL.objects;t++)
  	  for (i=0;objects[t]->out[i].to_port!=-1;i++)
		if ((objects[t]->out[i].to_port==port)&&(objects[objects[t]->out[i].to_object]==(BASE_CL*) obj))
		  return(&objects[t]->out[i]);
    return(NULL);
}

int count_inports(BASE_CL * obj)
{
    int i,z,m;

	m=-1;
	for (i=0;i<GLOBAL.objects;i++)
		for (z=0;objects[i]->out[z].to_port!=-1;z++)
			if ((objects[objects[i]->out[z].to_object]==obj)&&(objects[i]->out[z].to_port>m)) m=objects[i]->out[z].to_port;
    return(m+2);
}

void set_dimensions(struct LINKStruct * act,float max, float min, char * dim, char * desc)
{
	
	struct LINKStruct * next;
	int t;


		act->max=max; 
		act->min=min; 
		strcpy(act->dimension,dim);
		strcpy(act->description,desc);

		if(objects[act->to_object]->in_ports[act->to_port].get_range)
		{
			objects[act->to_object]->in_ports[act->to_port].in_min=min;
			objects[act->to_object]->in_ports[act->to_port].in_max=max;
			strcpy(objects[act->to_object]->in_ports[act->to_port].in_dim,dim);
			strcpy(objects[act->to_object]->in_ports[act->to_port].in_desc,desc);
		}

		
		for(next=objects[act->to_object]->out;next->to_port!=-1;next++)
		{	
			for (t=0;t<objects[act->to_object]->outports;t++)
			if (objects[act->to_object]->out_ports[t].get_range==act->to_port)
			{
				objects[act->to_object]->out_ports[t].out_max=act->max;
				objects[act->to_object]->out_ports[t].out_min=act->min;
				strcpy(objects[act->to_object]->out_ports[t].out_dim,act->dimension);
				strcpy(objects[act->to_object]->out_ports[t].out_desc,act->description);
	  		    set_dimensions(next,act->max,act->min,act->dimension,act->description);
			}
		}
	
}

void update_dimensions(void)
{
	int t,i;
	struct LINKStruct * act;

	for(t=0;t<GLOBAL.objects;t++)
	  for(act=objects[t]->out;act->to_port!=-1;act++)
	  {
		  i=objects[t]->out_ports[act->from_port].get_range;
		  if (i>-1)
		  {
			  objects[t]->out_ports[act->from_port].out_max=objects[t]->in_ports[i].in_max;
			  objects[t]->out_ports[act->from_port].out_min=objects[t]->in_ports[i].in_min;
		
		  }
		  if (objects[t]->out_ports[act->from_port].get_range==-1)
			  set_dimensions(act,objects[t]->out_ports[act->from_port].out_max,objects[t]->out_ports[act->from_port].out_min,objects[t]->out_ports[act->from_port].out_dim,objects[t]->out_ports[act->from_port].out_desc);
	  }
	for(t=0;t<GLOBAL.objects;t++) objects[t]->update_inports();
}


void update_samplingrate(int newrate)
{
	int t,error=0;
	char sztemp[30],szorder[5];

	for (t=0;t<GLOBAL.objects;t++)
 	switch (objects[t]->type)
	{ 
	   case OB_FILTER:
		   {
			   FILTEROBJ * st = (FILTEROBJ *) objects[t];

			   if (st->par1>newrate/2) { error=1; break;}
 			   if ((FILTERTYPE[st->filtertype].param==2) && (st->par2>newrate/2)) { error=1; break;}
		   }
		   break;
	   case OB_MAGNITUDE:
		   {
   			    MAGNITUDEOBJ * st = (MAGNITUDEOBJ *) objects[t];
			   if ((st->wid>newrate/2) ||(st->center>newrate/2))  { error=1; break;}
		   }
		   break;
	   case OB_EEG:
		   {
			switch (TTY.devicetype) {
					case DEV_IBVA:
						if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
						{
							char str[15];
							wsprintf(str,"SR %d\r",TTY.samplingrate);
							write_string_to_comport(str);
						}
						break;
					case DEV_MONOLITHEEG_P21:
						update_p21state();
						break;
				}
		   }
		   break;
	}

	if (error) {report_error ("Cannot change Sampling Rate, please check corner frequencies of filter- or magnitude elements"); return;}

	PACKETSPERSECOND=newrate;
	for (t=0;t<GLOBAL.objects;t++)
 	switch (objects[t]->type)
	{ 
	   case OB_FILTER:
		   {
			   FILTEROBJ * st = (FILTEROBJ *) objects[t];

			   if (st->fbuf!=NULL) fid_run_freebuf(st->fbuf);
			   if (st->filt!=NULL) fid_run_free(st->filt);
 			   strcpy(sztemp,FILTERTYPE[st->filtertype].init);
			   wsprintf(szorder,"%d",st->par0);
			   strcat(sztemp,szorder);			
			   st->filt=fid_design(sztemp, PACKETSPERSECOND, st->par1, st->par2, 0, 0);
			   st->run= fid_run_new(st->filt, &(st->funcp));
			   st->fbuf=fid_run_newbuf(st->run);
		   }
		   break;
	   case OB_MAGNITUDE:
		   {
   			    MAGNITUDEOBJ * st = (MAGNITUDEOBJ *) objects[t];

		   		strcpy(sztemp,PASSTYPE[st->filtertype].init);
				sprintf(szorder,"%d",st->order);
				strcat(sztemp,szorder);

				if (st->lp1fbuf!=NULL)	 fid_run_freebuf(st->lp1fbuf);
				if (st->lp1filt!=NULL)   fid_run_free(st->lp1filt);
				if (st->lp2fbuf!=NULL)	 fid_run_freebuf(st->lp2fbuf);
				if (st->lp2filt!=NULL)   fid_run_free(st->lp2filt);
				st->lp1filt= fid_design(sztemp, PACKETSPERSECOND, (double)st->wid, 0, 0, 0);
				st->lp1run= fid_run_new(st->lp1filt, &(st->lp1funcp));
				st->lp1fbuf=fid_run_newbuf(st->lp1run);
				st->lp2filt= fid_design(sztemp, PACKETSPERSECOND,(double)st->wid, 0, 0, 0);
				st->lp2run= fid_run_new(st->lp2filt, &(st->lp2funcp));
				st->lp2fbuf=fid_run_newbuf(st->lp2run);
		   }

	}
	init_system_time();
	get_session_length();
}


int check_OS(void) 
{ 
   OSVERSIONINFOEX InfoStruct; 
   int bOSVERSIONINFOEX; 
   string temp="";
   int osid=0;

   write_logfile ("checking OS version");
   ZeroMemory(&InfoStruct,sizeof(OSVERSIONINFOEX)); 
   InfoStruct.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX); 

   if(!(bOSVERSIONINFOEX = GetVersionEx((OSVERSIONINFO*)&InfoStruct))) 
   { 
      InfoStruct.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
      if(!GetVersionEx((OSVERSIONINFO*)&InfoStruct)) 
         return -1; 
   } 

   if(InfoStruct.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
   { 
      if(InfoStruct.dwMajorVersion == 4 && InfoStruct.dwMinorVersion == 0) 
      { 
         temp += "Windows 95 "; 
         if(InfoStruct.szCSDVersion[1] == 'C' || InfoStruct.szCSDVersion[1] == 'A') 
            temp += "OSR2"; 
      } 
      else if(InfoStruct.dwMajorVersion == 4 && InfoStruct.dwMinorVersion == 10) 
      { 
         temp += "Windows 98 "; 
         if(InfoStruct.szCSDVersion[1] == 'A') 
            temp += "SE"; 
      } 
      else if(InfoStruct.dwMajorVersion == 4 && InfoStruct.dwMinorVersion == 90) 
      { 
         temp += "Windows Me"; 
      } 
   } 
   else if(InfoStruct.dwPlatformId == VER_PLATFORM_WIN32s) 
   { 
      temp += "Microsoft Win32s"; 
   } 
   else if(InfoStruct.dwPlatformId == VER_PLATFORM_WIN32_NT) 
   { 
      if(InfoStruct.dwMajorVersion == 5 && InfoStruct.dwMinorVersion == 2) 
         temp += "Windows Server 2003 family"; 
      else if(InfoStruct.dwMajorVersion == 5 && InfoStruct.dwMinorVersion == 1) 
         temp += "Windows XP "; 
      else if(InfoStruct.dwMajorVersion == 5 && InfoStruct.dwMinorVersion == 0) 
         temp += "Windows 2000 "; 
      else if(InfoStruct.dwMajorVersion <= 4) 
         temp += "Windows NT "; 
      else if(InfoStruct.dwMajorVersion == 6) 
      { 
         if( InfoStruct.dwMinorVersion == 0 )
         {
            if( InfoStruct.wProductType == VER_NT_WORKSTATION )
                temp +="Windows Vista ";
            else temp +="Windows Server 2008 ";
         }

         if ( InfoStruct.dwMinorVersion == 1 )
         {
            if( InfoStruct.wProductType == VER_NT_WORKSTATION )
			{    temp += "Windows 7 "; osid=1; }
            else temp +="Windows Server 2008 R2 ";
         }
      } 
      if(bOSVERSIONINFOEX) 
      { 
         if(InfoStruct.wProductType == VER_NT_WORKSTATION) 
         { 
            if(InfoStruct.dwMajorVersion == 4) 
               temp += "Workstation 4.0 "; 
            else if(InfoStruct.wSuiteMask & VER_SUITE_PERSONAL) 
               temp += "Home Edition "; 
            else temp += "Professional "; 
         } 
      } 
      else if(InfoStruct.wProductType == VER_NT_SERVER) 
      { 
         if(InfoStruct.dwMajorVersion == 5 && InfoStruct.dwMinorVersion == 2) 
         { 
            if(InfoStruct.wSuiteMask & VER_SUITE_DATACENTER) 
               temp += "Datacenter Edition "; 
            else if(InfoStruct.wSuiteMask & VER_SUITE_ENTERPRISE) 
               temp += "Enterprise Edition "; 
            else if(InfoStruct.wSuiteMask & VER_SUITE_BLADE) 
               temp += "Web Edition"; 
            else 
               temp += "stan**** Edition "; 
         } 
         else if(InfoStruct.dwMajorVersion == 5 && InfoStruct.dwMinorVersion == 0) 
         { 
            if(InfoStruct.wSuiteMask & VER_SUITE_DATACENTER) 
               temp += "Datacenter Server "; 
            else if(InfoStruct.wSuiteMask & VER_SUITE_ENTERPRISE) 
               temp += "Advanced Server "; 
            else 
               temp += "Server "; 
         } 
         else 
         { 
            if(InfoStruct.wSuiteMask & VER_SUITE_ENTERPRISE) 
               temp += "Server 4.0, Enterprise Edition "; 
            else 
               temp += "Server 4.0 "; 
         } 
      } 
   } 
   write_logfile ("OS version: %s",(char *)temp.data());
   return osid; 
} 

//for array_data_ports
void set_inports(BASE_CL *st, int num){
	if (st->inports<=num){
		st->inports = num;
		return;
	}

	int i,t,object_index;
	for (object_index=0;actobject!=objects[object_index];object_index++);
	for(t=0;t<GLOBAL.objects;t++)						 
		for (i=0;i<MAX_CONNECTS;i++)
		{
		while (objects[t]->out[i].to_object==object_index)
		{
			memcpy(&objects[t]->out[i],&objects[t]->out[i+1],sizeof(LINKStruct)*(MAX_CONNECTS-i));
			objects[t]->out[MAX_CONNECTS-1].to_object=-1;
			objects[t]->out[MAX_CONNECTS-1].from_port=-1;
			objects[t]->out[MAX_CONNECTS-1].to_port=-1;
		}
		if (objects[t]->out[i].to_object>object_index) 
			objects[t]->out[i].to_object--;
		if (objects[t]->out[i].from_object>object_index) 
			objects[t]->out[i].from_object--;
		}
	st->inports = num;
	get_session_length();

}

//for array_data_ports
void set_outports(BASE_CL *st, int num){
	if (st->outports<=num){
		st->outports = num;
		return;
	} 
	
	for (int i=num;i<st->outports;i++){
		for (int j=0;j<MAX_CONNECTS;j++){
			if (st->out[j].from_port==i){
				delete_connection(&st->out[j]);
			}
		}
	}
	st->outports = num;
	get_session_length();
}

//for array_data_ports
void delete_connection(LINKStruct *myactconnect){
	int o,c;
	o=myactconnect->to_object;c=-1;
	for(;myactconnect->to_port!=-1;myactconnect++)
		memcpy(myactconnect,(void *)(myactconnect+1),sizeof(struct LINKStruct));

	close_toolbox();
}
