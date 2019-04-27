/* -----------------------------------------------------------------------------

   BrainBay  -  OpenSource Biofeedback Software

  MODULE:  OB_AVERAGE.CPP
  Author:  Chris Veigl


  This Object outputs a min/max - limiterd input signal

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_limiter.h"

using namespace std;

LIMITEROBJ::LIMITEROBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
    strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
    upper = 1000.0f;
    lower = -1000.0f;
}
	

void LIMITEROBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_LIMITERBOX, ghWndStatusbox, (DLGPROC)LimiterDlgHandler));
}

void LIMITEROBJ::load(HANDLE hFile) 
{
   load_object_basics(this);
   load_property("upper",P_FLOAT,&upper);
   load_property("lower",P_FLOAT,&lower);
}

void LIMITEROBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile,this);
    save_property(hFile,"upper",P_FLOAT,&upper);
    save_property(hFile,"lower",P_FLOAT,&lower);
}
	
void LIMITEROBJ::incoming_data(int port, float value)
{
	actval=value;
}
	
void LIMITEROBJ::work(void)
{
	if (actval!=INVALID_VALUE)
	{
		if (actval>upper) actval=upper;
		if (actval<lower) actval=lower;
	}
	pass_values(0, actval);
}


LIMITEROBJ::~LIMITEROBJ() {}

LRESULT CALLBACK LimiterDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char sztemp[50];
	LIMITEROBJ * st;
	
	st = (LIMITEROBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_LIMITER)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				sprintf(sztemp,"%.4f",st->upper);
				SetDlgItemText(hDlg, IDC_UPPERLIMIT, sztemp);
				sprintf(sztemp,"%.4f",st->lower);
				SetDlgItemText(hDlg, IDC_LOWERLIMIT, sztemp);
                break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
				switch (LOWORD(wParam)) {
			
				case IDC_UPPERLIMIT:
						GetDlgItemText(hDlg,IDC_UPPERLIMIT,sztemp,sizeof(sztemp)); 
						sscanf(sztemp,"%f",&st->upper);
						break;
				case IDC_LOWERLIMIT:
						GetDlgItemText(hDlg,IDC_LOWERLIMIT,sztemp,sizeof(sztemp)); 
						sscanf(sztemp,"%f",&st->lower);
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
