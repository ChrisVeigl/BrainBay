/* -----------------------------------------------------------------------------

	BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_BIOSEMI.CPP:  contains the interface to
		  BIOSEMI Active Two devices (Mk1 and Mk2)
  Author: Denny Yi-Jhong Han (the codes are extensively referenced and copied from
		  Lab Streaming Layer (LSL) projects, https://github.com/sccn/labstreaminglayer
		  credits also goes to them!)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_biosemi.h" 

#define BIOSEMI_DLL "BioSemi\\DLL\\Win32\\Labview_DLL.dll"

#include <iostream>
#include <time.h>

// amplifier parameters
bool is_mk2_;       // whether the amp is a MK2 amplifier
int speed_mode_;    // amplifier speed mode
int srate_;         // native sampling rate
int nbsync_;        // number of synchronization channels
int nbtrig_;        // number of trigger channels
int nbeeg_;         // number of EEG channels
int nbexg_;         // number of ExG channels
int nbaux_;			// number of AUX channels
int nbaib_;			// number of AIB channels
int nbchan_;        // total number of channels
bool battery_low_;  // whether the battery is low


// maximum waiting time when trying to connect
const float max_waiting_time = 3.0;
// size of the initial probing buffer
const int buffer_bytes = 8 * 1024 * 1024;
// preferred size of final buffer (note: must be a multiple of 512; also, ca. 32MB is a good size according to BioSemi)
const int buffer_samples = 60 * 512;

// 64 zero bytes
const unsigned char msg_enable[64] = { 0 };
// 0xFF followed by 63 zero bytes
const unsigned char msg_handshake[64] = { 255,0 };

// ring buffer pointer (from the driver)
uint32_t* ring_buffer_;
int last_idx_;

// send a chunk every approx. this many ms
// to get a uniform lag distribution this should not be a divisor of 64 (= the default stride of the BioSemi driver)
const int send_interval_ms = 13;
//const int send_interval_ms = 100;

// the age of each chunk received (in seconds) is anywhere between 0 and send_interval_ms milliseconds old, and is on average half of the maximum
const double buffer_lag = (send_interval_ms / 2000.0);

// allocate temp data & resamplers...
BIOSEMIOBJ::chunk_t raw_chunk;
std::vector<std::vector<double>> scaled_chunk_tr;

// the biosemi backend
std::shared_ptr<BIOSEMIOBJ> biosemi_;

// misc meta-data from the config
std::string location_system_;
std::string location_manufacturer_;
std::vector<std::string> channels_;
std::vector<std::string> types_;
std::vector<int> index_set_;

// connection handle
void* hConn_;
// function pointers
OPEN_DRIVER_ASYNC_t OPEN_DRIVER_ASYNC;
USB_WRITE_t USB_WRITE;
READ_MULTIPLE_SWEEPS_t READ_MULTIPLE_SWEEPS;
READ_POINTER_t READ_POINTER;
CLOSE_DRIVER_ASYNC_t CLOSE_DRIVER_ASYNC;

/* Driver library filename */
static char DrvLibName[256];
static char NB_DirName[256];

/* Driver library handle */
static HMODULE DrvLib = NULL;
BIOSEMIOBJ* NB_OBJ = NULL;

uint32_t start_idx; // index of the first sample that was recorded
uint32_t cur_idx;

int DevActive; // toggle device status
int started; // toggle session start/stop status

//gui flow control
int first_update = 1;
int chanset_changed = 1;
int output_changed = 1;
int loaded = 0;

// eeg chanset
std::string chanset_mat [] = {
	"all", "160", "128", "64", "32", "all, no AUX", "160, no AUX", "128, no AUX", "64, no AUX", "32, no AUX"};

std::string thirtytwo_memory[] = {
	"Fz", "Cz", "Pz", "Oz", "Fp1", "F7", "F5", "F3", "F1", "Fp2", "F8", "F6", "F4", "F2", "T7", "C5",
	"C3", "C1", "T8", "C6", "C4", "C2", "P7", "P5", "P3", "P1", "P8", "P6", "P4", "P2", "O1", "O2" };

std::string thirtytwo[] = {
	"Fp1", "AF3", "F7", "F3", "FC1", "FC5", "T7", "C3", "CP1", "CP5", "P7", "P3", "Pz", "PO3", "O1", "Oz", 
	"O2", "PO4", "P4", "P8", "CP6", "CP2", "C4", "T8", "FC6", "FC2", "F4", "F8", "AF4", "Fp2", "Fz","Cz"};

std::string sixtyfour[] = {
	"Fp1", "AF7", "AF3", "F1", "F3", "F5", "F7", "FT7", "FC5", "FC3", "FC1", "C1", "C3", "C5", "T7", "TP7",  
	"CP5", "CP3", "CP1", "P1", "P3", "P5", "P7", "P9", "PO7", "PO3", "O1", "Iz", "Oz", "POz", "Pz", "CPz", 
	"Fpz", "Fp2", "AF8", "AF4", "Afz", "Fz", "F2", "F4", "F6", "F8", "FT8", "FC6", "FC4", "FC2", "FCz", "Cz", 
	"C2", "C4", "C6", "T8", "TP8", "CP6", "CP4", "CP2", "P2", "P4", "P6", "P8", "P10", "PO8", "PO4", "O2"};

BIOSEMIOBJ* gBIOSEMI = NULL;

/* Init BioSemi driver library use */
static HMODULE InitBioSemiDrvLib(char* drvLibName)
{
	HMODULE drv_lib;

	drv_lib = LoadLibrary(BIOSEMI_DLL);

	// check if DLL loaded successfully
	if (!drv_lib) {
		char errmsg[80] = "Cannot load BioSemi DLL";
		report_error(strcat(errmsg, drvLibName));
		return NULL;
	}
	DevActive = 1;

	if (!(OPEN_DRIVER_ASYNC = (OPEN_DRIVER_ASYNC_t)GetProcAddress(drv_lib, "OPEN_DRIVER_ASYNC")) ||
		!(USB_WRITE = (USB_WRITE_t)GetProcAddress(drv_lib, "USB_WRITE")) ||
		!(READ_MULTIPLE_SWEEPS = (READ_MULTIPLE_SWEEPS_t)GetProcAddress(drv_lib, "READ_MULTIPLE_SWEEPS")) ||
		!(READ_POINTER = (READ_POINTER_t)GetProcAddress(drv_lib, "READ_POINTER")) ||
		!(CLOSE_DRIVER_ASYNC = (CLOSE_DRIVER_ASYNC_t)GetProcAddress(drv_lib, "CLOSE_DRIVER_ASYNC")))
	{
		report_error("At least one of the library functions cannot be found");
		DevActive = 0;
		return NULL;
	}

	return drv_lib;
}


