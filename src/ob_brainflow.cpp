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
#include "ob_brainflow.h"


#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"

#define BOARD_ID_OFFSET 3   // Board IDs start at -3 (!) 

// see https://brainflow.readthedocs.io/en/stable/SupportedBoards.html
// and https://brainflow.readthedocs.io/en/stable/UserAPI.html

char supported_boards[][40] = { "PLAYBACK_FILE_BOARD", "STREAMING_BOARD", "SYNTHETIC_BOARD",
    "CYTON_BOARD", "GANGLION_BOARD", "CYTON_DAISY_BOARD","GALEA_BOARD","GANGLION_WIFI_BOARD","CYTON_WIFI_BOARD","CYTON_DAISY_WIFI_BOARD",
    "BRAINBIT_BOARD", "UNICORN_BOARD", "CALLIBRI_EEG_BOARD", "CALLIBRI_EMG_BOARD", "CALLIBRI_ECG_BOARD",
    "FASCIA_BOARD", "NOTION_OSC1_BOARD", "NOTION_2_BOARD", "IRONBCI_BOARD", "GFORCE_PRO_BOARD",
    "FREEEEG32_BOARD", "BRAINBIT_BLED_BOARD","GFORCE_DUAL_BOARD", "GALEA_SERIAL_BOARD",
    "MUSE_S_BLED_BOARD", "MUSE_2_BLED_BOARD","CROWN_BOARD","ANT_NEURO_EE_410_BOARD", "ANT_NEURO_EE_411_BOARD",
    "ANT_NEURO_EE_430_BOARD", "ANT_NEURO_EE_211_BOARD","ANT_NEURO_EE_212_BOARD","ANT_NEURO_EE_213_BOARD",
    "ANT_NEURO_EE_214_BOARD", "ANT_NEURO_EE_215_BOARD", "ANT_NEURO_EE_221_BOARD", "ANT_NEURO_EE_222_BOARD",
    "ANT_NEURO_EE_223_BOARD",  "ANT_NEURO_EE_224_BOARD","ANT_NEURO_EE_225_BOARD",
    "ENOPHONE_BOARD", "" };


using namespace std;

BoardShim* board = NULL;
struct BrainFlowInputParams params;

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

    BoardShim::enable_dev_board_logger();

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
    params.ip_port = st->ipport;

}


/* Read parameters from given file to new device context.
    The function returns device context (>=0), and on error it returns negative
    value. */
int ReadCfgFile(BRAINFLOWOBJ* st, const char* fname)
{
    DWORD len, n;
    char* buf;
    HANDLE cf;
    int r = 0;

    cf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (cf == INVALID_HANDLE_VALUE)
        return -1;
    if ((len = GetFileSize(cf, NULL)) == -1)
        r = -2;
    else if (!(buf = (char*)malloc(len)))
        r = -3;
    else if (!ReadFile(cf, buf, len, &n, NULL) || len != n)
        r = -4;
   
    CloseHandle(cf);
    if (buf)
        free(buf);
    return r;
}

/* Write parameters of given device context to given file.
    The function returns number of written bytes; on error (e.g. too small buffer)
    it returns 0. */
