/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_EMOTIV.CPP:  contains the interface to the 
          Emotiv Epoc neuroheadset. 
  Author: Dominik Koller

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainbay.h"
#include "ob_emotiv.h"
#include <vector>

#define MAX_CHANNELS 14
#define BUFFER_SECS 0.1	//size of Emotiv data buffer in seconds

EE_DataChannel_t targetChannelList[] = {
		ED_COUNTER,
		ED_AF3, ED_F7, ED_F3, ED_FC5, ED_T7, 
		ED_P7, ED_O1, ED_O2, ED_P8, ED_T8, 
		ED_FC6, ED_F4, ED_F8, ED_AF4, ED_GYROX, ED_GYROY, ED_TIMESTAMP, 
		ED_FUNC_ID, ED_FUNC_VALUE, ED_MARKER, ED_SYNC_SIGNAL
	};


	HMODULE drv_lib=NULL;
	TES_GetContactQualityFromAllChannels ES_GetContactQualityFromAllChannels;
	TES_GetWirelessSignalStatus ES_GetWirelessSignalStatus;
	TES_GetBatteryChargeLevel ES_GetBatteryChargeLevel;

	TEE_DataGet EE_DataGet;
	TEE_EngineConnect EE_EngineConnect;
	TEE_EngineDisconnect EE_EngineDisconnect;
	TEE_EmoEngineEventCreate EE_EmoEngineEventCreate;
	TEE_EmoEngineEventFree EE_EmoEngineEventFree;
	TEE_EmoStateCreate EE_EmoStateCreate;
	TEE_EmoStateFree EE_EmoStateFree;
	TEE_EmoEngineEventGetType EE_EmoEngineEventGetType;
	TEE_EmoEngineEventGetUserId EE_EmoEngineEventGetUserId;
	TEE_EngineGetNextEvent EE_EngineGetNextEvent;
	TEE_DataCreate EE_DataCreate;
	TEE_DataFree EE_DataFree;
	TEE_DataUpdateHandle EE_DataUpdateHandle;
	TEE_DataGetNumberOfSample EE_DataGetNumberOfSample;
	TEE_DataSetBufferSizeInSec EE_DataSetBufferSizeInSec;
	TEE_DataAcquisitionEnable EE_DataAcquisitionEnable; 




/* Indexes of indicator colors */
enum {
	CI_RED,		//very bad signal
	CI_GREEN,	//good signal
	CI_YELLOW,	//poor signal
	CI_PURPLE,	//fair signal
	CI_GRAY,	//no signal

	/* Number of color indexes. HAVE TO be at the end. */
	CI_NUM
};


COLORREF COLORS_Q[CI_NUM]=
{
	RGB(255, 0, 0),
	RGB(0,255, 0),
	RGB(255,255, 0),
	RGB(204,0, 204),
	RGB(150,150, 150)
};


EMOTIVOBJ * gEMOTIV = NULL;

COLORREF chncolor[MAX_CHANNELS]={CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY, CI_GRAY, CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY,CI_GRAY, CI_GRAY, CI_GRAY, CI_GRAY};
COLORREF wirelesscol = CI_GRAY;
int BatteryLevel = 0;

EmoEngineEventHandle eEvent;
EmoStateHandle eState;
DataHandle hData;
bool readytocollect = false;
unsigned int userID = 100, userID_selected;
unsigned int nSamplesTaken;
std::vector<float> buffer[14];

/*Driver library file name*/
static char DrvLibNameEDK[256];
static char DrvLibNameUtils[256];

LRESULT CALLBACK EMOTIVDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	EMOTIVOBJ * st;
	char userBuffer[18];
	int t;
	
	st = (EMOTIVOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_EMOTIV)) return(FALSE);

	switch (message)
	{
		case WM_INITDIALOG:
			{
				for (t = 0; t < 18; t++)
				{
					wsprintf( userBuffer , "User %d" , t + 1 );
					SendDlgItemMessage(hDlg, IDC_USERID_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) userBuffer);
				}
			}
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_USERID_COMBO:
				{
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
						userID_selected = SendDlgItemMessage(hDlg, IDC_USERID_COMBO, CB_GETCURSEL, 0, 0 );
					}
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		case WM_PAINT:
			color_button(GetDlgItem(hDlg,IDC_CQ_AF3),COLORS_Q[chncolor[0]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_F7),COLORS_Q[chncolor[1]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_F3),COLORS_Q[chncolor[2]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_FC5),COLORS_Q[chncolor[3]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_T7),COLORS_Q[chncolor[4]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_P7),COLORS_Q[chncolor[5]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_O1),COLORS_Q[chncolor[6]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_O2),COLORS_Q[chncolor[7]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_P8),COLORS_Q[chncolor[8]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_T8),COLORS_Q[chncolor[9]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_FC6),COLORS_Q[chncolor[10]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_F4),COLORS_Q[chncolor[11]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_F8),COLORS_Q[chncolor[12]]);
			color_button(GetDlgItem(hDlg,IDC_CQ_AF4),COLORS_Q[chncolor[13]]);

			color_button(GetDlgItem(hDlg,IDC_WIRELESS_QUALITY),COLORS_Q[wirelesscol]);

			//SetDlgItemInt(hDlg, IDC_BATTERY_CHARGE_LEVEL, BatteryLevel, FALSE);
			break;
		case WM_SIZE:
			break;
		case WM_MOVE:
			update_toolbox_position(hDlg);
			break;
			return TRUE;
	}

	return FALSE;
}


