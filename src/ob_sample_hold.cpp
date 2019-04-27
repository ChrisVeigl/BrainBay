/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_SAMPLE_HOLD.CPP:  functions for the Sample-and-hold-Object


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_sample_hold.h"

SAMPLE_HOLDOBJ::SAMPLE_HOLDOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 2;
	width=75;
	strcpy(out_ports[0].out_name,"out");
	strcpy(in_ports[0].in_name,"in");
	strcpy(in_ports[1].in_name,"trigger");

    hold = 0;
	trigger=0;mode=0;
    resetvalue=0;

}
	
void SAMPLE_HOLDOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_SAMPLE_HOLDBOX, ghWndStatusbox, (DLGPROC)Sample_HoldDlgHandler));
}

void SAMPLE_HOLDOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("hold",P_FLOAT,&hold);
    load_property("trigmode",P_INT,&mode);
    load_property("resetvalue",P_FLOAT,&resetvalue);
}

void SAMPLE_HOLDOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"hold",P_FLOAT,&hold);
	save_property(hFile,"trigmode",P_INT,&mode);
	save_property(hFile,"resetvalue",P_FLOAT,&resetvalue);
}
	

void SAMPLE_HOLDOBJ::session_start(void)
{
	hold = resetvalue;
	trigger=0;

}

void SAMPLE_HOLDOBJ::session_reset(void)
{
	hold = resetvalue;
	trigger=0;

}

  	
void SAMPLE_HOLDOBJ::incoming_data(int port, float value)
{
	if (port==0) { act=value; }
	if (port==1)
	{
		old_value=trigger;
		trigger=value;
	}

}


void SAMPLE_HOLDOBJ::work(void)
{
	if ((mode==1) && (trigger!=INVALID_VALUE) && (old_value==INVALID_VALUE)) 
		hold=act;

	if ((mode==2) && (trigger==INVALID_VALUE)&& (old_value!=INVALID_VALUE)) 
		hold=act;

	pass_values(0, hold);

}


SAMPLE_HOLDOBJ::~SAMPLE_HOLDOBJ() {}

LRESULT CALLBACK Sample_HoldDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char tmp[50];
	SAMPLE_HOLDOBJ * st;
	
	st = (SAMPLE_HOLDOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_SAMPLE_HOLD)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:
				sprintf(tmp,"%.4f",st->hold);
				SetDlgItemText(hDlg, IDC_HOLD, tmp);
				sprintf(tmp,"%.4f",st->resetvalue);
				SetDlgItemText(hDlg, IDC_RESETVALUE, tmp);
				switch (st->mode) 
				{
					case 0: CheckDlgButton(hDlg, IDC_TRG_MANUAL,TRUE); break;
					case 1: CheckDlgButton(hDlg, IDC_TRG_RISING,TRUE); break;
					case 2: CheckDlgButton(hDlg, IDC_TRG_FALLING,TRUE); break;
				}
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_SAMPLE:
					st->hold=st->act;
					sprintf(tmp,"%.2f",st->hold);
					SetDlgItemText(hDlg,IDC_HOLD,tmp);
					break;

				case IDC_RESETVALUE:
				  if (HIWORD(wParam) == EN_KILLFOCUS)
				  {
					GetDlgItemText(hDlg,IDC_RESETVALUE,tmp,sizeof(tmp));
					st->resetvalue=(float)atof(tmp);
				  }
				  break;

				case IDC_TRG_MANUAL:
					st->mode=0;
					break;
				case IDC_TRG_RISING:
					st->mode=1;
					break;
				case IDC_TRG_FALLING:
					st->mode=2;
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




