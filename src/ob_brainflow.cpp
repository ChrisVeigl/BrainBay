/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_BRAINFLOW.CPP:  contains the interface to the Brainflow library
  Author: Chris Veigl

  This code is heavily based upon the example code from the Brainflow
  project, see: https://brainflow.readthedocs.io/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"

#if _MSC_VER >= 1900

#include "ob_brainflow.h"


#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"


// see https://brainflow.readthedocs.io/en/stable/SupportedBoards.html
// and https://brainflow.readthedocs.io/en/stable/UserAPI.html

struct board_id {
    char name[40];
    int id;
};

struct board_id supported_boards[] = {
    { "Synthetic Board (simulated data)", -1 } ,
    { "BrainAlive Device"               , 40 },
    { "Callibri EEG"                    , 9 },
    { "Callibri EMG"                    , 10 },
    { "Callibri ECG"                    , 11 },
    { "EmotiBit Board"                  , 47 },
    { "Enophone"                        , 37 },
    { "GTec Unicorn"                    , 8 },
    { "Mentalab Explore 4 Channel"      , 44 },
    { "Mentalab Explore 8 Channel"      , 45 },
    { "Muse S"                          , 39 },
    { "Muse 2"                          , 38 },
    { "Muse 2016"                       , 41 },
    { "Muse S BLED"                     , 21 },
    { "Muse 2 BLED"                     , 22 },
    { "Muse 2016 BLED"                  , 42 },
    { "Neuroidss FreeEEG32"             , 17 },
    { "NeuroMD Brainbit"                , 7 },
    { "NeuroMD Brainbit BLED"           , 18 },
    { "Neurosity Crown"                 , 23 },
    { "Neurosity Notion OSC1"           , 13 },
    { "Neurosity Notion 2"              , 14 },
    { "NTL WIFI"                        , 50 },
    { "OpenBCI Cyton"                   , 0 } ,
    { "OpenBCI Cyton-Daisy"             , 2 } ,
    { "OpenBCI Cyton Wifi"              , 5 } ,
    { "OpenBCI Cyton-Daisy Wifi "       , 6 },
    { "OpenBCI Ganglion"                , 1 } ,
    { "OpenBCI Ganglion Native"         , 46 },
    { "OpenBCI Ganglion Wifi"           , 4 } ,
    { "OpenBCI Galea Serial V4"         , 49 },
    { "OpenBCI Galea V4"                , 48 },
//    { "OpenBCI Galea", 3 } ,
//    { "OpenBCI Galea Serial", 20 },
//    { "Fascia", 12 },
    { "OYMotion gForce Pro EMG"         , 16 },
    { "OYMotion gForce Dual EMG"        , 19 },
    { "Ant Neuro EE 410"                , 24 },
    { "Ant Neuro EE 411"                , 25 },
    { "Ant Neuro EE 430"                , 26 },
    { "Ant Neuro EE 211"                , 27 },
    { "Ant Neuro EE 212"                , 28 },
    { "Ant Neuro EE 213"                , 29 },
    { "Ant Neuro EE 214"                , 30 },
    { "Ant Neuro EE 215"                , 31 },
    { "Ant Neuro EE 221"                , 32 },
    { "Ant Neuro EE 222"                , 33 },
    { "Ant Neuro EE 223"                , 34 },
    { "Ant Neuro EE 224"                , 35 },
    { "Ant Neuro EE 225"                , 36 },
    { "Ant Neuro EE 511"                , 51 }

};


using namespace std;

BoardShim* board = NULL;
struct BrainFlowInputParams params;
float current_channelValue[MAX_EEG_CHANNELS] = { 0.0 };
BRAINFLOWOBJ* bf;

// bool parse_args(int argc, char* argv[], struct BrainFlowInputParams* params, int* board_id);


int bf_releaseBoard() {

    if (board == NULL) return(0);

    try {
        cout << "Brainflow: release current Board." << std::endl;
        if (board->is_prepared())
        {
            board->stop_stream();
            cout << "Brainflow: stream stopped" << std::endl;
            board->release_session();
            cout << "Brainflow: session released" << std::endl;
        }
    }
    catch (const BrainFlowException& err)
    {
        cout << "Brainflow: Exception handler triggered." << std::endl;
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
    }

    cout << "Brainflow: delete Board class" << std::endl;
    delete board;
    board = NULL;
    cout << "Brainflow: Board release done." << std::endl;

    return(1);
}


