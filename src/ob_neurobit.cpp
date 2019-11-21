/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_NEUROBIT.CPP:  contains the interface to the 
          Neurobit OPTIMA/lite devices. 
  Author: Chris Veigl

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_neurobit.h"
#include "neurobit_api\\api.h"
#include "neurobit_api\\param.h"

/* Directory of Neurobit Driver runtime files */
#define NEUROBIT_DLL "NeurobitRuntime\\NeurobitDrv.dll"
#define NEUROBIT_DIR "NeurobitRuntime\\"
#define NB_DEFCONFIGFILE "NB_OPTIMA_DefConfig.nb4"

#define MAX_SIGNALS 16
#define MAX_PACK_PHASES 100


/* Indexes of indicator colors */
enum {
	CI_RED,
	CI_GREEN,
	CI_YELLOW,
	CI_PURPLE,
	CI_GRAY,

	/* Number of color indexes. HAVE TO be at the end. */
	CI_NUM
};

float current_chn[MAX_SIGNALS]={0.0};
COLORREF COLORS[CI_NUM]=
{
	RGB(255, 0, 0),
	RGB(0,255, 0),
	RGB(255,255, 0),
	RGB(204,0, 204),
	RGB(150,150, 150)
};

COLORREF chncol[MAX_SIGNALS]={CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY};

/* Pointers to driver library functions */
TStartMeasurement NdStartMeasurement;
TStopMeasurement NdStopMeasurement;
TProtocolEngine NdProtocolEngine;

TEnumDevices NdEnumDevices;
TOpenDevContext NdOpenDevContext;
TSelectDevContext NdSelectDevContext;
TCloseDevContext NdCloseDevContext;
TGetDevConfig NdGetDevConfig;
TSetDevConfig NdSetDevConfig;
TChangeDevice NdChangeDevice;

TGetDevName NdGetDevName;
TEnumParams NdEnumParams;
TParamInfo NdParamInfo;
TGetParam NdGetParam;
TSetParam NdSetParam;
TGetOptNum NdGetOptNum;
TParam2Str NdParam2Str;
TParamList2Str NdParamList2Str;
TStr2Param NdStr2Param;
TSetDefaults NdSetDefaults;
TDevServices NdDevServices;
TCreateDevWindow NdCreateDevWindow;

/* Number of device models supported by API */
word DevNum;
static int dev_chans=0;


/* Array of names of supported device models */
const char * const *DevTab;

/* Current device context */
short DevCtx = -1;

/* Handle of device configuration window */
HWND HDevWin = NULL;

/* Driver library filename */
static char DrvLibName[256];
static char NB_DirName[256];

/* Driver library handle */
static HMODULE DrvLib = NULL;
NEUROBITOBJ * NB_OBJ=NULL;

void setDefaultNeurobitDevice(NEUROBITOBJ * st,const char * name);


/*----------------------------------------------------------------------*/


/* Display message for an user */
void NdUserMsg(word dc, const char *msg)
{
	char s[128];
	int n;

	n = sprintf(s, "(%i) ", dc);
	strncpy(s+n, msg+(*msg=='!'), sizeof(s)-n);
	s[sizeof(s)-1] = '\0';
	
	if (*msg == '!') {
		SetDlgItemText(ghWndStatusbox,IDC_STATUS, s);
		write_logfile("Neurobit message: %s",s);
	} else {
		SetDlgItemText(ghWndStatusbox,IDC_STATUS, s);
		write_logfile("Neurobit message: %s",s);
	}
}

/* Update specified indicator of measurement state at the level of user interface.
	data argument pass additional information specific to a given indicator. */