int WriteCfgFile( const char* fname)
{
    DWORD len, n = 0;
    char* buf;
    HANDLE cf;
    int r = 0;

    cf = CreateFile(fname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    return n;
}

int prepare_fileRead(BRAINFLOWOBJ* st) {

    st->filehandle = CreateFile(st->archivefile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (st->filehandle == INVALID_HANDLE_VALUE) {
        st->filemode = 0;
        return(0);
    }

    get_session_length();
    GLOBAL.neurobit_available = 0;
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
        (st->board_id == 17) || (st->board_id == 18) || (st->board_id == 21) || (st->board_id == 22)) {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_SERIALPORT), TRUE);
    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_BF_SERIALPORT), FALSE);
    }

    if (st->board_id == 37) {
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
        while (strlen(supported_boards[i]) > 0)  {
        SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)supported_boards[i]); i++; }

        SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_SETCURSEL, st->board_id + BOARD_ID_OFFSET, 0L);
        SetDlgItemText(hDlg, IDC_BF_DEVICECOMBO, supported_boards[st->board_id + BOARD_ID_OFFSET]);

        SetDlgItemText(hDlg, IDC_BF_SERIALPORT, st->serialport);
        SetDlgItemText(hDlg, IDC_BF_MACADDRESS, st->macaddress);
        SetDlgItemText(hDlg, IDC_BF_IPADDRESS, st->ipaddress);
        SetDlgItemInt(hDlg, IDC_BF_IPPORT, st->ipport,FALSE);

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
                int sel = SendDlgItemMessage(hDlg, IDC_BF_DEVICECOMBO, CB_GETCURSEL, 0, 0) - BOARD_ID_OFFSET;  // this is quite hacky ... fits device name/selection to board IDs!

                if (sel < -1) {
                    MessageBox(NULL, "The Playback- and Streaming- board types are not supported by now, please select another board/device ...", "Information", MB_OK);
                    return true;
                }
                st->board_id = sel;
                cout << "Brainflow: Device ID selected: " << st->board_id << std::endl;

                bf_createBoard(st->board_id);
                st->update_channelinfo();
                updateDialog(hDlg, st);
                // InvalidateRect(hDlg,NULL,FALSE);
            }
            break;

        case IDC_BF_APPLY_DEVICE:
            GetDlgItemText(hDlg, IDC_BF_SERIALPORT, st->serialport, sizeof(st->serialport - 1));
            GetDlgItemText(hDlg, IDC_BF_IPADDRESS, st->ipaddress, sizeof(st->ipaddress - 1));
            GetDlgItemInt(hDlg, IDC_BF_IPPORT, &st->ipport, false);
            GetDlgItemText(hDlg, IDC_BF_MACADDRESS, st->macaddress, sizeof(st->macaddress - 1));
            bf_setparams(st);
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

    strcpy(serialport, "COM4");
    strcpy(ipaddress, "192.168.4.1");
    ipport = 4567;
    strcpy(macaddress, "00:11:22:33:FF:EE");
    strcpy(archivefile, "none");

    board_id = -1;  // default: SYNTHETIC_BOARD, id -1
    sync = -1;      // first sync packet number will be 0

    bf_createBoard(board_id);
    update_channelinfo();
    
    bf = this;
}
void BRAINFLOWOBJ::make_dialog(void)
{
	display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_BRAINFLOWBOX, ghWndStatusbox, (DLGPROC)BrainflowDlgHandler));
}
void BRAINFLOWOBJ::load(HANDLE hFile)
{
	load_object_basics(this);
    load_property("archivefile", P_STRING, archivefile);
    load_property("board_id", P_INT, &board_id);

    bf_createBoard(board_id);
    update_channelinfo();
    bf_setparams(this);
}