BIOSEMIOBJ::BIOSEMIOBJ(int num) : BASE_CL()
{
	outports = 16;
	inports = 0;
	width = 75;

	// default channel subset entry
	chansetn = 9;
	chansetn_tmp = chansetn;
	// default labelling system
	labeln = 0;
	labeln_tmp = labeln;
	// memory montage?
	memory_montage = 1;
	memory_montage_tmp = memory_montage;
	
	//gui flow control
	first_update = 1;
	chanset_changed = 1;
	output_changed = 1;

	// default output channel 1-16 entry
	for (int c = 0; c < 16; c++) {
		opn.push_back(0);
	}

	/* Init protocol driver library */
	strcat(DrvLibName, BIOSEMI_DLL);

	strcpy(archivefile, "none");
	filehandle = INVALID_HANDLE_VALUE;

	strcpy(NB_DirName, GLOBAL.resourcepath);

	std::cout << "Loading BioSemi driver dll..." << std::endl;
	DrvLib = InitBioSemiDrvLib(DrvLibName);

	filehandle = INVALID_HANDLE_VALUE;
	filemode = 0;	
	gBIOSEMI = this;
}

void BIOSEMIOBJ::make_dialog(void)  // will be called when element is right-clicked
{
	display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_BIOSEMIBOX, ghWndStatusbox, (DLGPROC)BioSemiDlgHandler));
}

