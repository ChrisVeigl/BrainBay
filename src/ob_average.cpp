/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_AVERAGE.CPP
  Author:  Chris Veigl


  This Object outputs the Average of n Samples captured from it's input-port

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_average.h"
#include <iostream>

using namespace std;

AVERAGEOBJ::AVERAGEOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	width=75;
    strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
    accumulator = 0.0;
    for (int i = 0; i < AVGSAMPLES; i++)
    {
    	samples[i] = 0.0;
    }
    interval = 1;
    writepos = 0;
    added = 0;
}
	
void AVERAGEOBJ::session_start(void)
{
  //	added = 0;
  //	accumulator = 0;
}
void AVERAGEOBJ::session_reset(void)
{
	added = 0;
	accumulator = 0;
}

void AVERAGEOBJ::session_pos(long pos)
{
	added = 0;
	accumulator = 0;
}

void AVERAGEOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_AVERAGEBOX, ghWndStatusbox, (DLGPROC)AverageDlgHandler));
}

void AVERAGEOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("interval",P_INT,&interval);
}

void AVERAGEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"interval",P_INT,&interval);
}
	
void AVERAGEOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	{
   		accumulator += value;
		added++;

		if (interval) 
		{
			samples[writepos] = value;
			if (added > interval)
			{
				int oldest = writepos - interval;
				if (oldest < 0)
	    			oldest += AVGSAMPLES;
			    accumulator -= samples[oldest];
				added = interval;
			}
			writepos++;
			if (writepos >= AVGSAMPLES)
    			writepos = 0;
		}
	}
}
	
void AVERAGEOBJ::work(void)
{
    float average;
	if (added)
	{  
		average = accumulator / added;
		pass_values(0, average);
	}
}

void AVERAGEOBJ::change_interval(int newinterval)
{
	interval = newinterval;
	added = 0;
	accumulator = 0;
}

AVERAGEOBJ::~AVERAGEOBJ() {}

LRESULT CALLBACK AverageDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	AVERAGEOBJ * st;
	
	st = (AVERAGEOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_AVERAGE)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:

				SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=0; lpsi.nMax=AVGSAMPLES - 1;
				SetScrollInfo(GetDlgItem(hDlg,IDC_AVERAGEINTERVALBAR),SB_CTL,&lpsi, TRUE);
				
				init = true;

				SetScrollPos(GetDlgItem(hDlg,IDC_AVERAGEINTERVALBAR), SB_CTL,st->interval, TRUE);
				SetDlgItemInt(hDlg, IDC_AVERAGEINTERVAL, st->interval, FALSE);
                
				init = false;
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if (!init && (nNewPos = get_scrollpos(wParam,lParam)) >= 0)
			{   
				if (lParam == (long) GetDlgItem(hDlg,IDC_AVERAGEINTERVALBAR))  
				{
					SetDlgItemInt(hDlg, IDC_AVERAGEINTERVAL, nNewPos, TRUE);
                    st->change_interval(nNewPos);
				}
			}
			break;
		}
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}
