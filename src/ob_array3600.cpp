/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_ARRAY3600.H:  declarations for the ARRAY Power Supply Controller
  Authors: Chris Veigl, Gerhard Nussbaum

  The ARRAY Power Supply Controller Object can be used to write Bytes to an
  Array 3600 remote controllable Power Supply via a Com-Port.
  Maximum values for Voltage, Current and Power can be specified,
  Voltage can be adjusted periodically according to the element's input values.
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_array3600.h"

extern char * szBaud[];
extern DWORD  BaudTable[];



LRESULT CALLBACK Array3600DlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szBuffer[50];
    int wPosition,t;
	ARRAY3600OBJ * st;
	
	st = (ARRAY3600OBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_ARRAY3600)) return(FALSE);
    
	switch( message )
	{
		case WM_INITDIALOG:
	
			for (t = 0; t < MAX_COMPORT; t++) 
			{
				wsprintf( szBuffer, "COM%d", t + 1 ) ;
				SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) szBuffer ) ;
			}
			if (st->comport) SendDlgItemMessage( hDlg, IDC_PORTCOMBO, CB_SETCURSEL, (WPARAM) (st->comport - 1), 0L ) ;
			else SetDlgItemText( hDlg, IDC_PORTCOMBO, "none") ;
			for (t = 0; BaudTable[t]!=0 ; t++) 
			{
				wPosition = LOWORD( SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_ADDSTRING, 0, (LPARAM) (LPSTR) szBaud[t] ) ) ;
				SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_SETITEMDATA, (WPARAM) wPosition, (LPARAM) BaudTable[t]) ;
				if (BaudTable[t] == st->baudrate) SendDlgItemMessage( hDlg, IDC_BAUDCOMBO, CB_SETCURSEL, (WPARAM) wPosition, 0L ) ;
			}

			if (st->comdev==INVALID_HANDLE_VALUE)
				CheckDlgButton(hDlg, IDC_CONNECTED, FALSE);
			else CheckDlgButton(hDlg, IDC_CONNECTED, TRUE);
			
			SetDlgItemInt(hDlg,IDC_MAXVOLTAGE,st->maxvoltage,0);
			SetDlgItemInt(hDlg,IDC_MAXCURRENT,st->maxcurrent,0);
			SetDlgItemInt(hDlg,IDC_MAXPOWER,st->maxpower,0);
			SetDlgItemInt(hDlg,IDC_VOLTAGE,st->voltage,0);
			SetDlgItemInt(hDlg,IDC_ADDRESS,st->address,0);
			SetDlgItemInt(hDlg,IDC_PERIOD,st->period,0);

			CheckDlgButton(hDlg, IDC_PERIODIC, st->periodic);
	        break;

		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_PORTCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{
						st->comport=SendDlgItemMessage(hDlg, IDC_PORTCOMBO, CB_GETCURSEL, 0, 0 )+1 ;
						st->BreakDownComPort(); 
						CheckDlgButton(hDlg,IDC_CONNECTED,FALSE);
					}
					break;
				case IDC_BAUDCOMBO:
					if (HIWORD(wParam)==CBN_SELCHANGE)
					{   int sel;
					    
						sel=SendDlgItemMessage(hDlg, IDC_BAUDCOMBO, CB_GETCURSEL, 0, 0 ) ;
						st->baudrate=BaudTable[sel];
						if (st->comdev!=INVALID_HANDLE_VALUE)
						{
						    st->BreakDownComPort(); 
							st->connected=st->SetupComPort(st->comport);
							CheckDlgButton(hDlg, IDC_CONNECTED, st->connected);
						}
					}
					break;
					case IDC_CONNECT:
						if (st->connected)
						{
							st->BreakDownComPort();
							CheckDlgButton(hDlg, IDC_CONNECTED, FALSE);
						}
						else
						{
							st->connected=st->SetupComPort(st->comport);
							CheckDlgButton(hDlg, IDC_CONNECTED, st->connected);
						}
					break;

				case IDC_CONNECTED:
						CheckDlgButton(hDlg,IDC_CONNECTED, st->connected);
					break;

				case IDC_SETNOW:
					  st->setParameters(st->address, st->maxcurrent, st->maxvoltage, st->maxpower, st->voltage);
                    break;

				case IDC_CONTROL_ON:
					  st->setToPcControlOn();
					break;
				case IDC_CONTROL_OFF:
					  st->setToPcControlOff();
					break;
				case IDC_PERIODIC:
					st->periodic=IsDlgButtonChecked(hDlg, IDC_PERIODIC);
					break;
				case IDC_PERIOD:
					st->period=GetDlgItemInt(hDlg, IDC_PERIOD, 0,0);
					break;
				case IDC_MAXVOLTAGE:
					st->maxvoltage=GetDlgItemInt(hDlg, IDC_MAXVOLTAGE, 0,0);
					break;
				case IDC_MAXCURRENT:
					st->maxcurrent=GetDlgItemInt(hDlg, IDC_MAXCURRENT, 0,0);
					break;
				case IDC_MAXPOWER:
					st->maxpower=GetDlgItemInt(hDlg, IDC_MAXPOWER, 0,0);
					break;
				case IDC_VOLTAGE:
					st->voltage=GetDlgItemInt(hDlg, IDC_VOLTAGE, 0,0);
					break;
				case IDC_ADDRESS:
					st->address=GetDlgItemInt(hDlg, IDC_ADDRESS, 0,0);
					break;
            }
			return TRUE;
			break;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
	}
	return FALSE;
}