LRESULT CALLBACK BioSemiDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BIOSEMIOBJ* st;

	st = (BIOSEMIOBJ*)actobject;
	if ((st == NULL) || (st->type != OB_BIOSEMI)) return(FALSE);

	switch (message)
	{
		case WM_INITDIALOG:			// the user dialog is to be created
			// IDC_CHANSET: eeg channel set
			for (int k = 0; k < 10; k++) {
				SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) chanset_mat[k].c_str());}
			SendDlgItemMessage(hDlg, IDC_CHANSET, CB_SETCURSEL, st->chansetn, 0L);
			//st->chansetn = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
			SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETLBTEXT, st->chansetn, (LPARAM)st->chanset_string);
				
			// IDC_BIOSEMI_TenTwenty, IDC_BIOSEMI_ABC, radio buttons toggle between labelling systems	
			// check if the selected channel subset doesn't have 10-20 labelling
			switch (st->labeln)	{
				case 0: CheckDlgButton(hDlg, IDC_BIOSEMI_TenTwenty, TRUE); break;
				case 1: CheckDlgButton(hDlg, IDC_BIOSEMI_ABC, TRUE); break;
			}
			
			// IDC_BIOSEMI_MEM
			// enable only when 32 channel options and 10-20 systems are selected
			if (st->labeln == 0 && ((st->chansetn == 4) || (st->chansetn == 9))) {
				EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), TRUE);
				CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage);
			}
			else {
				EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), FALSE);
				st->memory_montage_tmp = 0;
				CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage_tmp);
			}

			// disable update button if no change
			if (chanset_changed == 0) EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), FALSE);
			// disable apply button if no change
			if (output_changed == 0) EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), FALSE);

			// if channel matrix exists			
			if (!channels_.empty())
			{
				// send retrieved channel labels to all the comboboxes
				for (int k = 0; k < channels_.size(); k++)
				{
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
				}
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_SETCURSEL, st->opn[0], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_SETCURSEL, st->opn[1], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_SETCURSEL, st->opn[2], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_SETCURSEL, st->opn[3], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_SETCURSEL, st->opn[4], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_SETCURSEL, st->opn[5], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_SETCURSEL, st->opn[6], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_SETCURSEL, st->opn[7], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_SETCURSEL, st->opn[8], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_SETCURSEL, st->opn[9], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_SETCURSEL, st->opn[10], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_SETCURSEL, st->opn[11], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_SETCURSEL, st->opn[12], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_SETCURSEL, st->opn[13], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_SETCURSEL, st->opn[14], 0L);
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_ADDSTRING, 0, (LPARAM) "None");
				SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_SETCURSEL, st->opn[15], 0L);
			}		
			else {
				// if previous output assignments exist, after loading a design
				if (st->out_ports[0].out_name != "1") {
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_ADDSTRING, 0, (LPARAM)st->out_ports[0].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_ADDSTRING, 0, (LPARAM)st->out_ports[1].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_ADDSTRING, 0, (LPARAM)st->out_ports[2].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_ADDSTRING, 0, (LPARAM)st->out_ports[3].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_ADDSTRING, 0, (LPARAM)st->out_ports[4].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_ADDSTRING, 0, (LPARAM)st->out_ports[5].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_ADDSTRING, 0, (LPARAM)st->out_ports[6].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_ADDSTRING, 0, (LPARAM)st->out_ports[7].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_ADDSTRING, 0, (LPARAM)st->out_ports[8].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_ADDSTRING, 0, (LPARAM)st->out_ports[9].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_ADDSTRING, 0, (LPARAM)st->out_ports[10].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_ADDSTRING, 0, (LPARAM)st->out_ports[11].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_ADDSTRING, 0, (LPARAM)st->out_ports[12].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_ADDSTRING, 0, (LPARAM)st->out_ports[13].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_ADDSTRING, 0, (LPARAM)st->out_ports[14].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_ADDSTRING, 0, (LPARAM)st->out_ports[15].out_name);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_SETCURSEL, 0, 0L);
				}
			}			

			return TRUE;

		case WM_CLOSE:		// the user dialog is to be closed
			// discard changes
			chanset_changed = 0;  output_changed = 0; 
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;

		case WM_COMMAND:	// a dialog item received a message
			switch (LOWORD(wParam))	// get the ID of the dialog item
			{
				// IDC_BIOSEMI_UPDATEBOX: update channel info
				case IDC_BIOSEMI_UPDATEBOX:
				{
					//if (GLOBAL.biosemi_available == 1) report_error("session ongoing - stop the session first"); return;
					if (chanset_changed == 1) {
						SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "...Updating");
						started = 1;

						// retrieve the latest channel subset setting
						st->chansetn = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
						SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETLBTEXT, st->chansetn, (LPARAM)st->chanset_string);
						st->labeln = st->labeln_tmp;
						st->memory_montage = st->memory_montage_tmp;

						st->biosemi_io();
						
						if (DevActive == 1) st->get_channel_set_index();
						if (DevActive == 1) {
							// don't reset output selection if it's the first time from loading a design
							if (loaded == 1) loaded = 0;
							else st->reset_output_ui();	}
						if (DevActive == 1) st->release_biosemi();
						if (DevActive == 1) st->clear_ui();
						if (DevActive == 1) st->update_output_ui();
						if (DevActive == 1) st->update_outports();
						
						if (DevActive == 1) {
							SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "Updated!");
							// disable update button
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), FALSE);
							// enable apply button
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);
							chanset_changed = 0; 
							first_update = 0; 
							output_changed = 1;
						}
						else {
							chanset_changed = 1; 
							first_update = 1; 
							output_changed = 1;
						}
					}
					else SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "Nothing to update!");
					break;
				}
				
				// IDC_BIOSEMI_APPLY_SELECTION, update box
				case IDC_BIOSEMI_APPLY_SELECTION:
				{
					if (output_changed == 1) {
						SetDlgItemText(hDlg, IDC_BIOSEMI_APPLY_STATUS, "...Applying");
						// save the output channels chosen
						st->save_ui();
						// update box
						if (DevActive == 1) st->update_outports();
						SetDlgItemText(hDlg, IDC_BIOSEMI_APPLY_STATUS, "Applied!");
						// disable apply button
						EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), FALSE);
						output_changed = 0; first_update = 0; 
					}					
					else SetDlgItemText(hDlg, IDC_BIOSEMI_APPLY_STATUS, "Nothing to apply!");
					break;
				}
			
				// IDC_APPLY: OK button
				case IDC_APPLY:
				{
					// retrieve channel subset setting
					st->chansetn = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
					SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETLBTEXT, st->chansetn, (LPARAM)st->chanset_string);
					// don't reset output selection if it's the first time from loading a design
					if (loaded == 1) {
						st->labeln = st->labeln_tmp;
						st->memory_montage = st->memory_montage_tmp;
						//loaded = 0;
					}
					else st->save_ui(); // else save the output channels chosen

					// update the box if the ui options have been changed but not yet updated (either of the two update buttons was not pressed)
					// or when the first time of bringing up the dialogue
					if (chanset_changed == 1 || (first_update == 1)) {
						SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "...Updating");
						started = 1;
						st->biosemi_io();
						if (DevActive == 1) st->get_channel_set_index();
						if (DevActive == 1) {
							// don't reset output selection if it's the first time from loading a design
							if (loaded == 1) loaded = 0;
							else st->reset_output_ui();
						}
						if (DevActive == 1) st->update_outports();
						if (DevActive == 1) st->release_biosemi();
						SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "Updated!");
						// disable update button
						EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), FALSE);
						// disable apply button
						EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), FALSE);
						chanset_changed = 0; first_update = 0; output_changed = 0;
					}
					// when output options are changed - apply to the box
					if (output_changed == 1) {
						SetDlgItemText(hDlg, IDC_BIOSEMI_APPLY_STATUS, "...Applying");
						// save the output channels chosen
						st->save_ui(); 

						// update box
						if (DevActive == 1) st->update_outports();
						SetDlgItemText(hDlg, IDC_BIOSEMI_APPLY_STATUS, "Applied!");
					}
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
					break;
				}

				// channel subset
				case IDC_CHANSET: 
				{
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						// Enable update button
						EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), TRUE);

						st->chansetn_tmp = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
						if ((st->chansetn_tmp == 0) || (st->chansetn_tmp == 1) || (st->chansetn_tmp == 2) || (st->chansetn_tmp == 2) ||
							(st->chansetn_tmp == 5) || (st->chansetn_tmp == 6) || (st->chansetn_tmp == 7)) {
							// disable IDC_BIOSEMI_TenTwenty and change option to BioSemi-ABC
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_TenTwenty), FALSE);
							st->labeln_tmp = 1;
							switch (st->labeln_tmp) {
							case 0: CheckDlgButton(hDlg, IDC_BIOSEMI_TenTwenty, TRUE); CheckDlgButton(hDlg, IDC_BIOSEMI_ABC, FALSE); break;
							case 1: CheckDlgButton(hDlg, IDC_BIOSEMI_ABC, TRUE);  CheckDlgButton(hDlg, IDC_BIOSEMI_TenTwenty, FALSE); break;
							}
						}
						else {
							// enable IDC_BIOSEMI_TenTwenty
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_TenTwenty), TRUE);
							switch (st->labeln_tmp) {
							case 0: CheckDlgButton(hDlg, IDC_BIOSEMI_TenTwenty, TRUE); CheckDlgButton(hDlg, IDC_BIOSEMI_ABC, FALSE); break;
							case 1: CheckDlgButton(hDlg, IDC_BIOSEMI_ABC, TRUE);  CheckDlgButton(hDlg, IDC_BIOSEMI_TenTwenty, FALSE); break;
							}
						}
						// IDC_BIOSEMI_MEM
						// enable only when 32 channel options and 10-20 systems are selected
						if (st->labeln_tmp == 0 && ((st->chansetn_tmp == 4) || (st->chansetn_tmp == 9))) {
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), TRUE);
							st->memory_montage_tmp = 1;
							CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage_tmp);
						}
						else {
							EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), FALSE);
							st->memory_montage_tmp = 0;
							CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage_tmp);
						}
						chanset_changed = 1; output_changed = 1;
					}
					break;
				}

				// labelling system
				case IDC_BIOSEMI_TenTwenty: {
					st->labeln_tmp = 0;
					// enable only for 32 channel options
					if ((st->chansetn_tmp == 4) || (st->chansetn_tmp == 9)) {
						EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), TRUE);
						CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage_tmp);
					}
					chanset_changed = 1; output_changed = 1;
					// enable update button
					EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), TRUE);
					break;
				}
				case IDC_BIOSEMI_ABC: {
					st->labeln_tmp = 1;
					EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_MEM), FALSE);
					st->memory_montage_tmp = 0;
					CheckDlgButton(hDlg, IDC_BIOSEMI_MEM, st->memory_montage_tmp);
					chanset_changed = 1; output_changed = 1;
					// enable update button
					EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), TRUE);
					break;
				}

				// IDC_BIOSEMI_MEM, memory montage? apply to only 32 channel options
				case IDC_BIOSEMI_MEM: {
					st->memory_montage_tmp = IsDlgButtonChecked(hDlg, IDC_BIOSEMI_MEM);
					chanset_changed = 1; output_changed = 1;
					// enable update button
					EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_UPDATEBOX), TRUE);
				}	

				// output options 1-16
				case IDC_BIOSEMI_OP1: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE); break;
				case IDC_BIOSEMI_OP2: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE); break;
				case IDC_BIOSEMI_OP3: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP4: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP5: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP6: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP7: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP8: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP9: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP10: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP11: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP12: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP13: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP14: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP15: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
				case IDC_BIOSEMI_OP16: if (HIWORD(wParam) == CBN_SELCHANGE) output_changed = 1;	EnableWindow(GetDlgItem(hDlg, IDC_BIOSEMI_APPLY_SELECTION), TRUE);	break;
					
				case IDC_BIOSEMI_CANCEL: {
					// discard changes
					chanset_changed = 0;  output_changed = 0;
					EndDialog(hDlg, LOWORD(wParam)); return TRUE; break;
				}
			}
			return TRUE;

		case WM_SIZE:		// when the user dialog was moved or sized@			
		case WM_MOVE:
			update_toolbox_position(hDlg); // save the new position
			break;
			return TRUE;
	}
	return FALSE;
}

