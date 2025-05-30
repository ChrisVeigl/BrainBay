/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_LSL_READER.CPP:  contains the LSL Stream Reader-Object
  Author: Chris Veigl


  This Object reads data from an LSL stream and feeds the signals to 
  the output ports.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "windows.h"
#include "brainBay.h"
#include "ob_lsl_receive.h"

int open_textarchive(LSL_RECEIVEOBJ * st)
{
	char tmp1[256],tmp[2];
	int i,op=0;
	unsigned int max=0;
	DWORD dwRead;

    /*
	st->file=CreateFile(st->filename, GENERIC_READ,  FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	 if (st->file==INVALID_HANDLE_VALUE) 
	 {
		 strcpy(st->filename,"none");
		 st->state=0;
		 return(0);
	 }

	 i=0;
	 if (st->format<5)
	 {
		 do 
		 {
			 tmp[0]=0;
 			 ReadFile(st->file,tmp,1,&dwRead,NULL);
		 }  while ((dwRead!=0) && (tmp[0]!=10));

		 i=0;tmp[1]=0;tmp1[0]=0;
		 do 
		 {
			 tmp[0]=0;
 			 ReadFile(st->file,tmp,1,&dwRead,NULL);
			 if ((tmp[0] != 13) && (tmp[0] != 10) && (tmp[0] != 9) && (tmp[0] != ','))
			 {
				 if (strlen(tmp1) < 250) strcat(tmp1, tmp);
			 }
			 else if ((tmp[0]==9)||(tmp[0]==',')||(tmp[0]==13))
			 { 
				 if (strlen(tmp1)>19) tmp1[19]=0;
				 if (max<strlen(tmp1)) max=strlen(tmp1);

				 // strcpy(st->out_ports[op].out_name,tmp1);
				 // strcpy(st->out_ports[op++].out_desc,tmp1);

				 op++;
				 sprintf(st->out_ports[op].out_name, "chn%d", op);
				 sprintf(st->out_ports[op].out_desc, "chn%d", op);
				 tmp1[0]=0;
			 }
		 }  while ((dwRead!=0) && (tmp[0]!=10));
		 if (op>0) st->outports=op;
	 }
	 else
	 {
		 st->outports=1;
		 strcpy(st->out_ports[0].out_name,"chn1");
	 }

	 st->height=CON_START+st->outports*CON_HEIGHT+5;
	 if (max<10) max=0; else max-=10;
	 st->width=70+max*5;
	 */
	 return(1);
}


LRESULT CALLBACK LSLReceiveDlgHandler( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )    // KDS 240829  for diaglog
{
	char szBuffer[ 20 ], strfloat[21];
    WORD wPosition;
	int t;
	
	LSL_RECEIVEOBJ * st;
	static int actchn;
	
	st = (LSL_RECEIVEOBJ *) actobject;
    if ((st==NULL)||(st->type!= OB_LSL_RECEIVER)) return(FALSE);
 

	switch( message )
	{
		case WM_INITDIALOG:
		
			SCROLLINFO lpsi;

			st->m_streams = lsl::resolve_streams();

			for (t = 0; t < st->m_streams.size(); t++)
			{
				wsprintf(szBuffer, "%s", st->m_streams[t]);
				SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
			}
			/*
		    for (t = 0; t < MAX_COMPORT; t++) 
			{
				wsprintf( szBuffer, "COM%d", t + 1 ) ;
				SendDlgItemMessage( hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0,(LPARAM) (LPSTR) szBuffer ) ;
			}
			*/
			//if (TTY.PORT) SendDlgItemMessage( hDlg, IDC_STREAMCOMBO, CB_SETCURSEL, (WPARAM) (TTY.PORT - 1), 0L ) ;
			//else SetDlgItemText( hDlg, IDC_STREAMCOMBO, "none") ;
			SetDlgItemText(hDlg, IDC_STREAMCOMBO, "none");

			SetDlgItemText(hDlg,IDC_DEVICETYPE,devicetypes[TTY.devicetype]);

			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=0; lpsi.nMax=1000;

			CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);

			return TRUE;

		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return(TRUE);
			
		case WM_COMMAND:
            switch (LOWORD(wParam)) 
			{
				case IDC_STREAMCOMBO:
					st->select_stream = SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_GETCURSEL, 0, 0) + 1;
					//st->m_current_stream = st->m_streams[st->select_stream];
					break;
				case IDC_REFRESH:
					st->m_streams = lsl::resolve_streams();

					for (t = 0; t < st->m_streams.size(); t++)
					{
						wsprintf(szBuffer, "%s", st->m_streams[t]);
						SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
					}
					break;
				case IDC_CONNECT:
					/*
					if (TTY.CONNECTED) {
							if (TTY.devicetype == DEV_NIA){		
								DisconnectNIA(); 
								TTY.CONNECTED=FALSE;			
							} else {
								BreakDownCommPort();
							}
							CheckDlgButton(hDlg, IDC_CONNECTED, FALSE);
						} else {
							if (TTY.devicetype == DEV_NIA){		
								if (TTY.COMDEV!=INVALID_HANDLE_VALUE)
									DisconnectNIA();								
								if ((TTY.CONNECTED = ConnectNIA(hDlg)) == FALSE) {
									report_error("No NIA found!");
									break;
								}
							} else {
								if (TTY.COMDEV!=INVALID_HANDLE_VALUE) 
									BreakDownCommPort();								
								TTY.CONNECTED=SetupCommPort(TTY.PORT);
								if ((TTY.COMDEV!=INVALID_HANDLE_VALUE) && (TTY.devicetype==DEV_IBVA))
								{
									write_string_to_comport("SR 256\r");
								}
							}
							CheckDlgButton(hDlg, IDC_CONNECTED, TTY.CONNECTED);
						}
						*/
						
					break;
			}	
			return(TRUE);


		case WM_SIZE:
		case WM_MOVE:  
				update_toolbox_position(hDlg);
				return(TRUE);
	}
    return FALSE;
} 


