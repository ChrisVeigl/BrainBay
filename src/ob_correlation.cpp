/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_CORRELATION.CPP:  functions for the Correlation-Object
  Author: Jeremy Wilkerson

  The Correlation between two input-streams is calculated and presented 
  at the output port.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/


#include "brainBay.h"
#include "ob_correlation.h"
#include <iostream>
#include <cmath>

using namespace std;

CORRELATIONOBJ::CORRELATIONOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
	width=75;
	strcpy(out_ports[0].out_name,"out");

	out_ports[0].get_range=-1;
	strcpy(out_ports[0].out_dim,"none");
	strcpy(out_ports[0].out_desc,"Correlation");
	out_ports[0].out_max=1.0f;
    out_ports[0].out_min=-1.0f;

    accum1 = 0.0;
    accum2 = 0.0;
    for (int i = 0; i < NUMSAMPLES; i++)
    {
    	samples1[i] = 0.0;
        samples2[i] = 0.0;
    }
    interval = 100;
    writepos1 = 0;
    writepos2 = 0;
    added1 = 0;
    added2 = 0;
}
	
void CORRELATIONOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_CORRBOX, ghWndStatusbox, (DLGPROC)CorrDlgHandler));
}

void CORRELATIONOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("interval",P_INT,&interval);
}

void CORRELATIONOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"interval",P_INT,&interval);
}
	


  	
void CORRELATIONOBJ::incoming_data(int port, float value)
{
	float *accum, *samples;
    int *writepos, *added;
	switch (port)
    {
    	case 0:
        	accum = &accum1;
            samples = samples1;
            writepos = &writepos1;
            added = &added1;
        	break;
        case 1:
        	accum = &accum2;
            samples = samples2;
            writepos = &writepos2;
            added = &added2;
        	break;
        default:
        	return;
     }
	samples[*writepos] = value;
   	(*accum) += value;
    (*added)++;
    if ((*added) > interval)
    {
	    int oldest = (*writepos) - interval;
	    if (oldest < 0)
	    	oldest += NUMSAMPLES;
	    (*accum) -= samples[oldest];
        (*added) = interval;
    }
	(*writepos)++;
    if ((*writepos) >= NUMSAMPLES)
    	(*writepos) = 0;
}
	
void CORRELATIONOBJ::work(void)
{
	float diff1,diff2;
	float mean1 = accum1 / added1;
    float mean2 = accum2 / added2;
    
    float C12 = 0, V1 = 0, V2 = 0;
    int pos1 = writepos1;
    int pos2 = writepos2;
    int added = (added1 < added2)?added1:added2;
    for (int i = 0; i < added; i++)
    {
    	diff1 = samples1[pos1] - mean1;
        diff2 = samples2[pos2] - mean2;
        C12 += diff1 * diff2;
        V1 += diff1 * diff1;
        V2 += diff2 * diff2;
        
        pos1--;
        pos2--;
        if (pos1 < 0)
        	pos1 = NUMSAMPLES - 1;
        if (pos2 < 0)
        	pos2 = NUMSAMPLES - 1;
    }
    
    C12 /= interval;
    V1 /= interval;
    V2 /= interval;
    float correlation = C12 / sqrt(V1 * V2);
 	pass_values(0, correlation);
}

void CORRELATIONOBJ::change_interval(int newinterval)
{
	interval = newinterval;
	added1 = 0;
    added2 = 0;
	accum1 = 0;
    accum2 = 0;
}

CORRELATIONOBJ::~CORRELATIONOBJ() {}

LRESULT CALLBACK CorrDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool init;
	CORRELATIONOBJ * st;
	
	st = (CORRELATIONOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_CORR)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:

				SCROLLINFO lpsi;
			    lpsi.cbSize=sizeof(SCROLLINFO);
				lpsi.fMask=SIF_RANGE|SIF_POS;
				lpsi.nMin=1; lpsi.nMax=NUMSAMPLES - 1;
				SetScrollInfo(GetDlgItem(hDlg,IDC_CORRINTERVALBAR),SB_CTL,&lpsi, TRUE);
				SetDlgItemText(hDlg, IDC_TAG, st->tag);
				
				init = true;

				SetScrollPos(GetDlgItem(hDlg,IDC_CORRINTERVALBAR), SB_CTL,st->interval, TRUE);
				SetDlgItemInt(hDlg, IDC_CORRINTERVAL, st->interval, FALSE);
                
				init = false;
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
//			switch (LOWORD(wParam))
//			{
//			}
//			return TRUE;
			break;
		case WM_HSCROLL:
		{
			int nNewPos; 
			if (!init && (nNewPos = get_scrollpos(wParam,lParam)) >= 0)
			{   
				if (lParam == (long) GetDlgItem(hDlg,IDC_CORRINTERVALBAR))  
				{
					SetDlgItemInt(hDlg, IDC_CORRINTERVAL, nNewPos, TRUE);
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