void NdUserInd(word dc, int ind, word data)
{
	NDGETVAL gv;

	const int LinkInd[ND_LINK_ERR+1] = {
	/* ND_LINK_DISC */     CI_RED,
	/* ND_LINK_CON */      CI_GREEN,
	/* ND_LINK_ERR */      CI_YELLOW
	};
	const int BatInd[ND_BAT_FRESH+1] = {
	/* ND_BAT_FLAT */      CI_RED,
	/* ND_BAT_WEAK */      CI_YELLOW,
	/* ND_BAT_OK */        CI_GREEN,
	/* ND_BAT_FRESH */     CI_GREEN
	};
	const int SigInd[ND_SIG_LOSS+1] = {
	/* ND_SIG_OK */        CI_GREEN,
	/* ND_SIG_LACK */      CI_YELLOW,
	/* ND_SIG_OVERRANGE */ CI_RED,
	/* ND_SIG_OVERLOAD */  CI_PURPLE
	};

	switch (ind) {
	case ND_IND_LINK:
//		IndBrushes[ind] = CIbrushes[data<=ND_LINK_ERR ? LinkInd[data] : CI_GRAY];
		break;
		
	case ND_IND_BAT:
//		IndBrushes[ind] = CIbrushes[data<=ND_BAT_FRESH ? BatInd[data] : CI_GRAY];
		break;
		
	case ND_IND_PAUSE:
//		IndBrushes[ind] = CIbrushes[data==ND_PAUSE_ON ? CI_YELLOW : CI_GRAY];
		break;

	case ND_IND_SIGNAL: {
			word ofs = data>>8;
			word stat = data&0x00ff;
//			IndBrushes[ind + ofs] = CIbrushes[st<=ND_SIG_LOSS ? SigInd[st] : CI_GRAY];
//			idc += ofs;
            chncol[ofs]=  stat<=ND_SIG_LOSS ? SigInd[stat] : CI_GRAY;
//            chncol[ofs]=  COLORS[stat<=ND_SIG_LOSS ? SigInd[stat] : CI_GRAY];
						}
		break;
	}

    int i, dev_chans;
	if (NdGetParam(ND_PAR_CHAN_NUM, 0, &gv) || !((gv.type&~ND_T_LIST)==ND_T_INT))
		dev_chans=0; else dev_chans = gv.val.i;
		  
	for (i=0;i<dev_chans;i++)  {
		  if (!NdGetParam(ND_PAR_CH_EN, i, &gv)) 
				 if (!gv.val.i) chncol[i+1]=CI_GRAY;
	}
						
}

/* Process received samples */
void NdProcSamples(word dc, word phase, word sum_st, const NdPackChan *chans)
{
	/* Example: Write samples to text file */

	/* For simplicity:
		* Continuous time is assumed (phase is not checked).
		* Detailed artifact info for each channel is not used here.
	*/

	/* Number of a sample value characters in output file */
	#define FLOAT_WIDTH 9
	static char s[MAX_PACK_PHASES * (MAX_SIGNALS * (FLOAT_WIDTH+1) + 2) + 1];
	/* Number of channels */
	static int i;
	/* Array of sample scaling factors */
	static float coeff[MAX_SIGNALS];
	static word last_epoch_ph;
	word endphase = 0;  /* Flag of the last phase in input sample packet */
	word snum[MAX_SIGNALS]; /* Array of indexes of current samples from individual channels */

    if (!dev_chans)   // first time: get channel number and coeff
	{
	  NDGETVAL v;
	  if (NdGetParam(ND_PAR_CHAN_NUM, 0, &v) || !((v.type&~ND_T_LIST)==ND_T_INT))
		dev_chans=0;
	  else dev_chans = v.val.i;

	  for (i=0;i<dev_chans;i++)
	  {
	    if (NdGetParam(ND_PAR_CH_RANGE_MAX, i, &v) || !((v.type&~ND_T_LIST)==ND_T_FLOAT))
  	       coeff[i]=1;
	    else
		   coeff[i] = v.val.f / 0x80000000ul;
	  }
	}

	memset(snum, 0, sizeof(snum));
	/* Loop by sampling phases */
	do {
		word i, *n;
		const NdPackChan *ch;
		
		/* Loop by channels */
		for (i=0, ch=chans, n=snum; i<dev_chans; i++, ch++, n++) {
			if (!ch->samps)
				/* Channel disabled */
				continue;
			if ((phase+1)&ch->mask) {
				/* Channel was not sampled at current phase */				
				continue;
			}
			/* There is a sample from this channel in current phase. */
			current_chn[i]=((float)ch->samps[*n] * coeff[i]);
			// write_logfile("chn %d (%.2f * %.12f) -> %.2f",i,(float)ch->samps[*n],coeff[i],current_chn[i]); 
			(*n)++;
			if (*n==ch->num)
				endphase = 1;
		}
		phase++;
		process_packets();
	} while (!endphase);

}