int bf_createBoard(int bid) {

    if (board != NULL) bf_releaseBoard();

    try {
        board = new BoardShim(bid, params);
        cout << "Brainflow: Board created." << std::endl;
        return (1);
    }
    catch (const BrainFlowException& err)
    {
        cout << "Brainflow: Exception handler triggered." << std::endl;
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
    }
    return(0);

}


void bf_setparams(BRAINFLOWOBJ* st) {
    params.serial_port = std::string(st->serialport);
    params.ip_address = std::string(st->ipaddress);
    params.mac_address = std::string(st->macaddress);
    params.timeout = st->timeout;
    params.ip_port = st->ipport;
}

int prepare_fileRead(BRAINFLOWOBJ* st) {

    st->filehandle = CreateFile(st->archivefile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (st->filehandle == INVALID_HANDLE_VALUE) {
        st->filemode = 0;
        return(0);
    }

    get_session_length();
    GLOBAL.brainflow_available = 0;
    st->filemode = FILE_READING;

    GLOBAL.addtime = 0;
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    DWORD dwRet;

    if (GetFileTime(st->filehandle, &ftCreate, &ftAccess, &ftWrite))
    {
        FileTimeToSystemTime(&ftWrite, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        GLOBAL.addtime = stLocal.wHour * 3600 + stLocal.wMinute * 60 + stLocal.wSecond + (float)stLocal.wMilliseconds / 1000;
    }
    return(1);
}

int prepare_fileWrite(BRAINFLOWOBJ* st) {
    st->filehandle = CreateFile(st->archivefile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (st->filehandle == INVALID_HANDLE_VALUE)
    {
        st->filemode = 0;
        return(0);
    }
    st->filemode = FILE_WRITING;
    return(1);
}


void updateDialog(HWND hDlg, BRAINFLOWOBJ* st)
{

    // see https://brainflow.readthedocs.io/en/stable/SupportedBoards.html

    if ((st->board_id == 0) || (st->board_id == 1) || (st->board_id == 2) ||
        (st->board_id == 17) || (st->board_id == 18) || (st->board_id == 21) || (st->board_id == 22) || (st->board_id == 42)) {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_SERIALPORT), TRUE);
    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_SERIALPORT), FALSE);
    }

    if ((st->board_id == 1) || (st->board_id == 18) || (st->board_id == 37)) {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_MACADDRESS), TRUE);
    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_MACADDRESS), FALSE);
    }

    if ((st->board_id == 4) || (st->board_id == 5) || (st->board_id == 6)) {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_IPADDRESS), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_BF_IPPORT), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_IPADDRESS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BF_IPPORT), FALSE);
    }


    switch (st->filemode)
    {
    case 0:
        // SetDlgItemText(hDlg,IDC_FILESTATUS,"no file opened");
        EnableWindow(GetDlgItem(hDlg, IDC_REC_BF_ARCHIVE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_OPEN_BF_ARCHIVE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_END_BF_RECORDING), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_BF_ARCHIVE), FALSE);
        SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, "none");
        break;
    case FILE_READING:
        EnableWindow(GetDlgItem(hDlg, IDC_REC_BF_ARCHIVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_OPEN_BF_ARCHIVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_END_BF_RECORDING), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_BF_ARCHIVE), TRUE);
        SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, st->archivefile);
        break;
    case FILE_WRITING:
        EnableWindow(GetDlgItem(hDlg, IDC_REC_BF_ARCHIVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_OPEN_BF_ARCHIVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_END_BF_RECORDING), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_CLOSE_BF_ARCHIVE), FALSE);
        SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, st->archivefile);
        break;
    }
    InvalidateRect(ghWndDesign, NULL, TRUE);
}