//
//  Object Implementation
//

LSL_RECEIVEOBJ::LSL_RECEIVEOBJ(int num) : BASE_CL()	
{
    outports = 1;
    inports = 0;
    width = 70;
    height = 50;
    samplecount = 0;

    state = 0;
    inlet = NULL;
    strcpy(stream_name, "none");
}

void LSL_RECEIVEOBJ::session_start(void) 
{	
    if (inlet && outports > 0)
    {
        state = 1;
        if (hDlg == ghWndToolbox)
        {
            SetDlgItemText(hDlg, IDC_FILESTATUS, "Reading LSL Stream");
            EnableWindow(GetDlgItem(hDlg, IDC_START), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_STOP), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), FALSE);
        }
    }
}

void LSL_RECEIVEOBJ::session_stop(void) 
{	
    if (inlet) state = 0;
    if (hDlg == ghWndToolbox)
    {
        SetDlgItemText(hDlg, IDC_FILESTATUS, "Stopped");
        EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
    }
}

void LSL_RECEIVEOBJ::session_reset(void) 
{	
    state = 0;
    if (inlet)
    { 
        if (hDlg == ghWndToolbox)
        {  
            SetDlgItemText(hDlg, IDC_FILESTATUS, "Stopped");
            EnableWindow(GetDlgItem(hDlg, IDC_START), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_OPENFILE), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CLOSE), TRUE);
        } 
    }	
}

void LSL_RECEIVEOBJ::make_dialog(void) 
{  
    display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_LSLBOX, ghWndStatusbox, (DLGPROC)LSLReceiveDlgHandler)); 

}

void LSL_RECEIVEOBJ::load(HANDLE hFile) 
{
    load_object_basics(this);
    load_property("stream_name", P_STRING, &stream_name);
    height = CON_START + outports * CON_HEIGHT + 5;

    // TODO: Code to open the LSL stream using stream_name
}

void LSL_RECEIVEOBJ::save(HANDLE hFile) 
{
    save_object_basics(hFile, this);
    save_property(hFile, "stream_name", P_STRING, &stream_name);
}

void LSL_RECEIVEOBJ::work(void) 
{
    int x;
    float sample[1];
    int32_t timestamp;
    
    if ((outports == 0) || (state == 0) || (inlet == NULL)) return;

    // Receive a sample from the LSL stream
	/*
    if (lsl_pull_sample_f(inlet, sample, 1, 0, &timestamp) > 0)
    {
        for (x = 0; x < outports; x++)
        {
            pass_values(x, sample[0]);
        }
    } */
}

LSL_RECEIVEOBJ::~LSL_RECEIVEOBJ()
{	
    /*
	if (inlet)
    {
        lsl_destroy_inlet(inlet);
    } */
}