/* Read parameters from given file to new device context.
	The function returns device context (>=0), and on error it returns negative
	value. */
int ReadCfgFile(NEUROBITOBJ * st, const char *fname)
{
	DWORD len, n;
	char *buf;
	HANDLE cf;
	int r = 0;

	cf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (cf == INVALID_HANDLE_VALUE)
		return -1;
	if ((len=GetFileSize(cf, NULL)) == -1)
		r = -2;
	else if (!(buf=(char*)malloc(len)))
		r = -3;
	else if (!ReadFile(cf, buf, len, &n, NULL) || len!=n)
		r = -4;
	else if ((r=NdSetDevConfig(DevCtx, buf, len)) < 0)  {  // try to switch config for existing context
		if (r==-3) {  // inadequate configuration for the current model
			NdCloseDevContext(DevCtx);
			DevCtx=NdSetDevConfig(ND_NO_CONTEXT, buf, len);
		    report("Configuration inadequate to the previously selected device. The device model has been changed, please check.");
			setDefaultNeurobitDevice(st,NdGetDevName());
			r=0;
		} else r = -5;
	}
	CloseHandle(cf);
	if (buf)
		free(buf);
	return r;
}

/* Write parameters of given device context to given file.
	The function returns number of written bytes; on error (e.g. too small buffer)
	it returns 0. */
