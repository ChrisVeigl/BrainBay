/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org
  
  MODULE: OB_CONSTANT.CPP:  contains functions for the Constant-Source Object
  Author: Chris Veigl

  This Object outputs a constant value on it's port.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_constant.h"



CONSTANTOBJ::CONSTANTOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 0;
	width=75;
	strcpy(out_ports[0].out_name,"out");

	out_ports[0].get_range=-1;
	strcpy(out_ports[0].out_dim,"none");
	strcpy(out_ports[0].out_desc,"Constant");
	out_ports[0].out_max=1000.0f;
    out_ports[0].out_min=-1000.0f;

	value=0.0f;
}
	
void CONSTANTOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_CONSTANTBOX, ghWndStatusbox, (DLGPROC)ConstantDlgHandler));
}

void CONSTANTOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
    load_property("value",P_FLOAT,&value);
}

void CONSTANTOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
    save_property(hFile,"value",P_FLOAT,&value);
}
	

void CONSTANTOBJ::incoming_data(int port, float value)
{
}
  	
	
void CONSTANTOBJ::work(void)
{
 	pass_values(0, value);
}


CONSTANTOBJ::~CONSTANTOBJ() {}

LRESULT CALLBACK ConstantDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char tmp[50];
	CONSTANTOBJ * st;
	
	st = (CONSTANTOBJ *) actobject;
	if ((st==NULL)||(st->type!=OB_CONSTANT)) return(FALSE);
	
	switch( message )
	{
		case WM_INITDIALOG:

				sprintf(tmp,"%.2f",st->value);
				SetDlgItemText(hDlg, IDC_VALUE, tmp);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_APPLY:
					GetDlgItemText(hDlg,IDC_VALUE,tmp,30);
					sscanf(tmp,"%f",&st->value);
					break;
			}
			return TRUE;
		case WM_SIZE:
		case WM_MOVE:  update_toolbox_position(hDlg);
		break;
		return(TRUE);
	}
	return FALSE;
}

