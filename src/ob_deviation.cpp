/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_DEVIATION.CPP
  Author:  Chris Veigl


  This Object outputs the Standard Deviation and Mean of n 
  Samples captured from it's input-port

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_deviation.h"



LRESULT CALLBACK DeviationDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	DEVIATIONOBJ * st;
	
	st = (DEVIATIONOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_DEVIATION)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:

				SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=1; lpsi.nMax=NUMSAMPLES - 1;
				SetScrollInfo(GetDlgItem(hDlg,IDC_DEVIATIONINTERVALBAR),SB_CTL,&lpsi, TRUE);
				
				init = true;

				SetScrollPos(GetDlgItem(hDlg,IDC_DEVIATIONINTERVALBAR), SB_CTL,st->interval, TRUE);
				SetDlgItemInt(hDlg, IDC_DEVIATIONINTERVAL, st->interval, FALSE);
                
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
				if (lParam == (long) GetDlgItem(hDlg,IDC_DEVIATIONINTERVALBAR))  
				{
					SetDlgItemInt(hDlg, IDC_DEVIATIONINTERVAL, nNewPos, TRUE);
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

DEVIATIONOBJ::DEVIATIONOBJ(int num) : BASE_CL()
{
	outports = 2;
	inports = 1;
	width=75;
    strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"dev");
	strcpy(out_ports[1].out_name,"mean");
    meanaccu = 0.0; devaccu=0.0;
    for (int i = 0; i < NUMSAMPLES; i++)
    {
    	samples[i] = 0.0;
    }
    interval = 256;
    writepos = 0;
    added = 0;
}
	
void DEVIATIONOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_DEVIATIONBOX, ghWndStatusbox, (DLGPROC)DeviationDlgHandler));
}

void DEVIATIONOBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("interval",P_INT,&interval);
}

void DEVIATIONOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"interval",P_INT,&interval);
}
	
void DEVIATIONOBJ::incoming_data(int port, float value)
{
	if (value!=INVALID_VALUE)
	{
		samples[writepos] = value;
   		meanaccu += value;
		added++;
		if (added > interval)
		{
		    int oldest = writepos - interval;
			if (oldest < 0)
	    		oldest += NUMSAMPLES;
		    meanaccu -= samples[oldest];
			devaccu -=  squares[oldest];
			added = interval;
		}
		mean=meanaccu / added;

		squares[writepos]=(value-mean)*(value-mean);
		devaccu += squares[writepos];

		deviation = (float) sqrt ((double) (devaccu / added));

		writepos++;
		if (writepos >= NUMSAMPLES)
    		writepos = 0;
	}
}
	
void DEVIATIONOBJ::work(void)
{	
	pass_values(0, deviation);
	pass_values(1, mean);
	
}

void DEVIATIONOBJ::change_interval(int newinterval)
{
	interval = newinterval;
	added = 0;
	meanaccu = 0;
	devaccu=0;
}

DEVIATIONOBJ::~DEVIATIONOBJ() {}