void BIOSEMIOBJ::session_start(void) // will be called when Play -button is pressed
{
	if (DrvLib == NULL) return;
	if (GLOBAL.biosemi_available == 1) return; // don't do again if the amp is active
	if ((filehandle == INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
	{
		if (biosemi_)
			biosemi_.reset();

		started = 1;		
		
		biosemi_io();		

		// retrieve channel subset setting, if the gui isn't opened
		if (loaded == 1) strcpy(chanset_string, chanset_mat[chansetn].c_str());

		if (DevActive != 1) return;
		
		get_channel_set_index();

		// if start the session right away without config the gui options
		if (first_update == 1) {
			if (DevActive == 1) {
				// don't reset output selection if it's the first time from loading a design
				if (loaded == 1) loaded = 0;
				else reset_output_ui();
			}
			// update ui in case it's active
			update_output_ui(); chanset_changed = 0; output_changed = 0; first_update = 0;
		}		
		update_outports();
		update_samplingrate(srate_);
		// for timer.cpp to initiate process_biosemi()
		GLOBAL.biosemi_available = 1;	
	}
	else { 
		GLOBAL.biosemi_available = 0;
	}
}

void BIOSEMIOBJ::work(void) // generate output value
{
	// data streaming is handled by process_biosemi() in timer.cpp
	if (DrvLib == NULL) return;
	if (DevActive == 0) return;	
}

void process_biosemi(void) // to be called by timer.cpp while GLOBAL.biosemi_available == 1
{	
	// don't steam data if no active outports is specified
	if (gBIOSEMI->active_outports.empty()) return;

	// get a chunk from the device --> raw_chunk is [#insamples x #channels]
	biosemi_->get_chunk(raw_chunk);

	int outchannels = channels_.size();
	int insamples = raw_chunk.size();

	if (insamples > 0) {
		// convert to microvolts --> scaled_chunk_tr is [#channels x #insamples]
		scaled_chunk_tr.resize(outchannels);
		for (int c = 0, e = outchannels; c < e; c++) {
			scaled_chunk_tr[c].resize(insamples);
			if (types_[c] == "Trigger") {
				// don't scale the trigger channel
				for (int s = 0, e = insamples; s < e; s++)
					scaled_chunk_tr[c][s] = raw_chunk[s][index_set_[c]] / 256;
			}
			else {
				// scale all the rest (note that there is a version of the analog input boxes for which the voltage is actually 4x this value)
				for (int s = 0, e = insamples; s < e; s++)
					scaled_chunk_tr[c][s] = raw_chunk[s][index_set_[c]] / 256 * 0.03125;  // 31.25 nV per bit
			}
		}

		// send it off to outports
		for (int s = 0, e = insamples; s < e; s++) {
			for (int c = 0, e = gBIOSEMI->active_outports.size(); c < e; c++) {
				gBIOSEMI->pass_values(gBIOSEMI->active_outports[c], 
					scaled_chunk_tr[gBIOSEMI->opn[gBIOSEMI->active_outports[c]]].at(s));
			}
			process_packets();
		}
	}
	// sleep until we get the next chunk
	Sleep(send_interval_ms);
}

void BIOSEMIOBJ::session_stop(void) // will be called when Stop- button 
{
	if (DrvLib == NULL) return;
	if (GLOBAL.biosemi_available == 0) return; // don't do anything is no amp active
	release_biosemi();
	GLOBAL.biosemi_available = 0;
}

void BIOSEMIOBJ::session_reset(void) // will be called when Reset- button
{
	//report("Session has been reset");
}

void BIOSEMIOBJ::session_pos(long pos) // will be called when Positioning the
{									   // archive in the status bar was done

}


// need to plot all the fields on the GUI
void BIOSEMIOBJ::load(HANDLE hFile)
{
	loaded = 1;
	load_object_basics(this);
	load_property("chansetn", P_INT, &chansetn);
	//load_property("chansetn_tmp", P_INT, &chansetn_tmp);
	//load_property("chanset_string", P_STRING, &chanset_string);
	load_property("labeln", P_INT, &labeln);
	load_property("labeln_tmp", P_INT, &labeln_tmp);
	load_property("memory_montage", P_INT, &memory_montage);
	load_property("memory_montage_tmp", P_INT, &memory_montage_tmp);
	load_property("opn[0]", P_INT, &opn[0]);
	load_property("opn[1]", P_INT, &opn[1]);
	load_property("opn[2]", P_INT, &opn[2]);
	load_property("opn[3]", P_INT, &opn[3]);
	load_property("opn[4]", P_INT, &opn[4]);
	load_property("opn[5]", P_INT, &opn[5]);
	load_property("opn[6]", P_INT, &opn[6]);
	load_property("opn[7]", P_INT, &opn[7]);
	load_property("opn[8]", P_INT, &opn[8]);
	load_property("opn[9]", P_INT, &opn[9]);
	load_property("opn[10]", P_INT, &opn[10]);
	load_property("opn[11]", P_INT, &opn[11]);
	load_property("opn[12]", P_INT, &opn[12]);
	load_property("opn[13]", P_INT, &opn[13]);
	load_property("opn[14]", P_INT, &opn[14]);
	load_property("opn[15]", P_INT, &opn[15]);
	load_property("out_ports[0].out_name", P_STRING, &out_ports[0].out_name);
	load_property("out_ports[1].out_name", P_STRING, &out_ports[1].out_name);
	load_property("out_ports[2].out_name", P_STRING, &out_ports[2].out_name);
	load_property("out_ports[3].out_name", P_STRING, &out_ports[3].out_name);
	load_property("out_ports[4].out_name", P_STRING, &out_ports[4].out_name);
	load_property("out_ports[5].out_name", P_STRING, &out_ports[5].out_name);
	load_property("out_ports[6].out_name", P_STRING, &out_ports[6].out_name);
	load_property("out_ports[7].out_name", P_STRING, &out_ports[7].out_name);
	load_property("out_ports[8].out_name", P_STRING, &out_ports[8].out_name);
	load_property("out_ports[9].out_name", P_STRING, &out_ports[9].out_name);
	load_property("out_ports[10].out_name", P_STRING, &out_ports[10].out_name);
	load_property("out_ports[11].out_name", P_STRING, &out_ports[11].out_name);
	load_property("out_ports[12].out_name", P_STRING, &out_ports[12].out_name);
	load_property("out_ports[13].out_name", P_STRING, &out_ports[13].out_name);
	load_property("out_ports[14].out_name", P_STRING, &out_ports[14].out_name);
	load_property("out_ports[15].out_name", P_STRING, &out_ports[15].out_name);
}

void BIOSEMIOBJ::save(HANDLE hFile)  // hFile will be the opened configfile
{
	save_object_basics(hFile, this);
	save_property(hFile, "chansetn", P_INT, &chansetn);
	//save_property(hFile, "chansetn_tmp", P_INT, &chansetn_tmp);
	//save_property(hFile, "chanset_string", P_STRING, &chanset_string);
	save_property(hFile, "labeln", P_INT, &labeln);
	save_property(hFile, "memory_montage", P_INT, &memory_montage);
	save_property(hFile, "labeln_tmp", P_INT, &labeln_tmp);
	save_property(hFile, "memory_montage_tmp", P_INT, &memory_montage_tmp);
	save_property(hFile, "opn[0]", P_INT, &opn[0]);
	save_property(hFile, "opn[1]", P_INT, &opn[1]);
	save_property(hFile, "opn[2]", P_INT, &opn[2]);
	save_property(hFile, "opn[3]", P_INT, &opn[3]);
	save_property(hFile, "opn[4]", P_INT, &opn[4]);
	save_property(hFile, "opn[5]", P_INT, &opn[5]);
	save_property(hFile, "opn[6]", P_INT, &opn[6]);
	save_property(hFile, "opn[7]", P_INT, &opn[7]);
	save_property(hFile, "opn[8]", P_INT, &opn[8]);
	save_property(hFile, "opn[9]", P_INT, &opn[9]);
	save_property(hFile, "opn[10]", P_INT, &opn[10]);
	save_property(hFile, "opn[11]", P_INT, &opn[11]);
	save_property(hFile, "opn[12]", P_INT, &opn[12]);
	save_property(hFile, "opn[13]", P_INT, &opn[13]);
	save_property(hFile, "opn[14]", P_INT, &opn[14]);
	save_property(hFile, "opn[15]", P_INT, &opn[15]);
	save_property(hFile, "out_ports[0].out_name", P_STRING, &out_ports[0].out_name);
	save_property(hFile, "out_ports[1].out_name", P_STRING, &out_ports[1].out_name);
	save_property(hFile, "out_ports[2].out_name", P_STRING, &out_ports[2].out_name);
	save_property(hFile, "out_ports[3].out_name", P_STRING, &out_ports[3].out_name);
	save_property(hFile, "out_ports[4].out_name", P_STRING, &out_ports[4].out_name);
	save_property(hFile, "out_ports[5].out_name", P_STRING, &out_ports[5].out_name);
	save_property(hFile, "out_ports[6].out_name", P_STRING, &out_ports[6].out_name);
	save_property(hFile, "out_ports[7].out_name", P_STRING, &out_ports[7].out_name);
	save_property(hFile, "out_ports[8].out_name", P_STRING, &out_ports[8].out_name);
	save_property(hFile, "out_ports[9].out_name", P_STRING, &out_ports[9].out_name);
	save_property(hFile, "out_ports[10].out_name", P_STRING, &out_ports[10].out_name);
	save_property(hFile, "out_ports[11].out_name", P_STRING, &out_ports[11].out_name);
	save_property(hFile, "out_ports[12].out_name", P_STRING, &out_ports[12].out_name);
	save_property(hFile, "out_ports[13].out_name", P_STRING, &out_ports[13].out_name);
	save_property(hFile, "out_ports[14].out_name", P_STRING, &out_ports[14].out_name);
	save_property(hFile, "out_ports[15].out_name", P_STRING, &out_ports[15].out_name);
}


BIOSEMIOBJ::~BIOSEMIOBJ()
{
	release_biosemi();
	GLOBAL.biosemi_available = 0;
	FreeLibrary(DrvLib);
	DevActive = 0;
	channels_.clear(); // for the next insertion
} // deconstructor

void BIOSEMIOBJ::biosemi_io(void)
{
	DevActive = 1;
	// connect to driver
	std::cout << "Connecting to driver..." << std::endl;
	hConn_ = OPEN_DRIVER_ASYNC();
	if (!hConn_) {
		//FreeLibrary(DrvLib);
		DevActive = 0;
		report_error("Could not open connection to BioSemi driver.");
	}

	// if the usb box isn't connected, return
	if (DevActive == 0)	return;
	
	// initialize USB2 interface
	std::cout << "Initializing USB interface..." << std::endl;
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		DevActive = 0;
		report_error("Could not initialize BioSemi USB2 interface.");
	}
	
	// === allocate ring buffer and begin acquisiton ===
	// initialize the initial (probing) ring buffer
	std::cout << "Initializing ring buffer..." << std::endl;
	ring_buffer_ = new uint32_t[buffer_bytes];
	if (!ring_buffer_) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		DevActive = 0;
		report_error("Could not allocate ring buffer (out of memory).");
	}
	memset(ring_buffer_, 0, buffer_bytes);

	// begin acquisition
	READ_MULTIPLE_SWEEPS(hConn_, (char*)ring_buffer_, buffer_bytes);

	// enable handshake
	std::cout << "Enabling handshake..." << std::endl;
	if (!USB_WRITE(hConn_, &msg_handshake[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		DevActive = 0;
		report_error("Could not enable handshake with BioSemi USB2 interface.");
	}
	
	// === read the first sample ===
	// obtain buffer head pointer
	std::cout << "Querying buffer pointer..." << std::endl;
	if (!READ_POINTER(hConn_, &start_idx)) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		DevActive = 0;
		report_error("Can not obtain ring buffer pointer from BioSemi driver.");
	}

	// check head pointer validity
	if (start_idx > buffer_bytes) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		DevActive = 0;
		report_error("Buffer pointer returned by BioSemi driver is not in the valid range.");
	}

	// read the first sample
	std::cout << "Waiting for data..." << std::endl;
	clock_t start_time = clock();

	while (1) {
		// error checks...
		if (!READ_POINTER(hConn_, &cur_idx)) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			//FreeLibrary(DrvLib);
			delete ring_buffer_;
			DevActive = 0;
			report_error("Ring buffer handshake with BioSemi driver failed unexpectedly.");
			break;
		}
		if (((double)(clock() - start_time)) / CLOCKS_PER_SEC > max_waiting_time) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			//FreeLibrary(DrvLib);
			delete ring_buffer_;
			DevActive = 0;
			if (cur_idx - start_idx < 8) {
				report_error("BioSemi driver does not transmit data. Is the box turned on?");
			}
			else
				report_error("Did not get a sync signal from BioSemi driver. Is the battery charged?");
			break;
		}

		if ((cur_idx - start_idx >= 8) && (ring_buffer_[0] == 0xFFFFFF00)) {
			// got a sync signal on the first index...
			start_idx = 0;
			break;
		}
		if ((cur_idx - start_idx >= 8) && (ring_buffer_[start_idx / 4] == 0xFFFFFF00))
			// got the sync signal!
			break;
	}

	// if the box isn't on, return
	if (DevActive == 0)	return;

	// === parse amplifier configuration ===
	// read the trigger channel data ...
	
	std::cout << "Checking status..." << std::endl;
	uint32_t status = ring_buffer_[start_idx / 4 + 1] >> 8;
	std::cout << "  status: " << status << std::endl;

	// determine channel configuration
	is_mk2_ = ((status & (1 << 23)) != 0);
	std::cout << "  MK2: " << is_mk2_ << std::endl;

	// check speed mode
	speed_mode_ = ((status & (1 << 17)) >> 17) + ((status & (1 << 18)) >> 17) + ((status & (1 << 19)) >> 17) + ((status & (1 << 21)) >> 18);
	std::cout << "  speed mode: " << speed_mode_ << std::endl;

	// check for problems...
	if (speed_mode_ > 9) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		if (is_mk2_)
			report_error("BioSemi amplifier speed mode wheel must be between positions 0 and 8 (9 is a reserved value); recommended for typical use is 4.");
		else
			report_error("BioSemi amplifier speed mode wheel must be between positions 0 and 8 (9 is a reserved value); recommended for typical use is 4.");
	}

	// determine sampling rate (http://www.biosemi.com/faq/adjust_samplerate.htm)
	switch (speed_mode_ & 3) {
	case 0: srate_ = 2048; break;
	case 1: srate_ = 4096; break;
	case 2: srate_ = 8192; break;
	case 3: srate_ = 16384; break;
	}
	// speed modes lower than 3 are special on Mk2 and are for daisy-chained setups (@2KHz)
	bool multibox = false;
	if (is_mk2_ && speed_mode_ < 4) {
		srate_ = 2048;
		multibox = true;
	}
	std::cout << "  sampling rate: " << srate_ << std::endl;

	// determine channel configuration -- this is written according to:
	//   http://www.biosemi.com/faq/make_own_acquisition_software.htm
	//   http://www.biosemi.com/faq/adjust_samplerate.htm
	if (is_mk2_) {
		// in an Mk2 the speed modes 0-3 are for up to 4 daisy-chained boxes; these are 
		// multiplexed, have 128ch EEG each and 8ch EXG each, plus 16 extra channels each (howdy!)
		switch (speed_mode_) {
		case 0:
		case 1:
		case 2:
		case 3:
			nbeeg_ = 4 * 128; nbexg_ = 4 * 8; nbaux_ = 4 * 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 610; break;
			// spd modes 4-7 are the regular ones and have 8 EXG's added in
		case 4: nbeeg_ = 256; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 282; break;
		case 5: nbeeg_ = 128; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 154; break;
		case 6: nbeeg_ = 64; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 90; break;
		case 7: nbeeg_ = 32; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 58; break;
			// spd mode 8 adds
		case 8: nbeeg_ = 256; nbexg_ = 8; nbaux_ = 16; nbaib_ = 32; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 314; break;
		}
	}
	else {
		// in a Mk1 the EXG's are off in spd mode 0-3 and on in spd mode 4-7 (but subtracted from the EEG channels)
		switch (speed_mode_) {
			// these are all-EEG modes
		case 0: nbeeg_ = 256; nbexg_ = 0; nbaux_ = 0; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 258; break;
		case 1: nbeeg_ = 128; nbexg_ = 0; nbaux_ = 0; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 130; break;
		case 2: nbeeg_ = 64; nbexg_ = 0; nbaux_ = 0; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 66; break;
		case 3: nbeeg_ = 32; nbexg_ = 0; nbaux_ = 0; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 34; break;
			// in these modes there are are 8 EXGs and 16 aux channels
		case 4: nbeeg_ = 232; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 258; break;
		case 5: nbeeg_ = 104; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 130; break;
		case 6: nbeeg_ = 40; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 66; break;
		case 7: nbeeg_ = 8; nbexg_ = 8; nbaux_ = 16; nbaib_ = 0; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 34; break;
			// in spd mode 8 there are 32 AIB channels from an Analog Input Box (AIB) multiplexed in (EXGs are off)
		case 8: nbeeg_ = 256; nbexg_ = 0; nbaux_ = 0; nbaib_ = 32; nbtrig_ = 1; nbsync_ = 1; nbchan_ = 290; break;
		}
	}
	std::cout << "  channels: " << nbchan_ << "(" << nbsync_ << "xSync, " << nbtrig_ << "xTrigger, " << nbeeg_ << "xEEG, " << nbexg_ << "xExG, " << nbaux_ << "xAUX, " << nbaib_ << "xAIB)" << std::endl;

	// check for additional problematic conditions
	battery_low_ = (status & (1 << 22)) != 0;
	if (battery_low_)
		std::cout << "  battery: low" << std::endl;
	else
		std::cout << "  battery: good" << std::endl;
	if (battery_low_)
		std::cout << "The BioSemi battery is low; amplifier will shut down within 30-60 minutes." << std::endl;

	// compute a proper buffer size (needs to be a multiple of 512, a multiple of nbchan, as well as ~32MB in size)
	std::cout << "Reallocating optimized ring buffer..." << std::endl;

	// === shutdown current coonnection ===
	// shutdown current connection
	std::cout << "Sending the enable message again..." << std::endl;
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		report_error("Error while disabling the handshake.");
	}
	std::cout << "Closing the driver..." << std::endl;
	if (!CLOSE_DRIVER_ASYNC(hConn_)) {
		//FreeLibrary(DrvLib);
		report_error("Error while disconnecting.");
	}

	// reset to a new ring buffer
	std::cout << "Freeing the ring buffer..." << std::endl;
	delete ring_buffer_;

	// === reinitialize acquisition ===

	std::cout << "Allocating a new ring buffer..." << std::endl;
	ring_buffer_ = new uint32_t[buffer_samples * nbchan_];
	if (!ring_buffer_) {
		//FreeLibrary(DrvLib);
		report_error("Could not reallocate ring buffer (out of memory?).");
	}
	std::cout << "Zeroing the ring buffer..." << std::endl;

	memset(ring_buffer_, 0, buffer_samples * 4 * nbchan_);

	// reconnect to driver
	std::cout << "Opening the driver..." << std::endl;
	hConn_ = OPEN_DRIVER_ASYNC();
	if (!hConn_) {
		//FreeLibrary(DrvLib);
		report_error("Could not open connection to BioSemi driver.");
	}
	// reinitialize USB2 interface
	std::cout << "Reinitializing the USB interface..." << std::endl;
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		report_error("Could not initialize BioSemi USB2 interface.");
	}

	// begin acquisition
	std::cout << "Starting data acquisition..." << std::endl;
	READ_MULTIPLE_SWEEPS(hConn_, (char*)ring_buffer_, buffer_samples * 4 * nbchan_);
	// enable handshake
	std::cout << "Enabling handshake..." << std::endl;
	if (!USB_WRITE(hConn_, &msg_handshake[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		report_error("Could not reenable handshake with BioSemi USB2 interface.");
	}

	// === check for correctness of new data ===
	// make sure that we can read the buffer pointer
	std::cout << "Attempt to read buffer pointer..." << std::endl;
	if (!READ_POINTER(hConn_, &start_idx)) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		//FreeLibrary(DrvLib);
		delete ring_buffer_;
		report_error("Can not obtain ring buffer pointer from BioSemi driver.");
	}

	std::cout << "Verifying channel format..." << std::endl;
	start_time = clock();
	while (1) {
		// error checks
		if (!READ_POINTER(hConn_, &cur_idx)) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			//FreeLibrary(DrvLib);
			delete ring_buffer_;
			report_error("Ring buffer handshake with BioSemi driver failed unexpectedly.");
		}
		if (((double)(clock() - start_time)) / CLOCKS_PER_SEC > max_waiting_time) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			//FreeLibrary(DrvLib);
			delete ring_buffer_;
			if (cur_idx - start_idx < 8)
				report_error("BioSemi driver does not transmit data. Is the box turned on?");
			else
				report_error("Did not get a sync signal from BioSemi driver. Is the battery charged?");
		}
		if ((cur_idx - start_idx >= 4 * nbchan_) && (ring_buffer_[0] == 0xFFFFFF00))
			// got a sync signal on the first index
			start_idx = 0;
		if ((cur_idx - start_idx >= 4 * nbchan_) && (ring_buffer_[start_idx / 4] == 0xFFFFFF00)) {
			if (ring_buffer_[start_idx / 4 + nbchan_] != 0xFFFFFF00) {
				USB_WRITE(hConn_, &msg_enable[0]);
				CLOSE_DRIVER_ASYNC(hConn_);
				//FreeLibrary(DrvLib);
				delete ring_buffer_;
				report_error("Sync signal did not show up at the expected position.");
			}
			else {
				std::cout << "Channel format is correct..." << std::endl;
				//report("all correct");
				// all correct
				break;
			}
		}
	}

	// === derive channel labels ===

	channel_labels_.clear();
	channel_types_.clear();
	for (int k = 1; k <= nbsync_; k++) {
		channel_labels_.push_back(std::string("Sync") += std::to_string(static_cast<long long>(k)));
		channel_types_.push_back("Sync");
	}
	for (int k = 1; k <= nbtrig_; k++) {
		channel_labels_.push_back(std::string("Trig") += std::to_string(static_cast<long long>(k)));
		channel_types_.push_back("Trigger");
	}
	if (multibox) {
		// multi-box setup
		for (int b = 0; b <= 3; b++) {
			const char* boxid[] = { "_Box1","_Box2","_Box3","_Box4" };
			for (int k = 1; k <= nbeeg_ / 4; k++) {
				std::string tmp = "A"; tmp[0] = 'A' + (k - 1) / 32;
				channel_labels_.push_back((std::string(tmp) += std::to_string(static_cast<long long>(1 + (k - 1) % 32))) += boxid[b]);
				channel_types_.push_back("EEG");
			}
			for (int k = 1; k <= nbexg_ / 4; k++) {
				channel_labels_.push_back((std::string("EX") += std::to_string(static_cast<long long>(k))) += boxid[b]);
				channel_types_.push_back("EXG");
			}
			for (int k = 1; k <= nbaux_ / 4; k++) {
				channel_labels_.push_back((std::string("AUX") += std::to_string(static_cast<long long>(k))) += boxid[b]);
				channel_types_.push_back("AUX");
			}
			for (int k = 1; k <= nbaib_ / 4; k++) {
				channel_labels_.push_back((std::string("AIB") += std::to_string(static_cast<long long>(k))) += boxid[b]);
				channel_types_.push_back("Analog");
			}
		}
	}
	else {
		// regular setup
		for (int k = 1; k <= nbeeg_; k++) {
			std::string tmp = "A"; tmp[0] = 'A' + (k - 1) / 32;
			channel_labels_.push_back(std::string(tmp) += std::to_string(static_cast<long long>(1 + (k - 1) % 32)));
			channel_types_.push_back("EEG");
		}
		for (int k = 1; k <= nbexg_; k++) {
			channel_labels_.push_back(std::string("EX") += std::to_string(static_cast<long long>(k)));
			channel_types_.push_back("EXG");
		}
		for (int k = 1; k <= nbaux_; k++) {
			channel_labels_.push_back(std::string("AUX") += std::to_string(static_cast<long long>(k)));
			channel_types_.push_back("AUX");
		}
		for (int k = 1; k <= nbaib_; k++) {
			channel_labels_.push_back(std::string("AIB") += std::to_string(static_cast<long long>(k)));
			channel_types_.push_back("Analog");
		}
	}

	last_idx_ = 0;
}