int WriteCfgFile(word dc, const char *fname)
{
	DWORD len, n = 0;
	char *buf;
	HANDLE cf;
	int r = 0;

	len = NdGetDevConfig(dc, NULL, 0);
	if (!(buf=(char*)malloc(len)))
		return 0;
	if ((len=NdGetDevConfig(dc, buf, len))) {
		cf = CreateFile(fname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		if (cf != INVALID_HANDLE_VALUE) {
			if (WriteFile(cf, buf, len, &n, NULL)) {
				if (len!=n)
					n = 0;
			}
			CloseHandle(cf);
		}
	}
	free(buf);
	return n;
}

int prepare_fileRead (NEUROBITOBJ * st) {

	st->filehandle = CreateFile(st->archivefile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (st->filehandle==INVALID_HANDLE_VALUE) {
		st->filemode=0;
		return(0);
	}

	get_session_length();
	GLOBAL.neurobit_available=0;
	st->filemode=FILE_READING;

	GLOBAL.addtime=0;
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet;
								
	if (GetFileTime(st->filehandle, &ftCreate, &ftAccess, &ftWrite))
	{ 
		FileTimeToSystemTime(&ftWrite, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		GLOBAL.addtime=stLocal.wHour*3600 + stLocal.wMinute*60 + stLocal.wSecond+ (float)stLocal.wMilliseconds/1000;
	}
	return(1);
}

int prepare_fileWrite(NEUROBITOBJ * st) {
	st->filehandle = CreateFile(st->archivefile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0,NULL);
	if (st->filehandle==INVALID_HANDLE_VALUE)
	{
		st->filemode=0;
		return(0);
	}
	st->filemode=FILE_WRITING;
	return(1);
}


/* Init Neurobit driver library use */
static HMODULE InitNeurobitDrvLib(char * drvLibName)
{
	HMODULE drv_lib;

	/* Pointers to pointers to callback functions */
	TUserMsg *pUserMsg;
	TUserInd *pUserInd;
	TProcSamples *pProcSamples;

	/* Load Neurobit driver library */
	drv_lib = LoadLibrary(drvLibName);
	if (!drv_lib) {
		char errmsg[80] = "Cannot load library ";
		report_error(strcat(errmsg, DrvLibName));
		return NULL;
	}

	if (!(NdStartMeasurement = (TStartMeasurement) GetProcAddress(drv_lib, "NdStartMeasurement")) ||
		!(NdStopMeasurement = (TStopMeasurement) GetProcAddress(drv_lib, "NdStopMeasurement")) ||
		!(NdProtocolEngine = (TProtocolEngine) GetProcAddress(drv_lib, "NdProtocolEngine")) ||

		!(NdEnumDevices = (TEnumDevices) GetProcAddress(drv_lib, "NdEnumDevices")) ||
		!(NdOpenDevContext = (TOpenDevContext) GetProcAddress(drv_lib, "NdOpenDevContext")) ||
		!(NdSelectDevContext = (TSelectDevContext) GetProcAddress(drv_lib, "NdSelectDevContext")) ||
		!(NdCloseDevContext = (TCloseDevContext) GetProcAddress(drv_lib, "NdCloseDevContext")) ||
		!(NdGetDevConfig = (TGetDevConfig) GetProcAddress(drv_lib, "NdGetDevConfig")) ||
		!(NdSetDevConfig = (TSetDevConfig) GetProcAddress(drv_lib, "NdSetDevConfig")) ||
		!(NdChangeDevice = (TChangeDevice) GetProcAddress(drv_lib, "NdChangeDevice")) ||

		!(NdGetDevName = (TGetDevName) GetProcAddress(drv_lib, "NdGetDevName")) ||
		!(NdEnumParams = (TEnumParams) GetProcAddress(drv_lib, "NdEnumParams")) ||
		!(NdParamInfo = (TParamInfo) GetProcAddress(drv_lib, "NdParamInfo")) ||
		!(NdGetParam = (TGetParam) GetProcAddress(drv_lib, "NdGetParam")) ||
		!(NdSetParam = (TSetParam) GetProcAddress(drv_lib, "NdSetParam")) ||
		!(NdGetOptNum = (TGetOptNum) GetProcAddress(drv_lib, "NdGetOptNum")) ||
		!(NdParam2Str = (TParam2Str) GetProcAddress(drv_lib, "NdParam2Str")) ||
		!(NdParamList2Str = (TParamList2Str) GetProcAddress(drv_lib, "NdParamList2Str")) ||
		!(NdStr2Param = (TStr2Param) GetProcAddress(drv_lib, "NdStr2Param")) ||
		!(NdSetDefaults = (TSetDefaults) GetProcAddress(drv_lib, "NdSetDefaults")) ||
		!(NdDevServices = (TDevServices) GetProcAddress(drv_lib, "NdDevServices")) ||
		!(NdCreateDevWindow = (TCreateDevWindow) GetProcAddress(drv_lib, "NdCreateDevWindow")) ||

		!(pUserMsg =   (TUserMsg*) GetProcAddress(drv_lib, "NdUserMsg")) ||
		!(pUserInd = (TUserInd*) GetProcAddress(drv_lib, "NdUserInd")) ||
		!(pProcSamples = (TProcSamples*) GetProcAddress(drv_lib, "NdProcSamples")))
	{
		report_error("Cannot find library function");
		return NULL;
	}
	*pUserMsg = NdUserMsg;
	*pUserInd = NdUserInd;
	*pProcSamples = NdProcSamples;

	return drv_lib;
}


void updateDialog(HWND hDlg, NEUROBITOBJ * st)
{
	switch (st->filemode)
	{
		case 0:
			// SetDlgItemText(hDlg,IDC_FILESTATUS,"no file opened");
			EnableWindow(GetDlgItem(hDlg, IDC_REC_NB_ARCHIVE), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_NB_ARCHIVE), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_NB_RECORDING), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_NB_ARCHIVE), FALSE);			
			SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,"none");
			break;
		case FILE_READING:
			EnableWindow(GetDlgItem(hDlg, IDC_REC_NB_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_NB_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_NB_RECORDING), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_NB_ARCHIVE), TRUE);			
			SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,st->archivefile);
		break;
		case FILE_WRITING:
			EnableWindow(GetDlgItem(hDlg, IDC_REC_NB_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_OPEN_NB_ARCHIVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_END_NB_RECORDING), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_NB_ARCHIVE), FALSE);			
			SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,st->archivefile);
		break;
	}
	InvalidateRect(ghWndDesign,NULL,TRUE);
}