void BRAINFLOWOBJ::save(HANDLE hFile)
{
	save_object_basics(hFile, this);
    save_property(hFile, "archivefile", P_STRING, archivefile);
    save_property(hFile, "board_id", P_INT, &board_id);
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

    json desc = BoardShim::get_board_descr(board_id);

    // for (auto it = desc.begin(); it != desc.end(); it++) {
    //    std::cout << "key: " << it.key() << " : " << it.value() << std::endl;
    // }

    // auto it_eeg = desc.find("eeg_channels");
    // std::cout << std::boolalpha;
    // std::cout << "eeg_channels was found: " << (it_eeg != desc.end());
    // std::cout << ", value: " << *it_eeg << '\n';


    try {
        cout << "Update channels for Device ID " << board_id;
        cout << ", device name = " << BoardShim::get_device_name(board_id) << std::endl;
        cout << "  Board description = " << BoardShim::get_board_descr(board_id) << std::endl;
        cout << "  the package number is at channel " << BoardShim::get_package_num_channel(board_id) << std::endl;
        cout << "  the sampling rate is " << BoardShim::get_sampling_rate(board_id) << std::endl;
        cout << "  available channels are as follows:" << std::endl;
    }
    catch (const BrainFlowException& err) { cout << "Brainflow: Exception handler triggered." << std::endl;   BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); }

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
        if (desc.find("other_channels") != desc.end()) channels += add_channels(BoardShim::get_other_channels(board_id), "other");

        if (desc.find("package_num_channel") != desc.end()) {
            std::vector<int> vec;
            vec.push_back(BoardShim::get_package_num_channel(board_id));
            channels += add_channels(vec, "pcount");
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
    catch (const BrainFlowException& err) { 
        cout << "Brainflow: Exception handler triggered." << std::endl;
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); 
    }

    cout << "Added total number of " << channels << " data channels!" << std::endl;

    char tmp[40];
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
    if ((filehandle == INVALID_HANDLE_VALUE) || (filemode != FILE_READING))
    {

    }
    else { 
        //GLOBAL.neurobit_available = 0; 
    }

    if (!board) return;

    try
    {
        cout << "Brainflow: start session." << std::endl;

        board->prepare_session();
        cout << "Brainflow: Session prepared." << std::endl;

        board->start_stream();
        cout << "Brainflow: Stream started." << std::endl;

        GLOBAL.brainflow_available = 1;
    }
    catch (const BrainFlowException& err) { 
        cout << "Brainflow: Exception handler triggered." << std::endl;   
        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what()); 
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
    SetFilePointer(filehandle, pos * (sizeof(float)) * 4, NULL, FILE_BEGIN);
}

long BRAINFLOWOBJ::session_length(void)
{
    if ((filehandle != INVALID_HANDLE_VALUE) && (filemode == FILE_READING))
    {
        DWORD sav = SetFilePointer(filehandle, 0, NULL, FILE_CURRENT);
        filelength = SetFilePointer(filehandle, 0, NULL, FILE_END) / (sizeof(float)) / 4;
        SetFilePointer(filehandle, sav, NULL, FILE_BEGIN);
        return(filelength);
    }
    return(0);
}


void process_brainflow(void) {  // to be called by timer.cpp while GLOBAL.brainflow_available == 1
    try {
        BrainFlowArray<double, 2> unprocessed_data = board->get_board_data();

        for (int i = 0; i < unprocessed_data.get_size(1); i++) {

            int actsync = unprocessed_data.at(0, i);
            bf->sync = (bf->sync + 1) % 256;
            if (bf->sync != actsync) {
                cout << " Sync lost: expected:" << bf->sync << " but got: " << unprocessed_data.at(0, i) << std::endl;
            }
            bf->sync = actsync;

            for (int c = 0; c < bf->channels; c++) {
                // cout << "\tchn " << c << ":" << unprocessed_data.at(c, i); cout << std::endl;
                bf->pass_values(c, unprocessed_data.at(c, i));
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
	
	if ((!TIMING.dialog_update) && (hDlg == ghWndToolbox))
	{
	}


}

BRAINFLOWOBJ::~BRAINFLOWOBJ()
{
	// free object
    if (board != NULL) bf_releaseBoard();
    GLOBAL.brainflow_available = 0;

}




/*
bool parse_args(int argc, char* argv[], struct BrainFlowInputParams* params, int* board_id)
{
    bool board_id_found = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == std::string("--board-id"))
        {
            if (i + 1 < argc)
            {
                i++;
                board_id_found = true;
                *board_id = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_address = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_port = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--serial-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_port = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-protocol"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_protocol = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--timeout"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->timeout = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--other-info"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->other_info = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--mac-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->mac_address = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--serial-number"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_number = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--file"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->file = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
    }
    if (!board_id_found)
    {
        std::cerr << "board id is not provided" << std::endl;
        return false;
    }
    return true;
}

*/