LRESULT CALLBACK BrainflowDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int init;
	char sztemp[20];
	BRAINFLOWOBJ* st;

	st = (BRAINFLOWOBJ*)actobject;
	if ((st == NULL) || (st->type != OB_BRAINFLOW)) return(FALSE);



    switch (message)
    {
    case WM_INITDIALOG:
    {
        int i = 0;
        while (i < sizeof (supported_boards) / sizeof (struct board_id))  {
            SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)supported_boards[i].name); i++; 
        }

        SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_SETCURSEL, st->board_selection, 0L);
        SetDlgItemText(hDlg, IDC_BF_DEVICECOMBO, supported_boards[st->board_selection].name);

        SetDlgItemText(hDlg, IDC_BF_SERIALPORT, st->serialport);
        SetDlgItemText(hDlg, IDC_BF_MACADDRESS, st->macaddress);
        SetDlgItemText(hDlg, IDC_BF_IPADDRESS, st->ipaddress);
        SetDlgItemInt(hDlg, IDC_BF_IPPORT, st->ipport,FALSE);
        SetDlgItemInt(hDlg, IDC_BF_TIMEOUT, st->timeout, FALSE);

        SetDlgItemText(hDlg, IDC_BF_CONFIG, st->bfConfigString);

        CheckDlgButton(hDlg, IDC_SHOW_POSITION, st->show_position);
        CheckDlgButton(hDlg, IDC_SHOW_EXTRACHANNELS, st->show_extrachannels);

        SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, st->archivefile);
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
        case IDC_BF_DEVICECOMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                st->board_selection = SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_GETCURSEL, 0, 0);
                st->board_id = supported_boards[st->board_selection].id;
                cout << "Brainflow: Device ID selected: " << st->board_id << std::endl;
                updateDialog(hDlg, st);
            }
            break;

        case IDC_SHOW_POSITION:
            st->show_position = IsDlgButtonChecked(hDlg, IDC_SHOW_POSITION);
            break;

        case IDC_SHOW_EXTRACHANNELS:
            st->show_extrachannels = IsDlgButtonChecked(hDlg, IDC_SHOW_EXTRACHANNELS);
            break;

        case IDC_BF_APPLY_DEVICE:

            GetDlgItemText(hDlg, IDC_BF_SERIALPORT, st->serialport, sizeof(st->serialport)-1);
            GetDlgItemText(hDlg, IDC_BF_IPADDRESS, st->ipaddress, sizeof(st->ipaddress)-1);
            st->ipport = GetDlgItemInt(hDlg, IDC_BF_IPPORT, NULL, false);
            GetDlgItemText(hDlg, IDC_BF_MACADDRESS, st->macaddress, sizeof(st->macaddress)-1);
            st->timeout = GetDlgItemInt(hDlg, IDC_BF_TIMEOUT, NULL, false);
            GetDlgItemText(hDlg, IDC_BF_CONFIG, st->bfConfigString, sizeof(st->bfConfigString) - 1);

            try
            {
                bf_setparams(st);
                bf_createBoard(st->board_id);

               // if (strlen(st->bfConfigString) > 0) {
               //     cout << "Brainflow: Send Config-String " << st->bfConfigString << std::endl;
               //     board->config_board(st->bfConfigString);
               // }
            }
            catch (const BrainFlowException& err) {
                cout << "Brainflow: Exception handler triggered." << std::endl;
                BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
                MessageBox(NULL, err.what(), "Brainflow error", MB_OK);
            }

            st->update_channelinfo();
            break;

        case IDC_OPEN_BF_ARCHIVE:
            strcpy(st->archivefile, GLOBAL.resourcepath);
            strcat(st->archivefile, "ARCHIVES\\*.bfa");

            if (open_file_dlg(ghWndMain, st->archivefile, FT_NB_ARCHIVE, OPEN_LOAD))
            {
                if (prepare_fileRead(st))
                {
                    SendMessage(ghWndStatusbox, WM_COMMAND, IDC_STOPSESSION, 0);
                    SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, st->archivefile);
                    SendMessage(ghWndStatusbox, WM_COMMAND, IDC_RESETBUTTON, 0);
                }
                else
                {
                    report_error("Could not open Archive-File");
                }
                get_session_length();
                updateDialog(hDlg, st);
            }
            break;

        case IDC_CLOSE_BF_ARCHIVE:
            if (st->filehandle != INVALID_HANDLE_VALUE)
            {
                if (!CloseHandle(st->filehandle))
                    report_error("could not close Archive file");
                st->filehandle = INVALID_HANDLE_VALUE;
                SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, "none");
                GLOBAL.addtime = 0;
            }
            st->filemode = 0;
            get_session_length();
            updateDialog(hDlg, st);
            break;

        case IDC_REC_BF_ARCHIVE:
            strcpy(st->archivefile, GLOBAL.resourcepath);
            strcat(st->archivefile, "ARCHIVES\\*.bfa");
            if (open_file_dlg(ghWndMain, st->archivefile, FT_BF_ARCHIVE, OPEN_SAVE))
            {
                if (!prepare_fileWrite(st))
                    report_error("Could not open Archive-File");
            }
            updateDialog(hDlg, st);
            break;

        case IDC_END_BF_RECORDING:
            if (st->filehandle != INVALID_HANDLE_VALUE)
            {
                if (!CloseHandle(st->filehandle))
                    report_error("could not close Archive file");
                st->filehandle = INVALID_HANDLE_VALUE;
                SetDlgItemText(hDlg, IDC_BF_ARCHIVEFILE, "none");
            }
            st->filemode = 0;
            updateDialog(hDlg, st);
            break;
        }
        return TRUE;

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