static HMODULE InitEmotivEDKLib(const char * drvLibName)
{
	HMODULE drv_lib;

	/* Load Emotiv driver library */
	char actfile[256];

	strcpy(actfile,drvLibName);
	strcat(actfile,"\\edk_utils.dll");

    FILE * testfile = fopen (actfile, "r");
	if (testfile != NULL)
	{ 
		    std::cout << "Emotiv .dll found at path:"  << actfile << std::endl;
			write_logfile ("Emotiv ed.dll file found");
			fclose (testfile);

			drv_lib = LoadLibrary(actfile);
			if (!drv_lib) { 
			    std::cout << "Error loading EDK_UTILS.DLL\n";
				write_logfile ("Error loading EDK_UTILS.DLL");
				return NULL;
			}
		    std::cout << "LOADING EDK_UTILS.DLL succeeded.\n";
			write_logfile ("LOADING EDK_UTILS.DLL succeeded.");
	}
	else {
		    std::cout << "Error: Emotiv .dll not found at path:"  << actfile << std::endl;
			write_logfile ("Error: Emotiv .dll file not found");
			return NULL;
	}


	strcpy(actfile,drvLibName);
	strcat(actfile,"\\edk.dll");
	drv_lib = LoadLibrary(actfile);

	if (!drv_lib) { 
		std::cout << "Error loading edk.DLL from path:" << actfile << "\n";
		write_logfile ("Error loading EDK_UTILS.DLL");
		return NULL;
	}
	std::cout << "loading " << actfile <<" succeeded.\n";
	write_logfile ("LOADING EDK_UTILS.DLL succeeded.");

	if (!(EE_DataGet = (TEE_DataGet) GetProcAddress(drv_lib, "EE_DataGet"))  ||
		!(EE_EngineConnect = (TEE_EngineConnect) GetProcAddress(drv_lib, "EE_EngineConnect")) ||
		!(EE_EngineDisconnect = (TEE_EngineDisconnect) GetProcAddress(drv_lib, "EE_EngineDisconnect")) ||
		!(EE_EmoEngineEventCreate = (TEE_EmoEngineEventCreate) GetProcAddress(drv_lib, "EE_EmoEngineEventCreate")) ||
		!(EE_EmoEngineEventFree = (TEE_EmoEngineEventFree) GetProcAddress(drv_lib, "EE_EmoEngineEventFree")) ||
		!(EE_EmoStateCreate = (TEE_EmoStateCreate) GetProcAddress(drv_lib, "EE_EmoStateCreate")) ||
		!(EE_EmoStateFree = (TEE_EmoStateFree) GetProcAddress(drv_lib, "EE_EmoStateFree")) ||
		!(EE_EmoEngineEventGetType = (TEE_EmoEngineEventGetType) GetProcAddress(drv_lib, "EE_EmoEngineEventGetType")) ||
		!(EE_EmoEngineEventGetUserId = (TEE_EmoEngineEventGetUserId) GetProcAddress(drv_lib, "EE_EmoEngineEventGetUserId")) ||
		!(EE_EngineGetNextEvent = (TEE_EngineGetNextEvent) GetProcAddress(drv_lib, "EE_EngineGetNextEvent")) ||
		!(EE_DataCreate = (TEE_DataCreate) GetProcAddress(drv_lib, "EE_DataCreate")) ||
		!(EE_DataFree = (TEE_DataFree) GetProcAddress(drv_lib, "EE_DataFree")) ||
		!(EE_DataUpdateHandle = (TEE_DataUpdateHandle) GetProcAddress(drv_lib, "EE_DataUpdateHandle")) ||
		!(EE_DataGetNumberOfSample = (TEE_DataGetNumberOfSample) GetProcAddress(drv_lib, "EE_DataGetNumberOfSample")) ||
		!(EE_DataSetBufferSizeInSec = (TEE_DataSetBufferSizeInSec) GetProcAddress(drv_lib, "EE_DataSetBufferSizeInSec")) ||
		!(ES_GetContactQualityFromAllChannels = (TES_GetContactQualityFromAllChannels) GetProcAddress(drv_lib, "ES_GetContactQualityFromAllChannels")) ||
		!(ES_GetWirelessSignalStatus = (TES_GetWirelessSignalStatus) GetProcAddress(drv_lib, "ES_GetWirelessSignalStatus")) ||
		!(ES_GetBatteryChargeLevel = (TES_GetBatteryChargeLevel) GetProcAddress(drv_lib, "ES_GetBatteryChargeLevel")) ||
		!(EE_DataAcquisitionEnable = (TEE_DataAcquisitionEnable) GetProcAddress(drv_lib, "EE_DataAcquisitionEnable"))) 
		{
			std::cout << "Error getting Function entry points from EDK.dll !\n";
			write_logfile("Error getting Function entry points from EDK.dll");
			return NULL;
		}
		std::cout << "INFO: Successfully loaded Emotiv EDK dll from " << drvLibName << "\n";
		write_logfile("INFO: Successfully loaded Emotiv EDK dll from %s",drvLibName);
		return drv_lib;
}