void setDefaultNeurobitDevice(NEUROBITOBJ * st,const char * name)
{
	strcpy (st->device,name);
	strcpy(GLOBAL.neurobit_device,name);
	save_settings();
}

LRESULT CALLBACK OPTIMADlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static int init;
	NEUROBITOBJ * st;
	
	st = (NEUROBITOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_NEUROBIT)) return(FALSE);


	switch( message )
	{
		case WM_INITDIALOG:
			{
				for (int t = 0; DevTab[t] != NULL; t++) 
					SendDlgItemMessage( hDlg, IDC_NB_DEVICECOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) DevTab[t] ) ;
	  
				SetDlgItemText(hDlg,IDC_NB_DEVICECOMBO,st->device);
				st->update_channelinfo();
				updateDialog(hDlg, st);
			}
			return TRUE;
	
		case WM_CLOSE:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_NB_DEVICECOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   int sel;
					    sel=SendDlgItemMessage(hDlg, IDC_NB_DEVICECOMBO, CB_GETCURSEL, 0, 0 );
						setDefaultNeurobitDevice(st, DevTab[sel]);

						int r=NdChangeDevice(st->device);
						if (r==0) { 
							report_error("Could not apply device settings... using default settings");
						    if (DevCtx>=0) NdCloseDevContext(DevCtx);
						    DevCtx=NdOpenDevContext(st->device);
						} 
						else if (r<0) {
							report("Default settings have been applied for your Neurobit device - please check settings...");
						}
				        st->update_channelinfo();
						//InvalidateRect(hDlg,NULL,FALSE);
					}
					break;

			case IDC_DISPLAYSETTINGS:
				if (DevCtx>=0) {
					HDevWin = NdCreateDevWindow(hInst, ghWndMain, NB_DirName);
					NB_OBJ=st;
				} else write_logfile ("could not change Neurobit device settings (no device context)");
				break;
			
			case IDC_OPEN_NB_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.nba");
					
					if (open_file_dlg(ghWndMain,st->archivefile, FT_NB_ARCHIVE, OPEN_LOAD))
					{
						if (prepare_fileRead(st))
						{
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
							SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,st->archivefile);
							SendMessage(ghWndStatusbox,WM_COMMAND,IDC_RESETBUTTON,0);
						}
						else
						{
							report_error("Could not open Archive-File");
						}
						get_session_length();
					    updateDialog(hDlg, st);
					}
				break;
			case IDC_CLOSE_NB_ARCHIVE:
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,"none");
					GLOBAL.addtime=0;
				}
				st->filemode=0;
				get_session_length();
				updateDialog(hDlg, st);
				break;
			case IDC_REC_NB_ARCHIVE:
					strcpy(st->archivefile,GLOBAL.resourcepath);
					strcat(st->archivefile,"ARCHIVES\\*.nba");
					if (open_file_dlg(ghWndMain,st->archivefile, FT_NB_ARCHIVE, OPEN_SAVE))
					{
						if (!prepare_fileWrite(st)) 
							report_error("Could not open Archive-File");
					}
					updateDialog(hDlg, st);
			break;
			case IDC_END_NB_RECORDING :
				if (st->filehandle!=INVALID_HANDLE_VALUE)
				{
		 			if (!CloseHandle(st->filehandle))
						report_error("could not close Archive file");
					st->filehandle=INVALID_HANDLE_VALUE;
					SetDlgItemText(hDlg,IDC_NB_ARCHIVE_NAME,"none");
				}
				st->filemode=0;
				updateDialog(hDlg, st);
				break;
			}
			return TRUE;

		case WM_PAINT:
					color_button(GetDlgItem(hDlg,IDC_QCHN1),COLORS[chncol[1]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN2),COLORS[chncol[2]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN3),COLORS[chncol[3]]);
					color_button(GetDlgItem(hDlg,IDC_QCHN4),COLORS[chncol[4]]);
					color_button(GetDlgItem(hDlg,IDC_COMVOLT),COLORS[chncol[0]]);
			break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return TRUE;
	}
    return FALSE;
}