BRAINFLOWOBJ::BRAINFLOWOBJ(int num) : BASE_CL()
{
    cout << "initialising Brainflow object ..." << std::endl;
    
    inports = 0;
    outports = 0;
    channels = 0;
	width = 90;

    bf_channels = 0;

    strcpy(serialport, "COM4");
    strcpy(ipaddress, "192.168.4.1");
    ipport = 4567;
    timeout = 0;
    strcpy(macaddress, "");
    strcpy(bfConfigString, "");

    strcpy(archivefile, "none");
    filehandle = INVALID_HANDLE_VALUE;
    filemode = 0;

    show_extrachannels = 1;
    show_position = 0;
    syncChannel = -1;


    board_id = -1;  // default: SYNTHETIC_BOARD, id -1
    board_selection = 0;
    sync = -1;      // first sync packet number will be 0

    BoardShim::enable_board_logger();
    BoardShim::set_log_file("brainflow_error_log.log");


    bf_createBoard(board_id);
    update_channelinfo();
    updateDialog(hDlg, this);

    
    bf = this;
}
void BRAINFLOWOBJ::make_dialog(void)
{
	display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_BRAINFLOWBOX, ghWndStatusbox, (DLGPROC)BrainflowDlgHandler));
}
void BRAINFLOWOBJ::load(HANDLE hFile)
{
	load_object_basics(this);
    load_property("board_selection", P_INT, &board_selection);
    load_property("board_id", P_INT, &board_id);
    load_property("serialport", P_STRING, serialport);
    load_property("ipaddress", P_STRING, ipaddress);
    load_property("ipport", P_INT, &ipport);
    load_property("macaddress", P_STRING, macaddress);
    load_property("show_position", P_INT, &show_position);
    load_property("show_extrachannels", P_INT, &show_extrachannels);
    load_property("timeout", P_INT, &timeout);
    load_property("bfConfigString", P_STRING, bfConfigString);


    load_property("archivefile", P_STRING, archivefile);
    load_property("filemode", P_INT, &filemode);
    if (filemode == FILE_READING) {
        prepare_fileRead(this);
    }
    else if (filemode == FILE_WRITING) {
        prepare_fileWrite(this);
    }

    // update board selection index if necessary
    if (supported_boards[board_selection].id != board_id) {
        for (int i = 0; i < sizeof(supported_boards) / sizeof(struct board_id); i++) {
            if (supported_boards[i].id == board_id) board_selection = i;
        }
    }

    bf_setparams(this);
    bf_createBoard(board_id);
    update_channelinfo();
}

void BRAINFLOWOBJ::save(HANDLE hFile)
{
	save_object_basics(hFile, this);
    save_property(hFile, "board_selection", P_INT, &board_selection);
    save_property(hFile, "board_id", P_INT, &board_id);
    save_property(hFile, "serialport", P_STRING, serialport);
    save_property(hFile, "ipaddress", P_STRING, ipaddress);
    save_property(hFile, "ipport", P_INT, &ipport);
    save_property(hFile, "macaddress", P_STRING, macaddress);
    save_property(hFile, "show_position", P_INT, &show_position);
    save_property(hFile, "show_extrachannels", P_INT, &show_extrachannels);
    save_property(hFile, "timeout", P_INT, &timeout);
    save_property(hFile, "bfConfigString", P_STRING, bfConfigString);

    save_property(hFile, "archivefile", P_STRING, archivefile);
    save_property(hFile, "filemode", P_INT, &filemode);
}