void BIOSEMIOBJ::get_channel_set_index(void)
{
	index_set_.clear();
	channels_.clear();
	types_.clear();
	std::vector<std::string> channels = channel_labels_;
	std::vector<std::string> types = channel_types_;
	std::string subset = chanset_string;

	for (int k = 0; k < channels.size(); k++) {
		bool skip = types[k] == "Sync"
			|| ((subset == "160" || subset == "160, no AUX") && types[k] == "EEG" && channels[k][0] > 'E')
			|| ((subset == "128" || subset == "128, no AUX") && types[k] == "EEG" && channels[k][0] > 'D')
			|| ((subset == "64" || subset == "64, no AUX") && types[k] == "EEG" && channels[k][0] > 'B')
			|| ((subset == "32" || subset == "32, no AUX") && types[k] == "EEG" && channels[k][0] > 'A')
			|| ((subset == "all, no AUX" || subset == "160, no AUX" || subset == "128, no AUX" || subset == "64, no AUX" || subset == "32, no AUX") && types[k] == "AUX");
		if (!skip) {
			index_set_.push_back(k);
			channels_.push_back(channels[k]);
			types_.push_back(types[k]);
		}
	}

	// toggle 10-20 and biosemi-abc labelling, overwrite the labels if 10-20 is chosen
	if (labeln == 0) { // 10-20 labelling is chosen 
		if ((subset == "32") || (subset == "32, no AUX")) { // 32 channel
			if (memory_montage == 1) { // memory montage was chosen
				for (int k = 1; k <= 32; k++) {
					channels_.at(k) = thirtytwo_memory[k - 1];
				}
			}
			else { // NOT memory montage
				for (int k = 1; k <= 32; k++) {
					channels_.at(k) = thirtytwo[k - 1];
				}
			}
		}
		else { // 64 channel
			for (int k = 1; k <= 64; k++) {
				channels_.at(k) = sixtyfour[k - 1];
			}
		}
	}
}