EMOTIVOBJ::EMOTIVOBJ(int num) : BASE_CL()	//constructor
{
	inports = 0;
	outports = MAX_CHANNELS;	//Emotiv Epoc Channels

	/*outports are entiteled according to the international 10-20 location system*/
	strcpy(out_ports[0].out_name, "AF3");
	strcpy(out_ports[1].out_name, "F7");
	strcpy(out_ports[2].out_name, "F3");
	strcpy(out_ports[3].out_name, "FC5");
	strcpy(out_ports[4].out_name, "T7");
	strcpy(out_ports[5].out_name, "P7");
	strcpy(out_ports[6].out_name, "O1");
	strcpy(out_ports[7].out_name, "O2");
	strcpy(out_ports[8].out_name, "P8");
	strcpy(out_ports[9].out_name, "T8");
	strcpy(out_ports[10].out_name, "FC6");
	strcpy(out_ports[11].out_name, "F4");
	strcpy(out_ports[12].out_name, "F8");
	strcpy(out_ports[13].out_name, "AF4");

	/* Init protocol driver library */
	if ((drv_lib=InitEmotivEDKLib(GLOBAL.emotivpath))==NULL)
	{
		std::cout << "trying to load Emotiv EDK dlls from " << GLOBAL.emotivpath << "\n";
		write_logfile("trying to load Emotiv EDK dlls from %s", GLOBAL.emotivpath);
		report_error("could not load Emotiv EDK dlls\n Emotiv EPOC cannot be used. Please check Path to Emotiv SDK in Application Settings...");
	}

	filehandle = INVALID_HANDLE_VALUE;
	filemode = 0;
	gEMOTIV=this;
}

void EMOTIVOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_EMOTIVBOX, ghWndStatusbox, (DLGPROC)EMOTIVDlgHandler));
}

void EMOTIVOBJ::load(HANDLE hFile)
{
	load_object_basics(this);
}

void EMOTIVOBJ::updateHeadsetStatus(void)
{
	int chargeLevel, maxChargeLevel;
	EE_EEG_ContactQuality_t SignalQuality[MAX_CHANNELS+3];

	std::cout << "UpdateHeadsetStatus started.\n";
	if (drv_lib==NULL) return;
	///*BATTERY CHARGE LEVEL*/

	std::cout << "calling GetBatteryChargeLevel\n";
	ES_GetBatteryChargeLevel(eState, &chargeLevel, &maxChargeLevel);	//references current charge level and maximum charge level
	BatteryLevel = (chargeLevel/maxChargeLevel)*100;

	///*WIRELESS SIGNAL QUALITY*/

	const int WirelessInd[GOOD_SIGNAL+1] = {
		/*NO_SIGNAL*/			CI_GRAY,
		/*BAD_SIGNAL*/			CI_RED,
		/*GOOD_SIGNAL*/			CI_GREEN
	};

	std::cout << "calling GetWirelessSignalStatus\n";
	wirelesscol = WirelessInd[ES_GetWirelessSignalStatus(eState)];

	///*ELECTRODE SIGNAL QUALITY*/

	const int SigInd[EEG_CQ_GOOD+1] = {
		/*EEG_CQ_NO_SIGNAL*/	CI_GRAY,
		/*EEG_CQ_VERY_BAD*/		CI_RED,
		/*EEG_CQ_POOR*/			CI_PURPLE,
		/*EEG_CQ_FAIR*/			CI_YELLOW,
		/*EEG_CQ_GOOD*/			CI_GREEN,
	};

	std::cout << "calling GetContactQualityFromAllChannels\n";
	int numofCQ = ES_GetContactQualityFromAllChannels(eState, SignalQuality, MAX_CHANNELS+3);
	for(int i=0; i < MAX_CHANNELS; i++)
	{
		//SetDlgItemInt(ghWndStatusbox,IDC_STATUS, SignalQuality[i+3],0);
		std::cout << "Signal Quality Indicator for channel " << i << " is " << SignalQuality[i+3] << "\n";
		chncolor[i] = SigInd[SignalQuality[i+3]];		
			//The ordering of the array is consistent with the ordering of the logical input channels in EE_InputChannels_enum.
			//is only returning the active electrodes. NOT DRL or CMS
	}
}


