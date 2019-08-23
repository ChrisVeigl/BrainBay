/* -----------------------------------------------------------------------------

	BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_BIOSEMI.CPP:  contains the interface to
		  BIOSEMI Active Two devices (Mk1 and Mk2)
  Author: Yi-Jhong Han (the codes are extensively referenced and copied from
		  Lab Straming Layer (LSL) projects, credits also goes to them!)

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

// the age of each chunk received (in seconds) is anywhere between 0 and send_interval_ms miliseconds old, and is on average half of the maximum
const double buffer_lag = (send_interval_ms / 2000.0);

// allocate temp data & resamplers...
BIOSEMIOBJ::chunk_t raw_chunk;
std::vector<std::vector<float> > scaled_chunk_tr;

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

//gui
int updated = 0;
int changed = 0;
int first_update = 1;

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
	outports = 32;
	inports = 0;
	width = 75;

	// default channel subset entry
	chansetn = 5;

	/* Init protocol driver library */
	strcat(DrvLibName, BIOSEMI_DLL);

	strcpy(archivefile, "none");
	filehandle = INVALID_HANDLE_VALUE;

	strcpy(NB_DirName, GLOBAL.resourcepath);

	std::cout << "Loading BioSemi driver dll..." << std::endl;
	DrvLib = InitBioSemiDrvLib(DrvLibName);

	filehandle = INVALID_HANDLE_VALUE;
	filemode = 0;	
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
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "all");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "160");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "128");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "64");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "32");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "all, no AUX");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "160, no AUX");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "128, no AUX");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "64, no AUX");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_ADDSTRING, 0, (LPARAM) "32, no AUX");
		SendDlgItemMessage(hDlg, IDC_CHANSET, CB_SETCURSEL, st->chansetn, 0L);
		st->chansetn = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETLBTEXT, st->chansetn, (LPARAM)st->chanset_string);

		break;

	case WM_CLOSE:		// the user dialog is to be closed
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
		break;

	case WM_COMMAND:	// a dialog item received a message
		switch (LOWORD(wParam))	// get the ID of the dialog item
		{
			// IDC_BIOSEMI_UPDATEBOX: update channel info on the box
			case IDC_BIOSEMI_UPDATEBOX:
			{
				SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "...Updating");
				started = 1; 
				st->biosemi_io();
				if (DevActive == 1) st->get_channel_set_index();
				if (DevActive == 1) st->update_outports();
				if (DevActive == 1) st->release_biosemi();
				SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "Updated!");
				updated = 1; first_update = 0;
				break;
			}

			// channel subset
			case IDC_CHANSET:
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					st->chansetn = SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETCURSEL, 0, 0);
					SendMessage(GetDlgItem(hDlg, IDC_CHANSET), CB_GETLBTEXT, st->chansetn, (LPARAM)st->chanset_string);
					changed = 1;
				}
				break;
			}

			// IDC_APPLY: apply button
			case IDC_APPLY:
			{
				// update the box if the subset has been changed but not yet updated (the update button was not pressed)
				// or when the first time of bringing up the dialogue
				if ((changed == 1 && updated == 0) || (first_update == 1)) {
					SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "...Updating");
					started = 1; 
					st->biosemi_io();
					if (DevActive == 1) st->get_channel_set_index();
					if (DevActive == 1) st->update_outports();
					if (DevActive == 1) st->release_biosemi();
					SetDlgItemText(hDlg, IDC_BIOSEMI_UPDATE_STATUS, "Updated!");
				}			
				changed = 0; updated = 0; first_update = 0;
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
			}
			case IDC_BIOSEMI_CANCEL:
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
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

void BIOSEMIOBJ::session_start(void) // will be called when Play -button
{

	if (DrvLib == NULL) return;
	if ((filehandle == INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
	{
		if (biosemi_)
			biosemi_.reset();

		started = 1;
		biosemi_io();
		if (DevActive == 0) return;
		
		get_channel_set_index();
		update_outports();

		GLOBAL.biosemi_available = 1;		
	}
	else {
		GLOBAL.biosemi_available = 0;
	}
}

void BIOSEMIOBJ::work(void) // generate output value
{
	if (DrvLib == NULL) return;
	if (DevActive == 0) return;
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
		for (int c = 0, e = outchannels; c < e; c++) {
			for (int s = 0, e = insamples; s < e; s++) {
				pass_values(c, scaled_chunk_tr[c].at(s));
			}
		}

		// sleep until we get the next chunk
		Sleep(send_interval_ms);
	}
}

void BIOSEMIOBJ::session_stop(void) // will be called when Stop- button 
{
	if (DrvLib == NULL) return;
	release_biosemi();
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
	load_object_basics(this);
	load_property("chansetn", P_INT, &chansetn);
}

void BIOSEMIOBJ::save(HANDLE hFile)  // hFile will be the opened configfile
{
	save_object_basics(hFile, this);
	save_property(hFile, "chansetn", P_INT, &chansetn);
}



BIOSEMIOBJ::~BIOSEMIOBJ()
{
	release_biosemi();
	GLOBAL.biosemi_available = 0;
	FreeLibrary(DrvLib);
	DevActive = 0;
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
			if (cur_idx - start_idx < 8)
				report_error("BioSemi driver does not transmit data. Is the box turned on?");
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
}

void BIOSEMIOBJ::update_outports(void)
{
	// update outports of the box, to be called by an Update button on the gui
	outports = channels_.size();
	height = CON_START + outports * CON_HEIGHT + 5;

	for (int k = 0; k < channels_.size(); k++) {
		// channels go to output labels
		strcpy(out_ports[k].out_name, channels_[k].c_str());
		// channels (labels) also go to description, for the correct labels in the edf writer
		strcpy(out_ports[k].out_desc, channels_[k].c_str());
		// output unit mV
		strcpy(out_ports[k].out_dim, "uV");

		// other stuff
		//out_ports[k].get_range = -1;
		//out_ports[k].out_min = -500.0f;
		//out_ports[k].out_max = 500.0f;
	}
	InvalidateRect(ghWndDesign, NULL, TRUE);
}

void BIOSEMIOBJ::get_chunk(chunk_t& result) // get data from the amp? 
{
	// get current buffer offset
	int cur_idx;
	if (!READ_POINTER(hConn_, (unsigned*)&cur_idx))
		report_error("Reading the updated buffer pointer gave an error.");
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