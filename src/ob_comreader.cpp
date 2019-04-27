/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  MODULE: OB_COMREADER.H:  implementation of the ComPort Reader Element
  Authors: Chris Veigl

  The ComPort Reader can be used to open a serial port and read data from it
  Thus, external devices or Software like Mitsar Psytask can be connected and
  transfer information to a running BrainBay Session
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_comreader.h"

extern char * szBaud[];
extern DWORD  BaudTable[];



LRESULT CALLBACK ComReaderDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char szBuffer[50];
    int wPosition,t;
	COMREADEROBJ * st;
	
	st = (COMREADEROBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_COMREADER)) return(FALSE);
    
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
		
			SetDlgItemInt(hDlg,IDC_RECEIVED,st->received,0);
			SetDlgItemInt(hDlg,IDC_PROCESSED,st->processed,0);

			SetDlgItemInt(hDlg,IDC_SENT,st->sent,0);
			SetDlgItemInt(hDlg,IDC_ACTVALUE,0,0);
			SetDlgItemInt(hDlg,IDC_MINTIME,st->mintime,0);

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
						st->received=st->processed=0;
						st->inpos=st->outpos=0;

					break;

				case IDC_CONNECTED:
						CheckDlgButton(hDlg,IDC_CONNECTED, st->connected);
					break;

				case IDC_PERIOD:
					st->mintime=GetDlgItemInt(hDlg, IDC_MINTIME, 0,0);
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



BOOL COMREADEROBJ::SetupComPort(int port)
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
 	        write_logfile("COMPORT %d open failed.",port );		
			report_error(sztemp);		
			comport=sav_port;
			comdev=INVALID_HANDLE_VALUE;
			connected=FALSE;
			return FALSE;
		}

}

int COMREADEROBJ::ReadComPort(HANDLE device, unsigned char * buffer, unsigned int pos, unsigned int maxlen)
{
	DWORD dwRead;
    DWORD dwBytesTransferred,i;
    struct _COMSTAT status;
    unsigned long   etat;
	//
	if (device==INVALID_HANDLE_VALUE) return(FALSE);
	
	dwBytesTransferred = 0;
    if (ClearCommError(device, &etat, &status))
      dwBytesTransferred = status.cbInQue;
   
	if (dwBytesTransferred) 
	{
		i=0;
   		if (dwBytesTransferred+pos >= maxlen)
		{
 		    if (dwBytesTransferred > maxlen) dwBytesTransferred=maxlen;

			i=maxlen-pos;
   		    ReadFile (device, buffer+pos, i, &dwRead, 0);
			pos=0;
		}

		ReadFile (device, buffer+pos, dwBytesTransferred-i, &dwRead, 0);
	}
	return((int)dwBytesTransferred);
}

BOOL COMREADEROBJ::WriteComPort(HANDLE device, unsigned char * data, unsigned int len)
{
	DWORD dwWritten;
	if (device==INVALID_HANDLE_VALUE) return(FALSE);
	if (!WriteFile(device, data, len, &dwWritten, 0) || ((int)dwWritten!=len)) return(FALSE);
	return(TRUE);
}


BOOL COMREADEROBJ::BreakDownComPort()
{	
	connected=FALSE;
	if (comdev==INVALID_HANDLE_VALUE) return TRUE;
	if (!PurgeComm(comdev, PURGE_FLAGS))  report_error("PurgeComm failed..");
    if (!EscapeCommFunction(comdev, CLRDTR)) report_error("EscapeCommFunction failed");
    CloseHandle(comdev);
	comdev=INVALID_HANDLE_VALUE;
	return TRUE;
}


COMREADEROBJ::COMREADEROBJ(int num) : BASE_CL()
{
	inports = 1;
	outports = 1;
	width=75;
	strcpy(out_ports[0].out_name,"rcv");
	strcpy(in_ports[0].in_name,"send");

	comport=0;
	inpos=0;
	outpos=0;
    received=0;
	processed=0;
	sent=0;
	act_value=0;
	cnt=0;
	mintime=100;
	baudrate=57600;
	comdev=INVALID_HANDLE_VALUE;
	
}
	
void COMREADEROBJ::make_dialog(void) 
{    display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_COMREADER, ghWndStatusbox, (DLGPROC)ComReaderDlgHandler)); }

void COMREADEROBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("comport",P_INT,&comport);
	load_property("baudrate",P_INT,&baudrate);
	load_property("connected",P_INT,&connected);
	load_property("mintime",P_INT,&mintime);
	if (connected) 
	{ 
		connected=SetupComPort(comport);
	}

}

void COMREADEROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"comport",P_INT,&comport);
	save_property(hFile,"baudrate",P_INT,&baudrate);
	save_property(hFile,"connected",P_INT,&connected);
	save_property(hFile,"mintime",P_INT,&mintime);
}
	
void COMREADEROBJ::incoming_data(int port, float value)
{
	 if (port==0) input1=value;
}
	
void COMREADEROBJ::work(void)
{
	int i;

	if (connected) 
	{
		cnt+= 1000.0f/(float) PACKETSPERSECOND;
		if (cnt >= (float)mintime)
		{
			cnt-= (float) mintime;

			act_value=(unsigned char)((input1-(float)in_ports[0].in_min)/((float)in_ports[0].in_max-(float)in_ports[0].in_min)*256);
	        WriteComPort(comdev, &act_value, 1);			
			sent++;
		}

		// attention: data lost in case of buffer overrun ..
		i=ReadComPort(comdev,buffer, inpos,COMREADERBUFLEN);
		received+=i;
		inpos=(inpos+i) % COMREADERBUFLEN;
		if (inpos!=outpos)
		{
			float act_outvalue = (float)buffer[outpos] / 256 * ((float)out_ports[0].out_max -(float)out_ports[0].out_min) + (float)out_ports[0].out_min;
			pass_values(0,act_outvalue);
			outpos= (++outpos) % COMREADERBUFLEN;
			processed++;
		}
		//else pass_values(0,0);

		if ((hDlg==ghWndToolbox) && (!TIMING.dialog_update))
		{ 
			SetDlgItemInt(hDlg,IDC_RECEIVED,inpos,0);
			SetDlgItemInt(hDlg,IDC_PROCESSED,outpos,0);
			SetDlgItemInt(hDlg,IDC_SENT,sent,0);
			SetDlgItemInt(hDlg,IDC_ACTVALUE,act_value,0);
		}
	}
}
	
COMREADEROBJ::~COMREADEROBJ() 
{
	  BreakDownComPort();
}