void BIOSEMIOBJ::save_ui(void)
{
	opn[0] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP1), CB_GETCURSEL, 0, 0);
	opn[1] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP2), CB_GETCURSEL, 0, 0);
	opn[2] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP3), CB_GETCURSEL, 0, 0);
	opn[3] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP4), CB_GETCURSEL, 0, 0);
	opn[4] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP5), CB_GETCURSEL, 0, 0);
	opn[5] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP6), CB_GETCURSEL, 0, 0);
	opn[6] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP7), CB_GETCURSEL, 0, 0);
	opn[7] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP8), CB_GETCURSEL, 0, 0);
	opn[8] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP9), CB_GETCURSEL, 0, 0);
	opn[9] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP10), CB_GETCURSEL, 0, 0);
	opn[10] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP11), CB_GETCURSEL, 0, 0);
	opn[11] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP12), CB_GETCURSEL, 0, 0);
	opn[12] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP13), CB_GETCURSEL, 0, 0);
	opn[13] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP14), CB_GETCURSEL, 0, 0);
	opn[14] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP15), CB_GETCURSEL, 0, 0);
	opn[15] = SendMessage(GetDlgItem(hDlg, IDC_BIOSEMI_OP16), CB_GETCURSEL, 0, 0);
	labeln = labeln_tmp;
	memory_montage = memory_montage_tmp;
}