byte ARRAY3600OBJ::getCheckByte(byte * command) {
        int cb=0;
        for (byte i=0; i < 25; i++) {
                cb += (int) command[i];
        }
        //cb = cb >> 8;
        return (byte)(cb);
}

byte ARRAY3600OBJ::getHighByte(int val) {
        return (byte)(val >> 8);
}

byte ARRAY3600OBJ::getLowByte(int val) {
        return (byte)(val);
}


void ARRAY3600OBJ::setToSelfControl() {
        byte command[] = { 0xAA, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C };
        WriteComPort(comdev, command, 26);
}

void ARRAY3600OBJ::setToPcControlOn() {
        byte command[] = { 0xAA, 0x00, 0x82, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F };
        command[25] = getCheckByte(command);
        WriteComPort(comdev, command, 26);
}

void ARRAY3600OBJ::setToPcControlOff() {
        byte command[] = { 0xAA, 0x00, 0x82, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E };
        command[25] = getCheckByte(command);
        WriteComPort(comdev, command, 26);
}

void ARRAY3600OBJ::setParameters(byte address, int maxI, int maxU, int maxP, int u) {
        byte command[] = { 0xAA, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        command[1] = address;
        command[3] = getLowByte(maxI);
        command[4] = getHighByte(maxI);
        command[5] = getLowByte(maxU);
        command[6] = getHighByte(maxU);
        command[9] = getLowByte(maxP);
        command[10] = getHighByte(maxP);
        command[11] = getLowByte(u);
        command[12] = getHighByte(u);
        command[25] = getCheckByte(command);
		WriteComPort(comdev, command, 26);
}



BOOL ARRAY3600OBJ::SetupComPort(int port)
{	
	connected=FALSE;
	int sav_port;
    DCB dcb = {0};
	char PORTNAME[10];

	sav_port=comport;
    BreakDownComPort();
	comport= port ;

	// set tty structure for the specified port	
	if (!port) goto failed;
	sprintf(PORTNAME,"\\\\.\\COM%d",port);
    comdev = CreateFile( PORTNAME,GENERIC_READ | GENERIC_WRITE, 
                  0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

    if (comdev == INVALID_HANDLE_VALUE) goto failed;  
    if (!GetCommState(comdev, &dcb))      // get current DCB settings
	{ report_error("GetCommState");goto failed; }

    // update DCB rate, byte size, parity, and stop bits size
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = baudrate;
    dcb.ByteSize = 8;
	dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
	dcb.EvtChar = '\0';
    // update flow control settings
    dcb.fDtrControl     =  DTR_CONTROL_ENABLE;
    dcb.fRtsControl     =  RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow    = FALSE;
    dcb.fOutxDsrFlow    = FALSE;
    dcb.fDsrSensitivity = FALSE;;
    dcb.fOutX           = FALSE;
    dcb.fInX            = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.XonChar         = 0;
    dcb.XoffChar        = 0;
    dcb.XonLim          = 0;
    dcb.XoffLim         = 0;
    dcb.fParity = FALSE; //TRUE;
    if (!SetCommState(comdev, &dcb))     // set new state
	{ report_error("SetCommState failed"); goto failed;}
    if (!SetupComm(comdev, 1024, 1024))  // set comm buffer sizes
	{  report_error("SetupComm failed");goto failed;}
    if (!EscapeCommFunction(comdev, SETDTR))        // raise DTR
	{  report_error("EscapeCommFunction failed");goto failed;}
	SetCommMask (comdev, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

	return(TRUE);

failed:
		{  
			char sztemp[100];
			sprintf(sztemp, "The Port COM%d is not available. Please select another Com-Port.",port);
 	        write_logfile("COMPORT %d open failed.",port);		
			report_error(sztemp);		
			comport=sav_port;
			comdev=INVALID_HANDLE_VALUE;
			connected=FALSE;
			return FALSE;
		}

}

BOOL ARRAY3600OBJ::WriteComPort(HANDLE device, unsigned char * data, unsigned int len)
{
	DWORD dwWritten;
	if (device==INVALID_HANDLE_VALUE) return(FALSE);
	if (!WriteFile(device, data, len, &dwWritten, 0) || ((int)dwWritten!=len)) return(FALSE);
	return(TRUE);
}

BOOL ARRAY3600OBJ::BreakDownComPort()
{	
	connected=FALSE;
	if (comdev==INVALID_HANDLE_VALUE) return TRUE;
	if (!PurgeComm(comdev, PURGE_FLAGS))  report_error("PurgeComm failed..");
    if (!EscapeCommFunction(comdev, CLRDTR)) report_error("EscapeCommFunction failed");
    CloseHandle(comdev);
	comdev=INVALID_HANDLE_VALUE;
	return TRUE;
}


ARRAY3600OBJ::ARRAY3600OBJ(int num) : BASE_CL()
{
	inports = 1;
	width=75;
	strcpy(in_ports[0].in_name,"voltage");

	input1 = INVALID_VALUE;
	comport=0;
	baudrate=38400;
	periodic=FALSE;
	comdev=INVALID_HANDLE_VALUE;
	cnt=0;
	address=1;
	voltage=0;
	maxvoltage=10000;
	maxcurrent=150;
	maxpower=10000;
	period=50;
	
}
	
void ARRAY3600OBJ::make_dialog(void) 
{    display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_ARRAY3600, ghWndStatusbox, (DLGPROC)Array3600DlgHandler)); }

void ARRAY3600OBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("comport",P_INT,&comport);
	load_property("baudrate",P_INT,&baudrate);
	load_property("connected",P_INT,&connected);
	load_property("periodic",P_INT,&periodic);
	load_property("period",P_INT,&period);
	load_property("maxvoltage",P_INT,&maxvoltage);
	load_property("maxcurrent",P_INT,&maxcurrent);
	load_property("maxpower",P_INT,&maxpower);
	load_property("address",P_INT,&address);
	load_property("voltage",P_INT,&voltage);
	if (connected) 
	{ 
		connected=SetupComPort(comport);
	    if (connected) setToPcControlOn(); 
	}

}