int BRAINFLOWOBJ::add_channels(vector <int> channelList, char* type)
{
    int chans = 0;
    cout << "Now trying to add " << type << " channels:";

    for (std::vector<int>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
        int actChn = *it;

        if (channelList.size() > 1) {
            sprintf(out_ports[actChn].out_name, "%s%d", type, chans + 1);
            sprintf(out_ports[actChn].out_desc, "%s%d", type, chans + 1);
        }
        else {
            strcpy(out_ports[actChn].out_name, type);
            strcpy(out_ports[actChn].out_desc, type);
        }

        cout << "   chn" << *it << ":" << type;

        out_ports[actChn].get_range = -1;

        if ((!strcmp(type, "eeg")) || (!strcmp(type, "ecg")) || (!strcmp(type, "eog")) || (!strcmp(type, "exg"))) {
            strcpy(out_ports[actChn].out_dim, "uV");
            out_ports[actChn].out_max = 150;
            out_ports[actChn].out_min = -150;
        }
        else if ((!strcmp(type, "pcount"))) {
            strcpy(out_ports[actChn].out_dim, " ");
            out_ports[actChn].out_max = 255;
            out_ports[actChn].out_min = 0;
        }
        else if ((!strcmp(type, "res"))) {
            strcpy(out_ports[actChn].out_dim, "Ohms");
            out_ports[actChn].out_max = 200;
            out_ports[actChn].out_min = 0;
        }
        else if ((!strcmp(type, "accel"))) {
            strcpy(out_ports[actChn].out_dim, "g");
            out_ports[actChn].out_max = 5;
            out_ports[actChn].out_min = -5;
        }
        else if ((!strcmp(type, "temp"))) {
            strcpy(out_ports[actChn].out_dim, "degC");
            out_ports[actChn].out_max = 40;
            out_ports[actChn].out_min = 35;
        }
        else if ((!strcmp(type, "batt"))) {
            strcpy(out_ports[actChn].out_dim, "%");
            out_ports[actChn].out_max = 100;
            out_ports[actChn].out_min = 0;
        }
        else {
            strcpy(out_ports[actChn].out_dim, " ");
            out_ports[actChn].out_max = 100;
            out_ports[actChn].out_min = -100;
        }
        chans++;
    }
    cout << std::endl;
    return (chans);
}

