/* -----------------------------------------------------------------------------

  BrainBay  -  Version 1.7, GPL 2003-2010

  MODULE:  OB_NOT.CPP
  Authors: Jeremy Wilkerson, Chris Veigl


  This Object performs the NOT-operation on it's two Input-Values and presents the
  result at the output-port. FALSE it represented by the constant INVALID_VALUE, TRUE
  is represented by the constand TRUE_VALUE (def: 512.0f )

 This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_not.h"

LRESULT CALLBACK NotDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	NOTOBJ * st;
	
	st = (NOTOBJ *) actobject;
    if ((st==NULL)||(st->type!=OB_NOT)) return(FALSE);	

	switch( message )
	{
		case WM_INITDIALOG:
				CheckDlgButton(hDlg, IDC_BINARY, st->binary);
				SetDlgItemInt(hDlg,IDC_BITS,st->bits,0);
				break;		
		case WM_CLOSE:
			    EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_BINARY:
					st->binary=IsDlgButtonChecked(hDlg,IDC_BINARY);
                    break;

				case IDC_BITS:
					st->bits=GetDlgItemInt(hDlg,IDC_BITS,NULL,0);
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


NOTOBJ::NOTOBJ(int num) : BASE_CL()
{
	outports = 1;
	inports = 1;
	strcpy(in_ports[0].in_name,"in");
	strcpy(out_ports[0].out_name,"out");
	input = INVALID_VALUE;
	binary=0;
	bits=127;
}
	
void NOTOBJ::make_dialog(void)
{
	display_toolbox(hDlg=CreateDialog(hInst, (LPCTSTR)IDD_NOTBOX, ghWndStatusbox, (DLGPROC)NotDlgHandler));
}


void NOTOBJ::load(HANDLE hFile) 
{
	load_object_basics(this);
	load_property("binary",P_INT,&binary);
	load_property("bits",P_INT,&bits);
}

void NOTOBJ::save(HANDLE hFile) 
{
	save_object_basics(hFile, this);
	save_property(hFile,"binary",P_INT,&binary);
	save_property(hFile,"bits",P_INT,&bits);
}
	
void NOTOBJ::incoming_data(int port, float value)
{
	if (port == 0)
		input = value;
}
	
void NOTOBJ::work(void)
{
	float value;

	if (!binary)
	{
		if (input == INVALID_VALUE)
    		value = TRUE_VALUE;
		else value = INVALID_VALUE;
	}
	else
	{
		value=(float) (  ((int)input) ^ bits );
	}

	pass_values(0, value);
}
	
NOTOBJ::~NOTOBJ() {}