//
//  Object Implementation
//


NEUROBITOBJ::NEUROBITOBJ(int num) : BASE_CL()	
	  {
		outports = 4;
		inports = 0;
		width=135;

		device[0]=0;

		strcpy(out_ports[0].out_name,"chn1");
	    strcpy(out_ports[0].out_dim,"uV");
	    out_ports[0].get_range=-1;
	    strcpy(out_ports[0].out_desc,"channel1");
	    out_ports[0].out_min=-500.0f;
	    out_ports[0].out_max=500.0f;

		strcpy(out_ports[1].out_name,"chn2");
	    strcpy(out_ports[1].out_dim,"uV");
	    out_ports[1].get_range=-1;
	    strcpy(out_ports[1].out_desc,"channel2");
	    out_ports[1].out_min=-500.0f;
	    out_ports[1].out_max=500.0f;

		strcpy(out_ports[2].out_name,"chn3");
	    strcpy(out_ports[2].out_dim,"uV");
	    out_ports[2].get_range=-1;
	    strcpy(out_ports[2].out_desc,"channel3");
	    out_ports[2].out_min=-500.0f;
	    out_ports[2].out_max=500.0f;

		strcpy(out_ports[3].out_name,"chn4");
	    strcpy(out_ports[3].out_dim,"uV");
	    out_ports[3].get_range=-1;
	    strcpy(out_ports[3].out_desc,"channel4");
	    out_ports[3].out_min=-500.0f;
	    out_ports[3].out_max=500.0f;

		strcpy(out_ports[4].out_name,"chn5");
	    strcpy(out_ports[4].out_dim,"uV");
	    out_ports[4].get_range=-1;
	    strcpy(out_ports[4].out_desc,"channel5");
	    out_ports[4].out_min=-500.0f;
	    out_ports[4].out_max=500.0f;

		/* Init protocol driver library */
		strcpy(DrvLibName,GLOBAL.resourcepath);
		strcat(DrvLibName,NEUROBIT_DLL);

		strcpy(archivefile,"none");
		filehandle=INVALID_HANDLE_VALUE;

		strcpy(NB_DirName,GLOBAL.resourcepath);
		strcat(NB_DirName,NEUROBIT_DIR);

		strcpy(device,GLOBAL.neurobit_device);

		DrvLib = InitNeurobitDrvLib(DrvLibName);
		if (DrvLib)
		{
			/* Get device list */
			DevNum = NdEnumDevices(&DevTab);
			if (!DevNum) {
				report_error("List of supported devices is empty");
				FreeLibrary(DrvLib);
			}
			else DevCtx=NdOpenDevContext(device); 

		}
		else report_error("Neurobit Driver Lib error"); 
	    filehandle=INVALID_HANDLE_VALUE;
	    filemode=0;
}

	void NEUROBITOBJ::update_channelinfo(void)
	{
  	  	  NDGETVAL gv;
		  int chans,i;
		  float max_sr;

		  if (DevCtx<0) { 
			  write_logfile ("could not open Neurobit device context");
			  return;
		  }
  		  if (NdGetParam(ND_PAR_CHAN_NUM, 0, &gv) || (gv.type&~ND_T_LIST)!=ND_T_INT) 
			  return;

		  max_sr=0;
		  chans = gv.val.i;
		  outports = chans;
		  height=CON_START+outports*CON_HEIGHT+5;

		  strcpy(out_ports[0].out_dim,"uV");
		  chncol[0]=CI_GREEN;
		  for (i=1; i<5; i++) chncol[i]=CI_GRAY;
	  	  for (i=0; i<chans; i++) {
			/* Update channel indicator field info */
			if (!NdGetParam(ND_PAR_CH_LABEL, i, &gv)) {
				 strcpy( out_ports[i].out_name,gv.val.t);
			}
			if (!NdGetParam(ND_PAR_CH_RANGE_MAX, i, &gv)) {
				out_ports[i].out_max = gv.val.f;
			}
			if (!NdGetParam(ND_PAR_CH_RANGE_MIN, i, &gv)) {
				out_ports[i].out_min = gv.val.f;
			}

			strcpy(out_ports[i].out_dim,NdParamInfo(ND_PAR_CH_RANGE_MAX, i)->unit);

/*
			if (!NdGetParam(ND_PAR_CH_FUNC, i, &gv)) {
				if (!strcmp(gv.val.t,"Voltage")) strcpy(out_ports[i].out_dim,"uV");
				if (!strcmp(gv.val.t,"Temperature")) strcpy(out_ports[i].out_dim,"Deg");
				if (!strcmp(gv.val.t,"Conductance")) strcpy(out_ports[i].out_dim,"uS");
				if (!strcmp(gv.val.t,"Resistance")) strcpy(out_ports[i].out_dim,"kOhm");

			}*/
			if (!NdGetParam(ND_PAR_CH_EN, i, &gv)) {
				if (gv.val.b) {
					chncol[i+1]= CI_GREEN;
					if (!NdGetParam(ND_PAR_CH_SR, i, &gv)) {
						if (gv.val.f>max_sr) max_sr=gv.val.f;
					}
				} else chncol[i+1]=CI_GRAY;
			}
		  }
         if (max_sr>0) update_samplingrate((int)(max_sr+0.5f));

		 dev_chans=0;
		 if (!GLOBAL.loading) update_dimensions();
 	 	 reset_oscilloscopes();

		 InvalidateRect(ghWndMain,NULL,TRUE);
		 InvalidateRect(ghWndDesign,NULL,TRUE);
		 if (ghWndToolbox == hDlg) InvalidateRect(ghWndToolbox,NULL,FALSE);
	}

	void NEUROBITOBJ::load_device_config(void)
	{
		char szFileName[256];
		char *extension;

		strcpy(szFileName,GLOBAL.configfile);
		if (extension = strstr(szFileName, ".con"))
			strcpy(extension,".nb");
		else strcat(szFileName,".nb");

		write_logfile("loading Neurobit device configuration from %s",szFileName);

		GLOBAL.neurobit_available=0;
		//
		//DevCtx=
		int r=ReadCfgFile(this, szFileName);
		if (r<0) {
	        report_error ("Could not load Neurobit Config File - using default configuration");
			if (DevCtx>=0) NdCloseDevContext(DevCtx);
			DevCtx=NdOpenDevContext(device);
		}
		strcpy(device,NdGetDevName());
		GLOBAL.neurobit_available=1;

		InvalidateRect(hDlg,NULL,FALSE);
		if (!GLOBAL.loading) update_channelinfo();
		SetDlgItemText(hDlg,IDC_NB_DEVICECOMBO,device);
	}

	void NEUROBITOBJ::save_device_config(void)
	{
		char szFileName[256];
		char *extension;

		strcpy(szFileName,GLOBAL.configfile);
		if (extension = strstr(szFileName, ".con"))
			strcpy(extension,".nb");
		else strcat(szFileName,".nb");

		if (DevCtx>=0)
		{
	  		 write_logfile("saving Neurobit device context to %s",szFileName);
			 if (!WriteCfgFile(DevCtx,szFileName))
				report_error("Could not save Neurobit Config File");
		} else write_logfile("ERROR: No Neurobit Device Context to save ");
	}


	  void NEUROBITOBJ::make_dialog(void)
	  {
		  	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_NEUROBITBOX, ghWndStatusbox, (DLGPROC)OPTIMADlgHandler));
	  }
	  void NEUROBITOBJ::load(HANDLE hFile) 
	  {
  		  load_object_basics(this);
		  load_property("device",P_STRING,device);
		  load_device_config();

		  update_channelinfo();
 		  GLOBAL.neurobit_available=1;

  		  load_property("archivefile",P_STRING,archivefile);
		  load_property("filemode",P_INT,&filemode);
		  if (filemode == FILE_READING) {
			prepare_fileRead (this);
		  } else if (filemode == FILE_WRITING) {
			prepare_fileWrite (this);
		  }
		  if (hDlg==ghWndToolbox) updateDialog(hDlg, this);
	  }
		
	  void NEUROBITOBJ::save(HANDLE hFile) 
	  {	   
		  save_object_basics(hFile, this);
		  save_property(hFile,"test",P_INT,&test);
		  save_property(hFile,"device",P_STRING,device);
		  save_property(hFile,"archivefile",P_STRING,archivefile);
		  save_property(hFile,"filemode",P_INT,&filemode);
		  save_device_config();
	  }

	  void NEUROBITOBJ::session_reset(void) 
	  {
	  }

	  void NEUROBITOBJ::session_start(void)
	  {
		  short r;

  		  if ((filehandle==INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
		  {  
			  if (DevCtx>=0)
			  {
				update_channelinfo();
 				GLOBAL.neurobit_available=1;
				r = NdStartMeasurement(DevCtx, ND_MEASURE_NORMAL);
				if(r<0) report_error ("Invalid device, its state or measurement mode");
				else if(r>0) { report_error("Cannot connect with the Device"); 
				      SendMessage(ghWndStatusbox,WM_COMMAND, IDC_STOPSESSION,0);}
			  }
		  } 
		  else {GLOBAL.neurobit_available=0;  }
	  }
	  void NEUROBITOBJ::session_stop(void)
	  {
			if (DevCtx>=0) 
			{
				NdStopMeasurement(DevCtx);
				for (int t =1; t<1000;t++)
				{
					NdProtocolEngine();
					Sleep(1);
				}
			//	NdCloseDevContext(DevCtx);
			}
			//DevCtx=-1;
	  }

  	  void NEUROBITOBJ::session_pos(long pos) 
	  {	
			if(filehandle==INVALID_HANDLE_VALUE) return;
			if (pos>filelength) pos=filelength;
			SetFilePointer(filehandle,pos*(sizeof(float))*4,NULL,FILE_BEGIN);
	  } 

	  long NEUROBITOBJ::session_length(void) 
	  {
			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode==FILE_READING))
			{
				DWORD sav= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
				filelength= SetFilePointer(filehandle,0,NULL,FILE_END)/(sizeof(float))/4;
				SetFilePointer(filehandle,sav,NULL,FILE_BEGIN);
				return(filelength);
			}
		    return(0);
	  }

	  void NEUROBITOBJ::work(void) 
	  {
			DWORD dwWritten,dwRead;

			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
			{
				ReadFile(filehandle,current_chn,sizeof(float)*outports, &dwRead, NULL);
				if (dwRead != sizeof(float)*outports) SendMessage (ghWndStatusbox,WM_COMMAND,IDC_STOPSESSION,0);
				else 
				{
					DWORD x= SetFilePointer(filehandle,0,NULL,FILE_CURRENT);
					x=x*1000/filelength/TTY.bytes_per_packet;
					SetScrollPos(GetDlgItem(ghWndStatusbox, IDC_SESSIONPOS), SB_CTL, x, 1);
				}
			}
			
			for (int i=0; i<outports; i++) {
				pass_values(i, current_chn[i]);
			}

			if ((filehandle!=INVALID_HANDLE_VALUE) && (filemode == FILE_WRITING)) 
					WriteFile(filehandle,current_chn,sizeof(float)*outports, &dwWritten, NULL);

			if ((!TIMING.dialog_update) && (hDlg==ghWndToolbox)) 
			{
				InvalidateRect(hDlg,NULL,FALSE);
			}

	  }

NEUROBITOBJ::~NEUROBITOBJ()
	  {
			GLOBAL.neurobit_available=0;
			if (DevCtx>=0) NdCloseDevContext(DevCtx);

		// free object
	  }  
