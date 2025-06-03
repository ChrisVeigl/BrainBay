/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
    
  MODULE: OB_LSL_READER.H:  contains the LSL Stream Reader-Object
  org arch. MODULE from : ob_tcp_receive.cpp
  lib from : https://github.com/sccn/liblsl  
  Author: Keum D.S., Chris Veigl


  This Object reads data from an LSL stream and feeds the signals to 
  the output ports.

  lib from : https://github.com/sccn/liblsl

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.
  
-----------------------------------------------------------------------------*/

#include "windows.h"
#include "brainBay.h"
#include "ob_lsl_receive.h"

int open_current_stream(LSL_RECEIVEOBJ* st)
{
	unsigned int max = 0;

	st->height = CON_START + st->outports * CON_HEIGHT + 5;
	if (max < 10) max = 0; else max -= 10;
	st->width = 70 + max * 5;

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

			//st->m_streams = lsl::resolve_streams();
			
			if (st->LSL_CONNECTED)
			{
				for (t = 0; t < st->m_streams.size(); t++)
				{
					wsprintf(szBuffer, "%s", st->m_streams[t]);
					SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
				}
			}
			
			if (st->select_stream > 0) SendDlgItemMessage( hDlg, IDC_STREAMCOMBO, CB_SETCURSEL, (WPARAM) (st->select_stream-1), 0L ) ;
			else SetDlgItemText( hDlg, IDC_STREAMCOMBO, "none") ;
			//SetDlgItemText(hDlg, IDC_STREAMCOMBO, "none");

			SetDlgItemText(hDlg,IDC_DEVICETYPE,"LSL");

			lpsi.cbSize=sizeof(SCROLLINFO);
			lpsi.fMask=SIF_RANGE|SIF_POS;
			lpsi.nMin=0; lpsi.nMax=1000;

			
			CheckDlgButton(hDlg, IDC_CONNECTED, st->LSL_CONNECTED);

			return TRUE;

		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return(TRUE);
			
		case WM_COMMAND:
            switch (LOWORD(wParam)) 
			{
				case IDC_STREAMCOMBO:
					//st->select_stream = SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_GETCURSEL, 0, 0) + 1;
					st->select_stream = SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_GETCURSEL, 0, 0) + 1;
					// KDS 240901 Get list COMBO index
					//st->m_current_stream = st->m_streams[st->select_stream];
					break;
				case IDC_REFRESH:
					
					st->m_streams = lsl::resolve_streams();  // KDS 240901 get available list

					if (st->m_streams.size() == 0)
					{
						//SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
						SetDlgItemText(hDlg, IDC_STREAMCOMBO, "none");
						st->LSL_CONNECTED = FALSE;
					}
					else
					{
						SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_RESETCONTENT, 0, 0);
						for (t = 0; t < st->m_streams.size(); t++)
						{
							wsprintf(szBuffer, "%s", st->m_streams[t]);
							SendDlgItemMessage(hDlg, IDC_STREAMCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
							// KDS 240901 ADD streams to COMBO list
						}
					}
					break;
				case IDC_CONNECT:
					
					if (st->LSL_CONNECTED) {
						    
						// KDS 240901 close stream 
						st->LSL_CONNECTED = FALSE;
						CheckDlgButton(hDlg, IDC_CONNECTED, FALSE);
						st->m_inlet->close_stream();
						st->select_stream = 0;
						st->state = 0;
						delete st->m_inlet;

						//lsl_destroy_inlet(st->m_inlet);
					} else {
						if (st->select_stream > 0)
						{
							st->m_inlet = new lsl::stream_inlet(st->m_streams[st->select_stream-1]);
							st->m_inlet->open_stream();
							st->outports = st->m_inlet->info().channel_count();

							open_current_stream(st);

							st->LSL_CONNECTED = TRUE;
						} else
						{ 
							st->LSL_CONNECTED = FALSE;
						}
						CheckDlgButton(hDlg, IDC_CONNECTED, st->LSL_CONNECTED);
					}

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
	LSL_CONNECTED = FALSE;

}

void LSL_RECEIVEOBJ::session_start(void) 
{	
	state = 1;
}

void LSL_RECEIVEOBJ::session_stop(void) 
{	
    if (m_inlet) state = 0;
 
}

void LSL_RECEIVEOBJ::session_reset(void) 
{	
    state = 0;
    if (m_inlet)
    { 
 
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
    std::vector<float>   sample;
    double timestamp;

	if ((outports == 0) || (state == 0) || (m_inlet == NULL)) return;
	if (!LSL_CONNECTED) return;
	timestamp = m_inlet->pull_sample(sample);
    
	// Receive a sample from the LSL stream
	for (x = 0; x < outports; x++)
    {
            pass_values(x, sample[x]);
    }
}

LSL_RECEIVEOBJ::~LSL_RECEIVEOBJ()
{	
    
	if (m_inlet)
    {
		//delete m_inlet;
    } 
}