void BRAINFLOWOBJ::update_channelinfo(void)
{
    channels = 0;
    for (int i = 0; i < MAX_PORTS; i++) {
        strcpy(out_ports[i].out_name, "unused");
        channelMap[i] = i;
    }

    json desc = BoardShim::get_board_descr(board_id);

    // log general device info (just for debugging)
    try {
        cout << "Update channels for Device ID " << board_id;
        cout << ", device name = " << BoardShim::get_device_name(board_id) << std::endl;
        cout << "  Board description = " << BoardShim::get_board_descr(board_id) << std::endl;
        cout << "  the package number is at channel " << BoardShim::get_package_num_channel(board_id) << std::endl;
        cout << "  the sampling rate is " << BoardShim::get_sampling_rate(board_id) << std::endl;
        cout << "  Number of rows:"<< BoardShim::get_num_rows(board_id) << std::endl;
        cout << "  available channels are as follows:" << std::endl;
    }
    catch (const BrainFlowException& err) { cout << "Brainflow: Exception handler triggered." << std::endl;   BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); }

    bf_channels = BoardShim::get_num_rows(board_id);

    // build the output ports according to available channels
    try {

        if ((desc.find("eeg_channels") != desc.end()) || (desc.find("emg_channels") != desc.end()) || 
            (desc.find("eog_channels") != desc.end())  || (desc.find("ecg_channels") != desc.end()))
            channels += add_channels(BoardShim::get_exg_channels(board_id), "exg");

        if (desc.find("eda_channels") != desc.end()) channels += add_channels(BoardShim::get_eda_channels(board_id), "eda");
        if (desc.find("resistance_channels") != desc.end()) channels += add_channels(BoardShim::get_resistance_channels(board_id), "res");
        if (desc.find("temperature_channels") != desc.end()) channels += add_channels(BoardShim::get_temperature_channels(board_id), "temp");
        if (desc.find("accel_channels") != desc.end()) channels += add_channels(BoardShim::get_accel_channels(board_id), "accel");
        if (desc.find("gyro_channels") != desc.end()) channels += add_channels(BoardShim::get_gyro_channels(board_id), "gyro");
        if (desc.find("ppg_channels") != desc.end()) channels += add_channels(BoardShim::get_ppg_channels(board_id), "ppg");
        if (desc.find("analog_channels") != desc.end()) channels += add_channels(BoardShim::get_analog_channels(board_id), "analog");

        if (show_extrachannels) {

            if (desc.find("other_channels") != desc.end()) channels += add_channels(BoardShim::get_other_channels(board_id), "other");

            if (desc.find("package_num_channel") != desc.end()) {
                std::vector<int> vec;
                vec.push_back(BoardShim::get_package_num_channel(board_id));
                channels += add_channels(vec, "pcount");
                syncChannel = BoardShim::get_package_num_channel(board_id);
            }
            else {
                syncChannel = -1;
            }

            if (desc.find("battery_channel") != desc.end()) {
                std::vector<int> vec;
                vec.push_back(BoardShim::get_battery_channel(board_id));
                channels += add_channels(vec, "batt");
            }
            if (desc.find("timestamp_channel") != desc.end()) {
                std::vector<int> vec;
                vec.push_back(BoardShim::get_timestamp_channel(board_id));
                channels += add_channels(vec, "time");
            }
            if (desc.find("marker_channel") != desc.end()) {
                std::vector<int> vec;
                vec.push_back(BoardShim::get_marker_channel(board_id));
                channels += add_channels(vec, "marker");
            }
        }
    }
    catch (const BrainFlowException& err) { 
        cout << "Brainflow: Exception handler triggered." << std::endl;
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); 
    }

    cout << "Added total number of " << channels << " data channels!" << std::endl;

    if (show_position) {
        // if available, use EEG channel names for description of the output ports
        auto it_names = desc.find("eeg_names");
        if (it_names != desc.end()) {
            auto it_eegChannels = desc.find("eeg_channels");
            vector <int> channelList = *it_eegChannels;
            std::stringstream names((std::string)(*it_names));
            vector<string> result;
            while (names.good()) {
                string substr;
                getline(names, substr, ',');
                result.push_back(substr);
            }

            int pos = 0;
            for (std::vector<int>::iterator it = channelList.begin(); it != channelList.end(); ++it) {
                int actChn = *it;
                cout << "  EEG Channel " << actChn << " has name " << result.at(pos) << std::endl;
                strcpy(out_ports[actChn].out_name, result.at(pos).c_str());
                strcpy(out_ports[actChn].out_desc, result.at(pos).c_str());
                pos++;
            }
        }
    }

    // collapse unused channels if necessary 
    if (!show_extrachannels) {
        int activeChannel = 0;
        for (int i = 0; i < bf_channels; i++) {
            if (strcmp(out_ports[i].out_name, "unused")) {
                channelMap[i] = activeChannel;
                cout << " Using OUT PORT " << activeChannel << " for BF-Channel " << i << std::endl;
                memcpy(&out_ports[activeChannel], &out_ports[i], sizeof(out_ports[0]));
                activeChannel++;
            }
            else {
                channelMap[i] = -1;
                cout << " skipping Channel " << i <<  std::endl;
            }
        }
    }


    // assign a new element tag (caption)
    std::string str = BoardShim::get_device_name(board_id) + "(BF)";
    char* cstr = new char[str.length() + 1];
    strcpy(tag, str.c_str());
    delete[] cstr;

    outports = channels;
    height = CON_START + outports * CON_HEIGHT + 5;

    update_samplingrate(BoardShim::get_sampling_rate(board_id));

    if (!GLOBAL.loading) update_dimensions();
    reset_oscilloscopes();

    InvalidateRect(ghWndMain, NULL, TRUE);
    InvalidateRect(ghWndDesign, NULL, TRUE);
    if (ghWndToolbox == hDlg) InvalidateRect(ghWndToolbox, NULL, FALSE);
}


void BRAINFLOWOBJ::session_reset(void)
{
}

