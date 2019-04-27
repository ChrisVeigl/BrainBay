/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_DELAY.CPP
  Author:  Chris Veigl


  This Object outputs the input signal, delayed by n samples

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_delay.h"

using namespace std;

DELAYOBJ::DELAYOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
    strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
    for (int i = 0; i < DELAYSAMPLES; i++)
    {
    	samples[i] = 0.0;
    }
	act_out=0;
    interval = 1;
    writepos = 0;
	added=0;
}
	
void DELAYOBJ::session_start(void)
{
	added = 0;
}
void DELAYOBJ::session_reset(void)
{
	added = 0;
}
void DELAYOBJ::session_pos(long pos)
{
	added = 0;
}

void DELAYOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_DELAYBOX, ghWndStatusbox, (DLGPROC)DelayDlgHandler));
}

void DELAYOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("interval",P_INT,&interval);
}

void DELAYOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"interval",P_INT,&interval);
}
	
void DELAYOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	{
		samples[writepos] = value;
 		added++;
		if (added > interval)
		{
		    int oldest = writepos - interval;
			if (oldest < 0)
	    		oldest += DELAYSAMPLES;
		    act_out = samples[oldest];
			added = interval;
		}
		writepos++;
		if (writepos >= DELAYSAMPLES)
    		writepos = 0;
	}
}
	
void DELAYOBJ::work(void)
{
	if (added>=interval)
	{  
		pass_values(0, act_out);
	}
}

void DELAYOBJ::change_interval(int newinterval)
{
	interval = newinterval;
	added = 0;
}

DELAYOBJ::~DELAYOBJ() {}

LRESULT CALLBACK DelayDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	DELAYOBJ * st;
	
	st = (DELAYOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_DELAY)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				SetDlgItemInt(hDlg, IDC_DELAYTIME, st->interval, FALSE);
   				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:	
				switch (LOWORD(wParam)) 
				{
					case IDC_DELAYTIME:
						//if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							st->interval=GetDlgItemInt(hDlg, IDC_DELAYTIME, NULL, 0);
							st->added=0;
						}
					break;
				}
			return TRUE;
			break;

		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}