void process_emotiv(void)	//calls process_packets() function
{
	if (drv_lib==NULL) return;
	if(EE_EngineGetNextEvent(eEvent) == EDK_OK)
	{
		EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent);

		if (eventType == EE_UserAdded) 
		{
			std::cout << "User added event detected\n";

			EE_EmoEngineEventGetUserId(eEvent, &userID);
			// if (userID == userID_selected)
			//{
				std::cout << "User ID is: " << userID <<"\n";

				EE_DataAcquisitionEnable(userID,true);
				readytocollect = true;
			//}
			// else std::cout << "User ID does not match - acquisition not enabled\n";
		}
	}

	if(readytocollect)  // && (userID == userID_selected))
	{
		std::cout << "-";
		EE_DataUpdateHandle(userID, hData);
		EE_DataGetNumberOfSample(hData, &nSamplesTaken);
	
		if(nSamplesTaken != 0)
		{
			for (int i = 0; i < MAX_CHANNELS; i++)
				buffer[i].clear();

			double* data = new double[nSamplesTaken];

			for (int channel = 0 ; channel < MAX_CHANNELS; channel++)
			{
				EE_DataGet(hData, targetChannelList[channel+1], data, nSamplesTaken);
				// std::cout << " DataGet for channel " << channel << " returned :";

				for (unsigned int j = 0; j < nSamplesTaken; j++)
				{
					buffer[channel].push_back((const float)data[j]);
					//std::cout << data[j] << ", ";
				}
 				//std::cout << "\n";
			}
			delete[] data;

			for (unsigned int j = 0; j < nSamplesTaken; j++)
			{
				for (int channel = 0 ; channel < MAX_CHANNELS; channel++) {
					// std::cout << "now sending sample " << j+1 << " to output port " << channel  <<": " << buffer[channel].at(j) << "\n";
					gEMOTIV->pass_values(channel, buffer[channel].at(j));
				}
				process_packets();
				std::cout << "*";
			}
			std::cout << "\n";
		}
	}
}


void EMOTIVOBJ::save(HANDLE hFile)
{
	save_object_basics(hFile, this);
}

void EMOTIVOBJ::session_start(void)
{
	if (drv_lib==NULL) return;
	if((filehandle == INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
	{
		std::cout << "calling EmoEngineEventCreate\n";
		eEvent = EE_EmoEngineEventCreate();
		std::cout << "calling EmoStateCreate\n";
		eState = EE_EmoStateCreate();

		std::cout << "calling EngineConnect\n";
		if (EE_EngineConnect("Emotiv Systems-5") == EDK_OK)
		{
			std::cout << "calling DataCreate\n";
			hData = EE_DataCreate();
			std::cout << "calling DataSetBufferSize\n";
			EE_DataSetBufferSizeInSec((float)BUFFER_SECS);
			std::cout << "Connected - Emotiv Interface available.\n";
			GLOBAL.emotiv_available = 1;
		}
		else
		{
			GLOBAL.emotiv_available = 0;
			report_error("Emotiv Engine start up failed");
			SendMessage(ghWndStatusbox,WM_COMMAND, IDC_STOPSESSION,0);
		}
	}
}


void EMOTIVOBJ::work(void)
{
	static int updateCounter=0;
	if (drv_lib==NULL) return;

	if (!((updateCounter++)%200)) updateHeadsetStatus();
}


void EMOTIVOBJ::session_stop(void)
{
	if (drv_lib==NULL) return;
	if(GLOBAL.emotiv_available == 1)
	{

		std::cout << "Free Emotiv Resources\n";
		EE_DataFree(hData);
		EE_EngineDisconnect();
		EE_EmoStateFree(eState);
		EE_EmoEngineEventFree(eEvent);
		GLOBAL.emotiv_available = 0;
	}
}

void EMOTIVOBJ::session_reset(void)
{
	// GLOBAL.emotiv_available = 0;
}


EMOTIVOBJ::~EMOTIVOBJ() 
{
	GLOBAL.emotiv_available = 0;
}	//deconstructor