void BRAINFLOWOBJ::session_start(void)
{
    short r;
    if ( (board != NULL) && ((filehandle == INVALID_HANDLE_VALUE) || (filemode != FILE_READING)))
    {
        try
        {
            cout << "Brainflow: start session." << std::endl;

            if (!board->is_prepared()) {
                board->prepare_session();
                cout << "Brainflow: Session prepared." << std::endl;
            }
            
            if (strlen(bfConfigString) > 0) {
                cout << "Brainflow: Send Config-String " << bfConfigString << std::endl;
                board->config_board(bfConfigString);
            }

            if (GLOBAL.brainflow_available == 0) {
                board->start_stream();
                cout << "Brainflow: Stream started." << std::endl;
            }

            GLOBAL.brainflow_available = 1;
            sync = 255;
        }
        catch (const BrainFlowException& err) {
            cout << "Brainflow: Exception handler triggered." << std::endl;
            BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
            GLOBAL.brainflow_available = 0;
            MessageBox(NULL, err.what(), "Brainflow error", MB_OK);
        }
    }
    else { 
        GLOBAL.brainflow_available = 0; 
    }
}
void BRAINFLOWOBJ::session_stop(void)
{
    GLOBAL.brainflow_available = 0;
    if (!board) return;

    try
    {
        board->stop_stream();
        cout << "Brainflow: Stream stopped." << std::endl;
        board->release_session();
        cout << "Brainflow: session released" << std::endl;
    }
    catch (const BrainFlowException& err) { cout << "Brainflow: Exception handler triggered." << std::endl;   BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); }

}

void BRAINFLOWOBJ::session_pos(long pos)
{
    if (filehandle == INVALID_HANDLE_VALUE) return;
    if (pos > filelength) pos = filelength;
    SetFilePointer(filehandle, pos * (sizeof(float)) * bf_channels, NULL, FILE_BEGIN);
}

long BRAINFLOWOBJ::session_length(void)
{
    if ((filehandle != INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
    {
        DWORD sav = SetFilePointer(filehandle, 0, NULL, FILE_CURRENT);
        filelength = SetFilePointer(filehandle, 0, NULL, FILE_END) / (sizeof(float)) / bf_channels;
        SetFilePointer(filehandle, sav, NULL, FILE_BEGIN);
        return(filelength);
    }
    return(0);
}


void process_brainflow(void) {  // to be called by timer.cpp while GLOBAL.brainflow_available == 1
    try {
        BrainFlowArray<double, 2> unprocessed_data = board->get_board_data();

        for (int i = 0; i < unprocessed_data.get_size(1); i++) {

            if (bf->syncChannel > -1) {
                int actsync = unprocessed_data.at(bf->syncChannel, i);
                bf->sync = (bf->sync + 1) % 256;
                if (bf->sync != actsync) {
                    cout << " Sync lost: expected:" << bf->sync << " but got: " << unprocessed_data.at(0, i) << std::endl;
                    GLOBAL.syncloss++;
                }
                bf->sync = actsync;
            }

            for (int c = 0; c < bf->bf_channels; c++) {
                double value = unprocessed_data.at(c, i);
                if (bf->channelMap[c] != -1) {
                    bf->pass_values(bf->channelMap[c], value);
                }
                current_channelValue[c] = value;
            }
            process_packets();
        }
    }
    catch (const BrainFlowException& err)
    {
        cout << "Brainflow: Exception handler triggered." << std::endl;
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
    }
}

void BRAINFLOWOBJ::work(void)
{
	
    DWORD dwWritten, dwRead;

    if ((filehandle != INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
    {
        ReadFile(filehandle, current_channelValue, sizeof(float) * bf_channels, &dwRead, NULL);
        if (dwRead != sizeof(float) * bf_channels) SendMessage(ghWndStatusbox, WM_COMMAND, IDC_STOPSESSION, 0);
        else
        {
            DWORD x = SetFilePointer(filehandle, 0, NULL, FILE_CURRENT);
            x = x * 1000 / filelength / TTY.bytes_per_packet;
            SetScrollPos(GetDlgItem(ghWndStatusbox, IDC_SESSIONPOS), SB_CTL, x, 1);
        }
        for (int i = 0; i < bf_channels; i++) {
            if (channelMap[i]!=-1)
                pass_values(channelMap[i], current_channelValue[i]);
        }
    }

    if ((filehandle != INVALID_HANDLE_VALUE) && (filemode == FILE_WRITING))
        WriteFile(filehandle, current_channelValue, sizeof(float) * bf_channels, &dwWritten, NULL);

//    if ((!TIMING.dialog_update) && (hDlg == ghWndToolbox))
//    {
//        InvalidateRect(hDlg, NULL, FALSE);
//    }

}

BRAINFLOWOBJ::~BRAINFLOWOBJ()
{
	// free object
    if (board != NULL) bf_releaseBoard();
    GLOBAL.brainflow_available = 0;

}

#endif