void BIOSEMIOBJ::reset_output_ui(void)
{
	std::string subset = chanset_string;
	// set new default item - all pointed to the channels_.size (None)
	// except the first and the last two outputs
	opn[0] = 1; // A1 or 10-20 equivalent
	for (int k = 1; k < opn.size(); k++) opn[k] = channels_.size();
	if ((subset == "32") || (subset == "64") ||
		(subset == "128") || (subset == "160") ||
		(subset == "all")) {
		// these subsets include 16 AUX channel at the end of the channel list
		opn.at(14) = channels_.size() - 2 - 16; // EX7
		opn.at(15) = channels_.size() - 1 - 16; // EX8
	}
	else {
		opn.at(14) = channels_.size() - 2; // EX7
		opn.at(15) = channels_.size() - 1; // EX8
	}
}

void BIOSEMIOBJ::clear_ui(void)
{
	// reset the contents of the combobox
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_RESETCONTENT, NULL, NULL);
}

void BIOSEMIOBJ::update_output_ui(void)
{
	// send retrieved channel labels to all the comboboxes
	for (int k=0; k < channels_.size(); k++)
	{
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
		SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_ADDSTRING, 0, (LPARAM)channels_[k].c_str());
	}
	
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP1, CB_SETCURSEL, opn[0], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP2, CB_SETCURSEL, opn[1], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP3, CB_SETCURSEL, opn[2], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP4, CB_SETCURSEL, opn[3], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP5, CB_SETCURSEL, opn[4], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP6, CB_SETCURSEL, opn[5], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP7, CB_SETCURSEL, opn[6], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_ADDSTRING, 0, (LPARAM) "None"); 
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP8, CB_SETCURSEL, opn[7], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP9, CB_SETCURSEL, opn[8], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP10, CB_SETCURSEL, opn[9], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP11, CB_SETCURSEL, opn[10], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP12, CB_SETCURSEL, opn[11], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP13, CB_SETCURSEL, opn[12], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP14, CB_SETCURSEL, opn[13], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP15, CB_SETCURSEL, opn[14], 0L);
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_ADDSTRING, 0, (LPARAM) "None");
	SendDlgItemMessage(hDlg, IDC_BIOSEMI_OP16, CB_SETCURSEL, opn[15], 0L);
}