void ARRAY3600OBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"comport",P_INT,&comport);
	save_property(hFile,"baudrate",P_INT,&baudrate);
	save_property(hFile,"connected",P_INT,&connected);
	save_property(hFile,"periodic",P_INT,&periodic);
	save_property(hFile,"period",P_INT,&period);
	save_property(hFile,"maxvoltage",P_INT,&maxvoltage);
	save_property(hFile,"maxcurrent",P_INT,&maxcurrent);
	save_property(hFile,"maxpower",P_INT,&maxpower);
	save_property(hFile,"address",P_INT,&address);
	save_property(hFile,"voltage",P_INT,&voltage);
}
	
void ARRAY3600OBJ::incoming_data(int port, float value)
{
	if (port==0) input1=value;
}
	
void ARRAY3600OBJ::work(void)
{
	if (periodic) 
	{
		cnt+= 1000.0f/(float) PACKETSPERSECOND;
		if (cnt >= (float)period)
		{
			cnt-= (float) period;
			voltage=(int)((input1-(float)in_ports[0].in_min)/((float)in_ports[0].in_max-(float)in_ports[0].in_min)*maxvoltage);
			setParameters(address, maxcurrent, maxvoltage, maxpower, voltage);
			
			if (hDlg==ghWndToolbox)
 				SetDlgItemInt(hDlg,IDC_VOLTAGE,voltage,0);
			
		}
	}
	
	

}
	
ARRAY3600OBJ::~ARRAY3600OBJ() 
{
	  setToPcControlOff();
	  setToSelfControl();
	  BreakDownComPort();
}



