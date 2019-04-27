/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_INTEGRATE.CPP:  functions for the Intergrator-Object
  Author: Chris Veigl

  The Integrator - Object sums up the stream of input-values and outputs the results.
  It can be reset to a desired value

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_integrate.h"

INTEGRATEOBJ::INTEGRATEOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	width=75;
	strcpy(out_ports[0].out_name,"out");
	strcpy(in_ports[0].in_name,"in");

	i_value=0;max=0;min=0;
}
	
void INTEGRATEOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_INTEGRATEBOX, ghWndStatusbox, (DLGPROC)IntegrateDlgHandler));
}


void INTEGRATEOBJ::session_start(void)
{
	i_value=0;
}
void INTEGRATEOBJ::session_reset(void)
{
	i_value=0;
}
void INTEGRATEOBJ::session_pos(long pos)
{
	i_value=0;
}

void INTEGRATEOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("ivalue",P_INT,&i_value);
	load_property("imin",P_FLOAT,&min);
	load_property("imax",P_FLOAT,&max);
}

void INTEGRATEOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"ivalue",P_INT,&i_value);
	save_property(hFile,"imin",P_FLOAT,&min);
	save_property(hFile,"imax",P_FLOAT,&max);
}
 	
void INTEGRATEOBJ::incoming_data(int port, float value)
{
	i_value+=value;
}
	
void INTEGRATEOBJ::work(void)
{
	if (max!=min) { if (i_value>max) i_value=max; if (i_value<min) i_value=min; }
 	pass_values(0, i_value);
}


INTEGRATEOBJ::~INTEGRATEOBJ() {}

LRESULT CALLBACK IntegrateDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INTEGRATEOBJ * st;
	char tmp[30];
	
	st = (INTEGRATEOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_INTEGRATE)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:
				sprintf(tmp,"%.2f",st->min);
				SetDlgItemText(hDlg, IDC_MIN, tmp);
				sprintf(tmp,"%.2f",st->max);
				SetDlgItemText(hDlg, IDC_MAX, tmp);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
			case IDC_MIN:
					GetDlgItemText(hDlg,IDC_MIN,tmp,30);
					sscanf(tmp,"%f",&st->min);
					break;
			case IDC_MAX:
					GetDlgItemText(hDlg,IDC_MAX,tmp,30);
					sscanf(tmp,"%f",&st->max);
					break;
			case IDC_RESET:
					st->i_value=0;
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