void BIOSEMIOBJ::update_outports(void)
{
	// update outports of the box, to be called by an Update button on the gui
	// outports = channels_.size();
	// height = CON_START + outports * CON_HEIGHT + 5;

	//for (int k = 0; k < channels_.size(); k++) {
	active_outports.clear();
	for (int k = 0; k < opn.size(); k++) {
		if (opn[k] < channels_.size())
		// channels selected are not "None"
		{
			// channels go to output labels
			strcpy(out_ports[k].out_name, channels_[opn[k]].c_str());
			// channels (labels) also go to description, for the correct labels in the edf writer
			strcpy(out_ports[k].out_desc, channels_[opn[k]].c_str());
			// output unit mV
			strcpy(out_ports[k].out_dim, "uV");
			active_outports.push_back(k);
			// other stuff
			out_ports[k].get_range = -1;
			out_ports[k].out_min = -500.0f;
			out_ports[k].out_max = 500.0f;
		}		
		else
		// channels selected are "None"
		{
			strcpy(out_ports[k].out_name, "None");
			strcpy(out_ports[k].out_desc, "None");
			strcpy(out_ports[k].out_dim, "None");
		}
	}
	InvalidateRect(ghWndDesign, NULL, TRUE);
}

void BIOSEMIOBJ::get_chunk(chunk_t& result) // get data from the amp? 
{
	
	if (DevActive == 0) return;
	// get current buffer offset
	int cur_idx;
	if (!READ_POINTER(hConn_, (unsigned*)& cur_idx))
		//release_biosemi(); GLOBAL.biosemi_available = 0;
		report_error("Reading the updated buffer pointer gave an error.");
		//return;
	cur_idx = cur_idx / 4;

	// forget about incomplete sample data
	cur_idx = cur_idx - cur_idx % nbchan_;
	if (cur_idx < 0)
		cur_idx = cur_idx + buffer_samples * nbchan_;

	result.clear();
	if (cur_idx != last_idx_) {
		if (cur_idx > last_idx_) {
			// sequential read: copy intermediate part between offsets
			int chunklen = (cur_idx - last_idx_) / nbchan_;
			result.resize(chunklen);
			for (int k = 0; k < chunklen; k++) {
				result[k].resize(nbchan_);
				memcpy(&result[k][0], &ring_buffer_[last_idx_ + k * nbchan_], nbchan_ * 4);
			}
		}
		else {
			// wrap-around read: concatenate two parts
			int chunklen = (cur_idx + buffer_samples * nbchan_ - last_idx_) / nbchan_;
			result.resize(chunklen);
			int first_section = buffer_samples - last_idx_ / nbchan_;
			for (int k = 0; k < first_section; k++) {
				result[k].resize(nbchan_);
				memcpy(&result[k][0], &ring_buffer_[last_idx_ + k * nbchan_], nbchan_ * 4);
			}
			int second_section = chunklen - first_section;
			for (int k = 0; k < second_section; k++) {
				result[first_section + k].resize(nbchan_);
				memcpy(&result[first_section + k][0], &ring_buffer_[k * nbchan_], nbchan_ * 4);
			}
		}
		// update status flags
		uint32_t status = ring_buffer_[cur_idx] >> 8;
		battery_low_ = (status & (1 << 22)) != 0;
	}

	// update last buffer pointer
	last_idx_ = cur_idx;
}

void BIOSEMIOBJ::release_biosemi(void)
{
	if (started == 1 && DevActive == 1) {
		// shutdown current connection, if any
		std::cout << "Sending the enable message again..." << std::endl;
		if (!USB_WRITE(hConn_, &msg_enable[0])) {
			CLOSE_DRIVER_ASYNC(hConn_);
			//FreeLibrary(DrvLib);
			report_error("Error while disabling the handshake.");
		}
		std::cout << "Closing the driver..." << std::endl;
		if (!CLOSE_DRIVER_ASYNC(hConn_)) {
			//FreeLibrary(DrvLib);
			report_error("Error while disconnecting.");
		}
		started = 0;
	}	
}